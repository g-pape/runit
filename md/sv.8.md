% sv(8)

# NAME

sv - control and manage services monitored by **runsv**(8)

# SYNOPSIS

**sv** \[-v\] \[-w *sec*\] *command* *services*

**/etc/init.d/***service* \[-w *sec*\] *command*

# DESCRIPTION

The **sv** program reports the current status and controls the state of
services monitored by the **runsv**(8) supervisor.

*services* consists of one or more arguments, each argument naming a
directory *service* used by **runsv**(8). If *service* doesn\'t start
with a dot or slash and doesn\'t end with a slash, it is searched in the
default services directory */service/*, otherwise relative to the
current directory.

*command* is one of up, down, status, once, pause, cont, hup, alarm,
interrupt, 1, 2, term, kill, or exit, or start, stop, restart, shutdown,
force-stop, force-reload, force-restart, force-shutdown, or a custom
command, see CUSTOMIZE COMMANDS below.

The **sv** program can be sym-linked to */etc/init.d/* to provide an LSB
init script interface. The *service* to be controlled then is specified
by the base name of the "init script".

# COMMANDS

**status**
:   Report the current status of the service, and the appendant log
    service if available, to standard output.

**up**
:   If the service is not running, start it. If the service stops,
    restart it.

**down**
:   If the service is running, send it the TERM signal, and the CONT
    signal. If ./run exits, start ./finish if it exists. After it stops,
    do not restart service.

**once**
:   If the service is not running, start it. Do not restart it if it
    stops.

**pause cont hup alarm interrupt quit 1 2 term kill**
:   If the service is running, send it the STOP, CONT, HUP, ALRM, INT,
    QUIT, USR1, USR2, TERM, or KILL signal respectively.

**exit**
:   If the service is running, send it the TERM signal, and the CONT
    signal. Do not restart the service. If the service is down, and no
    log service exists, **runsv**(8) exits. If the service is down and a
    log service exists, **runsv**(8) closes the standard input of the
    log service and waits for it to terminate. If the log service is
    down, **runsv**(8) exits. This command is ignored if it is given to
    an appendant log service.

Each of these *command*s can be abbreviated to its first character, and
*exit* also to **x**. The characters **D**, **T**, and **X** or **E**
are short for force-stop, force-reload, and force-shutdown, see below.
Any other abbreviation or longer form is not one of these *command*s,
but a custom command, see CUSTOMIZE COMMANDS below.

## Commands compatible to LSB init script actions

**status**
:   Same as *status*.

**start**
:   Same as *up*, but wait up to 7 seconds for the command to take
    effect. Then report the status or timeout. If the program *./check*
    exists in the service directory, **sv** runs this program to check
    whether the service is up and available; it\'s considered to be
    available if *./check* exits with 0.

**stop**
:   Same as *down*, but wait up to 7 seconds for the service to become
    down. Then report the status or timeout.

**reload**
:   Same as *hup*, and additionally report the status afterwards.

**restart**
:   Send the commands *term*, *cont*, and *up* to the service, and wait
    up to 7 seconds for the service to restart. Then report the status
    or timeout. If the program *./check* exists in the service directory,
    **sv** runs this program to check whether the service is up and
    available again; it\'s considered to be available if *./check* exits
    with 0.

**shutdown**
:   Same as *exit*, but wait up to 7 seconds for the **runsv**(8)
    process to terminate. Then report the status or timeout.

**force-stop**
:   Same as *down*, but wait up to 7 seconds for the service to become
    down. Then report the status, and on timeout send the service the
    *kill* command.

**force-reload**
:   Send the service the *term* and *cont* commands, and wait up to 7
    seconds for the service to restart. Then report the status, and on
    timeout send the service the *kill* command.

**force-restart**
:   Send the service the *term*, *cont* and *up* commands, and wait up
    to 7 seconds for the service to restart. Then report the status, and
    on timeout send the service the *kill* command. If the program
    *./check* exists in the service directory, **sv** runs this program
    to check whether the service is up and available again; it\'s
    considered to be available if *./check* exits with 0.

**force-shutdown**
:   Same as *exit*, but wait up to 7 seconds for the **runsv**(8)
    process to terminate. Then report the status, and on timeout send
    the service the *kill* command.

**try-restart**
:   if the service is running, send it the *term* and *cont* commands,
    and wait up to 7 seconds for the service to restart. Then report the
    status or timeout.

## Additional Commands

**check**
:   Check for the service to be in the state that\'s been requested (up
    or down). Wait up to 7 seconds for the service to reach the
    requested state, then report the status or timeout. If the requested
    state of the service is *up*, and the program *./check* exists in the
    service directory, **sv** runs this program to check whether the
    service is up and running; it\'s considered to be up if *./check*
    exits with 0.

# CUSTOMIZE COMMANDS

If the program *service*/command/*command* exists, **sv** runs it in the
service directory instead of doing the *command* for this *service*; an
abbreviated *command* is looked up by its full name, "`sv d`" runs
*service*/command/down. This way a service can replace the commands
described above, and provide custom ones. The name of a custom command
consists of up to 255 letters, digits, underscores, and hyphens, and
starts with a letter or digit. A custom command fails for a *service*
without the program, and doesn\'t need a running **runsv**(8).

If the program cannot be run, or exits non zero, the *command* fails for
this *service*; **sv** doesn\'t fall back to the *command* described
above. If **sv** may not search *service*/command/, it does the commands
described above, and custom commands fail.

If the program runs **sv** with the same *command* on its own service
directory, **sv** does the *command* described above instead of running
the program again, see \$SVCOMMAND. With -w, **sv** passes *sec* to the
program in \$SVWAIT.

For example, the executable file */service/mysql/command/stop* with the
lines "`#!/bin/sh`" and "`exec sv -w 60 stop "$PWD"`" makes
"`sv stop mysql`" wait up to 60 seconds; */service/nginx/command/rotate*
with "`exec sv 1 "$PWD"`" makes "`sv rotate nginx`" send the service the
USR1 signal.

The programs for several *services* run at the same time. **sv**
doesn\'t time out a program: if a program for *force-stop* or
*exit* hangs, so does stage 3 of **runit**(8) on shutdown. The programs
run with the privileges of the caller of **sv**, as *./check* does; keep
*service*/command/ writable by trusted users only.

# OPTIONS

**-v**
:   If the *command* is up, down, term, once, cont, or exit, then wait
    up to 7 seconds for the command to take effect. Then report the
    status or timeout.

**-w** *sec*
:   Override the default timeout of 7 seconds with *sec* seconds. This
    option implies *-v*.

# ENVIRONMENT

**SVDIR**
:   The environment variable $SVDIR overrides the default services
    directory */service/*.

**SVWAIT**
:   The environment variable $SVWAIT overrides the default 7 seconds to
    wait for a command to take effect. It is overridden by the -w
    option.

**SVCOMMAND**
:   Set by **sv** for the programs in *service*/command/, see CUSTOMIZE
    COMMANDS.

**SVLSB**
:   Set to 1 by **sv** for the programs in *service*/command/ if it is
    called with a base name other than **sv**, see EXIT CODES.

# EXIT CODES

**sv** exits 0, if the *command* was successfully sent to all
*services*, and, if it was told to wait, the *command* has taken effect
to all services.

For each *service* that caused an error (e.g. the directory is not
controlled by a **runsv**(8) process, or **sv** timed out while
waiting), **sv** increases the exit code by one and exits non zero. The
maximum is 99. **sv** exits 100 on error.

If **sv** is called with a base name other than **sv**: it exits 1 on
timeout or trouble sending the command; if the *command* is **status**,
it exits 3 if the service is down, and 4 if the status is unknown; it
exits 2 on wrong usage, and 151 on error.

If **sv** is called with a base name other than **sv** and runs a
program *service*/command/*command*, it exits with the exit code of this
program, or 1 if the program cannot be run, exits 127, or is killed by a
signal; a custom command without program is wrong usage.

If \$SVLSB is set and not empty, **sv** takes its arguments as usual,
but exits as if called with a base name other than **sv**; **status**
then deals with the first *service* only.

# SEE ALSO

runsv(8), chpst(8), svlogd(8), runsvdir(8), runsvchdir(8), runit(8),
runit-init(8)

https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
