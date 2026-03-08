% chpst(8)

# NAME

chpst - runs a program with a changed process state

# SYNOPSIS

**chpst** \[-vVP012\] \[-u *user*\] \[-U *user*\] \[-b *argv0*\] \[-e
*dir*\] \[-/ *root*\] \[-C *pwd*\] \[-n *inc*\] \[-l\|-L *lock*\] \[-m
*bytes*\] \[-d *bytes*\] \[-o *n*\] \[-p *n*\] \[-f *bytes*\] \[-c
*bytes*\] \[-t *seconds*\] *prog*

# DESCRIPTION

*prog* consists of one or more arguments.

**chpst** changes the process state according to the given options, and
runs *prog*.

# OPTIONS

**-u** *\[:\]user\[:group\]*
:   setuidgid. Set uid and gid to the *user*\'s uid and gid, as found in
    */etc/passwd*. If *user* is followed by a colon and a *group*, set
    the gid to *group*\'s gid, as found in */etc/group*, instead of
    *user*\'s gid. If *group* consists of a colon-separated list of
    group names, **chpst** sets the group ids of all listed groups. If
    *user* is prefixed with a colon, the *user* and all *group*
    arguments are interpreted as uid and gids respectively, and not
    looked up in the password or group file. All initial supplementary
    groups are removed.

**-U** *\[:\]user\[:group\]*
:   envuidgid. Set the environment variables $UID and $GID to the
    *user*\'s uid and gid, as found in */etc/passwd*. If *user* is
    followed by a colon and a *group*, set $GID to the *group*\'s gid,
    as found in */etc/group*, instead of *user*\'s gid. If *user* is
    prefixed with a colon, the *user* and *group* arguments are
    interpreted as uid and gid respectively, and not looked up in the
    password or group file.

**-b** *argv0*
:   argv0. Run *prog* with *argv0* as the 0th argument.

**-e** *dir*
:   envdir. Set various environment variables as specified by files in
    the directory *dir*: If *dir* contains a file named *k* whose first
    line is *v*, **chpst** removes the environment variable *k* if it
    exists, and then adds the environment variable *k* with the value
    *v*. The name *k* must not contain =. Spaces and tabs at the end of
    *v* are removed, and nulls in *v* are changed to newlines. If the
    file *k* is empty (0 bytes long), **chpst** removes the environment
    variable *k* if it exists, without adding a new variable.

**-/** *root*
:   chroot. Change the root directory to *root* before starting *prog*.

**-C** *pwd*
:   chdir. Change the working directory to *pwd* before starting *prog*.
    When combined with -/, the working directory is changed after the
    chroot.

**-n** *inc*
:   nice. Add *inc* to the **nice**(2) value before starting *prog*.
    *inc* must be an integer, and may start with a minus or plus.

**-l** *lock*
:   lock. Open the file *lock* for writing, and obtain an exclusive lock
    on it. *lock* will be created if it does not exist. If *lock* is
    locked by another process, wait until a new lock can be obtained.

**-L** *lock*
:   The same as -l, but fail immediately if *lock* is locked by another
    process.

**-m** *bytes*
:   limit memory. Limit the data segment, stack segment, locked physical
    pages, and total of all segment per process to *bytes* bytes each.

**-d** *bytes*
:   limit data segment. Limit the data segment per process to *bytes*
    bytes.

**-o** *n*
:   limit open files. Limit the number of open file descriptors per
    process to *n*.

**-p** *n*
:   limit processes. Limit the number of processes per uid to *n*.

**-f** *bytes*
:   limit output size. Limit the output file size to *bytes* bytes.

**-c** *bytes*
:   limit core size. Limit the core file size to *bytes* bytes.

**-t** *seconds*
:   limit CPU time. Limit CPU time to *seconds* seconds, delivering a
    SIGXCPU thereafter.

**-v**
:   verbose. Print verbose messages to standard error. This includes
    warnings about limits unsupported by the system.

**-V**
:   version string. Print a version string to standard error.

**-P**
:   pgrphack. Run *prog* in a new process group.

**-0**
:   Close standard input before starting *prog*.

**-1**
:   Close standard output before starting *prog*.

**-2**
:   Close standard error before starting *prog*.

# EXIT CODES

**chpst** exits 100 when called with wrong options. It prints an error
message and exits 111 if it has trouble changing the process state.
Otherwise its exit code is the same as that of *prog*.

# EMULATION

If **chpst** is called as **envdir**, **envuidgid**, **pgrphack**,
**setlock**, **setuidgid**, or **softlimit**, it emulates the
functionality of these programs from the daemontools package
respectively.

# SEE ALSO

sv(8), runsv(8), setsid(2), runit(8), runit-init(8), runsvdir(8),
runsvchdir(8)

https://smarden.org/runit/\
https://cr.yp.to/daemontools.html

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
