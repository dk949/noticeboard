#include "noticeboard/backend.hpp"

namespace nb {

std::vector<nb::Action> const &BackendBase::noticeActions(NBNotice const &notice) const {
    return notice.m_actions;
}

std::vector<nb::Hint> const &BackendBase::noticeHints(NBNotice const &notice) const {
    return notice.m_hints;
}

nb::Category const &BackendBase::noticeCategory(NBNotice const &notice) const {
    return notice.m_category;
}

std::string const &BackendBase::noticeError(NBNotice const &notice) const {
    return notice.m_error;
}

std::unique_ptr<nb::BackendBase> const &BackendBase::noticeBackend(NBNotice const &notice) const {
    return notice.m_backend;
}

}  // namespace nb
