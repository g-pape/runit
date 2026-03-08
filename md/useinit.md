% runit - use with traditional init

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - use with traditional init

---

It\'s possible to use *runit*\'s service supervision without replacing
the *init* scheme of the system. Simply run the *stage 2* of *runit* as
a service with your current *init*.

Normally this is done by either adding an entry for
`/sbin/runsvdir-start` to `/etc/inittab`, or by adding
`/sbin/runsvdir-start` as command to /etc/rc.local, or by adding
`/sbin/runsvdir-start` to the system\'s `StartupItems`.

In any case, you first need to copy the *stage 2* script to
`/sbin/runsvdir-start`, and create the services directory `/service/`:

     # install -m0750 /package/admin/runit/etc/2 /sbin/runsvdir-start
     # mkdir -p /service

---

[How to use with sysvinit and inittab](#sysv)\
[How to use with sysvinit and upstart](#upstart)\
[How to use with \*BSD init](#bsd)\
[How to use with MacOSX init](#macosx)

---

[]{#sysv}

## Using with sysvinit and inittab

If your system uses a sysvinit alike init scheme with a `/etc/inittab`
file, do:

     # cat >>/etc/inittab <<EOT
     SV:123456:respawn:/sbin/runsvdir-start
     EOT

and tell *init* to re-read its configuration, e.g.:

     # init q

---

[]{#upstart}

## Using with sysvinit and upstart

If your system uses a sysvinit alike init scheme that utilizes upstart
instead of inittab, and which has start and stop scripts located in
`/etc/init/`, do:

     # cat >/etc/init/runsvdir.conf <<\EOT
     # for runit - manage /usr/sbin/runsvdir-start
     start on runlevel 2
     start on runlevel 3
     start on runlevel 4
     start on runlevel 5
     stop on shutdown
     respawn
     exec /usr/sbin/runsvdir-start
     EOT

and tell init to start the new service, e.g.:

     # start runsvdir

---

[]{#bsd}

## Using with \*BSD init

If your system uses a BSD alike init scheme with a `/etc/rc.local`
script, do:

     # cat >>/etc/rc.local <<EOT
     csh -cf '/sbin/runsvdir-start &'
     EOT

and reboot your system.

---

[]{#macosx}

## Using with MacOSX init

On MacOSX 10.2 create an entry for *runit* in
`/System/Library/StartupItems/`:

     # cd /System/Library/StartupItems
     # mkdir -p runit
     # cp -p /package/admin/runit/etc/macosx/StartupItems/* runit/

and reboot your system.

On MacOSX 10.4 create an entry for *runit* in `/Library/LaunchDaemons/`,
and tell *launchd* to start the new service:

     # cp /package/admin/runit/etc/macosx/org.smarden.runit.plist \
         /Library/LaunchDaemons/
     # launchctl load /Library/LaunchDaemons/org.smarden.runit.plist

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
