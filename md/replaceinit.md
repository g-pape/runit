% runit - replacing init

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - replacing init

---

[How to replace sysvinit on GNU/Linux](#sysv)\
[How to replace init on \*BSD](#bsd)\
[How to replace init on MacOSX](#macosx)\
[How to replace init on Solaris](#solaris)

---

[]{#sysv}

## Replacing sysvinit on GNU/Linux

Follow these steps to migrate from *init* to *runit* on a
[GNU/Linux](https://en.wikipedia.org/wiki/Linux) system that runs with
[sysvinit](https://en.wikipedia.org/wiki/Init#SYSV). The `/sbin/init`
program is not replaced until step 4. The result after step 4 is a
*runit* init setup with *sysvinit*\'s boot-time system
configuration/initialization.

Login as root to a local serial or virtual terminal.

### Step 1: The three stages

[runit](runit.8.html) looks for the three stages implementing the
system\'s *booting*, *running* and *shutdown* in `/etc/runit/1`,
`/etc/runit/2` and `/etc/runit/3` respectively. Create the scripts now:

    mkdir -p /etc/runit
    cp -p /package/admin/runit/etc/sysv/[123] /etc/runit/

Read the *inittab*(5) man page and look for `sysinit` in `/etc/inittab`
to check that `/etc/init.d/rcS` is the *sysvinit* boot-time system
configuration/initialization script on your system. If not, adapt
`/etc/runit/1` accordingly.

    man 5 inittab
    grep sysinit /etc/inittab

### Step 2: The runit programs

The *runit* programs must reside on the root partition, install them
into `/sbin`:

    install -m0755 /package/admin/runit/command/runit* /sbin/

### Step 3: The getties

When booting with *runit* as *init*, at least one `getty` should run in
stage 2 so that you are able to login to the terminal. Create the
getty-default service directory now:

    mkdir -p /etc/sv/getty-default

Run the `tty` command to get the name of the current terminal, and
create the corresponding `getty-default` run script:

    TTYNAME=$(tty); cat >/etc/sv/getty-default/run <<EOT
    #!/bin/sh
    exec agetty ${TTYNAME##*/}
    EOT
    chmod 0755 /etc/sv/getty-default/run

Check `/etc/inittab` again, now for the default invocation of a `getty`
program on your system, and if it's not `agetty`, adapt
`/etc/sv/getty-default/run` accordingly.

    grep getty /etc/inittab

Tell *runit* about the `getty-default` service:

    mkdir -p /service
    ln -s /etc/sv/getty-default /service/

Optionally create more `getty` services for additional terminals, look
at `/etc/inittab`.

### Step 4: Replace the `/sbin/init` program

Before replacing the `init` program, make sure that you are able to boot
the system by other means to restore the original `/sbin/init` if
anything goes wrong, e.g. with the installation image, a rescue system,
or save a snapshot.

Make a backup copy of the `/sbin/init` program and replace it with
`/sbin/runit-init`:

    cp -p /sbin/init /sbin/init.sysv
    install /sbin/runit-init /sbin/init

Boot your system with *runit* for the first time:

    init.sysv 6

Watch the console output while [runit](runit.8.html) starts the system,
and runs *sysvinit*\'s boot-time system configuration/initialization.
Switch to the same terminal as before, the default `getty` runs there to
login. Other services need to be migrated or started manually, see
below.

Use **init 6** to reboot and **init 0** to halt a system that runs with
*runit*. This will cause [runit](runit.8.html) to enter stage 3 and to
tell the Linux kernel to reboot or halt when stage 3 is done.

To report success:

    ( uname -a ; cat /etc/runit/[123] ) |mail pape-runit-2.3.1@xxiv.smarden.org

### Step 5: Service migration

By default *runit* runs *sysvinit*\'s `/etc/init.d/rcS` script in stage
1 as one time task, so boot-time system configuration/initialization is
done as before:

    #!/bin/sh
    # system one time tasks

    /etc/init.d/rcS
    /etc/init.d/rmnologin

    touch /etc/runit/stopit
    chmod 0 /etc/runit/stopit

The system runs with [runit](runit.8.html) as *init* and service
supervision enabled. You can now [add new services](faq.html#run), and
migrate services from the original *init* scheme to *runit* service
supervision, because of the [benefits](benefits.html). Take a look at
these [run scripts](runscripts.html) for popular services.

Login as root to a local terminal. Usually *sysvinit* enters runlevel 2
after booting, look for `initdefault` in `/etc/inittab`. Run
*sysvinit*\'s `rc` script to start the services from *sysvinit*\'s
default runlevel (replace "`2`" with your system's default):

    /etc/init.d/rc 2

To migrate a service, [create a service directory](faq.html#create),
stop the service again, and [tell runit](faq.html#tell) about the new
service.

#### Example: `sshd`

    mkdir -p /etc/sv/sshd
    cat >/etc/sv/sshd/run <<\EOT
    #!/bin/sh
    exec /usr/sbin/sshd -D
    EOT
    chmod 0755 /etc/sv/sshd/run
    /etc/init.d/sshd stop
    ln -s /etc/sv/sshd /service/

Check the status of the *runit* `sshd` service:

    sv status sshd

#### Example: `cron`

    mkdir -p /etc/sv/cron
    cat >/etc/sv/cron/run <<\EOT
    #!/bin/sh
    exec cron -f
    EOT
    chmod 0755 /etc/sv/cron/run
    /etc/init.d/cron stop
    ln -s /etc/sv/cron /service/

Check the status of the *runit* `cron` service:

    sv status cron

---

[]{#bsd}

## Replacing init on \*BSD

Follow these steps to migrate from *init* to *runit* on
[FreeBSD](https://www.freebsd.org/) (14.4 and older),
[NetBSD](https://netbsd.org) (10.1 and older), or
[OpenBSD](https://www.openbsd.org/) (7.8 and older). The `/sbin/init`
program is not replaced until step 4.

Login as root to a local serial or virtual terminal.

### Step 1: The three stages

[runit](runit.8.html) looks for the three stages implementing the
system\'s *booting*, *running* and *shutdown* in `/etc/runit/1`,
`/etc/runit/2` and `/etc/runit/3` respectively. Create the scripts now:

    mkdir -p /etc/runit
    cp -p /package/admin/runit/etc/bsd/[123] /etc/runit/

### Step 2: The runit programs

The *runit* programs must reside on the root partition, install them
into `/sbin`:

    install -m0500 /package/admin/runit/command/runit* /sbin/

### Step 3: The getties

When booting with *runit* as *init*, at least one `getty` should run in
stage 2 so that you are able to login to the terminal. Create the
`getty-default` service directory now:

    mkdir -p /etc/sv/getty-default

Run the `tty` command to get the name of the current terminal, and create
the corresponding `getty-default` run script:

    TTYNAME=$(tty); cat >/etc/sv/getty-default/run <<EOT
    #!/bin/sh
    exec /usr/libexec/getty Pc ${TTYNAME##*/}
    EOT
    chmod 0755 /etc/sv/getty-default/run

Tell *runit* about the `getty-default` service:

    mkdir -p /service
    ln -s /etc/sv/getty-default /service/

Optionally create more `getty` services for additional terminals, look
at `/etc/ttys`.

On a headless system you can also do the migration through `ssh` login
without setting up any `getty` service.

### Step 4: Replace the `/sbin/init` program

Before replacing the `init` program, make sure that you are able to boot
the system by other means to restore the original `/sbin/init` if
anything goes wrong, e.g. with the installation image, a rescue system,
or save a snapshot.

Make a backup copy of the `/sbin/init` program and replace it with
`/sbin/runit-init`:

    cp -p /sbin/init /sbin/init.bsd
    install /sbin/runit-init /sbin/init

Boot your system with *runit* for the first time:

    reboot

Watch the console output while [runit](runit.8.html) starts the system.
Switch to the same terminal as before, the default `getty` runs there to
login.

Use **init 6** to reboot and **init 0** to halt a system that runs with
*runit*. This will cause [runit](runit.8.html) to enter stage 3 which
runs `/sbin/reboot` or `/sbin/halt` as last command.

To report success:

    ( uname -a ; cat /etc/runit/[123] ) |mail pape-runit-2.3.1@xxiv.smarden.org

### Step 5: Service migration

By default *runit* runs the `/etc/rc` scripts in stage 1 as one time
task, so services are started as before:

    #!/bin/sh
    # system one time tasks

    /bin/sh /etc/rc autoboot

    touch /etc/runit/stopit
    chmod 0 /etc/runit/stopit

The system starts up as usual apart from the `getties`, but with
[runit](runit.8.html) as *init* and service supervision enabled. You can
now [add new services](faq.html#run), and migrate services from the
*/etc/rc.\** scheme to *runit* service supervision, because of the
[benefits](benefits.html). Take a look at these [run
scripts](runscripts.html) for popular services.

To migrate a service, [create a service directory](faq.html#create),
stop the service if it is running, disable the service in `/etc/rc.conf`
or remove the service startup from the `/etc/rc.*` scripts, and [tell
runit](faq.html#tell) about the new service.

#### Example: `sshd`

    mkdir -p /etc/sv/sshd
    cat >/etc/sv/sshd/run <<\EOT
    #!/bin/sh
    exec /usr/sbin/sshd -D
    EOT
    chmod 0755 /etc/sv/sshd/run
    /etc/rc.d/sshd stop

Edit `/etc/rc.conf` and disable `sshd` (ie. set to "`NO`") to prevent it
from being started again. Then enable the *runit* `sshd` service:

    ln -s /etc/sv/sshd /service/

Check the status of the *runit* `sshd` service:

    sv status sshd

#### Example: `cron`

    mkdir -p /etc/sv/cron
    cat >/etc/sv/cron/run <<\EOT
    #!/bin/sh
    exec cron -n
    EOT
    chmod 0755 /etc/sv/cron/run
    /etc/rc.d/cron stop

Edit `/etc/rc.conf` or `/etc/defaults/rc.conf` and disable `cron` (ie.
set to "`NO`") to prevent it from being started again. Then enable the
*runit* `cron` service:

    ln -s /etc/sv/cron /service/

Check the status of the *runit* `cron` service:

    sv status cron

---

[]{#macosx}

## Replacing init on MacOSX

Replacing init on MacOSX is not yet supported. Please refer to the
[instructions](useinit.html) on how to use *runit* service supervision
with the MacOSX init scheme.

---

[]{#solaris}

## Replacing init on Solaris

Replacing init on Solaris is not yet supported. Please refer to the
[instructions](useinit.html) on how to use *runit* service supervision
with the Solaris sysvinit scheme.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
