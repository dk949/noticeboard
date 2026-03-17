#include "noticeboard/dbus_backend.hpp"

#include "noticeboard/backend.hpp"

#include <dbus/dbus.h>

#include <concepts>
#include <cstdint>
#include <format>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
namespace rng = std::ranges;

using DbusErrorPtr = std::unique_ptr<DBusError, decltype([](DBusError *err) {
    dbus_error_free(err);
    delete err;
})>;
using DbusMessagePtr = std::unique_ptr<DBusMessage, decltype([](DBusMessage *msg) { dbus_message_unref(msg); })>;

struct StringToVariant { };

struct NotificationMessage {
    DbusMessagePtr msg;
    DBusMessageIter msg_iter;

    NotificationMessage(std::string const &app_name,
        std::string const &icon,
        nb::NoticeId replace,
        std::string_view header,
        std::string_view body,
        std::vector<nb::Action> const &actions,
        std::vector<nb::Hint> const &hints,
        std::string_view category,
        nb::Urgency urgency,
        bool transient,
        nb::Pos pos,
        nb::ExpireTime expiry)
            : msg(DbusMessagePtr {dbus_message_new_method_call(  //
                  "org.freedesktop.Notifications",               // destination
                  "/org/freedesktop/Notifications",              // object path
                  "org.freedesktop.Notifications",               // interface
                  "Notify"                                       // method
                  )}) {
        if (!msg) throw nb::DbusError("Failed to create a dbus message");
        dbus_message_iter_init_append(msg.get(), &msg_iter);
        pushStr(app_name);
        pushInt(replace == nb::NoticeId::NoReplace ? uint32_t {0} : static_cast<uint32_t>(replace));
        pushStr(icon);
        pushStr(std::string {header});
        pushStr(std::string {body});
        pushArr<std::string>(actions, [this](nb::Action const &action, DBusMessageIter *iter) {
            pushStr(action.name, iter);
            pushStr(action.text, iter);
        });
        pushArr<StringToVariant>(hints, [&, first = true](nb::Hint const &hint, DBusMessageIter *iter) mutable {
            if (first) {
                first = false;
                pushDictEntry([&](DBusMessageIter *dict_iter) {
                    pushStr("urgency", dict_iter);
                }, [&](DBusMessageIter *dict_iter) {
                    pushVariant([&] {
                        switch (urgency) {
                            case nb::Urgency::Low: return uint8_t {0};
                            default:
                            case nb::Urgency::Normal: return uint8_t {1};
                            case nb::Urgency::Critical: return uint8_t {2};
                        }
                    }(), dict_iter);
                }, iter);
                pushDictEntry([&](DBusMessageIter *dict_iter) {
                    pushStr("transient", dict_iter);
                }, [&](DBusMessageIter *dict_iter) { pushVariant(transient, dict_iter); }, iter);
                pushDictEntry([&](DBusMessageIter *dict_iter) {
                    pushStr("category", dict_iter);
                }, [&](DBusMessageIter *dict_iter) {
                    // NOTE: This is safe because all category classes are defined as char constants
                    pushVariant(category.data(), dict_iter);
                }, iter);
                if (pos != nb::Pos::NoPos) {
                    pushDictEntry([&](DBusMessageIter *dict_iter) {
                        pushStr("x", dict_iter);
                    }, [&](DBusMessageIter *dict_iter) { pushVariant(std::int32_t {pos.x}, dict_iter); }, iter);
                    pushDictEntry([&](DBusMessageIter *dict_iter) {
                        pushStr("y", dict_iter);
                    }, [&](DBusMessageIter *dict_iter) { pushVariant(std::int32_t {pos.y}, dict_iter); }, iter);
                }
            }
            pushDictEntry([&](DBusMessageIter *dict_iter) {
                pushStr(std::string {hint.name()}, dict_iter);
            }, [&](DBusMessageIter *dict_iter) {
                std::visit([&](auto const &v) { pushVariant(v, dict_iter); }, hint.value());
            }, iter);
        });
        pushInt([&] {
            switch (expiry) {
                case nb::ExpireTime::Default: return int32_t {-1};
                case nb::ExpireTime::None: return int32_t {0};
                default: return static_cast<int32_t>(expiry);
            }
        }());
    }

    template<std::integral Int>
    requires(!std::is_same_v<Int, bool>)
    void pushInt(Int i, DBusMessageIter *iter = nullptr) {
        auto real_iter = getIter(iter);
        checkMsg(dbus_message_iter_append_basic(real_iter, typeToDbusType<Int, false>(), &i));
    }

    void pushBool(bool b, DBusMessageIter *iter = nullptr) {
        dbus_bool_t as_bool_t {b};
        auto real_iter = getIter(iter);
        checkMsg(dbus_message_iter_append_basic(real_iter, DBUS_TYPE_BOOLEAN, &as_bool_t));
    }

    void pushStr(std::string const &str, DBusMessageIter *iter = nullptr) {
        return pushStr(str.c_str(), iter);
    }

    void pushStr(char const *str, DBusMessageIter *iter = nullptr) {
        auto real_iter = getIter(iter);
        checkMsg(dbus_message_iter_append_basic(real_iter, DBUS_TYPE_STRING, &str));
    }

    template<typename T, rng::range Rng, std::invocable<rng::range_value_t<Rng> const &, DBusMessageIter *> Fn>
    void pushArr(Rng const &r, Fn &&pushFn, DBusMessageIter *iter = nullptr) {
        auto real_iter = getIter(iter);
        DBusMessageIter array_iter;
        auto const type = typeToDbusType<T, true>();
        checkMsg(dbus_message_iter_open_container(real_iter, DBUS_TYPE_ARRAY, type, &array_iter));
        for (auto &&elem : r)
            pushFn(elem, &array_iter);
        checkMsg(dbus_message_iter_close_container(real_iter, &array_iter));
    }

    template<std::invocable<DBusMessageIter *> KeyFn, std::invocable<DBusMessageIter *> ValueFn>
    void pushDictEntry(KeyFn const &key_fn, ValueFn const &value_fn, DBusMessageIter *iter = nullptr) {
        DBusMessageIter dict_entry_iter;
        auto real_iter = getIter(iter);
        checkMsg(dbus_message_iter_open_container(real_iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry_iter));
        key_fn(&dict_entry_iter);
        value_fn(&dict_entry_iter);
        checkMsg(dbus_message_iter_close_container(real_iter, &dict_entry_iter));
    }

    template<typename T>
    void pushVariant(T const &val, DBusMessageIter *iter = nullptr) {
        auto real_iter = getIter(iter);
        DBusMessageIter variant_iter;
        checkMsg(
            dbus_message_iter_open_container(real_iter, DBUS_TYPE_VARIANT, typeToDbusType<T, true>(), &variant_iter));
        checkMsg(dbus_message_iter_append_basic(&variant_iter, typeToDbusType<T, false>(), valToPtr(val)));
        dbus_message_iter_close_container(real_iter, &variant_iter);
    }

    DbusMessagePtr sendSync(DBusConnection *conn, int timeout, DBusError *err) {
        return DbusMessagePtr {dbus_connection_send_with_reply_and_block(conn, msg.get(), timeout, err)};
    }
private:
    void checkMsg(dbus_bool_t res) {
        if (!res) throw nb::DbusError("Failed to append to dbug message: out of memory");
    }

    constexpr DBusMessageIter *getIter(DBusMessageIter *maybe_iter) {
        if (maybe_iter) return maybe_iter;
        return &msg_iter;
    }

#define NOTICEBOARD_RETURN(macro)     \
    do {                              \
        if constexpr (as_str)         \
            return macro##_AS_STRING; \
        else                          \
            return macro;             \
    } while (false)

    template<typename T, bool as_str>
    constexpr std::conditional_t<as_str, char const *, int> typeToDbusType() {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, char const *>)
            NOTICEBOARD_RETURN(DBUS_TYPE_STRING);
        else if constexpr (std::is_same_v<T, bool>)
            NOTICEBOARD_RETURN(DBUS_TYPE_BOOLEAN);
        else if constexpr (std::is_same_v<T, std::uint8_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_BYTE);
        else if constexpr (std::is_same_v<T, std::int16_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_INT16);
        else if constexpr (std::is_same_v<T, std::uint16_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_UINT16);
        else if constexpr (std::is_same_v<T, std::int32_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_INT32);
        else if constexpr (std::is_same_v<T, std::uint32_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_UINT32);
        else if constexpr (std::is_same_v<T, std::int64_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_INT64);
        else if constexpr (std::is_same_v<T, std::uint64_t>)
            NOTICEBOARD_RETURN(DBUS_TYPE_UINT64);
        else if constexpr (std::is_same_v<T, double>)
            NOTICEBOARD_RETURN(DBUS_TYPE_DOUBLE);
        else {
            if constexpr (std::is_same_v<T, StringToVariant>) {
                return DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING                 //
                    DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING  //
                        DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
            }
        }
    }

#undef NOTICEBOARD_RETURN

    template<typename T>
    constexpr auto valToPtr(T const &val) {
        if constexpr (std::is_same_v<T, std::string>) return valToPtr(val.c_str());
        if constexpr (std::is_same_v<T, char const *>      //
                      || std::is_same_v<T, bool>           //
                      || std::is_same_v<T, std::uint8_t>   //
                      || std::is_same_v<T, std::uint16_t>  //
                      || std::is_same_v<T, std::int16_t>   //
                      || std::is_same_v<T, std::uint32_t>  //
                      || std::is_same_v<T, std::int32_t>   //
                      || std::is_same_v<T, std::uint64_t>  //
                      || std::is_same_v<T, std::int64_t>   //
                      || std::is_same_v<T, double>         //
        )
            return &val;
    }
};

[[nodiscard]]
static DbusErrorPtr newDBusError() {
    auto *ptr = new DBusError {};
    dbus_error_init(ptr);
    return DbusErrorPtr {ptr};
}

[[nodiscard]]
static char const *errorMsg(DbusErrorPtr const &err) {
    return err->message ? err->message : "(Unknown error)";
}

namespace nb {
struct DbusBackend::Impl {
    DbusErrorPtr err;
    DBusConnection *conn;

    Impl()
            : err(newDBusError())
            , conn(dbus_bus_get(DBUS_BUS_SESSION, err.get())) {
        if (!conn) throw DbusError(std::format("Failed to connect to dbus session bus: {}", errorMsg(err)));
    }
};

class InternalDBusError : public BackendError {
    using BackendError::BackendError;
};

SendResponse DbusBackend::send(  //
    Notice const &notice,
    std::string_view header,
    std::string_view body,
    BackendOptions opts) const {

    auto msg = NotificationMessage(notice.app_name,
        notice.icon,
        opts.replace,
        header,
        body,
        noticeActions(notice),
        noticeHints(notice),
        notice.getCategory(),
        notice.urgency,
        notice.transient,
        opts.pos,
        notice.expire_time);
    if (opts.blocking) {
        auto reply = msg.sendSync(m_impl->conn, 1000, m_impl->err.get());
        if (!reply) throw DbusError(std::format("Failed to send dbus message: {}", errorMsg(m_impl->err)));

        std::uint32_t notice_id;
        if (!dbus_message_get_args(reply.get(), m_impl->err.get(), DBUS_TYPE_UINT32, &notice_id, DBUS_TYPE_INVALID))
            throw DbusError(std::format("Failed to retrieve dbus message contents: {}", errorMsg(m_impl->err)));
        return {static_cast<nb::NoticeId>(notice_id), {}};
    } else {
        throw InternalDBusError("Non blocking notifications not yet implemented in dbus backend");
    }
}

DbusBackend::DbusBackend()
        : m_impl(std::make_unique<Impl>()) { }

DbusBackend::~DbusBackend() = default;
}  // namespace nb

#if 0

static int send_notification(const char *summary, const char *body,
                             int timeout_ms) {
  DBusError err;
  DBusConnection *conn;
  DBusMessage *msg;
  DBusMessage *reply;
  DBusMessageIter args;
  dbus_uint32_t replaces_id = 0;
  dbus_uint32_t returned_id = 0;

  dbus_error_init(&err);

  /* Connect to the session bus (desktop notifications are session-scoped) */
  conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
  if (!conn) {
    fprintf(stderr, "Failed to connect to session bus: %s\n", err.message);
    dbus_error_free(&err);
    return -1;
  }

  /* Build method call:
   * destination: org.freedesktop.Notifications
   * object path: /org/freedesktop/Notifications
   * interface:  org.freedesktop.Notifications
   * method:     Notify
   */
  msg = dbus_message_new_method_call(
      "org.freedesktop.Notifications",  /* destination */
      "/org/freedesktop/Notifications", /* object path */
      "org.freedesktop.Notifications",  /* interface */
      "Notify"                          /* method */
  );
  if (!msg) {
    fprintf(stderr, "Failed to create message\n");
    return -1;
  }

  dbus_message_iter_init_append(msg, &args);

  /* 1) app_name (STRING) */
  const char *app_name = "notify_libdbus";
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_name)) {
    fprintf(stderr, "Out of memory (app_name)\n");
    dbus_message_unref(msg);
    return -1;
  }

  /* 2) replaces_id (UINT32) - 0 = new notification */
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &replaces_id)) {
    fprintf(stderr, "Out of memory (replaces_id)\n");
    dbus_message_unref(msg);
    return -1;
  }

  /* 3) app_icon (STRING) - empty for none */
  const char *app_icon = "";
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_icon)) {
    fprintf(stderr, "Out of memory (app_icon)\n");
    dbus_message_unref(msg);
    return -1;
  }

  /* 4) summary (STRING) */
  const char *summary_c = summary ? summary : "";
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &summary_c)) {
    fprintf(stderr, "Out of memory (summary)\n");
    dbus_message_unref(msg);
    return -1;
  }

  /* 5) body (STRING) */
  const char *body_c = body ? body : "";
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &body_c)) {
    fprintf(stderr, "Out of memory (body)\n");
    dbus_message_unref(msg);
    return -1;
  }

  /* 6) actions (as) - send empty array */
  {
    DBusMessageIter array_iter;
    if (!dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "s",
                                          &array_iter)) {
      fprintf(stderr, "Out of memory (actions array)\n");
      dbus_message_unref(msg);
      return -1;
    }
    /* no elements */
    dbus_message_iter_close_container(&args, &array_iter);
  }

  /* 7) hints (a{sv}) - send an 'urgency' hint (BYTE) */
  {
    DBusMessageIter hints_iter;
    DBusMessageIter dict_entry_iter;
    DBusMessageIter variant_iter;

    /* open array of dict entries, element signature is "{sv}" */
    if (!dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}",
                                          &hints_iter)) {
      fprintf(stderr, "Out of memory (hints array)\n");
      dbus_message_unref(msg);
      return -1;
    }

    /* Example hint: "urgency" -> byte 1 (normal)
     * To append a dict entry:
     *   - open DBUS_TYPE_DICT_ENTRY
     *   - append key (string)
     *   - open variant with the value's signature (e.g. "y" for byte)
     *   - append actual basic value inside variant
     *   - close variant and dict entry
     */

    if (!dbus_message_iter_open_container(&hints_iter, DBUS_TYPE_DICT_ENTRY,
                                          NULL, &dict_entry_iter)) {
      fprintf(stderr, "Out of memory (dict entry)\n");
      dbus_message_unref(msg);
      return -1;
    }

    /* key: "urgency" */
    const char *key = "urgency";
    if (!dbus_message_iter_append_basic(&dict_entry_iter, DBUS_TYPE_STRING,
                                        &key)) {
      fprintf(stderr, "Out of memory (hint key)\n");
      dbus_message_unref(msg);
      return -1;
    }

    /* value: variant containing a BYTE (DBUS signature "y") */
    if (!dbus_message_iter_open_container(&dict_entry_iter, DBUS_TYPE_VARIANT,
                                          "y", &variant_iter)) {
      fprintf(stderr, "Out of memory (variant)\n");
      dbus_message_unref(msg);
      return -1;
    }
    unsigned char urgency = 1; /* 0 = low, 1 = normal, 2 = critical */
    if (!dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BYTE,
                                        &urgency)) {
      fprintf(stderr, "Out of memory (urgency value)\n");
      dbus_message_unref(msg);
      return -1;
    }
    dbus_message_iter_close_container(&dict_entry_iter, &variant_iter);
    dbus_message_iter_close_container(&hints_iter, &dict_entry_iter);

    /* close hints array */
    dbus_message_iter_close_container(&args, &hints_iter);
  }

  /* 8) expire_timeout (INT32) in ms. -1 = server default, 0 = never */
  dbus_int32_t expire = (dbus_int32_t)timeout_ms;
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &expire)) {
    fprintf(stderr, "Out of memory (expire_timeout)\n");
    dbus_message_unref(msg);
    return -1;
  }

  /* Send the method call and block for reply */
  reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
  dbus_message_unref(msg);

  if (!reply) {
    fprintf(stderr, "Error sending message: %s\n",
            err.message ? err.message : "(no message)");
    dbus_error_free(&err);
    return -1;
  }

  /* The Notify method returns a UINT32 notification id */
  if (!dbus_message_get_args(reply, &err, DBUS_TYPE_UINT32, &returned_id,
                             DBUS_TYPE_INVALID)) {
    fprintf(stderr, "Failed to get reply args: %s\n",
            err.message ? err.message : "(no message)");
    dbus_message_unref(reply);
    dbus_error_free(&err);
    return -1;
  }

  dbus_message_unref(reply);
  return (int)returned_id;
}

int main(int argc, char **argv) {
  const char *summary = (argc > 1) ? argv[1] : "Hello from libdbus";
  const char *body =
      (argc > 2) ? argv[2] : "This notification was sent using libdbus.";
  int timeout_ms = -1;
  if (argc > 3)
    timeout_ms = atoi(argv[3]);

  int id = send_notification(summary, body, timeout_ms);
  if (id < 0) {
    fprintf(stderr, "Failed to send notification\n");
    return 1;
  }
  printf("Notification sent, id=%d\n", id);
  return 0;
}
#endif
