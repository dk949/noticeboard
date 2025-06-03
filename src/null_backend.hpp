#ifndef NOTICEBOARD_NULL_BACKEND_HPP
#define NOTICEBOARD_NULL_BACKEND_HPP
#include "backend.hpp"

#include <sched.h>

namespace nb {
class NullBackendError : public BackendError {
    using BackendError::BackendError;
};

class NullBackend : public BackendBase {
public:
    NullBackend() = default;
    SendResponse send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const override;
    BackendBase *clone() const override;
private:
    mutable int m_id = 0;
};
}  // namespace nb



#endif  // NOTICEBOARD_NULL_BACKEND_HPP
