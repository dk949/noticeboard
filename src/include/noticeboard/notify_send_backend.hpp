#ifndef NOTICEBOARD_NOTIFY_SEND_BACKEND_HPP
#define NOTICEBOARD_NOTIFY_SEND_BACKEND_HPP
#include "noticeboard/backend.hpp"

namespace nb {
class NotifySendError : public BackendError {
    using BackendError::BackendError;
};

class NotifySendBackend : public BackendBase {
public:
    NotifySendBackend();
    SendResponse send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const override;
private:
    std::vector<std::string> constructArgs(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const;
};

}  // namespace nb

#endif  // NOTICEBOARD_NOTIFY_SEND_BACKEND_HPP
