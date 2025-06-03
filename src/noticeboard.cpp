#include "noticeboard.hpp"

#include "backend.hpp"
#include "notify_send_backend.hpp"

#include <algorithm>
#include <bit>
#include <cstdarg>
#include <cstring>
#include <format>
#include <utility>
#include <variant>

namespace nb {
class InternalNoticeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
}

[[nodiscard]]
static nb::Notice *as(NBNotice *notice) {
    return static_cast<nb::Notice *>(notice);
}

[[nodiscard]]
static nb::Notice const *as(NBNotice const *notice) {
    return static_cast<nb::Notice const *>(notice);
}

[[nodiscard]]
static std::size_t as(unsigned index) {
    return static_cast<std::size_t>(index);
}

[[nodiscard]]
static unsigned as(std::size_t index) {
    return static_cast<unsigned>(index);
}

[[nodiscard]]
static NBHintType as(nb::HintType type) {
    return static_cast<NBHintType>(type);
}

[[nodiscard]]
static nb::StandardCategory as(NBStandardCategory type) {
    return static_cast<nb::StandardCategory>(type);
}

[[nodiscard]]
static nb::Urgency as(NBUrgency type) {
    return static_cast<nb::Urgency>(type);
}

[[nodiscard]]
static NBUrgency as(nb::Urgency type) {
    return static_cast<NBUrgency>(type);
}

[[nodiscard]]
static nb::Backend as(NBBackend type) {
    return static_cast<nb::Backend>(type);
}

[[nodiscard]]
static void const *hintValueToVoidP(nb::HintValue const &value) noexcept {
    return std::visit([]<typename T>(T const &v) {
        using D = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<bool, D> || std::is_same_v<std::uint8_t, D>)
            return std::bit_cast<void const *>(static_cast<std::uint64_t>(v));
        if constexpr (std::is_same_v<int, D>) return std::bit_cast<void const *>(static_cast<std::int64_t>(v));
        if constexpr (std::is_same_v<double, D>) return std::bit_cast<void const *>(v);
        if constexpr (std::is_same_v<std::string, D>) return std::bit_cast<void const *>(v.c_str());
    }, value);
}

static auto findHintByName(std::vector<nb::Hint> const &hints, std::string_view hint_name) noexcept {
    return std::find_if(hints.begin(), hints.end(), [&](auto const &hint) { return hint.name() == hint_name; });
}

static std::string_view catToString(nb::Category const &cat) {
    return std::visit([]<typename T>(T const &c) {
        using D = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<nb::StandardCategory, D>) {
            using namespace std::string_view_literals;
            using enum nb::StandardCategory;
            switch (c) {
                case None: return ""sv;
                case Call: return "call"sv;
                case CallEnded: return "call.ended"sv;
                case CallIncoming: return "call.incoming"sv;
                case CallUnanswered: return "call.unanswered"sv;
                case Device: return "device"sv;
                case DeviceAdded: return "device.added"sv;
                case DeviceError: return "device.error"sv;
                case DeviceRemoved: return "device.removed"sv;
                case Email: return "email"sv;
                case EmailArrived: return "email.arrived"sv;
                case EmailBounced: return "email.bounced"sv;
                case Im: return "im"sv;
                case ImError: return "im.error"sv;
                case ImReceived: return "im.received"sv;
                case Network: return "network"sv;
                case NetworkConnected: return "network.connected"sv;
                case NetworkDisconnected: return "network.disconnected"sv;
                case NetworkError: return "network.error"sv;
                case Presence: return "presence"sv;
                case PresenceOffline: return "presence.offline"sv;
                case PresenceOnline: return "presence.online"sv;
                case Transfer: return "transfer"sv;
                case TransferComplete: return "transfer.complete"sv;
                case TransferError: return "transfer.error"sv;
                default:
                    throw nb::InternalNoticeError(
                        std::format("Unknown standard categoty in variant: {}", std::to_underlying(c)));
            }
        }
        if constexpr (std::is_same_v<std::string, D>) {
            return std::string_view {c};
        }
    }, cat);
}

static std::unique_ptr<nb::BackendBase> backendFactory(nb::Backend backend) {
    using enum nb::Backend;
    switch (backend) {
        case Default:
        case NotifySend: return std::make_unique<nb::NotifySendBackend>();
        case DBUS: throw nb::NoticeError("DBUS backend not yet supported");
        default: throw nb::NoticeError(std::format("Unsupported backend type {}", std::to_underlying(backend)));
    }
}

static std::string_view hintNameToString(nb::HintName const &hint_name) {
    return std::visit([]<typename T>(T const &h) {
        using D = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<nb::StandardHint, D>) {
            using namespace std::string_view_literals;
            using enum nb::StandardHint;
            switch (h) {
                case ActionIcons: return "action-icons"sv;
                case DesktopEntry: return "desktop-entry"sv;
                case ImagePath: return "image-path"sv;
                case Resident: return "resident"sv;
                case SoundFile: return "sound_file"sv;
                case SoundName: return "sound-name"sv;
                case SuppressSound: return "suppress-sound"sv;
                default:
                    throw nb::InternalNoticeError(
                        std::format("Unknown standard hint in variant: {}", std::to_underlying(h)));
            }
        }
        if constexpr (std::is_same_v<std::string, D>) {
            return std::string_view {h};
        }
    }, hint_name);
}

void internalSetError(NBNotice const *notice, std::string error) noexcept {
    notice->m_error = std::move(error);
}

template<typename Fn, typename _ret = std::remove_cvref_t<decltype((std::declval<Fn>())())>>
auto tryCatch(NBNotice const *notice,
    std::string_view ctx,
    Fn &&fn,
    std::conditional_t<std::is_same_v<_ret, void>, std::monostate, _ret> err_out = {}) {
    try {
        return fn();
    } catch (nb::NoticeError const &e) {
        internalSetError(notice, std::format("Failed to {}: {}", ctx, e.what()));
    } catch (nb::BackendError const &e) {
        internalSetError(notice, std::format("Backend error: Failed to {}: {}", ctx, e.what()));
    } catch (nb::InternalNoticeError const &e) {
        internalSetError(notice, std::format("Unexpected internal error: Failed to {}: {}", ctx, e.what()));
    } catch (std::exception const &e) {
        internalSetError(notice, std::format("Unexpected error: Failed to {}: {}", ctx, e.what()));
    } catch (...) {
        internalSetError(notice, std::format("An unknown error occurred: Failed to {}", ctx));
    }
    if constexpr (!std::is_same_v<_ret, void>) return err_out;
}

extern "C" {

NBNotice *NB_NONNULL NBnewNotice(char const *NB_NONNULL app_name, NBBackend backend) {
    try {
        return new nb::Notice(app_name, as(backend));
    } catch (std::exception const &e) {
        auto notice = new nb::Notice(app_name, nullptr);
        internalSetError(notice, std::format("Failed to create NBNotice: {}", e.what()));
        return notice;
    }
}

NBNotice *NB_NONNULL NBcopyNotice(NBNotice const *NB_NONNULL notice) {
    return tryCatch(notice, "clone NBNotice", [&] {  //
        return new nb::Notice(*as(notice));
    });
}

void NBdeleteNotice(NBNotice *NB_NULLABLE notice) {
    delete as(notice);
}

char const *NB_NULLABLE NBerror(NBNotice const *NB_NONNULL notice) {
    if (notice->m_error.empty()) return nullptr;
    return notice->m_error.c_str();
}

void NBpushAction(NBNotice *NB_NONNULL notice, char const *NB_NONNULL name, char const *NB_NONNULL text) {
    if (!name[0] || !text[0]) {
        internalSetError(notice, "Both 'name' and 'text' have to not be empty");
        return;
    }
    return tryCatch(notice, "push action", [&] {
        as(notice)->pushAction(nb::Action {
            .name = name,
            .text = text,
        });
    });
}

void NBpopAction(NBNotice *NB_NONNULL notice) {
    as(notice)->popAction();
}

void NBclearActions(NBNotice *NB_NONNULL notice) {
    as(notice)->clearActions();
}

char const *NB_NULLABLE NBgetActionNameAt(NBNotice const *NB_NONNULL notice, unsigned index) {
    if (index >= as(notice)->actionCount()) return nullptr;
    auto const &name = as(notice)->actionAt(as(index)).name;
    return name.c_str();
}

char const *NB_NULLABLE NBgetActionTextAt(NBNotice const *NB_NONNULL notice, unsigned index) {
    if (index >= as(notice)->actionCount()) return nullptr;
    return as(notice)->actionAt(as(index)).text.c_str();
}

void NBpushHint(NBNotice *NB_NONNULL notice, int hint, ...) {
    va_list args;
    va_start(args, hint);
    return tryCatch(notice, "push hint", [&] {
        switch (hint) {
            case NB_H_ACTION_ICONS: as(notice)->pushHint(nb::Hint::actionIcons(va_arg(args, int))); break;
            case NB_H_DESKTOP_ENTRY: as(notice)->pushHint(nb::Hint::desktopEntry(va_arg(args, char const *))); break;
            case NB_H_IMAGE_PATH: as(notice)->pushHint(nb::Hint::imagePath(va_arg(args, char const *))); break;
            case NB_H_RESIDENT: as(notice)->pushHint(nb::Hint::resident(va_arg(args, int))); break;
            case NB_H_SOUND_FILE: as(notice)->pushHint(nb::Hint::soundFile(va_arg(args, char const *))); break;
            case NB_H_SOUND_NAME: as(notice)->pushHint(nb::Hint::soundName(va_arg(args, char const *))); break;
            case NB_H_SUPPRESS_SOUND: as(notice)->pushHint(nb::Hint::suppressSound(va_arg(args, int))); break;
            default: internalSetError(notice, std::format("Invalid standard hint: {}", hint));
        }
    });
    va_end(args);
}

void NBpushCustomHint(NBNotice *NB_NONNULL notice, NBHintType type, char const *NB_NONNULL name, ...) {
    va_list args;
    va_start(args, name);

    return tryCatch(notice, "push custom hint", [&] {
        switch (type) {
            case NB_HT_BOOLEAN: as(notice)->pushHint(nb::Hint::custom(name, va_arg(args, int) != 0)); break;
            case NB_HT_INT: as(notice)->pushHint(nb::Hint::custom(name, va_arg(args, int))); break;
            case NB_HT_DOUBLE: as(notice)->pushHint(nb::Hint::custom(name, va_arg(args, double))); break;
            case NB_HT_STRING:
                as(notice)->pushHint(nb::Hint::custom(name, std::string(va_arg(args, char const *))));
                break;
            case NB_HT_VOID: internalSetError(notice, "Invalid type for custom hint: NB_HT_VOID"); break;
            default:
                internalSetError(notice, std::format("Invalid type for custom hint: {}", std::to_underlying(type)));
        }
    });

    va_end(args);
}

void NBpopHint(NBNotice *NB_NONNULL notice) {
    as(notice)->popHint();
}

void NBclearHints(NBNotice *NB_NONNULL notice) {
    as(notice)->clearHints();
}

unsigned NBgetHintCount(NBNotice const *NB_NONNULL notice) {
    return as(as(notice)->hintCount());
}

char const *NB_NULLABLE NBgetHintNameAt(NBNotice const *NB_NONNULL notice, unsigned index) {
    if (as(notice)->hintCount() >= index) return nullptr;
    return as(notice)->hintAt(as(index)).name().data();
}

NBHintType NBgetHintTypeAt(NBNotice const *NB_NONNULL notice, unsigned index) {
    if (as(notice)->hintCount() >= index) return NB_HT_VOID;
    return as(as(notice)->hintAt(as(index)).type());
}

void const *NB_NULLABLE NBgetHintValueAt(NBNotice const *NB_NONNULL notice, unsigned index) {
    if (as(notice)->hintCount() >= index) return nullptr;
    return hintValueToVoidP(as(notice)->hintAt(as(index)).value());
}

NBHintType NBgetHintTypeByName(NBNotice const *NB_NONNULL notice, char const *NB_NONNULL name) {
    if (!as(notice)->hasHint(name)) return NB_HT_VOID;
    return as(as(notice)->hintAt(name).type());
}

void const *NB_NULLABLE NBgetHintValueByName(NBNotice const *NB_NONNULL notice, char const *NB_NONNULL name) {
    if (!as(notice)->hasHint(name)) return nullptr;
    return hintValueToVoidP(as(notice)->hintValueAt(name));
}

void NBsetCategory(NBNotice *NB_NONNULL notice, NBStandardCategory cat) {
    switch (cat) {
        case NB_C_NONE:
        case NB_C_CALL:
        case NB_C_CALL_ENDED:
        case NB_C_CALL_INCOMING:
        case NB_C_CALL_UNANSWERED:
        case NB_C_DEVICE:
        case NB_C_DEVICE_ADDED:
        case NB_C_DEVICE_ERROR:
        case NB_C_DEVICE_REMOVED:
        case NB_C_EMAIL:
        case NB_C_EMAIL_ARRIVED:
        case NB_C_EMAIL_BOUNCED:
        case NB_C_IM:
        case NB_C_IM_ERROR:
        case NB_C_IM_RECEIVED:
        case NB_C_NETWORK:
        case NB_C_NETWORK_CONNECTED:
        case NB_C_NETWORK_DISCONNECTED:
        case NB_C_NETWORK_ERROR:
        case NB_C_PRESENCE:
        case NB_C_PRESENCE_OFFLINE:
        case NB_C_PRESENCE_ONLINE:
        case NB_C_TRANSFER:
        case NB_C_TRANSFER_COMPLETE:
        case NB_C_TRANSFER_ERROR: as(notice)->setCategory(as(cat)); break;
        default: internalSetError(notice, std::format("Unknown standard category {}", std::to_underlying(cat)));
    }
}

void NBsetCustomCategory(NBNotice *NB_NONNULL notice, char const *NB_NONNULL name) {
    return tryCatch(notice, "set custom category", [&] {  //
        as(notice)->setCategory(name);
    });
}

char const *NB_NULLABLE NBgetCategory(NBNotice const *NB_NONNULL notice) {
    auto cat = as(notice)->getCategory();
    if (cat.empty()) return nullptr;
    return cat.data();
}

void NBsetUrgency(NBNotice *NB_NONNULL notice, NBUrgency urgency) {
    switch (urgency) {
        case NB_U_LOW:
        case NB_U_NORMAL:
        case NB_U_CRITICAL: as(notice)->urgency = as(urgency); break;
        default: internalSetError(notice, std::format("Unknwon urgency value {}", std::to_underlying(urgency)));
    }
}

NBUrgency NBgetUrgency(NBNotice *NB_NONNULL notice) {
    return as(notice->urgency);
}

void NBmakeTransient(NBNotice *NB_NONNULL notice) {
    notice->transient = true;
}

void NBmakeNotTransient(NBNotice *NB_NONNULL notice) {
    notice->transient = false;
}

int NBisTransient(NBNotice const *NB_NONNULL notice) {
    return notice->transient;
}

void NBsetAppName(NBNotice *NB_NONNULL notice, char const *NB_NONNULL name) {
    notice->app_name = name;
}

char const *NB_NONNULL NBgetAppName(NBNotice const *NB_NONNULL notice) {
    return notice->app_name.c_str();
}

void NBsetIcon(NBNotice *NB_NONNULL notice, char const *NB_NULLABLE icon) {
    notice->icon = icon;
}

char const *NB_NONNULL NBgetIcon(NBNotice const *NB_NONNULL notice) {
    return notice->icon.c_str();
}

void NBsetExpireTime(NBNotice *NB_NONNULL notice, int time) {
    notice->expire_time = time;
}

int NBgetExpireTime(NBNotice const *NB_NONNULL notice) {
    return notice->expire_time;
}

int NBSend(NBNotice *NB_NONNULL notice,  //
    char const *NB_NONNULL header,
    char const *NB_NULLABLE body) {
    return tryCatch(notice, "send", [&] {  //
        return std::to_underlying(as(notice)->send(header, body));
    });
}

int NBSendPos(NBNotice *NB_NONNULL notice,  //
    int x,
    int y,
    char const *NB_NONNULL header,
    char const *NB_NULLABLE body) {

    return tryCatch(notice, "send with position", [&] {  //
        return std::to_underlying(as(notice)->sendPos({x, y}, header, body));
    });
}

int NBSendSync(NBNotice *NB_NONNULL notice,  //
    char const *NB_NONNULL header,
    char const *NB_NULLABLE body,
    char const *NB_NULLABLE *NB_NULLABLE action_result) {
    return tryCatch(notice, "send synchrously", [&] {
        auto res = as(notice)->sendSync(header, body);
        if (res.action_taken)
            *action_result = res.action_taken->data();
        else
            *action_result = nullptr;
        return std::to_underlying(res.id);
    });
}

int NBSendPosSync(NBNotice *NB_NONNULL notice,
    int x,
    int y,
    char const *NB_NONNULL header,
    char const *NB_NULLABLE body,
    char const *NB_NULLABLE *NB_NULLABLE action_result) {
    return tryCatch(notice, "send synchrously with position", [&] {
        auto res = as(notice)->sendPosSync({x, y}, header, body);
        if (res.action_taken)
            *action_result = res.action_taken->data();
        else
            *action_result = nullptr;
        return std::to_underlying(res.id);
    });
}
}

NBNotice::NBNotice()
        : m_backend(nullptr) { }

NBNotice ::NBNotice(NBNotice const &other) {
    *this = other;
}

NBNotice::~NBNotice() = default;

NBNotice &NBNotice ::operator=(NBNotice const &other) {
    m_actions = other.m_actions;
    m_hints = other.m_hints;
    m_category = other.m_category;
    m_error = other.m_error;
    urgency = other.urgency;
    transient = other.transient;
    app_name = other.app_name;
    icon = other.icon;
    expire_time = other.expire_time;
    m_backend.reset(other.m_backend->clone());
    return *this;
}

namespace nb {
Notice::Notice(std::string name, Backend backend)
        : Notice(std::move(name), backendFactory(backend)) { }

Notice::Notice(std::string name, std::unique_ptr<BackendBase> backend) {
    NBNotice::app_name = std::move(name);
    NBNotice::m_backend = std::move(backend);
}

NoticeId Notice::send(std::string_view header, std::string_view body, NoticeId replace) const {
    return m_backend
        ->send(*this,
            header,
            body,
            {
                .pos = {-1, -1},
                .replace = replace,
                .blocking = false,
    })
        .id;
}

SendResponse Notice::sendSync(std::string_view header, std::string_view body, NoticeId replace) const {
    return m_backend->send(*this,
        header,
        body,
        {
            .pos = {-1, -1},
            .replace = replace,
            .blocking = true,
    });
}

NoticeId Notice::sendPos(Pos pos, std::string_view header, std::string_view body, NoticeId replace) const {
    return m_backend
        ->send(*this,
            header,
            body,
            {
                .pos = pos,
                .replace = replace,
                .blocking = false,
            })
        .id;
}

SendResponse Notice::sendPosSync(Pos pos, std::string_view header, std::string_view body, NoticeId replace) const {
    return m_backend->send(*this,
        header,
        body,
        {
            .pos = pos,
            .replace = replace,
            .blocking = true,
        });
}

void Notice::pushAction(Action action) {
    if (action.name.empty() || action.text.empty())
        throw nb::NoticeError("Both 'name' and 'text' in Action have to not be empty");

    m_actions.push_back(std::move(action));
}

void Notice::clearActions() noexcept {
    m_actions.clear();
}

Action Notice::popAction() noexcept {
    auto action = std::move(m_actions.back());
    m_actions.pop_back();
    return action;
}

[[nodiscard]]
Action const &Notice::actionAt(std::size_t idx) const {
    return m_actions.at(idx);
}

[[nodiscard]]
std::size_t Notice::actionCount() const noexcept {
    return m_actions.size();
}

void Notice::pushHint(Hint hint) {
    m_hints.push_back(std::move(hint));
}

Hint Notice::popHint() noexcept {
    auto hint = std::move(m_hints.back());
    m_hints.pop_back();
    return hint;
}

void Notice::clearHints() noexcept {
    m_hints.clear();
}

[[nodiscard]]
Hint const &Notice::hintAt(std::size_t idx) const {
    return m_hints.at(idx);
}

[[nodiscard]]
Hint const &Notice::hintAt(std::string_view name) const {
    if (auto found = findHintByName(m_hints, name); found != m_hints.end()) return *found;
    throw NoticeError(std::format("No such hint '{}'", name));
}

[[nodiscard]]
HintValue const &Notice::hintValueAt(std::string_view name) const {
    return hintAt(name).value();
}

[[nodiscard]]
std::size_t Notice::hintCount() const noexcept {
    return m_hints.size();
}

[[nodiscard]]
bool Notice::hasHint(std::string_view name) const noexcept {
    if (auto found = findHintByName(m_hints, name); found != m_hints.end())
        return true;
    else
        return false;
}

void Notice::setCategory(StandardCategory cat) noexcept {
    m_category = cat;
}

void Notice::setCategory(std::string name) noexcept {
    m_category = std::move(name);
}

[[nodiscard]]
std::string_view Notice::getCategory() const {
    return catToString(m_category);
}

std::string_view Notice::urgencyStr() const {
    switch (urgency) {
        case Urgency::Low: return "low";
        case Urgency::Normal: return "normal";
        case Urgency::Critical: return "critical";
        default: throw nb::InternalNoticeError(std::format("Unknwon urgency valud {}", std::to_underlying(urgency)));
    }
}

Hint::Hint(HintType type, HintName name, HintValue value) noexcept
        : m_name(std::move(name))
        , m_value(std::move(value))
        , m_type(type) { }

Hint Hint::actionIcons(bool value) {
    return {nb::HintType::Boolean, "action-icons", value};
}

Hint Hint::desktopEntry(std::string value) {
    return {nb::HintType::String, "desktop-entry", std::move(value)};
}

Hint Hint::imagePath(std::string value) {
    return {nb::HintType::String, "image-path", std::move(value)};
}

Hint Hint::resident(bool value) {
    return {nb::HintType::Boolean, "resident", value};
}

Hint Hint::soundFile(std::string value) {
    return {nb::HintType::String, "sound_file", std::move(value)};
}

Hint Hint::soundName(std::string value) {
    return {nb::HintType::String, "sound-name", std::move(value)};
}

Hint Hint::suppressSound(bool value) {
    return {nb::HintType::Boolean, "suppress-sound", value};
}

Hint Hint::custom(std::string name, HintValue value) noexcept {
    auto type = std::visit([]<typename T>(T const &) {
        using D = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<bool, D>) return nb::HintType::Boolean;
        if constexpr (std::is_same_v<std::uint8_t, D>) return nb::HintType::Byte;
        if constexpr (std::is_same_v<int, D>) return nb::HintType::Int;
        if constexpr (std::is_same_v<double, D>) return nb::HintType::Double;
        if constexpr (std::is_same_v<std::string, D>) return nb::HintType::String;
    }, value);
    return {type, std::move(name), std::move(value)};
}

[[nodiscard]]
HintType Hint::type() const noexcept {
    return m_type;
}

[[nodiscard]]
std::string_view Hint::typeStr() const {
    switch (auto const ty = m_type) {
        using enum HintType;
        case Boolean: return "BOOLEAN";
        case Byte: return "BYTE";
        case Int: return "INT";
        case Double: return "DOUBLE";
        case String: return "STRING";
        default: throw InternalNoticeError(std::format("Invaid hint type {}", std::to_underlying(ty)));
    };
}

[[nodiscard]]
std::string_view Hint::name() const noexcept {
    return hintNameToString(m_name);
}

[[nodiscard]]
HintValue const &Hint::value() const noexcept {
    return m_value;
}

[[nodiscard]]
std::string Hint::valueStr() const {
    return std::visit([](auto const &v) { return std::format("{}", v); }, m_value);
}


}  // namespace nb
