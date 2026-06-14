% runit(8)

# NAME

runit - a UNIX process no 1

# SYNOPSIS

**runit**

# DESCRIPTION

**runit** must be run as Unix process no 1. It performs the system\'s
booting, running, and shutdown in three stages:

# STAGE 1

**runit** runs */etc/runit/1* and waits for it to terminate. The
system\'s one time tasks are done here. */etc/runit/1* has full control
of */dev/console* to be able to start an emergency shell if the one time
initialization tasks fail. If */etc/runit/1* crashes, or exits 100,
**runit** will skip stage 2 and enter stage 3.

# STAGE 2

**runit** runs */etc/runit/2*, which should not return until system
shutdown; if it crashes, or exits 111, it will be restarted. Normally
*/etc/runit/2* starts **runsvdir**(8). **runit** is able to handle the
ctrl-alt-del keyboard request in stage 2, see below.

# STAGE 3

If **runit** is told to shutdown the system, or stage 2 returns, it
terminates stage 2 if it is running, and runs */etc/runit/3*. The
systems tasks to shutdown and possibly halt or reboot the system are
done here. If stage 3 returns, **runit** checks if the file
*/etc/runit/reboot* exists and has the execute by owner permission set.
If so, the system is rebooted, it\'s halted otherwise. If
*/etc/runit/nosync* exists, **runit** doesn\'t invoke sync(). This is
useful in vservers.

# CTRL-ALT-DEL

If **runit** receives the ctrl-alt-del keyboard request and the file
*/etc/runit/ctrlaltdel* exists and has the execute by owner permission
set, **runit** runs */etc/runit/ctrlaltdel*, waits for it to terminate,
and then sends itself a CONT signal.

# SIGNALS

**runit** only accepts signals in stage 2.

If **runit** receives a CONT signal and the file */etc/runit/stopit*
exists and has the execute by owner permission set, **runit** is told to
shutdown the system.

If **runit** receives an INT signal, a ctrl-alt-del keyboard request is
triggered.

# SEE ALSO

runit-init(8), runsvdir(8), runsvchdir(8), sv(8), runsv(8), chpst(8),
svlogd(8)

https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
