% runit - experiment with Linux control groups (cgroup v2)

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - experiment with Linux control groups (cgroup v2)

---

When using *runit* service supervision on Linux, [control groups (cgroup
v2)](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html)
can be used to control system resources for service daemons that run
under [runsv](runsv.8.html). In contrast to resource limits set with the
[chpst](chpst.8.html) program, [cgroup
v2](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html)
provides additional controls, can be applied to a group of processes
started separately, and more.

The *runit* programs do not need to change to utilize [Linux control
groups (cgroup
v2)](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html).

---

[Prepare for cgroups](#prepare)

[Create a cgroup specific to a service](#create)\
[Control resource settings for a service-specific cgroup](#control)\
[Remove a cgroup after a service stops](#remove)

[Add a service to an existing cgroup](#add)

[A prototype *runsv-cg* program for convenience](#runsv-cg)

---

## [Prepare the Linux systems for cgroups]{#prepare}

To use [cgroup
v2](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html)
on a Linux system, the cgroup hierarchy needs to be mounted and
controllers must be enabled.

Add

    mount -t cgroup2 none /sys/fs/cgroup
    for i in $(cat /sys/fs/cgroup/cgroup.controllers); do
      echo +$i >/sys/fs/cgroup/cgroup.subtree_control
    done

to the end of `/etc/runit/1`. The mount point `/sys/fs/cgroup` normally
exists when the
[sysfs](https://www.kernel.org/doc/html/latest/filesystems/sysfs.html)
filesystem is mounted. Any other mount point works too.

To create a child cgroup for *runit* services with all available
controllers enabled, add

    mkdir /sys/fs/cgroup/service
    for i in $(cat /sys/fs/cgroup/service/cgroup.controllers); do
      echo +$i >/sys/fs/cgroup/service/cgroup.subtree_control
    done

to the end of `/etc/runit/1` and reboot the system.

The system now is prepared to control system resources for *runit*
services in the `/sys/fs/cgroup/service` cgroup. Take a look

    ls -l /sys/fs/cgroup/service

---

## [Create a new cgroup specific to a service]{#create}

In the `run` script of the service, create a child cgroup specific to
this service and add the current process ID to this cgroup before
replacing the shell with the service daemon using `exec`.

    #!/bin/sh
    set -e
    mkdir -p /sys/fs/cgroup/service/cron
    echo $$ >/sys/fs/cgroup/service/cron/cgroup.procs
    exec cron -f

---

## [Control resource settings for a service-specific cgroup]{#control}

A cgroup's resource setting is adjusted by writing to [interface
files](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#interface-files).

As [above](#create), do this in the service's `run` script, e.g.

    #!/bin/sh
    set -e
    mkdir -p /sys/fs/cgroup/service/cron
    echo 14 >/sys/fs/cgroup/service/cron/pids.max
    echo $$ >/sys/fs/cgroup/service/cron/cgroup.procs
    exec cron -f

To make it more convenient to adjust settings, you can create a
subdirectory `cgroup` in the service directory, copy the interface files
you're interested in to that directory, and change the `run` script to
copy these files back when the service starts, e.g.

    cd /service/cron
    mkdir cgroup
    cp /sys/fs/cgroup/service/cron/pids.max cgroup/
    echo 14 >cgroup/pids.max

The changed `run` script:

    #!/bin/sh
    set -e
    mkdir -p /sys/fs/cgroup/service/cron
    cp cgroup/* /sys/fs/cgroup/service/cron/
    echo $$ >/sys/fs/cgroup/service/cron/cgroup.procs
    exec cron -f

---

## [Remove a cgroup after a service stops]{#remove}

When using service-specific cgroups, it may be desired to remove the
cgroup after a service stops. A cgroup cannot be removed while it still
contains running processes. A service daemon might have forked children
that did not exit yet. These can be forcefully killed to successfully
remove the cgroup. Add

    #!/bin/sh
    echo 1 >/sys/fs/cgroup/service/cron/cgroup.kill
    rmdir /sys/fs/cgroup/service/cron

to the `cron` service's `finish` script; if `finish` does not yet exist,
create it and make it executable.

---

## [Add a service to an existing cgroup]{#add}

To add a service daemon to an already existing cgroup, adjust the `run`
script to write its process ID to the corresponding `cgroup.procs` file,
e.g.

    echo $$ >/sys/fs/cgroup/service/existing-cgroup/cgroup.procs

You can prepare cgroups in `/etc/runit/1` by creating corresponding
directories in `/sys/fs/cgroup/service/`, so that they exist when
*runit* services start, e.g.

    mkdir /sys/fs/cgroup/service/existing-cgroup

and control the resource settings similar to [Control resource settings
for a service-specific cgroup](#control).

In contrast to service-specific cgroups, this way *runit* services can
be grouped flexibly in one or more existing cgroups, optionally
organizing them hierarchically.

---

## [A prototype *runsv-cg* program for some convenience]{#runsv-cg}

A small program can help to manage service-specific cgroups for *runit*
services. Here's an example.

Create the prototype *runsv-cg* program

    cat >/bin/runsv-cg <<\EOT
    #!/bin/sh
    set -e
    USAGE='runsv-cg create|control|remove|destroy|setup'
    usage() { echo usage: $USAGE >&2; exit 100; }
    fail() { echo $1 >&2; exit 111; }
    cg=/sys/fs/cgroup
    sv=$(pwd); sv=${sv##*/}
    setup() {
      mount -t cgroup2 none "$cg"
      for i in $(cat "$cg"/cgroup.controllers ); do
        echo +$i >"$cg"/cgroup.subtree_control
      done
      mkdir "$cg"/service
      for i in $(cat "$cg"/service/cgroup.controllers ); do
        echo +$i >"$cg"/service/cgroup.subtree_control
      done
    }
    add() { echo $PPID >"$cg"/service/"$sv"/cgroup.procs; }
    control() { cp cgroup/* "$cg"/service/"$sv"/ || :; }
    create() { mkdir "$cg"/service/"$sv" || :; control; add; }
    remove() { rmdir "$cg"/service/"$sv"; }
    destroy() { echo 1 >"$cg"/service/"$sv"/cgroup.kill; remove; }
    test -n "$1" || usage
    case "$1" in
      setup) setup ;;
      create|control|remove|destroy)
        test -d "$cg"/service || fail "$cgroot/service: not a directory"
        test -d supervise || fail "must run in service directory"
        test -x run || fail "must run in service directory"
        test -d cgroup || fail "./cgroup: not a directory"
        test -n "$sv" || fail "unable to get service name"
        "$1"
        ;;
      *) usage ;;
    esac
    exit 0
    EOT
    chmod 0755 /bin/runsv-cg

Instead of [above](#prepare), in `/etc/runit/1` just add

    runsv-cg setup

after the `sysfs` filesystem is mounted.

The example cron `run` script is

    #!/bin/sh
    set -e
    runsv-cg create
    exec cron -f

And the corresponding `finish` script

    #!/bin/sh
    runsv-cg remove

Optionally you can use `runsv-cg destroy` instead to kill forked
children that did not exit yet, if any.

Create the `cgroup` directory in the service directory, where also the
`run` script resides, and copy the controllers' interface files you're
interested in

    cd /service/cron
    mkdir cgroup
    cp /sys/fs/cgroup/service/pids.max cgroup/

Now control the system resources for the service-specific cgroup through
the files in the `cgroup` directory in the service directory. After
changing files, use `runsv-cg` to put them into effect

    cd /service/cron
    runsv-cg control

or restart the service.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
