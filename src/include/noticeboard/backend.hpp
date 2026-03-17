#ifndef NOTICEBOARD_BACKEND_HPP
#define NOTICEBOARD_BACKEND_HPP

#include "noticeboard/noticeboard.hpp"

namespace nb {
class BackendError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct BackendOptions {
    Pos pos;
    NoticeId replace;
    bool blocking;
};

class BackendBase {
public:
    BackendBase() = default;
    virtual ~BackendBase() = default;
    virtual SendResponse send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const = 0;
protected:

    [[nodiscard]]
    std::vector<nb::Action> const &noticeActions(NBNotice const &) const;
    [[nodiscard]]
    std::vector<nb::Hint> const &noticeHints(NBNotice const &) const;
    [[nodiscard]]
    nb::Category const &noticeCategory(NBNotice const &) const;
    [[nodiscard]]
    std::string const &noticeError(NBNotice const &) const;
    [[nodiscard]]
    std::unique_ptr<nb::BackendBase> const &noticeBackend(NBNotice const &) const;
};

}  // namespace nb

#endif  // NOTICEBOARD_BACKEND_HPP
