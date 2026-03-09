#ifndef NOTICEBOARD_H
#define NOTICEBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NOTICEBOARD_HAVE_NULLABLE
#    define NOTICEBOARD_NULLABLE _Nullable
#else
#    define NOTICEBOARD_NULLABLE
#endif

#ifdef NOTICEBOARD_HAVE_NONNULL
#    define NOTICEBOARD_NONNULL _Nonnull
#else
#    define NOTICEBOARD_NONNULL
#endif

typedef struct NBNotice NBNotice;

typedef enum NBStandardCategory {
    // No category
    NB_C_NONE,
    // "call" - A generic audio or video call notification that doesn't fit into any other category.
    NB_C_CALL,
    // "call.ended" - An audio or video call was ended.
    NB_C_CALL_ENDED,
    // "call.incoming" - A audio or video call is incoming.
    NB_C_CALL_INCOMING,
    // "call.unanswered" - An incoming audio or video call was not answered.
    NB_C_CALL_UNANSWERED,
    // "device" - A generic device-related notification that doesn't fit into any other category.
    NB_C_DEVICE,
    // "device.added" - A device, such as a USB device, was added to the system.
    NB_C_DEVICE_ADDED,
    // "device.error" - A device had some kind of error.
    NB_C_DEVICE_ERROR,
    // "device.removed" - A device, such as a USB device, was removed from the system.
    NB_C_DEVICE_REMOVED,
    // "email" - A generic e-mail-related notification that doesn't fit into any other category.
    NB_C_EMAIL,
    // "email.arrived" - A new e-mail notification.
    NB_C_EMAIL_ARRIVED,
    // "email.bounced" - A notification stating that an e-mail has bounced.
    NB_C_EMAIL_BOUNCED,
    // "im" - A generic instant message-related notification that doesn't fit into any other category.
    NB_C_IM,
    // "im.error" - An instant message error notification.
    NB_C_IM_ERROR,
    // "im.received" - A received instant message notification.
    NB_C_IM_RECEIVED,
    // "network" - A generic network notification that doesn't fit into any other category.
    NB_C_NETWORK,
    // "network.connected" - A network connection notification, such as successful sign-on to a network service. This
    // should not be confused with device.added for new network devices.
    NB_C_NETWORK_CONNECTED,
    // "network.disconnected" - A network disconnected notification. This should not be confused with device.removed for
    // disconnected network devices.
    NB_C_NETWORK_DISCONNECTED,
    // "network.error" - A network-related or connection-related error.
    NB_C_NETWORK_ERROR,
    // "presence" - A generic presence change notification that doesn't fit into any other category, such as going away or idle.
    NB_C_PRESENCE,
    // "presence.offline" - An offline presence change notification.
    NB_C_PRESENCE_OFFLINE,
    // "presence.online" - An online presence change notification.
    NB_C_PRESENCE_ONLINE,
    // "transfer" - A generic file transfer or download notification that doesn't fit into any other category.
    NB_C_TRANSFER,
    // "transfer.complete" - A file transfer or download complete notification.
    NB_C_TRANSFER_COMPLETE,
    // "transfer.error" - A file transfer or download error.
    NB_C_TRANSFER_ERROR,
} NBStandardCategory;

typedef enum NBUrgency {
    NB_U_LOW = -1,
    NB_U_NORMAL = 0,
    NB_U_CRITICAL = 1,
} NBUrgency;

typedef enum NBHintType {
    NB_HT_VOID,
    NB_HT_BOOLEAN,
    NB_HT_INT,
    NB_HT_DOUBLE,
    NB_HT_STRING,
    NB_HT_BYTE,
} NBHintType;

typedef enum NBStandardHint {
    // "action-icons" - When set, a server that has the "action-icons" capability will attempt to interpret any action
    // identifier as a named icon. The localized display name will be used to annotate the icon for accessibility
    // purposes. The icon name should be compliant with the Freedesktop.org Icon Naming Specification. >= 1.2
    NB_H_ACTION_ICONS,  // BOOLEAN
    // "desktop-entry" - This specifies the name of the desktop filename representing the calling program. This should
    // be the same as the prefix used for the application's .desktop file.
    NB_H_DESKTOP_ENTRY,  // STRING
    // "image-path" - Alternative way to define the notification image. See Icons and Images. >= 1.2
    NB_H_IMAGE_PATH,  // STRING
    // "resident" - When set the server will not automatically remove the notification when an action has been invoked.
    // The notification will remain resident in the server until it is explicitly removed by the user or by the sender.
    // This hint is likely only useful when the server has the "persistence" capability. >= 1.2
    NB_H_RESIDENT,  // BOOLEAN
    // "sound_file" - The path to a sound file to play when the notification pops up.
    NB_H_SOUND_FILE,  // STRING
    // "sound-name" - A themeable named sound from the freedesktop.org sound naming specification to play when the
    // notification pops up. Similar to icon-name, only for sounds. An example would be "message-new-instant".
    NB_H_SOUND_NAME,  // STRING
    // "suppress-sound" - Causes the server to suppress playing any sounds, if it has that ability. This is usually set
    // when the client itself is going to play its own sound.
    NB_H_SUPPRESS_SOUND,  // BOOLEAN
} NBStandardHint;

typedef enum NBBackend {
    NB_B_Default,
    NB_B_NotifySend,
    NB_B_Null,
    NB_B_DBUS,
} NBBackend;

NBNotice *NOTICEBOARD_NONNULL NBnewNotice(char const *NOTICEBOARD_NONNULL app_name, NBBackend backend);
NBNotice *NOTICEBOARD_NONNULL NBcopyNotice(NBNotice const *NOTICEBOARD_NONNULL notice);
void NBdeleteNotice(NBNotice *NOTICEBOARD_NULLABLE);

char const *NOTICEBOARD_NULLABLE NBerror(NBNotice const *NOTICEBOARD_NONNULL notice);

void NBpushAction(NBNotice *NOTICEBOARD_NONNULL,
    char const *NOTICEBOARD_NONNULL name,
    char const *NOTICEBOARD_NONNULL text);
void NBpopAction(NBNotice *NOTICEBOARD_NONNULL);
void NBclearActions(NBNotice *NOTICEBOARD_NONNULL);
char const *NOTICEBOARD_NULLABLE NBgetActionNameAt(NBNotice const *NOTICEBOARD_NONNULL notice, unsigned index);
char const *NOTICEBOARD_NULLABLE NBgetActionTextAt(NBNotice const *NOTICEBOARD_NONNULL notice, unsigned index);

void NBpushHint(NBNotice *NOTICEBOARD_NONNULL, int /*NBStandardHint*/ hint, ...);
void NBpushCustomHint(NBNotice *NOTICEBOARD_NONNULL, NBHintType type, char const *NOTICEBOARD_NONNULL name, ...);
void NBpopHint(NBNotice *NOTICEBOARD_NONNULL);
void NBclearHints(NBNotice *NOTICEBOARD_NONNULL);
unsigned NBgetHintCount(NBNotice const *NOTICEBOARD_NONNULL notice);
char const *NOTICEBOARD_NULLABLE NBgetHintNameAt(NBNotice const *NOTICEBOARD_NONNULL notice, unsigned index);
NBHintType NBgetHintTypeAt(NBNotice const *NOTICEBOARD_NONNULL notice, unsigned index);
void const *NOTICEBOARD_NULLABLE NBgetHintValueAt(NBNotice const *NOTICEBOARD_NONNULL notice, unsigned index);
NBHintType NBgetHintTypeByName(NBNotice const *NOTICEBOARD_NONNULL notice, char const *NOTICEBOARD_NONNULL name);
void const *NOTICEBOARD_NULLABLE NBgetHintValueByName(  //
    NBNotice const *NOTICEBOARD_NONNULL notice,
    char const *NOTICEBOARD_NONNULL name);

void NBsetCategory(NBNotice *NOTICEBOARD_NONNULL, NBStandardCategory);
void NBsetCustomCategory(NBNotice *NOTICEBOARD_NONNULL, char const *NOTICEBOARD_NONNULL);
char const *NOTICEBOARD_NULLABLE NBgetCategory(NBNotice const *NOTICEBOARD_NONNULL notice);

void NBsetUrgency(NBNotice *NOTICEBOARD_NONNULL, NBUrgency);
NBUrgency NBgetUrgency(NBNotice *NOTICEBOARD_NONNULL);

void NBmakeTransient(NBNotice *NOTICEBOARD_NONNULL);
void NBmakeNotTransient(NBNotice *NOTICEBOARD_NONNULL);
int NBisTransient(NBNotice const *NOTICEBOARD_NONNULL notice);

void NBsetAppName(NBNotice *NOTICEBOARD_NONNULL, char const *NOTICEBOARD_NONNULL);
char const *NOTICEBOARD_NONNULL NBgetAppName(NBNotice const *NOTICEBOARD_NONNULL notice);

void NBsetIcon(NBNotice *NOTICEBOARD_NONNULL, char const *NOTICEBOARD_NULLABLE);
char const *NOTICEBOARD_NONNULL NBgetIcon(NBNotice const *NOTICEBOARD_NONNULL notice);

void NBsetExpireTime(NBNotice *NOTICEBOARD_NONNULL, int);
int NBgetExpireTime(NBNotice const *NOTICEBOARD_NONNULL notice);

int NBSend(NBNotice *NOTICEBOARD_NONNULL,  //
    char const *NOTICEBOARD_NONNULL header,
    char const *NOTICEBOARD_NULLABLE body);

int NBSendPos(NBNotice *NOTICEBOARD_NONNULL,  //
    int x,
    int y,
    char const *NOTICEBOARD_NONNULL header,
    char const *NOTICEBOARD_NULLABLE body);

int NBSendSync(NBNotice *NOTICEBOARD_NONNULL,  //
    char const *NOTICEBOARD_NONNULL header,
    char const *NOTICEBOARD_NULLABLE body,
    char const * NOTICEBOARD_NULLABLE * NOTICEBOARD_NULLABLE action_result);

int NBSendPosSync(NBNotice *NOTICEBOARD_NONNULL,
    int x,
    int y,
    char const *NOTICEBOARD_NONNULL header,
    char const *NOTICEBOARD_NULLABLE body,
    char const * NOTICEBOARD_NULLABLE * NOTICEBOARD_NULLABLE action_result);

#ifdef __cplusplus
}
#endif

#endif  // NOTICEBOARD_H
