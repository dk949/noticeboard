#include "null_backend.hpp"

namespace nb {
SendResponse NullBackend::send(Notice const &, std::string_view, std::string_view, BackendOptions) const {
    return SendResponse {.id = NoticeId(m_id++), .action_taken = {}};
}

BackendBase *NullBackend::clone() const {
    return new NullBackend(*this);
}
}
