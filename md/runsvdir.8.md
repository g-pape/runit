% runsvdir(8)

# NAME

runsvdir - starts and monitors a collection of runsv(8) processes

# SYNOPSIS

**runsvdir** \[-P\] *dir* \[ *log* \]

# DESCRIPTION

*dir* must be a directory. *log* is a space holder for a readproctitle
log, and must be at least seven characters long or absent.

**runsvdir** starts a **runsv**(8) process for each subdirectory, or
symlink to a directory, in the services directory *dir*, up to a limit
of 1000 subdirectories, and restarts a **runsv**(8) process if it
terminates. **runsvdir** skips subdirectory names starting with dots.
**runsv**(8) must be in **runsvdir**\'s PATH.

At least every five seconds **runsvdir** checks whether the time of last
modification, the inode, or the device, of the services directory *dir*
has changed. If so, it re-scans the service directory, and if it sees a
new subdirectory, or new symlink to a directory, in *dir*, it starts a
new **runsv**(8) process; if **runsvdir** sees a subdirectory being
removed that was previously there, it sends the corresponding
**runsv**(8) process a TERM signal, stops monitoring this process, and
so does not restart the **runsv**(8) process if it exits.

If the *log* argument is given to **runsvdir**, all output to standard
error is redirected to this *log*, which is similar to the daemontools\'
**readproctitle** log. To see the most recent error messages, use a
process-listing tool such as **ps**(1). **runsvdir** writes a dot to the
readproctitle log every 15 minutes so that old error messages expire.

# OPTIONS

**-P**
:   use **setsid**(2) to run each **runsv**(8) process in a new session
    and separate process group.

# SIGNALS

If **runsvdir** receives a TERM signal, it exits with 0 immediately.

If **runsvdir** receives a HUP signal, it sends a TERM signal to each
**runsv**(8) process it is monitoring and then exits with 111.

# SEE ALSO

sv(8), runsv(8), runsvchdir(8), runit(8), runit-init(8), chpst(8),
svlogd(8), setsid(2)

https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
