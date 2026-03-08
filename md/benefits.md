% runit - benefits

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - benefits

---

[Service supervision](#supervision)\
[Clean process state](#state)\
[Reliable logging facility](#log)\
[Fast system boot up and shutdown](#fast)\
[Portability](#portability)\
[Packaging friendly](#packaging)\
[Small code size](#smallcode)

---

[]{#supervision}

### Service supervision

Each service is associated with a *service directory*, and each service
daemon runs as a child process of a supervising [runsv](runsv.8.html)
process running in this directory. The [runsv](runsv.8.html) program
provides a reliable interface for signalling the service daemon and
controlling the service and supervisor. Normally the [sv](sv.8.html)
program is used to send commands through this interface, and to query
status information about the service.

The [runsv](runsv.8.html) program supervises the corresponding service
daemon. By default a service is defined to be up, that means, if the
service daemon dies, it will be restarted. Of course you can [tell
runsv](sv.8.html) otherwise.

This reliable interface to control daemons and supervisors obsoletes
pid-guessing programs, such as `pidof`, `killall`, `start-stop-daemon`,
which, due to guessing, are prone to failures by design. It also
obsoletes so called `pid-files`, no need for each and every service
daemon to include code to daemonize, to write the new process id into a
file, and to take care that the file is removed properly on shutdown,
which might be very difficult in case of a crash.

---

[]{#state}

### Clean process state

*runit* guarantees each service a clean process state, no matter if the
service is activated for the first time or automatically at boot time,
reactivated, or simply restarted. This means that the service always is
started with the same environment, resource limits, open file
descriptors, and controlling terminals.

You don\'t necessarily have that with *sysv init* scripts for example.
It requires a carefully written init script that reliably cleans up and
sets the process state before starting the service daemon. This adds
even more complexity to the init script in comparison with a run script
used by *runit*. Many of today\'s init scripts don\'t provide a clean
process state, here is an example on what could happen:

     # /etc/init.d/foo-daemon start
     Starting foo daemon: food.
     #

Fine. Everything works, nothing to worry about. After rebooting the
system this shows up on the screen:

     ...
     Starting foo daemon: food: command not found
     failed.
     ...

The `food` program is installed in `/opt/foo/bin/`. When starting the
service for the first time using the init script, the `PATH` environment
variable contained `/opt/foo/bin`. After reboot `init` started the
service using the init script, but with a different value for the `PATH`
variable, not containing `/opt/foo/bin`. Of course the init script
should have set `PATH` before starting the daemon; the problem is that
it worked in the first place, and that the error didn\'t show up until
system reboot.

With bad init scripts miraculous things could also happen when just
doing

     # /etc/init.d/foo-daemon restart

at the command line.

The clean process state includes open file descriptors, obsoleting the
widely used hack in many service daemons to force-close all file
descriptors that might be open, up to the limit of available file
descriptors for the daemon process (often results in 1024 unnecessary
close() system calls in a great number of service daemon
implementations).

---

[]{#log}

### Reliable logging facility

The [runsv](runsv.8.html) program provides a reliable logging facility
for the service daemon. If configured, [runsv](runsv.8.html) creates a
pipe, starts and supervises an additional log service, redirects the log
daemon\'s standard input to read from the pipe, and redirects the
service daemon\'s standard output to write to the pipe. Restarting the
service does not require restarting the log service, and vice versa. A
good choice for a log daemon is *runit*\'s service logging daemon
[svlogd](svlogd.8.html).

The service daemon and the log daemon can run with different process
states, and under different user id\'s. *runit* supports easy and
reliable logging for service daemons running chroot\'ed.

If [runsv](runsv.8.html) is told to shutdown a service, e.g. at system
shutdown, it ensures that the log service stays up as long as the
corresponding service daemon is running and possibly writing to the log.

---

[]{#fast}

### Fast system boot up and shutdown

After the system\'s one time tasks (stage 1) are done, the system
services are started up in parallel. The operating system\'s process
scheduler takes care of having the services available as soon as
possible.

On system shutdown, stage 3 uses [runsv](runsv.8.html)\'s control
interface to wait until each service daemon is terminated and all logs
are written. Again, services are taken down in parallel. As soon as all
services are down, system halt or system reboot is initiated.

---

[]{#portability}

### Portability

*runit* comes ready-to-run for Debian GNU/Linux and BSD systems, and can
easily be configured to run on other UNIX systems. The UNIX system\'s
one time initialization tasks and tasks to shutdown the system must be
identified and *runit*\'s stages 1 and 3 configured accordingly.

Stages 1 and 3 handle one time tasks. They only run for short and exit
soon. Stage 2 handles the system\'s uptime tasks (via the
[runsvdir](runsvdir.8.html) program) and is running the whole system\'s
uptime.

*runit*\'s stage 2 is portable across UNIX systems. *runit* is well
suited for server systems and embedded systems, and also does its job
well on desktop systems.

---

[]{#packaging}

### Packaging friendly

*runit*\'s stages 1 and 3 are distribution specific. They normally are
shell scripts, and an operating system distribution with software
package management should adapt these scripts if they need support for
their package management.

Stage 2 is packaging friendly: all a software package that provides a
service needs to do is to include a *service directory* in the package,
and to provide a symbolic link to this directory in `/service/`. The
service will be started within five seconds, and automatically at boot
time. The package\'s install and update scripts can use the reliable
control interface to stop, start, restart, or send signals to the
service. On package removal, the symbolic link simply is removed. The
service will be taken down automatically.

---

[]{#smallcode}

### Small code size

One of the *runit* project\'s principles is to keep the code size small.
As of version 1.0.0 of *runit*, the `runit.c` source contains 330 lines
of code; the `runsvdir.c` source is 274 lines of code, the `runsv.c`
source 509. This minimizes the possibility of bugs introduced by
programmer\'s fault, and makes it more easy for security related people
to proofread the source code.

The *runit* core programs have a very small memory footprint and do not
allocate memory dynamically.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
