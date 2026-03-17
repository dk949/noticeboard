#include "noticeboard/dbus_backend.hpp"

namespace nb {
[[noreturn]]
static void error() {
    throw DbusError("libnoticeboard was not compiled with the dbus backend.");
}

struct DbusBackend::Impl { };

SendResponse DbusBackend::send(  //
    Notice const &notice,
    std::string_view header,
    std::string_view body,
    BackendOptions opts) const {
    error();
}

DbusBackend::DbusBackend()
        : m_impl(nullptr) {
    error();
}

DbusBackend::~DbusBackend() = default;

}  // namespace nb
