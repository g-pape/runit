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

## Replacing sysvinit (GNU/Linux)

Follow these steps to migrate from *sysvinit* to *runit* on [Debian
GNU/Linux (woody)](https://www.debian.org/releases/woody/). The
`/sbin/init` binary is not replaced until step 6, *runit* is the default
Unix process no 1 after step 7.

If you have installed the precompiled Debian package, start at step 3.

### Step 1: The three stages

*runit* looks for the three stages implementing the system\'s *booting*,
*running* and *shutdown* in `/etc/runit/1`, `/etc/runit/2` and
`/etc/runit/3`, create the files now:

     # mkdir -p /etc/runit
     # cp -p /package/admin/runit/etc/debian/[123] /etc/runit/

Create also a getty service directory:

     # mkdir -p /etc/sv/getty-5
     # cp -p /package/admin/runit/etc/debian/getty-tty5/run /etc/sv/getty-5/

If you want *runit* to handle the ctrl-alt-del keyboard request, do:

     # cp -p /package/admin/runit/etc/debian/ctrlaltdel /etc/runit/

### Step 2: The runit programs

The *runit* programs must reside on the root partition, copy them to
`/sbin`:

     # cp -p /package/admin/runit/command/runit* /sbin/

### Step 3: The getties

At least one getty must run in stage 2 so that you are able to login.
Choose a free `tty`, say `tty5`, where *sysvinit* is not running any
getty (edit `/etc/inittab` and `kill -HUP 1` if needed), and tell
[runsvdir](runsvdir.8.html) about the getty-5 *service*:

     # mkdir -p /service
     # ln -s /etc/sv/getty-5 /service/

Start *runit*\'s stage 2 for testing:

     # /etc/runit/2 &

And check that the getty is running.

### Step 4: Reboot into runit for testing

Boot your system with *runit* for the first time. This does not change
the default boot behavior of your system, *lilo* will be told to use
*runit* just once:

-   reboot the system
-   enter the following on the lilo prompt:\
    `init=/sbin/runit-init`
-   watch the console output while *runit* boots up the system
-   switch to `tty5` when stage 2 is reached, a `getty` should run
    there, you are able to login.

If you are not using *lilo* as boot loader, refer to the documentation
of your boot loader on how to pass `init=/sbin/runit-init` to the
kernel.

### Step 5: Service migration

The goal is to migrate all services from *sysvinit* scheme to the
*runit* service supervision design; take a look at these [run
scripts](runscripts.html) for popular services. The migration can be
done smoothly. For those services that are not migrated to use `run`
scripts yet, add the corresponding `init`-script startup to
`/etc/runit/1`, e.g.:

     #!/bin/sh
     # one time tasks

     /etc/init.d/kerneld start
     /etc/init.d/rmnologin

     touch /etc/runit/stopit
     chmod 0 /etc/runit/stopit

It is possible to just add `/etc/init.d/rc 2` for having all services
from the former runlevel 2 started as one time tasks, but keep the goal
above in mind, supervising services has great advantages.

To migrate a service, [create a service directory](faq.html#create),
disable the service if it is running, disable the service in
`/etc/rc.conf` or remove the service startup from the `/etc/rc.*`
scripts and [tell runsvdir](faq.html#tell) about the new service.

Repeat step 4 and 5, using **`/sbin/runit-init 6`** to reboot the
system, until you are satisfied with your services startup. If anything
goes wrong, reboot the system into the default *sysvinit* `/sbin/init`
and repair the *runit* stages, then start again at step 4.

### Step 6: Replace /sbin/init

Now it is time to replace the *sysvinit* `/sbin/init` binary:

     # mv /sbin/init /sbin/init.sysv
     # ln -s runit-init /sbin/init

### Step 7: Final reboot

The last step is to do the final reboot to boot the system with the new
default Unix process no 1 *runit*.

     # init 6

To report success:

     # ( uname -a ; cat /etc/runit/[123] ) |mail pape-runit-2.3.1@xxiv.smarden.org

---

[]{#bsd}

## Replacing init (\*BSD)

Follow these steps to migrate from *init* to *runit* on [OpenBSD
2.9](https://www.openbsd.org/) or [FreeBSD
4.4](https://www.freebsd.org/). The `/sbin/init` binary is not replaced
until step 4.

### Step 1: The three stages

*runit* looks for the three stages implementing the system\'s *booting*,
*running* and *shutdown* in `/etc/runit/1`, `/etc/runit/2` and
`/etc/runit/3` respectively. Create the scripts now:

     # mkdir -p /etc/runit

    OpenBSD 2.9:
     # cp -p /package/admin/runit/etc/openbsd/[123] /etc/runit/

    FreeBSD 4.4:
     # cp -p /package/admin/runit/etc/freebsd/[123] /etc/runit/

Remove the `svscanboot` startup from `/etc/rc.local` by deleting the
line `csh -cf '/command/svscanboot &'` (this normally is the last one);
*runit* will start [runsvdir](runsvdir.8.html) in stage 2 after running
`rc.local` in stage 1.

     # vi /etc/rc.local

### Step 2: The runit programs

The *runit* programs must reside on the root partition, install them
into `/sbin`:

     # install -m0500 /package/admin/runit/command/runit* /sbin/

### Step 3: The getties

At least one getty must run in stage 2 so that you are able to login. To
have it run on the virtual console no 5, create the getty-5 service
directory:

     # mkdir -p /etc/sv/getty-5

    OpenBSD 2.9:
     # cp -p /package/admin/runit/etc/openbsd/getty-ttyC4/run /etc/sv/getty-5/

    FreeBSD 4.4:
     # cp -p /package/admin/runit/etc/freebsd/getty-ttyv4/run /etc/sv/getty-5/

and tell [runsvdir](runsvdir.8.html) about the `getty-5` *service*:

     # mkdir -p /service
     # ln -s /etc/sv/getty-5 /service/

Start *runit*\'s stage 2 for testing:

     # /etc/runit/2 &

And check that the getty is running.

### Step 4: Replace the `/sbin/init` binary

Before replacing the `init` binary, make sure that you are able to boot
your system alternatively, e.g. with a boot floppy, to restore the
former `/sbin/init` if anything goes wrong.

Make a backup copy of the current `/sbin/init` program and replace it
with `/sbin/runit-init`:

     # cp -p /sbin/init /sbin/init.bsd
     # install /sbin/runit-init /sbin/init

Boot your system with *runit* for the first time:

     # reboot

Watch the console output while *runit* boots up the system. Switch to
the virtual console 5 (CTRL-ALT-F5) when stage 2 is reached, a getty
should run there, you are able to login.

Use **init 6** to reboot and **init 0** to halt a system that runs
*runit*. This will cause *runit* to enter stage 3 which runs
`/sbin/reboot` or `/sbin/halt` as last command.

To report success:

     # ( uname -a ; cat /etc/runit/[123] ) |mail pape-runit-2.3.1@xxiv.smarden.org

### Step 5: Service migration

The goal is to migrate all services from */etc/rc.\** scheme to the
*runit* service supervision design; take a look at these [run
scripts](runscripts.html) for popular services. The migration can be
done smoothly. By default *runit* runs the `/etc/rc` scripts in stage 1
as a one time task, so the services are started automatically:

     #!/bin/sh
     # system one time tasks

     /bin/sh /etc/rc autoboot

     touch /etc/runit/stopit
     chmod 0 /etc/runit/stopit

To migrate a service, [create a service directory](faq.html#create),
disable the service if it is running, disable the service in
`/etc/rc.conf` or remove the service startup from the `/etc/rc.*`
scripts and [tell runsvdir](faq.html#tell) about the new service.

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
