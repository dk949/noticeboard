#ifndef NOTICEBOARD_HPP
#define NOTICEBOARD_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#ifndef __cplusplus
#    error "this is a C++ file, do not include in C"
#endif

#include "noticeboard.h"

namespace nb {
enum struct StandardCategory {
    None = NBStandardCategory::NB_C_NONE,
    Call = NBStandardCategory::NB_C_CALL,
    CallEnded = NBStandardCategory::NB_C_CALL_ENDED,
    CallIncoming = NBStandardCategory::NB_C_CALL_INCOMING,
    CallUnanswered = NBStandardCategory::NB_C_CALL_UNANSWERED,
    Device = NBStandardCategory::NB_C_DEVICE,
    DeviceAdded = NBStandardCategory::NB_C_DEVICE_ADDED,
    DeviceError = NBStandardCategory::NB_C_DEVICE_ERROR,
    DeviceRemoved = NBStandardCategory::NB_C_DEVICE_REMOVED,
    Email = NBStandardCategory::NB_C_EMAIL,
    EmailArrived = NBStandardCategory::NB_C_EMAIL_ARRIVED,
    EmailBounced = NBStandardCategory::NB_C_EMAIL_BOUNCED,
    Im = NBStandardCategory::NB_C_IM,
    ImError = NBStandardCategory::NB_C_IM_ERROR,
    ImReceived = NBStandardCategory::NB_C_IM_RECEIVED,
    Network = NBStandardCategory::NB_C_NETWORK,
    NetworkConnected = NBStandardCategory::NB_C_NETWORK_CONNECTED,
    NetworkDisconnected = NBStandardCategory::NB_C_NETWORK_DISCONNECTED,
    NetworkError = NBStandardCategory::NB_C_NETWORK_ERROR,
    Presence = NBStandardCategory::NB_C_PRESENCE,
    PresenceOffline = NBStandardCategory::NB_C_PRESENCE_OFFLINE,
    PresenceOnline = NBStandardCategory::NB_C_PRESENCE_ONLINE,
    Transfer = NBStandardCategory::NB_C_TRANSFER,
    TransferComplete = NBStandardCategory::NB_C_TRANSFER_COMPLETE,
    TransferError = NBStandardCategory::NB_C_TRANSFER_ERROR,
};

enum struct Urgency : std::int8_t {
    Low = NBUrgency::NB_U_LOW,
    Normal = NBUrgency::NB_U_NORMAL,
    Critical = NBUrgency::NB_U_CRITICAL,
};

enum struct HintType {
    Boolean = NBHintType::NB_HT_BOOLEAN,
    Int = NBHintType::NB_HT_INT,
    Double = NBHintType::NB_HT_DOUBLE,
    String = NBHintType::NB_HT_STRING,
    Byte = NBHintType::NB_HT_BYTE,
};

enum struct StandardHint {
    ActionIcons = NBStandardHint::NB_H_ACTION_ICONS,
    DesktopEntry = NBStandardHint::NB_H_DESKTOP_ENTRY,
    ImagePath = NBStandardHint::NB_H_IMAGE_PATH,
    Resident = NBStandardHint::NB_H_RESIDENT,
    SoundFile = NBStandardHint::NB_H_SOUND_FILE,
    SoundName = NBStandardHint::NB_H_SOUND_NAME,
    SuppressSound = NBStandardHint::NB_H_SUPPRESS_SOUND,
};
enum struct Backend {
    Default = NBBackend::NB_B_Default,
    NotifySend = NBBackend::NB_B_NotifySend,
    DBUS = NBBackend::NB_B_DBUS,
};

using Category = std::variant<StandardCategory, std::string>;

struct Action {
    std::string name;
    std::string text;
};

using HintValue = std::variant<bool, std::uint8_t, int, double, std::string>;
using HintName = std::variant<std::string, StandardHint>;

struct Hint {
    friend struct Notice;
private:
    HintName m_name;
    HintValue m_value;
    HintType m_type;
public:
    static Hint actionIcons(bool = true);
    static Hint desktopEntry(std::string);
    static Hint imagePath(std::string);
    static Hint resident(bool = true);
    static Hint soundFile(std::string);
    static Hint soundName(std::string);
    static Hint suppressSound(bool = true);
    static Hint custom(std::string name, HintValue value) noexcept;

    [[nodiscard]]
    HintType type() const noexcept;
    [[nodiscard]]
    std::string_view typeStr() const;
    [[nodiscard]]
    std::string_view name() const noexcept;
    [[nodiscard]]
    HintValue const &value() const noexcept;
    [[nodiscard]]
    std::string valueStr() const;
private:
    Hint(HintType, HintName, HintValue) noexcept;
};

struct Pos {
    int x;
    int y;
    bool operator==(Pos const &) const = default;
};

enum struct NoticeId { };

struct SendResponse {
    NoticeId id;
    std::optional<std::string_view> action_taken;
};

inline constexpr auto const NO_REPLACE = NoticeId(-1);
inline constexpr auto const DEFAULT_EXPIRE = 0;

class BackendBase;
}  // namespace nb

struct NBNotice {
    friend char const *NBerror(NBNotice const *);
    friend void internalSetError(NBNotice const *, std::string) noexcept;
    friend class nb::BackendBase;
protected:
    std::vector<nb::Action> m_actions;
    std::vector<nb::Hint> m_hints;
    nb::Category m_category;
    mutable std::string m_error;
    std::unique_ptr<nb::BackendBase> m_backend;
public:
    nb::Urgency urgency = nb::Urgency::Normal;
    bool transient = false;
    std::string app_name;
    std::string icon;
    int expire_time = nb::DEFAULT_EXPIRE;

    NBNotice();
    ~NBNotice();
    NBNotice(NBNotice &&) = default;
    NBNotice &operator=(NBNotice &&) = default;
    NBNotice(NBNotice const &);
    NBNotice &operator=(NBNotice const &);
};

namespace nb {

class NoticeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Notice : public NBNotice {

    explicit Notice(std::string app_name, Backend backend = Backend::Default);
    Notice(std::string app_name, std::unique_ptr<BackendBase> backend);


    NoticeId send(std::string_view header, std::string_view body = {}, NoticeId replace = NO_REPLACE) const;
    SendResponse sendSync(std::string_view header, std::string_view body = {}, NoticeId replace = NO_REPLACE) const;
    NoticeId sendPos(Pos pos, std::string_view header, std::string_view body = {}, NoticeId replace = NO_REPLACE) const;
    SendResponse sendPosSync(  //
        Pos pos,
        std::string_view header,
        std::string_view body = {},
        NoticeId replace = NO_REPLACE) const;

    void pushAction(Action);
    void clearActions() noexcept;
    Action popAction() noexcept;
    [[nodiscard]]
    Action const &actionAt(std::size_t idx) const;
    [[nodiscard]]
    std::size_t actionCount() const noexcept;

    void pushHint(Hint);
    Hint popHint() noexcept;
    void clearHints() noexcept;
    [[nodiscard]]
    Hint const &hintAt(std::size_t idx) const;
    [[nodiscard]]
    Hint const &hintAt(std::string_view name) const;
    [[nodiscard]]
    HintValue const &hintValueAt(std::string_view name) const;
    [[nodiscard]]
    std::size_t hintCount() const noexcept;
    [[nodiscard]]
    bool hasHint(std::string_view name) const noexcept;

    void setCategory(StandardCategory) noexcept;
    void setCategory(std::string) noexcept;
    [[nodiscard]]
    std::string_view getCategory() const;

    [[nodiscard]]
    std::string_view urgencyStr() const;
};
}  // namespace nb

#endif  // NOTICEBOARD_HPP
