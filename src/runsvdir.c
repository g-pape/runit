#include "hasinotify.h"
#ifdef HASINOTIFY
#include <sys/inotify.h>
#include <limits.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "direntry.h"
#include "strerr.h"
#include "error.h"
#include "wait.h"
#include "env.h"
#include "open.h"
#include "pathexec.h"
#include "fd.h"
#include "byte.h"
#include "str.h"
#include "coe.h"
#include "iopause.h"
#include "sig.h"
#include "ndelay.h"

#define USAGE " [-P] dir"
#define VERSION "$Id$"

#define MAXSERVICES 1000

char *progname;
char *svdir;
unsigned long dev =0;
unsigned long ino =0;
struct {
  unsigned long dev;
  unsigned long ino;
  int pid;
  int isgone;
} sv[MAXSERVICES];
int svnum =0;
int check =1;
int selfpipe[2];
char *rplog =0;
int rploglen;
int logpipe[2];
#ifdef HASINOTIFY
#define INBUFSIZE (sizeof(struct inotify_event) +NAME_MAX +1 > 256 ? \
                   sizeof(struct inotify_event) +NAME_MAX +1 : 256)
#define IONUM 2
int watch[2];
#else
#define INBUFSIZE 256
#define IONUM 1
#endif
char inbuf[INBUFSIZE];
iopause_fd io[IONUM +1];
struct taia stamplog;
int exitsoon =0;
int pgrp =0;

void usage () { strerr_die4x(1, "usage: ", progname, USAGE, "\n"); }
void fatal(char *m1, char *m2) {
  strerr_die6sys(100, "runsvdir ", svdir, ": fatal: ", m1, m2, ": ");
}
void warn(char *m1, char *m2) {
  strerr_warn6("runsvdir ", svdir, ": warning: ", m1, m2, ": ", &strerr_sys);
}
void warn3x(char *m1, char *m2, char *m3) {
  strerr_warn6("runsvdir ", svdir, ": warning: ", m1, m2, m3, 0);
}
void s_term(int unused) { exitsoon =1; write(selfpipe[1], "", 1); }
void s_hangup(int unused) { exitsoon =2; write(selfpipe[1], "", 1); }
void s_child(int unused) { write(selfpipe[1], "", 1); }

void runsv(int no, char *name) {
  int pid;

  if ((pid =fork()) == -1) {
    warn("unable to fork for ", name);
    return;
  }
  if (pid == 0) {
    /* child */
    char *prog[3];

    prog[0] ="runsv";
    prog[1] =name;
    prog[2] =0;
    sig_uncatch(sig_hangup);
    sig_unblock(sig_hangup);
    sig_uncatch(sig_term);
    sig_unblock(sig_term);
    sig_uncatch(sig_child);
    sig_unblock(sig_child);
    if (pgrp) setsid();
    pathexec_run(*prog, prog, (char* const*)environ);
    fatal("unable to start runsv ", name);
  }
  sv[no].pid =pid;
}

void runsvdir() {
  DIR *dir;
  direntry *d;
  int i;
  struct stat s;

  if (! (dir =opendir("."))) {
    warn("unable to open directory ", svdir);
    return;
  }
  for (i =0; i < svnum; i++) sv[i].isgone =1;
  errno =0;
  while ((d =readdir(dir))) {
    if (d->d_name[0] == '.') continue;
    if (stat(d->d_name, &s) == -1) {
      warn("unable to stat ", d->d_name);
      errno =0;
      continue;
    }
    if (! S_ISDIR(s.st_mode)) continue;
    for (i =0; i < svnum; i++) {
      if ((sv[i].ino == s.st_ino) && (sv[i].dev == s.st_dev)) {
        sv[i].isgone =0;
        if (! sv[i].pid) runsv(i, d->d_name);
        break;
      }
    }
    if (i == svnum) {
      /* new service */
      if (svnum >= MAXSERVICES) {
        warn3x("unable to start runsv ", d->d_name, ": too many services.");
        continue;
      }
      sv[i].ino =s.st_ino;
      sv[i].dev =s.st_dev;
      sv[i].pid =0;
      sv[i].isgone =0;
      svnum++;
      runsv(i, d->d_name);
      check =1;
    }
  }
  if (errno) {
    warn("unable to read directory ", svdir);
    closedir(dir);
    check =1;
    return;
  }
  closedir(dir);

  /* SIGTERM removed runsv's */
  for (i =0; i < svnum; i++) {
    if (! sv[i].isgone) continue;
    if (sv[i].pid) kill(sv[i].pid, SIGTERM);
    sv[i--] =sv[--svnum];
    check =1;
  }
}

int setup_log() {
  if ((rploglen =str_len(rplog)) < 7) {
    warn3x("log must have at least seven characters.", 0, 0);
    return(0);
  }
  rplog +=5; rploglen -=5;
  if (pipe(logpipe) == -1) {
    warn3x("unable to create pipe for log.", 0, 0);
    return(-1);
  }
  coe(logpipe[1]);
  coe(logpipe[0]);
  ndelay_on(logpipe[0]);
  ndelay_on(logpipe[1]);
  if (fd_move(2, logpipe[1]) == -1) {
    warn3x("unable to set filedescriptor for log.", 0, 0);
    return(-1);
  }
  io[IONUM].fd =logpipe[0];
  io[IONUM].events =IOPAUSE_READ;
  taia_now(&stamplog);
  return(1);
}
#ifdef HASINOTIFY
unsigned int watch_inotify() {
  int w;

  if ((w =inotify_add_watch(io[1].fd, svdir, IN_DONT_FOLLOW|
          IN_DELETE_SELF|IN_MOVE_SELF)) == -1)
    return(0);
  if (watch[0] != w) inotify_rm_watch(io[1].fd, watch[0]);
  watch[0] =w;
  if ((w =inotify_add_watch(io[1].fd, svdir,
          IN_DELETE_SELF|IN_MOVE_SELF|IN_CREATE|IN_DELETE|IN_MOVE)) == -1)
    return(0);
  if (watch[1] != w) inotify_rm_watch(io[1].fd, watch[1]);
  watch[1] =w;
  return(1);
}
#endif

int main(int argc, char **argv) {
  struct stat s;
  time_t mtime =0;
  int wstat;
  int curdir;
  int pid;
  struct taia deadline;
  struct taia now;
  struct taia stampcheck;
  char ch;
  int i;

  progname =*argv++;
  if (! argv || ! *argv) usage();
  if (**argv == '-') {
    switch (*(*argv +1)) {
    case 'P': pgrp =1;
    case '-': ++argv;
    }
    if (! argv || ! *argv) usage();
  }
  svdir =*argv++;

  if (pipe(selfpipe) == -1) fatal("unable to create selfpipe", 0);
  coe(selfpipe[0]);
  coe(selfpipe[1]);
  ndelay_on(selfpipe[0]);
  ndelay_on(selfpipe[1]);
  io[0].fd =selfpipe[0];
  io[0].events =IOPAUSE_READ;

  if (argv && *argv) {
    rplog =*argv;
    if (setup_log() != 1) {
      rplog =0;
      warn3x("log service disabled.", 0, 0);
    }
  }
  if ((curdir =open_read(".")) == -1) 
    fatal("unable to open current directory", 0);
  coe(curdir);
#ifdef HASINOTIFY
  if ((io[1].fd =inotify_init()) == -1)
    fatal("unable to initialize inotify instance", 0);
  coe(io[1].fd);
  ndelay_on(io[1].fd);
  io[1].events =IOPAUSE_READ;
  if ((watch[0] =inotify_add_watch(io[1].fd, svdir, IN_DONT_FOLLOW|
          IN_DELETE_SELF|IN_MOVE_SELF)) == -1)
    fatal("unable to add watch to inotify instance", 0);
  if ((watch[1] =inotify_add_watch(io[1].fd, svdir,
          IN_DELETE_SELF|IN_MOVE_SELF|IN_CREATE|IN_DELETE|IN_MOVE)) == -1)
    fatal("unable to add watch to inotify instance", 0);
#endif
  sig_block(sig_term);
  sig_catch(sig_term, s_term);
  sig_block(sig_hangup);
  sig_catch(sig_hangup, s_hangup);
  sig_block(sig_child);
  sig_catch(sig_child, s_child);
  taia_now(&stampcheck);

  for (;;) {
    /* collect children */
    for (;;) {
      if ((pid =wait_nohang(&wstat)) <= 0) break;
      for (i =0; i < svnum; i++) {
        if (pid == sv[i].pid) {
          /* runsv has gone */
          sv[i].pid =0;
          check =1;
          break;
        }
      }
    }

    taia_now(&now);
    if (now.sec.x < (stampcheck.sec.x -3)) {
      /* time warp */
      warn3x("time warp: resetting time stamp.", 0, 0);
      taia_now(&stampcheck);
      taia_now(&now);
      if (rplog) taia_now(&stamplog);
    }
    if (taia_less(&now, &stampcheck) == 0) {
      /* wait at least a second */
      taia_uint(&deadline, 1);
      taia_add(&stampcheck, &now, &deadline);
      
      if (stat(svdir, &s) != -1) {
        if (check || \
            s.st_mtime != mtime || s.st_ino != ino || s.st_dev != dev) {
          /* svdir modified */
#ifdef HASINOTIFY
          if (!watch_inotify())
            warn("unable to add watch to inotify instance", 0);
#endif
          if (chdir(svdir) != -1) {
            mtime =s.st_mtime;
            dev =s.st_dev;
            ino =s.st_ino;
            check =0;
            runsvdir();
            while (fchdir(curdir) == -1) {
              warn("unable to change directory, pausing", 0);
              sleep(5);
            }
          }
          else
            warn("unable to change directory to ", svdir);
        }
      }
      else
        warn("unable to stat ", svdir);
    }

    if (rplog)
      if (taia_less(&now, &stamplog) == 0) {
        for (i =1; i < rploglen; i++) rplog[i -1] =rplog[i];
        rplog[rploglen -1] ='.';
        taia_uint(&deadline, 900);
        taia_add(&stamplog, &now, &deadline);
      }
    taia_uint(&deadline, check ? 1 : 5);
    taia_add(&deadline, &now, &deadline);
    if (rplog && taia_less(&stamplog, &deadline)) deadline =stamplog;

    sig_unblock(sig_hangup);
    sig_unblock(sig_term);
    sig_unblock(sig_child);
    iopause(io, IONUM + (rplog ? 1 : 0), &deadline, &now);
    sig_block(sig_child);
    sig_block(sig_term);
    sig_block(sig_hangup);

    if (io[0].revents) while (read(selfpipe[0], &ch, 1) == 1) {}
#ifdef HASINOTIFY
    if (io[1].revents) {
      check =1;
      while (read(io[1].fd, inbuf, sizeof(inbuf)) > 0) {}
    }
#endif
    if (rplog && io[IONUM].revents)
      while ((i =read(logpipe[0], inbuf, 256)) > 0) {
        int j;
        if (i < rploglen)
          for (j =0; j < rploglen -i; ++j) rplog[j] =rplog[j +i];
        j =(i > rploglen) ? rploglen : i;
        byte_copy(rplog +rploglen -j, j, inbuf +i -j);
      }

    switch(exitsoon) {
    case 1:
      _exit(0);
    case 2:
      for (i =0; i < svnum; i++) if (sv[i].pid) kill(sv[i].pid, SIGTERM);
      _exit(111);
    }
  }
  /* not reached */
  _exit(0);
}
