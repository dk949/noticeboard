#ifndef NOTICEBOARD_DBUS_BACKEND_HPP
#define NOTICEBOARD_DBUS_BACKEND_HPP

#include "noticeboard/backend.hpp"
#include <memory>

namespace nb {
class DbusError : public BackendError {
    using BackendError::BackendError;
};

class DbusBackend : public BackendBase {
    struct Impl;
    std::unique_ptr<Impl> m_impl;
public:
    DbusBackend();

    SendResponse send(  //
        Notice const &notice,
        std::string_view header,
        std::string_view body,
        BackendOptions opts) const override;

    ~DbusBackend() override;
};

}  // namespace nb

#endif  // NOTICEBOARD_DBUS_BACKEND_HPP
