% runit - runlevels

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - runlevels

---

[Prepare runit for using runlevels](#prepare)\
[Switching runlevels](#switch)\
[Creating runlevels](#create)

---

[]{#prepare}

### Prepare runit

If not yet done, configure your system to use [runit](runit.8.html) as
process no 1 by following the [instructions](replaceinit.html).

Create the following directories and symbolic links:

     # mkdir -p /etc/runit/runsvdir/default
     # mkdir -p /etc/runit/runsvdir/single
     # ln -s /etc/sv/getty-5 /etc/runit/runsvdir/single/
     # ln -s default /etc/runit/runsvdir/current

Copy the contents of `/service/` to `/etc/runit/runsvdir/current/` and
replace `/service/` with a symbolic link:

     # cp -pR /service/* /etc/runit/runsvdir/current/
     # mv -f /service /service.old && \
         ln -s /etc/runit/runsvdir/current /service

You have now created two runlevels: `default` and `single`. The
`current` runlevel is `default`. It is safe to remove `/service.old/` if
you don\'t need it anymore.

Finally edit `/etc/runit/2` to set the `default` runlevel when stage 2
starts:

     $ cat /etc/runit/2 
     #!/bin/sh
     PATH=/command:/usr/local/bin:/usr/local/sbin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/X11R6/bin
     
     runsvchdir default >/dev/null
     
     exec env - PATH=$PATH \
     runsvdir /service 'log: ...........................................................................................................................................................................................................................................................................................................................................................................................................'

---

[]{#switch}

### Switching runlevels

Switching runlevels with *runit* is done by switching the directory the
[runsvdir](runsvdir.8.html) program is running in. This is done by the
[runsvchdir](runsvchdir.8.html) program, e.g. to switch to the `single`
user runlevel, do:

     # runsvchdir single

To switch back to the `default` runlevel, do:

     # runsvchdir default

See [the runsvdir program](runsvdir.8.html) for a description of what
happens when *runsvdir* sees the directory changed. Note that there is
no guarantee that all services from the `previous` runlevel will stop,
the [runsv](runsv.8.html) processes have sent the service daemons a
SIGTERM and wait for them to terminate. You can check the status of the
`previous` runlevel through `/etc/runit/runsvdir/previous/`.

---

[]{#create}

### Creating new runlevels

To create a new runlevel, simply create a new directory in
`/etc/runit/runsvdir/`. The name of the directory is the name of the new
runlevel. The name must not start with a dot and must not be `current`,
`current.new`, or `previous`, e.g.:

     # mkdir /etc/runit/runsvdir/maintenance

Add the services you want to run in the runlevel `maintenance` to the
newly created directory, e.g.:

     # ln -s /etc/sv/getty-5 /etc/runit/runsvdir/maintenance/
     # ln -s /etc/sv/ssh /etc/runit/runsvdir/maintenance/
     # ln -s /etc/sv/dnscache /etc/runit/runsvdir/maintenance/

If you want to switch to the runlevel `maintenance`, do:

     # runsvchdir maintenance

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
