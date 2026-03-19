#include "noticeboard/dbus_backend.hpp"

namespace nb {
[[noreturn]]
static void error() {
    throw DbusError("libnoticeboard was not compiled with the dbus backend.");
}

struct DbusBackend::Impl { };

SendResponse DbusBackend::send(  //
    Notice const &,
    std::string_view,
    std::string_view,
    BackendOptions) const {
    error();
}

DbusBackend::DbusBackend()
        : m_impl(nullptr) {
    error();
}

DbusBackend::~DbusBackend() = default;

}  // namespace nb
