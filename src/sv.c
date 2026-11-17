#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "str.h"
#include "byte.h"
#include "strerr.h"
#include "error.h"
#include "sgetopt.h"
#include "open.h"
#include "env.h"
#include "buffer.h"
#include "fmt.h"
#include "scan.h"
#include "tai.h"
#include "taia.h"
#include "wait.h"
#include "sig.h"
#include "stralloc.h"
#include "pathexec.h"

#define USAGE " [-v] [-w sec] command service ..."
#define USAGELSB " [-w sec] command"

#define VERSION "$Id$"

#define FATAL   "fatal: "
#define FAIL    "fail: "
#define WARN    "warning: "
#define OK      "ok: "
#define RUN     "run: "
#define FINISH  "finish: "
#define DOWN    "down: "
#define TIMEOUT "timeout: "
#define KILL    "kill: "

char *progname;
char *action;
char *name;
char *acts =0; /* 0: custom command */
/* "unable to run command/<name>", doubles as path of the command script */
char unable[14 +8 +256 +8] ="unable to run command/";
#define cmd (unable +14)
char *chain =""; /* $SVCOMMAND: dev:inode:name, of the scripts calling us */
const char *waitarg =0; /* -w sec, passed on as $SVWAIT */
char *varservice ="/service/";
char **service;
char **servicex;
unsigned int services;
unsigned int rc =0;
unsigned int lsbrc =0; /* LSB: exit code of the first failing command script */
unsigned int lsb; /* LSB exit codes */
unsigned int initd; /* LSB init script interface */
unsigned int verbose =0;
unsigned long wait =7;
unsigned int kll =0;
unsigned int islog =0;
struct taia tstart, tnow, tdiff;
struct tai tstatus;

int (*act)(char*) =0;
int (*cbk)(char*) =0;

/* name, single character aliases, control characters,
   wait for the command to take effect: 1, and kill on timeout: 2 */
struct { char name[15]; char c; char c2; char acts[4]; char wait; } cmds[] ={
  { "up", 'u', 0, "u", 0 },
  { "down", 'd', 0, "d", 0 },
  { "status", 's', 0, "s", 0 },
  { "once", 'o', 0, "o", 0 },
  { "pause", 'p', 0, "p", 0 },
  { "cont", 'c', 0, "c", 0 },
  { "hup", 'h', 0, "h", 0 },
  { "alarm", 'a', 0, "a", 0 },
  { "interrupt", 'i', 0, "i", 0 },
  { "quit", 'q', 0, "q", 0 },
  { "1", '1', 0, "1", 0 },
  { "2", '2', 0, "2", 0 },
  { "term", 't', 0, "t", 0 },
  { "kill", 'k', 0, "k", 0 },
  { "exit", 'x', 'e', "x", 0 },
  { "start", 0, 0, "u", 1 },
  { "stop", 0, 0, "d", 1 },
  { "reload", 0, 0, "h", 1 },
  { "restart", 0, 0, "tcu", 1 },
  { "shutdown", 0, 0, "x", 1 },
  { "force-stop", 'D', 0, "d", 2 },
  { "force-reload", 'T', 0, "tc", 2 },
  { "force-restart", 0, 0, "tcu", 2 },
  { "force-shutdown", 'X', 'E', "x", 2 },
  { "try-restart", 0, 0, "tc", 1 },
  { "check", 0, 0, "C", 1 }
};

int curdir, fd, r;
char svstatus[20];
char sulong[FMT_ULONG];

void usage() {
  strerr_die4x(lsb ? 2 : 100, "usage: ", progname,
               initd ? USAGELSB : USAGE, "\n");
}
void done(unsigned int e) { if (curdir != -1) fchdir(curdir); _exit(e); }
void fatal(char *m1) {
  strerr_warn3(FATAL, m1, ": ", &strerr_sys);
  done(lsb ? 151 : 100);
}
void fatal2(char *m1, char *m2) {
  strerr_warn4(FATAL, m1, m2, ": ", &strerr_sys);
  done(lsb ? 151 : 100);
}
void out(char *p, char *m1) {
  buffer_puts(buffer_1, p);
  buffer_puts(buffer_1, *service);
  if (islog) buffer_puts(buffer_1, "/log");
  buffer_puts(buffer_1, ": ");
  buffer_puts(buffer_1, m1);
  if (errno) {
    buffer_puts(buffer_1, ": ");
    buffer_puts(buffer_1, error_str(errno));
  }
  buffer_puts(buffer_1, "\n");
  buffer_flush(buffer_1);
}
void fail(char *m1) { ++rc; out(FAIL, m1); }
void failx(char *m1) { errno =0; fail(m1); }
void warn(char *m1) { ++rc; out(WARN, m1); }
void warnx(char *m1) { errno =0; warn(m1); }
void ok(char *m1) { errno =0; out(OK, m1); }

void outs(const char *s) { buffer_puts(buffer_1, s); }
void flush(const char *s) { outs(s); buffer_flush(buffer_1); }
void outs2(const char *s) { buffer_puts(buffer_2, s); }
void flush2(const char *s) { outs2(s); buffer_flush(buffer_2); }

int svstatus_get() {
  if ((fd =open_write("supervise/ok")) == -1) {
    if (errno == error_nodevice) {
      *acts == 'x' ? ok("runsv not running") : failx("runsv not running");
      return(0);
    }
    if (islog && (errno == error_noent))
      warnx("supervise/ok not found, exit and restart runsv to enable log");
    else
      warn("unable to open supervise/ok");
    return(-1);
  }
  close(fd);
  if ((fd =open_read("supervise/status")) == -1) {
    warn("unable to open supervise/status");
    return(-1);
  }
  r =read(fd, svstatus, 20);
  close(fd);
  switch(r) {
  case 20: break;
  case -1: warn("unable to read supervise/status"); return(-1);
  default: warnx("unable to read supervise/status: bad format"); return(-1);
  }
  return(1);
}
unsigned int svstatus_print(char *m) {
  int pid;
  int normallyup =0;
  struct stat s;
 
  if (stat("down", &s) == -1) {
    if (errno != error_noent) {
      outs2(WARN); outs2("unable to stat "); outs2(*service); outs2("/down: ");
      outs2(error_str(errno)); flush2("\n");
      return(0);
    }
    normallyup =1;
  }
  pid =(unsigned char) svstatus[15];
  pid <<=8; pid +=(unsigned char)svstatus[14];
  pid <<=8; pid +=(unsigned char)svstatus[13];
  pid <<=8; pid +=(unsigned char)svstatus[12];
  tai_unpack(svstatus, &tstatus);
  switch (svstatus[19]) {
  case 0: outs(DOWN); break;
  case 1: outs(RUN); break;
  case 2: outs(FINISH); break;
  }
  outs(m); outs(": ");
  if (svstatus[19]) {
    outs("(pid "); sulong[fmt_ulong(sulong, pid)] =0;
    outs(sulong); outs(") ");
  }
  buffer_put(buffer_1, sulong,
    fmt_ulong(sulong, tnow.sec.x < tstatus.x ? 0 : tnow.sec.x -tstatus.x));
  outs("s");
  if (pid && !normallyup) outs(", normally down");
  if (!pid && normallyup) outs(", normally up");
  if (pid && svstatus[16]) outs(", paused");
  if (!pid && (svstatus[17] == 'u')) outs(", want up");
  if (pid && (svstatus[17] == 'd')) outs(", want down");
  if (pid && svstatus[18]) outs(", got TERM");
  return(pid ? 1 : 2);
}
int status(char *unused) {
  int rc;

  rc =svstatus_get();
  switch(rc) { case -1: if (lsb) done(4); case 0: return(0); }
  rc =svstatus_print(*service);
  islog =1;
  if (chdir("log") == -1) {
    if (errno != error_noent) {
      outs("; ");
      warn("unable to change directory");
    }
    else outs("\n");
  }
  else {
    outs("; ");
    if (svstatus_get() > 0) { svstatus_print("log"); outs("\n"); }
  }
  islog =0;
  flush("");
  if (lsb) switch(rc) { case 1: done(0); case 2: done(3); case 0: done(4); }
  return(rc);
}

int checkscript() {
  char *prog[2];
  struct stat s;
  int pid, w;

  if (stat("check", &s) == -1) {
    if (errno == error_noent) return(1);
    outs2(WARN); outs2("unable to stat "); outs2(*service); outs2("/check: ");
    outs2(error_str(errno)); flush2("\n");
    return(0);
  }
  /* if (!(s.st_mode & S_IXUSR)) return(1); */
  if ((pid =fork()) == -1) {
    outs2(WARN); outs2("unable to fork for "); outs2(*service);
    outs2("/check: "); outs2(error_str(errno)); flush2("\n");
    return(0);
  }
  if (!pid) {
    prog[0] ="./check";
    prog[1] =0;
    close(1);
    execve("check", prog, environ);
    outs2(WARN); outs2("unable to run "); outs2(*service); outs2("/check: ");
    outs2(error_str(errno)); flush2("\n");
    _exit(0);
  }
  while (wait_pid(&w, pid) == -1) {
    if (errno == error_intr) continue;
    outs2(WARN); outs2("unable to wait for child "); outs2(*service);
    outs2("/check: "); outs2(error_str(errno)); flush2("\n");
    return(0);
  }
  return(!wait_exitcode(w));
}

int check(char *a) {
  unsigned int pid;
  char c;

  if ((r =svstatus_get()) == -1) return(-1);
  while (*a) {
    if (r == 0) { if (*a == 'x') return(1); return(-1); }
    if ((c =*a) == 'C') {
      if (svstatus[17] == 'd') c ='d'; else c ='u';
    }
    pid =(unsigned char)svstatus[15];
    pid <<=8; pid +=(unsigned char)svstatus[14];
    pid <<=8; pid +=(unsigned char)svstatus[13];
    pid <<=8; pid +=(unsigned char)svstatus[12];
    switch (c) {
    case 'x': return(0);
    case 'u':
      if (!pid || svstatus[19] != 1) return(0);
      if (!checkscript()) return(0);
      break;
    case 'd': if (pid || svstatus[19] != 0) return(0); break;
    case 't':
    case 'k':
      if (!pid && svstatus[17] == 'd') break;
      tai_unpack(svstatus, &tstatus);
      if ((tstart.sec.x > tstatus.x) || !pid || svstatus[18] || !checkscript())
        return(0);
      break;
    case 'o':
      tai_unpack(svstatus, &tstatus);
      if ((!pid && tstart.sec.x > tstatus.x) || (pid && svstatus[17] != 'd'))
        return(0);
      break;
    case 'p': if (pid && !svstatus[16]) return(0); break;
    case 'c': if (pid && svstatus[16]) return(0); break;
    }
    ++a;
  }
  outs(OK); svstatus_print(*service); flush("\n");
  return(1);
}
int control(char *a) {
  if ((fd =open_write("supervise/control")) == -1) {
    if (errno != error_nodevice)
      warn("unable to open supervise/control");
    else
      *a == 'x' ? ok("runsv not running") : failx("runsv not running");
    return(-1);
  }
  r =write(fd, a, str_len(a));
  close(fd);
  if (r != str_len(a)) {
    warn("unable to write to supervise/control");
    return(-1);
  }
  return(1);
}

unsigned int fmt_hex(char *s, const unsigned char *b, unsigned int n) {
  unsigned int i;

  for (i =0; i < n; ++i) {
    *s++ ="0123456789abcdef"[b[i] >> 4];
    *s++ ="0123456789abcdef"[b[i] & 15];
  }
  return(2 * n);
}
/* start ./command/<name>: its pid, 0 if it's up to the built-in command,
   -1 on failure */
int command() {
  struct stat s;
  /* device:inode:name of the script, all bytes of device and inode in hex */
  char id[2 * (sizeof s.st_dev +sizeof s.st_ino) +2 +256];
  char *prog[2];
  char *x;
  unsigned int i, n;
  int pid;

  if (lstat(cmd, &s) == -1) {
    if ((errno == error_noent) || (errno == error_notdir)) {
      if (acts) return(0);
      if (lsb) usage();
    }
    /* command/ is not accessible: there still are the built-in commands */
    else if (acts && (errno == error_acces)) return(0);
    fail(unable); return(-1);
  }
  if (stat(".", &s) == -1) {
    fail("unable to stat service directory"); return(-1);
  }
  n =fmt_hex(id, (unsigned char*)&s.st_dev, sizeof s.st_dev);
  id[n++] =':';
  n +=fmt_hex(id +n, (unsigned char*)&s.st_ino, sizeof s.st_ino);
  id[n++] =':';
  i =str_len(name); byte_copy(id +n, i +1, name); n +=i;
  /* called by a command script: don't run the very same script again */
  for (x =chain; *x; ) {
    i =str_chr(x, ',');
    if ((i == n) && !byte_diff(x, n, id)) {
      if (acts) return(0);
      failx("command calls itself"); return(-1);
    }
    x +=i; if (*x) ++x;
  }
  buffer_flush(buffer_1);
  if ((pid =fork()) == -1) { fail("unable to fork"); return(-1); }
  if (!pid) {
    stralloc sa ={0};

    close(curdir);
    if (stralloc_copys(&sa, chain) && (!*chain || stralloc_cats(&sa, ","))
        && stralloc_catb(&sa, id, n) && stralloc_0(&sa)
        && pathexec_env("SVCOMMAND", sa.s)
        && (!waitarg || pathexec_env("SVWAIT", waitarg))
        && (!initd || pathexec_env("SVLSB", "1"))) {
      prog[0] =cmd; prog[1] =0;
      pathexec(prog);
    }
    fail(unable);
    _exit(127);
  }
  return(pid);
}
/* wait for ./command/<name> to finish */
void commandwait(int pid) {
  char *x;
  int w, e;

  if (wait_pid(&w, pid) == -1) { fail("unable to wait for command"); return; }
  if (w) {
    e =wait_crashed(w) ? 1 : wait_exitcode(w);
    /* 127: the child has told already that it's unable to run the script */
    if (e == 127) { ++rc; e =1; }
    else {
      /* init script: the script's exit code is the answer, e.g. 3 = down */
      if (initd && !wait_crashed(w)) done(e);
      x =cmd +8 +str_len(name);
      byte_copy(x, 8, " failed"); failx(cmd); *x =0;
    }
    if (!lsbrc) lsbrc =e;
  }
}

void sleep_microseconds(unsigned int microseconds) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec sleeptime;
  sleeptime.tv_sec = microseconds / 1000000;
  sleeptime.tv_nsec = (microseconds % 1000000) * 1000L;
  nanosleep(&sleeptime, NULL);
#else
  usleep(microseconds);
#endif
}

int main(int argc, char **argv) {
  unsigned int i, done;
  char *x;
  char c;
  int pid[argc]; /* of the command script of each service */

  progname =*argv;
  for (i =str_len(*argv); i; --i) if ((*argv)[i -1] == '/') break;
  *argv +=i;
  optprogname =progname =*argv;
  sig_uncatch(sig_child); /* ignored by the caller: no child to wait for */
  service =argv;
  services =1;
  lsb =initd =(str_diff(progname, "sv"));
  if ((x =env_get("SVDIR"))) varservice =x;
  if ((x =env_get("SVWAIT"))) scan_ulong(x, &wait);
  if ((x =env_get("SVCOMMAND"))) chain =x;
  /* a command script of an init script: LSB exit codes, but sv's arguments */
  if ((x =env_get("SVLSB")) && *x) lsb =1;
  while ((i =getopt(argc, (char* const*)argv, "w:vV")) != opteof) {
    switch(i) {
    case 'w': scan_ulong(optarg, &wait); waitarg =optarg;
    case 'v': verbose =1; break;
    case 'V': strerr_warn1(VERSION, 0);
    case '?': usage();
    }
  }
  argv +=optind; argc -=optind;
  if (!(action =*argv++)) usage();
  --argc;
  if (!initd) { service =argv; services =argc; }
  if (!*service) usage();

  taia_now(&tnow); tstart =tnow;
  if ((curdir =open_read(".")) == -1)
    fatal("unable to open current directory");

  for (c =*action, i =0; i < sizeof(cmds) / sizeof(*cmds); ++i)
    if ((c && !action[1]) ? (c == cmds[i].c) || (c == cmds[i].c2)
                          : (c == *cmds[i].name)
                            && !str_diff(action, cmds[i].name)) break;
  if (i < sizeof(cmds) / sizeof(*cmds)) {
    name =cmds[i].name;
    acts =cmds[i].acts;
    act =&control;
    if (verbose || cmds[i].wait) cbk =&check;
    kll =(cmds[i].wait == 2);
    /* LSB status is the status of the first service */
    if (*acts == 's') { act =&status; cbk =0; if (lsb) services =1; }
    if (*acts == 'C') act =0;
  }
  else {
    /* custom command: [A-Za-z0-9][A-Za-z0-9_-]*, a file name */
    for (name =action, i =0; (c =name[i]); ++i)
      if (((c < 'a') || (c > 'z')) && ((c < 'A') || (c > 'Z'))
          && ((c < '0') || (c > '9'))
          && (!i || ((c != '_') && (c != '-')))) usage();
    if (!i || (i > 255)) usage();
  }
  byte_copy(cmd +8, str_len(name) +1, name);

  servicex =service;
  for (i =0; i < services; ++i) {
    if ((**service != '/') && (**service != '.') && **service &&
        ((*service)[str_len(*service) -1] != '/')) {
      if ((chdir(varservice) == -1) || (chdir(*service) == -1)) {
        fail("unable to change to service directory");
        *service =0;
      }
    }
    else
      if (chdir(*service) == -1) {
        fail("unable to change to service directory");
        *service =0;
      }
    pid[i] =0;
    if (*service && !(pid[i] =command()))
      if (act && (act(acts) == -1)) *service =0;
    if (fchdir(curdir) == -1) fatal("unable to change to original directory");
    service++;
  }

  if (*cbk)
    for (;;) {
      taia_sub(&tdiff, &tnow, &tstart);
      service =servicex; done =1;
      for (i =0; i < services; ++i, ++service) {
        if (!*service || pid[i]) continue;
        if ((**service != '/') && (**service != '.')) {
          if ((chdir(varservice) == -1) || (chdir(*service) == -1)) {
            fail("unable to change to service directory");
            *service =0;
          }
        }
        else
          if (chdir(*service) == -1) {
            fail("unable to change to service directory");
            *service =0;
          }
        if (*service) { if (cbk(acts) != 0) *service =0; else done =0; }
        if (*service && taia_approx(&tdiff) > wait) {
          kll ? outs(KILL) : outs(TIMEOUT);
          if (svstatus_get() > 0) { svstatus_print(*service); ++rc; }
          flush("\n");
          if (kll) control("k");
          *service =0;
        }
        if (fchdir(curdir) == -1)
          fatal("unable to change to original directory");
      }
      if (done) break;
      sleep_microseconds(420000);
      taia_now(&tnow);
    }
  /* the command scripts have run at the same time as the other services */
  for (service =servicex, i =0; i < services; ++i, ++service)
    if (pid[i] > 0) commandwait(pid[i]);
    else if ((pid[i] == -1) && !lsbrc) lsbrc =1;
  if (lsb) return(lsbrc ? lsbrc : rc > 0);
  return(rc > 99 ? 99 : rc);
}
