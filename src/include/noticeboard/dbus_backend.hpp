#ifndef NOTICEBOARD_DBUS_BACKEND_HPP
#define NOTICEBOARD_DBUS_BACKEND_HPP

#include "noticeboard/backend.hpp"

namespace nb {
class DBusError : public BackendError {
    using BackendError::BackendError;
};

class DBusBackend : public BackendBase {
public:
    DBusBackend();

    SendResponse send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const override;

    DBusBackend *clone() const override;
};

}  // namespace nb

#endif  // NOTICEBOARD_DBUS_BACKEND_HPP
