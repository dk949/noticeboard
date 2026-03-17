#include "noticeboard/notify_send_backend.hpp"

namespace nb {

[[noreturn]]
static void error() {
    throw NotifySendError("libnoticeboard was not compiled with the notify-send backend.");
}

NotifySendBackend::NotifySendBackend() {
    error();
}

SendResponse NotifySendBackend::send(  //
    Notice const &,
    std::string_view,
    std::string_view,
    BackendOptions) const {
    error();
}

}  // namespace nb
