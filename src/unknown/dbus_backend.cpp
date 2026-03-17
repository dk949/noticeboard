#include "noticeboard/dbus_backend.hpp"

namespace nb {
[[noreturn]]
static void error() {
    throw DBusError("libnoticeboard was not compiled with the dbus backend.");
}

DBusBackend *DBusBackend::clone() const {
    error();
}

SendResponse DBusBackend::send(  //
    Notice const &notice,
    std::string_view header,
    std::string_view body,
    BackendOptions opts) const {
    error();
}

DBusBackend::DBusBackend() {
    error();
}
}  // namespace nb
