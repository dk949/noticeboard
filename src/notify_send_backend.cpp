#include "noticeboard/notify_send_backend.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;
namespace rng = std::ranges;
namespace vws = std::views;
static constexpr auto EXEC_ERROR = 76;

namespace nb {
class InternalNotifySendError : public BackendError {
    using BackendError::BackendError;
};
}

class FD {
    bool m_closed = false;
    int m_fd = -1;
public:
    explicit FD(int fd)
            : m_fd(fd) { }

    void close() {
        if (m_closed) return;
        m_closed = true;
        ::close(m_fd);
    }

    ~FD() {
        close();
    }

    FD(FD const &) = delete;
    FD &operator=(FD const &) = delete;

    FD(FD &&other) {
        *this = std::move(other);
    }

    FD &operator=(FD &&other) {
        if (&other == this) return *this;
        close();
        std::swap(m_fd, other.m_fd);
        std::swap(m_closed, other.m_closed);
        return *this;
    }

    int get() const {
        return m_fd;
    }

    int release() {
        m_closed = true;
        return m_fd;
    }
};

static std::string_view trimWS(std::string_view str) {
    return {
        std::find_if_not(  //
            str.begin(),
            str.end(),
            [](char ch) { return std::isspace(ch); }),
        std::find_if_not(  //
            str.rbegin(),
            str.rend(),
            [](char ch) {
        return std::isspace(ch);
    }).base(),
    };
}

static bool notifySendExists() {
    auto path = std::getenv("PATH");
    if (!path) return false;
    std::error_code ec;
    return rng::contains(  //
        rng::split_view(std::string_view {path}, ':'),
        true,
        [&](auto const &component) {
        return rng::contains(  //
            fs::directory_iterator(std::string_view {component}, ec),
            true,
            [](auto const &entry) { return entry.is_regular_file() && entry.path().filename() == "notify-send"; });
    });
}

[[noreturn]]
static void spawnNotifySend(std::vector<std::string> &args, FD fd) {
    dup2(fd.get(), STDERR_FILENO);
    dup2(fd.get(), STDOUT_FILENO);
    fd.close();
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    static char pname[] = "notify-send";
    argv.push_back(pname);
    for (auto &arg : args)
        argv.push_back(arg.data());
    argv.push_back(nullptr);
    execvp(argv[0], argv.data());
    std::printf("Failed to execvp(notify-send): %s\n{}", std::strerror(errno));
    std::exit(EXEC_ERROR);  // Exits the forked process, not the main process
}

static std::string readFDToString(FD fd) {
    std::string out;
    char ch;
    std::fflush(stdout);
    while (true) {
        switch (read(fd.get(), &ch, 1)) {
            case 1: out.push_back(ch); break;
            case 0: return out;
            default:
                throw nb::InternalNotifySendError(
                    std::format("Failed to read all bytes from notify-send: {}", std::strerror(errno)));
        }
    }
}

static void handleExitTsatus(int status, std::string const &text) {
    if (WIFEXITED(status)) {
        if (auto const real_status = WEXITSTATUS(status)) {
            if (real_status == EXEC_ERROR) throw nb::InternalNotifySendError(text);
            throw nb::NotifySendError(std::format("notify-send exited with non-zero status {}:\n{}", real_status, text));
        }
    } else if (WIFSIGNALED(status))
        throw nb::NotifySendError(std::format("notify-send was closed by signal {}:\n{}", WTERMSIG(status), text));
    else if (WIFSTOPPED(status))
        throw nb::NotifySendError(std::format("notify-send was stopped by signal {}:\n{}", WSTOPSIG(status), text));
}

static nb::SendResponse parseResponse(std::string_view sv, std::vector<nb::Action> const &actions) {
    int id;
    sv = trimWS(sv);
    auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), id);

    if (ec != std::errc {})
        throw nb::InternalNotifySendError(std::format("notify-send produced unexpected output: {}", sv));
    if (ptr == sv.end()) return {.id = nb::NoticeId(id), .action_taken = {}};
    if (actions.empty())
        throw nb::InternalNotifySendError(std::format("notify-send produced unexpected output: {}", sv));
    auto action = trimWS(ptr);
    auto taken = rng::find(actions, action, [](auto const &a) { return std::string_view {a.name}; });
    if (taken == actions.end())
        throw nb::InternalNotifySendError(std::format("Taken action '{}' is not one of the expected actions {}",
            action,
            actions | vws::transform([](nb::Action const &a) { return a.name + '=' + a.text; })));
    return {.id = nb::NoticeId(id), .action_taken = taken->name};
}

static std::string runNotifySend(std::vector<std::string> &args) {
    int fds[2];
    pipe(fds);
    FD read {fds[0]};
    FD write {fds[1]};
    std::string out;
    switch (auto const pid = fork()) {
        case -1: throw nb::NotifySendError(std::format("Failed to fork: {}", std::strerror(errno)));
        case 0: read.close(); spawnNotifySend(args, std::move(write));
        default:
            write.close();
            out = readFDToString(std::move(read));
            int status;
            waitpid(pid, &status, 0);
            handleExitTsatus(status, out);
            break;
    }
    return out;
}

namespace nb {

NotifySendBackend::NotifySendBackend() {
    if (!notifySendExists())
        throw NotifySendError("Cannot use the NotifySend Backend: notify-send executable not found");
}

std::vector<std::string> NotifySendBackend::constructArgs(  //
    Notice const &notice,
    std::string_view header,
    std::string_view body,
    BackendOptions opts) const {
    std::vector<std::string> out;

    auto const addHint = [&](Hint const &hint) {
        out.emplace_back("-h");
        out.push_back(std::format("{}:{}:{}", hint.typeStr(), hint.name(), hint.valueStr()));
    };

    out.emplace_back("-a");
    out.push_back(notice.app_name);

    out.emplace_back("-u");
    out.emplace_back(notice.urgencyStr());
    addHint(Hint::custom("urgency", std::uint8_t(std::to_underlying(notice.urgency) + 1)));

    out.emplace_back("-p");

    for (auto const &action : noticeActions(notice)) {
        out.emplace_back("-A");
        out.push_back(std::format("{}={}", action.name, action.text));
    }


    if (notice.expire_time != DEFAULT_EXPIRE) {
        out.emplace_back("-t");
        out.push_back(std::format("{}", notice.expire_time));
    }

    if (!notice.icon.empty()) {
        out.emplace_back("-i");
        out.push_back(notice.icon);
    }

    auto category = notice.getCategory();
    if (!category.empty()) {
        out.emplace_back("-c");
        out.emplace_back(category);
        addHint(Hint::custom("category", std::string {category}));
    }
    for (auto const &hint : noticeHints(notice))
        addHint(hint);

    if (notice.transient) {
        out.emplace_back("-e");
        addHint(Hint::custom("transient", true));
    }

    if (opts.replace != NoticeId::NoReplace) {
        out.emplace_back("-r");
        out.push_back(std::format("{}", std::to_underlying(opts.replace)));
    }
    if (opts.blocking) out.emplace_back("-w");
    if (opts.pos != Pos::NoPos) {
        addHint(Hint::custom("x", opts.pos.x));
        addHint(Hint::custom("y", opts.pos.y));
    }

    out.emplace_back(header);
    out.emplace_back(body);

    return out;
}

SendResponse NotifySendBackend::send(  //
    Notice const &notice,
    std::string_view header,
    std::string_view body,
    BackendOptions opts) const {
    if (!noticeActions(notice).empty() && !opts.blocking)
        throw NotifySendError("When used with the NotifySend Backend, supplying an Action implies synchronous mode");
    auto args = constructArgs(notice, header, body, opts);
    auto res = runNotifySend(args);
    return parseResponse(res, noticeActions(notice));
}

NotifySendBackend *NotifySendBackend::clone() const {
    return new NotifySendBackend(*this);
}
}  // namespace nb
