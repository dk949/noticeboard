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

#include "noticeboard/noticeboard.h"

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
    Null = NBBackend::NB_B_Null,
    DBUS = NBBackend::NB_B_DBUS,
    Win = NBBackend::NB_B_WIN,
    Darwin = NBBackend::NB_B_DARWIN,
};

using Category = std::variant<StandardCategory, std::string>;

struct Action {
    std::string name;
    std::string text;
    bool operator==(Action const &) const = default;
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

private:
    template<typename T>
    struct type_to_enum;


    template<HintType T>
    struct enum_to_type;

public:
    template<typename T>
    static inline constexpr auto type_to_enum_v = type_to_enum<T>::value;
    template<HintType T>
    using enum_to_type_t = enum_to_type<T>::type;
};

template<>
struct Hint::type_to_enum<bool> {
    static constexpr auto value = HintType::Boolean;
};

template<>
struct Hint::type_to_enum<int> {
    static constexpr auto value = HintType::Int;
};

template<>
struct Hint::type_to_enum<double> {
    static constexpr auto value = HintType::Double;
};

template<>
struct Hint::type_to_enum<std::string> {
    static constexpr auto value = HintType::String;
};

template<>
struct Hint::type_to_enum<std::uint8_t> {
    static constexpr auto value = HintType::Byte;
};

template<>
struct Hint::enum_to_type<HintType::Boolean> {
    using type = bool;
};

template<>
struct Hint::enum_to_type<HintType::Int> {
    using type = int;
};

template<>
struct Hint::enum_to_type<HintType::Double> {
    using type = double;
};

template<>
struct Hint::enum_to_type<HintType::String> {
    using type = std::string;
};

template<>
struct Hint::enum_to_type<HintType::Byte> {
    using type = std::uint8_t;
};

struct Pos {
    int x;
    int y;
    bool operator==(Pos const &) const = default;
    static Pos const NoPos;
};

inline constexpr Pos Pos::NoPos = {-1, -1};

enum struct NoticeId { NoReplace = -1 };
enum struct ExpireTime { Default = -1, None = 0 };

struct SendResponse {
    NoticeId id;
    std::optional<std::string_view> action_taken;
};


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
    nb::ExpireTime expire_time = nb::ExpireTime::Default;

    NBNotice();
    ~NBNotice();
    NBNotice(NBNotice &&) = default;
    NBNotice &operator=(NBNotice &&) = default;
    NBNotice(NBNotice const &) = delete;
    NBNotice &operator=(NBNotice const &) = delete;
};

namespace nb {

class NoticeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Notice : public NBNotice {

    explicit Notice(std::string app_name, Backend backend = Backend::Default);
    Notice(std::string app_name, std::unique_ptr<BackendBase> backend);

    NoticeId send(  //
        std::string_view header,
        std::string_view body = {},
        NoticeId replace = NoticeId::NoReplace) const;
    NoticeId send(  //
        Pos pos,
        std::string_view header,
        std::string_view body = {},
        NoticeId replace = NoticeId::NoReplace) const;
    SendResponse sendSync(  //
        std::string_view header,
        std::string_view body = {},
        NoticeId replace = NoticeId::NoReplace) const;
    SendResponse sendSync(  //
        Pos pos,
        std::string_view header,
        std::string_view body = {},
        NoticeId replace = NoticeId::NoReplace) const;

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

template<std::size_t N, typename Variant>
std::variant_alternative_t<N, Variant> const &get(Variant const &v) {
    return std::get<N>(v);
}

template<std::size_t N, typename Variant>
std::variant_alternative_t<N, Variant> &get(Variant &v) {
    return std::get<N>(v);
}

template<typename T, typename Variant>
T const &get(Variant const &v) {
    return std::get<T>(v);
}

template<typename T, typename Variant>
T &get(Variant &v) {
    return std::get<T>(v);
}

template<HintType T, typename Variant>
Hint::enum_to_type_t<T> const &get(Variant const &v) {
    return std::get<Hint::enum_to_type_t<T>>(v);
}

template<HintType T, typename Variant>
Hint::enum_to_type_t<T> &get(Variant &v) {
    return std::get<Hint::enum_to_type_t<T>>(v);
}

}  // namespace nb

#endif  // NOTICEBOARD_HPP
