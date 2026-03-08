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
force-stop, force-reload, force-restart, force-shutdown.

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

**sv** actually looks only at the first character of these *command*s.

## Commands compatible to LSB init script actions

**status**
:   Same as *status*.

**start**
:   Same as *up*, but wait up to 7 seconds for the command to take
    effect. Then report the status or timeout. If the script *./check*
    exists in the service directory, **sv** runs this script to check
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
    or timeout. If the script *./check* exists in the service directory,
    **sv** runs this script to check whether the service is up and
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
    on timeout send the service the *kill* command. If the script
    *./check* exists in the service directory, **sv** runs this script
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
    state of the service is *up*, and the script *./check* exists in the
    service directory, **sv** runs this script to check whether the
    service is up and running; it\'s considered to be up if *./check*
    exits with 0.

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

# SEE ALSO

runsv(8), chpst(8), svlogd(8), runsvdir(8), runsvchdir(8), runit(8),
runit-init(8)

https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
