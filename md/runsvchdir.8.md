% runsvchdir(8)

# NAME

runsvchdir - change services directory of runsvdir(8)

# SYNOPSIS

**runsvchdir** *dir*

# DESCRIPTION

*dir* is a services directory for the use with **runsvdir**(8). If *dir*
does not start with a slash, it is searched in **runsvchdir**\'s working
directory, by default */etc/runit/runsvdir/*. *dir* must not start with
a dot.

**runsvchdir** switches to its working directory, copies *current* to
*previous*, and replaces *current* with a symlink pointing to *dir*.

Normally */service* is a symlink to *current*, and **runsvdir**(8) is
running */service/*.

# ENVIRONMENT

**RUNSVDIR**
:   The environment variable $RUNSVDIR overrides the default working
    directory */etc/runit/runsvdir/*.

# EXIT CODES

**runsvchdir** prints an error message and exits 111 on error.
**runsvchdir** exits 0 on success.

# FILES

/etc/runit/runsvdir/previous\
/etc/runit/runsvdir/current\
/etc/runit/runsvdir/current.new

# SEE ALSO

runsvdir(8), runit(8), runit-init(8), sv(8), runsv(8)

https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
