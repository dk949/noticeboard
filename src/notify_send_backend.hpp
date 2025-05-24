#ifndef NOTIFY_SEND_BACKEND_HPP
#define NOTIFY_SEND_BACKEND_HPP
#include "backend.hpp"

namespace nb {

class NotifySendBackend : public BackendBase {
public:
    NotifySendBackend();
    int send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const override;
    NotifySendBackend *clone() const override;
private:
    std::vector<std::string> constructArgs(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const;
};

}  // namespace nb

#endif  // NOTIFY_SEND_BACKEND_HPP
