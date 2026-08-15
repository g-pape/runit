% runit - experiment with runit and other init schemes

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - experiment with runit and other init schemes

---

Debian 13.4.0 is a good test bed to experiment with *runit* and other
init schemes on *GNU/Linux*. The experiments below are reproduced in the
[Debian test bed](#install) on arm64 in qemu (UTM, MacBook M4).

---

[Install Debian 13.4 test bed](#install)

[Run runit as subinit](#subrunit)\
[Boot Debian test bed with runit](#runit)\
[Boot Debian test bed with runit and `/run/it` filesystem](#runit-fs)

[Run Debian's systemd init as subinit](#subsystemd)\
[Run Debian's sysv init as subinit](#subsysv)\
[Run an other init as subinit](#subother)

---

## [Install Debian 13.4 test bed]{#install}

Connect a (serial) operator console.

Make a standard Debian install using `debian-13.4.0-arm64-netinst.iso`,
basically

    Enter, Enter, Enter, Enter
    Enter, Enter, root, root, user, Enter, user, user, Enter
    Enter, Enter, Enter, Enter, Yes
    Enter, Enter, Enter, Enter
    Enter
    Enter
    Remove iso
    Enter

Login as `root` on console.

Prepare the system

    apt update
    apt install -y build-essential psmisc git

Install the *runit* programs

    git clone -b next https://github.com/g-pape/runit
    cd runit
    package/compile
    package/check
    install -m0755 command/* /bin/
    mv /bin/runit* /sbin/

Create a minimal *runit* configuration

    mkdir -p /etc/runit
    cat >/etc/runit/1 <<\EOT && chmod 0755 /etc/runit/1
    #!/bin/sh
    mount -oremount,rw /
    EOT
    cp -p etc/2 /etc/runit/
    cat >/etc/runit/3 <<\EOT && chmod 0755 /etc/runit/3
    #!/bin/sh
    sv force-shutdown /service/*
    mount -oremount,ro /
    EOT
    mkdir -p /service

The test bed is ready for experiments now.

---

## [Run runit as subinit]{#subrunit}

[Install the Debian test bed](#install), and create a *runit* `sshd`
service listening on port 2217 to be able to login to the *runit*
subinit

    mkdir -p /etc/sv/sshd
    cat >/etc/sv/sshd/run <<\EOT && chmod 0755 /etc/sv/sshd/run
    #!/bin/sh
    exec /usr/sbin/sshd -D -p2217
    EOT
    touch /etc/sv/sshd/down
    ln -s /etc/sv/sshd /service/

Run `runit-init` with `chpst -I` to start the *runit* subinit

    chpst -I /sbin/runit-init &

You can now [add new services](faq.html#run) to the *runit* subinit and
experiment with the *runit* configuration.

To login to the *runit* subinit, start the *runit* `sshd` service

    sv start sshd
    ssh -luser -p2217 localhost
    su -

To stop the *runit* subinit again, login to the *runit* subinit, become
root, and halt the system

    runit-init 0

You can create a Debian systemd service for "`chpst -I
/sbin/runit-init`", just instead of [using runit with current
init](useinit.html), and experiment with the differences.

---

## [Boot Debian test bed with runit]{#runit}

[Install the Debian test bed](#install), and create a `getty` service
for `/dev/console`

    mkdir -p /etc/sv/getty-console
    cat >/etc/sv/getty-console/run <<\EOT && chmod 0755 /etc/sv/getty-console/run
    #!/bin/sh
    exec agetty console
    EOT
    ln -s /etc/sv/getty-console /service/

Reboot with `runit-init` in place of Debian's `/sbin/init`

    mv /sbin/init /sbin/init.sysd
    ln -s runit-init /sbin/init
    reboot

Login as `root` on console, type

    pstree

It should print "`runit---runsvdir---runsv---login---bash---pstree`", a
minimal runit system with just one service running.

You can now [add new services](faq.html#run) to the *runit* service
supervision and experiment with the *runit* configuration.

---

## [Boot Debian test bed with runit and `/run/it` filesystem]{#runit-fs}

First [install the Debian test bed](#install), and [boot the Debian test
bed with runit](#runit).

Login as `root` on console.

Using the `/run/it` filesystem enables *runit* to maintain all runtime
data in that filesystem only, so that it doesn't require any other
writable storage.

The prototype `runit-fs` program in *stage 1* sets up the `/run/it`
filesystem as tmpfs

    /run/it size=42mb
    /run/it/service -> svdir/default
    /run/it/svdir/default/
    /run/it/sv/
    /run/it/etc/ctrlaltdel
    /run/it/etc/nosync
    /run/it/etc/reboot
    /run/it/etc/stopit
    /run/it/log/

And then stores services from the default directory `/service/` into
`/run/it/sv/` and enables them in `/run/it/service/`.

Finally it can store services from directories found in
`/etc/runit/runsvdir/` into `/run/it/sv/` and add these services
directories to `/run/it/svdir/`.

Create the prototype *runit-fs* program

    cat >/sbin/runit-fs <<\EOT && chmod 0755 /sbin/runit-fs
    #!/bin/sh
    set -e
    rc=0
    USAGE='runit-fs setup|enable-sv|store-sv|store-svdir dir ...'
    usage() { echo usage: $USAGE >&2; exit 100; }
    skip() { rc=$(($rc+1)); echo Skipping "$1"; }
    setup() {
      mkdir -p -m0755 /run/it
      mount -t tmpfs -omode=0755,size=42m,exec tmpfs /run/it
      for i in etc log svdir/default sv; do mkdir -p -m0755 /run/it/$i; done
      for i in stopit reboot ctrlaltdel nosync; do touch /run/it/etc/$i; done
      ln -s svdir/default /run/it/service
      echo Set up /run/it:
      find /run/it
    }
    store_sv() {
      test -d "$1" || { skip "$1: not a directory"; return; }
      d=$(cd "$1" && pwd -P); s=${1##*/}
      test ! -e /run/it/sv/"$s" || { skip "/run/it/sv/$s: already exists"; return; }
      mkdir -m0755 /run/it/sv/"$s"; cd /run/it/sv/"$s"; ln -s "$d" .source
      for i in run finish down check; do cp -p "$d"/$i ./ ||:; done 2>/dev/null
      test ! -d "$d"/log || {
        mkdir -m0755 log
        for i in run finish down; do cp -p "$d"/log/$i log/ ||:; done; } 2>/dev/null
      test ! -d "$d"/control || {
        mkdir -m0755 control
        for i in "$d"/control/?; do cp -p "$i" control/ ||:; done; } 2>/dev/null
      echo "Stored $d in /run/it/sv/$s:"
      find /run/it/sv/"$s"
    }
    enable() {
      test -d /run/it/sv/"$1" || { skip "/run/it/sv/$1: not a directory"; return; }
      test -d /run/it/svdir/"$2" || {
        skip "/run/it/svdir/$2: not a directory"; return; }
      test ! -e /run/it/svdir/"$2"/"$1" || {
        skip "/run/it/svdir/$2/$1: already exists"; return; }
      ln -s /run/it/sv/"$1" /run/it/svdir/"$2"/ || return 0;
      echo "Enabled $1 in /run/it/svdir/$2."
    }
    enable_sv() {
      store_sv "$1"
      enable "${1##*/}" "$2"
    }
    store_svdir() {
      test -d "$1" || { skip "$1: not a directory"; return; }
      mkdir -p -m0755 /run/it/svdir/"${1##*/}"
      for i in "$1"/*; do enable_sv "$i" "${1##*/}"; done
      echo "Stored $1 in /run/it/svdir/${1##*/}:"
      find /run/it/svdir/"${1##*/}"
    }
    diff_sv() {
      test -d /run/it/sv/"$1" || { skip "/run/it/sv/$1: not a directory"; return; }
      test -d /run/it/sv/"$1"/.source || {
        skip "/run/it/sv/$1/.source: not a directory"; return; }
      diff -ur $2 "$(cd /run/it/sv/"$1"/.source && pwd -P)" /run/it/sv/"$1"/ ||:
    }
    test -n "$2" || usage
    case "$1" in
      setup) test "$2" = /run/it || usage; setup ;;
      store-sv) shift; for i in "$@"; do store_sv "${i%/}"; done ;;
      enable-sv) shift; for i in "$@"; do enable_sv "${i%/}" default; done ;;
      store-svdir) shift; for i in "$@"; do store_svdir "${i%/}"; done ;;
      diff-sv) shift; for i in "$@"; do diff_sv "$i"; done ;;
      *) usage ;;
    esac
    exit $rc
    EOT

Replace *stage 1* to use the prototype *runit-fs* program:

    cat >/etc/runit/1 <<\EOT
    #!/bin/sh
    runit-fs setup /run/it
    runit-fs enable-sv /service/*
    EOT

Adjust *stage 2* and *stage 3* to run `runsvdir` in `/run/it/service`
instead of `/service`

    sed -i 's} /service} /run/it/service}' /etc/runit/[23]

Reboot into test bed with all runtime data in the `/run/it` filesystem

    init 6

Login as `root` on console, type

    cat /proc/mounts

It should show the `/run/it` filesystem mounted read-write and `/`
mounted read-only.

Since now `/run/it/service` is used instead of the default `/service`
directory, set the environment variable `SVDIR` accordingly

    export SVDIR=/run/it/service

To [add new services](faq.html#run) to the *runit* service supervision
and experiment with the *runit* configuration, remount the root
filesystem read-write

    mount -oremount,rw /

And don't forget the additional step to store new services into the
`/run/it` filesystem

    runit-fs enable-sv /service/new-service

It should print "`Enabled new-service in /run/it/svdir/default.`" as
last line.

---

## [Run Debian's systemd init as subinit]{#subsystemd}

First [install the Debian test bed](#install), and [boot the Debian test
bed with runit](#runit). Optionally [boot the Debian test bed with runit
and `/run/it` filesystem](#runit-fs) to have *runit* even more step
aside.

Login as `root` on console.

Make sure the root filesystem is writable

    mount -oremount,rw /

Create a service `subsystemd`

    mkdir -p /etc/sv/subsystemd
    cat >/etc/sv/subsystemd/run <<\EOT && chmod 0755 /etc/sv/subsystemd/run
    #!/bin/sh
    exec chpst -b /sbin/init -vI sh -ec '
    umount /dev/pts /dev /sys
    export container=
    exec /sbin/init.sysd
    '
    EOT
    mkdir -p /etc/sv/subsystemd/control
    cat >/etc/sv/subsystemd/control/t <<\EOT && chmod 0755 /etc/sv/subsystemd/control/t
    #!/bin/sh
    exec kill -SIGRTMIN+3 "$1"
    EOT
    touch /etc/sv/subsystemd/down
    ln -s /etc/sv/subsystemd /service/

When using `/run/it` for runtime data, store the subsystemd service into
that filesystem and adjust the environment, otherwise skip this

    runit-fs enable-sv /service/subsystemd
    export SVDIR=/run/it/service

Remove Debian's console getty service to prevent the subsystemd from
taking over the console with your login

    rm /lib/systemd/system/console-getty.service

Start Debian's systemd init as subinit

    sv once subsystemd

To login to the subsystemd, use Debian's default `sshd` service

    ssh -luser localhost
    su -

To stop it again, do

    sv stop subsystemd

It should print "`ok: down: subsystemd: 0s`".

---

## [Run Debian's sysv init as subinit]{#subsysv}

First [install the Debian test bed](#install), and [boot the Debian test
bed with runit](#runit).

Login as `root` on console.

Connect to the network

    mkdir -p /etc/sv/dhcpcd
    cat >/etc/sv/dhcpcd/run <<\EOT && chmod 0755 /etc/sv/dhcpcd/run
    #!/bin/sh
    exec dhcpcd -B
    EOT
    ln -s /etc/sv/dhcpcd /service/

Prepare Debian's sysv init

    rm -rf /run/systemd/system
    apt update
    apt -y --purge --allow-remove-essential install sysvinit-core libpam-elogind dbus-x11 systemd-sysv-

And make sure `runit-init` remains `/sbin/init`

    mv /sbin/init /sbin/init.sysv
    ln -s runit-init /sbin/init

Stop the dhcpcd service again and don't start it automatically

    sv stop dhcpcd
    touch /etc/sv/dhcpcd/down

Create a service `subsysv`

    mkdir -p /etc/sv/subsysv
    cat >/etc/sv/subsysv/run <<\EOT && chmod 0755 /etc/sv/subsysv/run
    #!/bin/sh
    exec chpst -b /sbin/init -I /sbin/init.sysv
    EOT
    touch /etc/sv/subsysv/down
    ln -s /etc/sv/subsysv /service/

Start Debian's sysv init as subinit

    sv once subsysv

Unfortunately this experiment is not that successful as for some reason
the attempt to login through Debian's default `sshd` service

    ssh -luser localhost

prints "`Host key verification failed.`" and fails.

To stop sysv subinit forcibly, do

    sv quit subsysv

The sysv subinit and all its children exit immediately.

---

## [Run an other init as subinit]{#subother}

Running other init schemes should be similar to [running runit as
subinit](#subrunit):

[Install the Debian test bed](#install). Optionally [boot the Debian
test bed with runit](#runit) and [boot the Debian test bed with runit
and `/run/it` filesystem](#runit-fs) to have *runit* even more step
aside.

Install the other init scheme and create a minimal configuration for it
if necessary.

Create a `sshd` service listening on port 2217 following the other
init's documentation to be able to login to the other subinit.

Install the `init` program of the other init to `/sbin/other-init` and
run it with `chpst -I` to start the subinit

    install /other/init/program /sbin/other-init
    chpst -b /sbin/init -I /sbin/other-init &

You can now add new services to the other subinit.

To login to the other subinit, use the `sshd` service on port 2217

    ssh -luser -p2217 localhost
    su -

You should be able to run "`chpst -b /sbin/init -I /sbin/other-init`"
under any init system as a service, similar to [running Debian's systemd
as subinit under runit](#subsystemd).

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
