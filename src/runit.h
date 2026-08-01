#define RUNIT "/sbin/runit"
#define STOPIT "/etc/runit/stopit"
#define REBOOT "/etc/runit/reboot"
#define NOSYNC "/etc/runit/nosync"
#define CTRLALTDEL "/etc/runit/ctrlaltdel"

#ifdef RUNSV_USE_SYSLIMITS
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 256
#endif
#define BUFSIZE (PATH_MAX > 256 ? PATH_MAX : 256)
#else
#define BUFSIZE 256
#endif
