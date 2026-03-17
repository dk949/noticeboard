#ifndef NOTICEBOARD_TESTS_TEST_BACKEND_HPP
#define NOTICEBOARD_TESTS_TEST_BACKEND_HPP

#include <noticeboard/backend.hpp>

#include <optional>
#include <string>
#include <utility>
#include <vector>

struct SentNotice {
    std::string header;
    std::string body;
    std::vector<nb::Action> actions {};
    std::vector<nb::Hint> hints {};
    nb::Category category {};
    std::string error {};
    nb::BackendOptions opts {};
};

class TestBackend : public nb::BackendBase {
    mutable std::vector<SentNotice> *m_notices;
    mutable nb::NoticeId m_id = nb::NoticeId {0};

public:
    TestBackend(std::vector<SentNotice> *notices)
            : m_notices(notices) { }

    nb::SendResponse send(nb::Notice const &notice,
        std::string_view header,
        std::string_view body,
        nb::BackendOptions opts) const override {

        m_notices->push_back({.header = std::string(header), .body = std::string(body)});
        m_notices->back().actions = noticeActions(notice);
        m_notices->back().hints = noticeHints(notice);
        m_notices->back().category = noticeCategory(notice);
        m_notices->back().error = noticeError(notice);
        m_notices->back().opts = opts;
        m_id = nb::NoticeId(std::to_underlying(m_id) + 1);

        return {.id = m_id, .action_taken = std::nullopt};
    }
};

#endif  // NOTICEBOARD_TESTS_TEST_BACKEND_HPP
