#include "catch2/catch_test_macros.hpp"
#include "test_backend.hpp"

#include <catch2/catch_all.hpp>
#include <noticeboard/noticeboard.hpp>

#include <memory>
#include <utility>
#include <vector>

TEST_CASE("send") {
    std::vector<SentNotice> notices;
    nb::Notice n {"test-app", std::make_unique<TestBackend>(&notices)};
    n.sendSync("send", "Sync");
    REQUIRE(notices.size() == 1);
    REQUIRE(notices.back().header == "send");
    REQUIRE(notices.back().body == "Sync");
    REQUIRE(notices.back().actions.empty());
    REQUIRE(notices.back().hints.empty());
    REQUIRE(notices.back().error.empty());
    REQUIRE(nb::get<nb::StandardCategory>(notices.back().category) == nb::StandardCategory::None);
    REQUIRE(notices.back().opts.blocking);
    REQUIRE(notices.back().opts.pos == nb::Pos::NoPos);
    REQUIRE(notices.back().opts.replace == nb::NoticeId::NoReplace);
    n.sendSync(nb::Pos {30, 40}, "sendSync pos", {}, nb::NoticeId {1});
    REQUIRE(notices.size() == 2);
    REQUIRE(notices.back().header == "sendSync pos");
    REQUIRE(notices.back().body.empty());
    REQUIRE(notices.back().actions.empty());
    REQUIRE(notices.back().hints.empty());
    REQUIRE(notices.back().error.empty());
    REQUIRE(nb::get<nb::StandardCategory>(notices.back().category) == nb::StandardCategory::None);
    REQUIRE(notices.back().opts.blocking);
    REQUIRE(notices.back().opts.pos == nb::Pos {30, 40});
    REQUIRE(notices.back().opts.replace == nb::NoticeId {1});
    n.send("send");
    REQUIRE(notices.size() == 3);
    REQUIRE(notices.back().header == "send");
    REQUIRE(notices.back().body.empty());
    REQUIRE(notices.back().actions.empty());
    REQUIRE(notices.back().hints.empty());
    REQUIRE(notices.back().error.empty());
    REQUIRE(nb::get<nb::StandardCategory>(notices.back().category) == nb::StandardCategory::None);
    REQUIRE(!notices.back().opts.blocking);
    REQUIRE(notices.back().opts.pos == nb::Pos::NoPos);
    REQUIRE(notices.back().opts.replace == nb::NoticeId::NoReplace);
    n.send(nb::Pos {}, "send", "pos");
    REQUIRE(notices.size() == 4);
    REQUIRE(notices.back().header == "send");
    REQUIRE(notices.back().body == "pos");
    REQUIRE(notices.back().actions.empty());
    REQUIRE(notices.back().hints.empty());
    REQUIRE(notices.back().error.empty());
    REQUIRE(nb::get<nb::StandardCategory>(notices.back().category) == nb::StandardCategory::None);
    REQUIRE(!notices.back().opts.blocking);
    REQUIRE(notices.back().opts.pos == nb::Pos {0, 0});
    REQUIRE(notices.back().opts.replace == nb::NoticeId::NoReplace);
}

TEST_CASE("hints") {
    std::vector<SentNotice> notices;
    nb::Notice n {"test-app", std::make_unique<TestBackend>(&notices)};
    REQUIRE(n.hintCount() == 0);

    n.pushHint(nb::Hint::resident());
    REQUIRE(n.hintCount() == 1);

    auto const popped = n.popHint();
    REQUIRE(popped.name() == "resident");
    REQUIRE(popped.type() == nb::HintType::Boolean);
    REQUIRE(nb::get<bool>(popped.value()));
    REQUIRE(n.hintCount() == 0);

    n.pushHint(nb::Hint::resident());
    n.clearHints();
    REQUIRE(n.hintCount() == 0);

    n.pushHint(nb::Hint::actionIcons());
    n.pushHint(nb::Hint::desktopEntry("desktopEntry"));
    n.pushHint(nb::Hint::imagePath("imagePath"));
    n.pushHint(nb::Hint::resident());
    n.pushHint(nb::Hint::soundFile("soundFile"));
    n.pushHint(nb::Hint::soundName("soundName"));
    n.pushHint(nb::Hint::suppressSound());
    n.pushHint(nb::Hint::custom("bool", true));
    n.pushHint(nb::Hint::custom("u8", std::uint8_t {27}));
    n.pushHint(nb::Hint::custom("int", 42));
    n.pushHint(nb::Hint::custom("double", 12.34));
    n.pushHint(nb::Hint::custom("string", "hello"));

    REQUIRE(n.hintCount() == 12);
    REQUIRE(n.hintAt(0).name() == "action-icons");
    REQUIRE(nb::get<bool>(n.hintAt(0).value()));
    REQUIRE(n.hintAt("resident").name() == "resident");
    REQUIRE_THROWS_AS(n.hintAt("not-a-hint"), nb::NoticeError);
    REQUIRE(n.hasHint("desktop-entry"));
    REQUIRE(!n.hasHint("not-a-hint"));
    REQUIRE(nb::get<bool>(n.hintValueAt("bool")));
    REQUIRE(n.hintAt("bool").type() == nb::HintType::Boolean);
    REQUIRE(n.hintAt("u8").type() == nb::HintType::Byte);
    REQUIRE(n.hintAt("int").type() == nb::HintType::Int);
    REQUIRE(n.hintAt("double").type() == nb::HintType::Double);
    REQUIRE(n.hintAt("string").type() == nb::HintType::String);

    n.sendSync("test");
    REQUIRE(notices.size() == 1);

    REQUIRE(notices.back().hints[0].name() == "action-icons");
    REQUIRE(nb::get<bool>(notices.back().hints[0].value()));
    REQUIRE(notices.back().hints[1].name() == "desktop-entry");
    REQUIRE(nb::get<std::string>(notices.back().hints[1].value()) == "desktopEntry");
    REQUIRE(notices.back().hints[2].name() == "image-path");
    REQUIRE(nb::get<std::string>(notices.back().hints[2].value()) == "imagePath");
    REQUIRE(notices.back().hints[3].name() == "resident");
    REQUIRE(nb::get<bool>(notices.back().hints[3].value()));
    REQUIRE(notices.back().hints[4].name() == "sound_file");
    REQUIRE(nb::get<std::string>(notices.back().hints[4].value()) == "soundFile");
    REQUIRE(notices.back().hints[5].name() == "sound-name");
    REQUIRE(nb::get<std::string>(notices.back().hints[5].value()) == "soundName");
    REQUIRE(notices.back().hints[6].name() == "suppress-sound");
    REQUIRE(nb::get<bool>(notices.back().hints[6].value()));
    REQUIRE(notices.back().hints[7].name() == "bool");
    REQUIRE(nb::get<bool>(notices.back().hints[7].value()));
    REQUIRE(notices.back().hints[8].name() == "u8");
    REQUIRE(nb::get<uint8_t>(notices.back().hints[8].value()) == 27);
    REQUIRE(notices.back().hints[9].name() == "int");
    REQUIRE(nb::get<int>(notices.back().hints[9].value()) == 42);
    REQUIRE(notices.back().hints[10].name() == "double");
    REQUIRE(nb::get<double>(notices.back().hints[10].value()) == 12.34);
    REQUIRE(notices.back().hints[11].name() == "string");
    REQUIRE(nb::get<std::string>(notices.back().hints[11].value()) == "hello");
}

TEST_CASE("actions") {
    std::vector<SentNotice> notices;
    nb::Notice n {"test-app", std::make_unique<TestBackend>(&notices)};
    REQUIRE(n.actionCount() == 0);
    n.pushAction({.name = "deleted name 0", .text = "deleted text 0"});
    n.pushAction({.name = "deleted name 1", .text = "deleted text 1"});
    auto [deleted_name, deleted_text] = n.popAction();
    REQUIRE(deleted_name == "deleted name 1");
    REQUIRE(deleted_text == "deleted text 1");
    REQUIRE(n.actionCount() == 1);
    n.clearActions();
    REQUIRE(n.actionCount() == 0);


    n.pushAction({.name = "action name", .text = "action text"});
    REQUIRE(n.actionAt(0) == nb::Action {.name = "action name", .text = "action text"});
    n.sendSync("test");
    REQUIRE(notices.size() == 1);
    REQUIRE(notices.back().actions.size() == 1);
    REQUIRE(notices.back().actions.back() == nb::Action {.name = "action name", .text = "action text"});
}

TEST_CASE("category") {
    std::vector<SentNotice> notices;
    nb::Notice n {"test-app", std::make_unique<TestBackend>(&notices)};
    REQUIRE(n.getCategory() == "");
    n.setCategory(nb::StandardCategory::Email);
    REQUIRE(n.getCategory() == "email");
    n.setCategory("my-custom-category");
    REQUIRE(n.getCategory() == "my-custom-category");
    n.sendSync("test");
    REQUIRE(notices.size() == 1);
    REQUIRE(nb::get<std::string>(notices.back().category) == "my-custom-category");
}

TEST_CASE("urgency") {
    std::vector<SentNotice> notices;
    nb::Notice n {"test-app", std::make_unique<TestBackend>(&notices)};
    REQUIRE(n.urgencyStr() == "normal");
    n.urgency = nb::Urgency::Low;
    REQUIRE(n.urgencyStr() == "low");
    n.urgency = nb::Urgency::Normal;
    REQUIRE(n.urgencyStr() == "normal");
    n.urgency = nb::Urgency::Critical;
    REQUIRE(n.urgencyStr() == "critical");
}

TEST_CASE("get") {
    nb::HintValue hv = "hello";
    REQUIRE(hv.index() == 4);
    REQUIRE(nb::get<4>(std::as_const(hv)) == "hello");
    nb::get<4>(hv).push_back('!');
    REQUIRE(nb::get<4>(hv) == "hello!");

    REQUIRE(nb::get<std::string>(std::as_const(hv)) == "hello!");
    nb::get<std::string>(hv).pop_back();
    REQUIRE(nb::get<std::string>(hv) == "hello");

    REQUIRE(nb::get<nb::HintType::String>(std::as_const(hv)) == "hello");
    nb::get<nb::HintType::String>(hv).push_back('@');
    REQUIRE(nb::get<nb::HintType::String>(hv) == "hello@");
}
