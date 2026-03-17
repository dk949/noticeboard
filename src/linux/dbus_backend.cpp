#include "noticeboard/dbus_backend.hpp"

namespace nb {
class InternalDBusError : public BackendError {
    using BackendError::BackendError;
};

DBusBackend *DBusBackend::clone() const {
    throw InternalDBusError("dbus backend not yet implemented");
}

SendResponse DBusBackend::send(  //
    Notice const &notice,
    std::string_view header,
    std::string_view body,
    BackendOptions opts) const {
    (void)noticeActions(notice);
    (void)noticeHints(notice);
    (void)noticeCategory(notice);
    throw InternalDBusError("dbus backend not yet implemented");
}

DBusBackend::DBusBackend() {
    throw InternalDBusError("dbus backend not yet implemented");
}
}  // namespace nb
