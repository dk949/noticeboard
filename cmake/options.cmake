option(NOTICEBOARD_ENABLE_DBUS "Compile dbus support" ON)
option(NOTICEBOARD_ENABLE_NOTIFY_SEND "Compile notify-send support" ON)
option(NOTICEBOARD_FORCE_NOTIFY_SEND "Compile notify-send support" ON)
set(NOTICEBOARD_FORCE_SYSTEM "" CACHE STRING "Force cmake system name (NOTE: does not perform cross-compilation!)")
