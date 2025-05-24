#ifndef BACKEND_HPP
#define BACKEND_HPP

#include "noticeboard.hpp"

namespace nb {
struct BackendOptions {
    Pos pos;
    int replace;
    bool blocking;
};

class BackendBase {
public:
    BackendBase() = default;
    virtual ~BackendBase() = default;
    virtual int send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const = 0;
    virtual BackendBase *clone() const = 0;
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

#endif  // BACKEND_HPP
