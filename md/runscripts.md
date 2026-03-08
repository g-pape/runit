% runit - collection of run scripts

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)\
[How to replace init](replaceinit.html)

---

# runit - collection of run scripts

---

This is a quite aged collection of `run` scripts for services to have
them supervised by the [daemontools](https://cr.yp.to/daemontools.html)
or [runit](https://smarden.org/runit/) package. It hasn't changed since
2007 with regards to content. These days search engines provide better
results than browsing this collection.

Thanks go to the following people for contributing run scripts:
[Alessandro Bono, Robin S. Socha, Claus Alboege, Paul Jarc, clemens
fischer, Jesse Cablek, Lukas Beeler, Thomas Baden, Ralf Hildebrandt,
Antonio Dias, Erich Schubert, Lang Martin, Tomasz Nidecki, Marek
Bartnikowski, Thomas Schwinge, Gael Canal, Woon Wai Keen, Richard A
Downing, Phil Genera, Stefan Karrmann, Kevin Berry, Karl Chen, Sascha
Huedepohl, Jason Smith, Kevin, marlowe, ed neville, xavier
dutoit.]{.small}

---

[atftpd](#atftpd)\
[apache](#apache)\
[apache2](#apache2)\
[atd](#atd)\
[bind9](#bind9)\
[boa](#boa)\
[cfengine](#cfengine)\
[cfsd](#cfsd)\
[chrony](#chrony)\
[clamav](#clamav)\
[clamsmtpd](#clamsmtpd)\
[courier-imap](#courier-imap)\
[cron](#cron)\
[cups](#cups)\
[dhclient](#dhclient)\
[dhcpcd](#dhcpcd)\
[dhcpd](#dhcpd)\
[dictd](#dictd)\
[dropbear](#dropbear)\
[exim](#exim)\
[expireproctitle](#expireproctitle)\
[fam](#fam)\
[fcron](#fcron)\
[fetchmail](#fetchmail)\
[gdm](#gdm)\
[getty](#getty)\
[gpm](#gpm)\
[hotwayd](#hotwayd)\
[identd](#identd)\
[inn](#inn)\
[jabberd](#jabberd)\
[junkbuster](#junkbuster)\
[kdm](#kdm)\
[keepalived](#keepalived)\
[klogd](#klogd)\
[leafnode](#leafnode)\
[logging Linux kernel messages](#loglinuxkernel)\
[mdadm](#mdadm)\
[minidentd](#minidentd)\
[mpd](#mpd)\
[nscd](#nscd)\
[ntpd](#ntpd)\
[nullidentd](#nullidentd)\
[polipo](#polipo)\
[pop3-ssl](#pop3-ssl)\
[pop3vscan](#pop3vscan)\
[portmap](#portmap)\
[postfix](#postfix)\
[postgresql](#postgresql)\
[powernowd](#powernowd)\
[ppp](#ppp)\
[privoxy](#privoxy)\
[proftpd](#proftpd)\
[pure-ftpd](#pure-ftpd)\
[radiusd](#radiusd)\
[rmrtg](#rmrtg)\
[rsyncd](#rsyncd)\
[samba (smbd, nmbd)](#samba)\
[shoutcast](#shoutcast)\
[spamd](#spamd)\
[squid](#squid)\
[sshd](#sshd)\
[sshd under tcpserver](#sshdtcpserver)\
[statd](#statd), [mountd](#mountd)\
[stunnel](#stunnel)\
[subversion](#subversion)\
[swat](#swat)\
[syslogd](#syslogd)\
[taiclockd](#taiclockd)\
[tmda-ofmipd](#tmda-ofmipd)\
[tomcat](#tomcat)\
[tor](#tor)\
[vsftpd](#vsftpd)\
[wvdial](#wvdial)\
[xdm](#xdm)\
[xfs](#xfs)

---

### [An `atftpd` run script]{#atftpd}

(*Debian*)

     #!/bin/sh
     exec in.tftpd --daemon --no-fork --tftpd-timeout 30 \
       --retry-timeout 5 --no-tsize --no-blksize --no-multicast \
       --maxthread 1000 --verbose=7 /var/atfpd 2>&1

---

### [An `apache` run script]{#apache}

(*SunOS*, Apache 1.\*, patched to run under supervise)

     #!/bin/sh
     exec 2>&1
     exec env -i \
     /pack/apache/1.3.27-1/sbin/httpd -F 2>&1

(*Debian woody*)

     #!/bin/sh
     exec 1>&2
     exec apache-ssl -F

---

### [An `apache2` run script]{#apache2}

(*SunOS*)

     #!/bin/sh
     exec 2>&1
     exec env -i /command/pgrphack \
     /pack/apache/2.0.43-1/sbin/httpd -DFOREGROUND 2>&1

(*HP-UX/GNU*, *Linux*)

     #!/bin/sh
     TZ=MET-1METDST
     export TZ
     exec /usr/local/apache2/bin/httpd -DNO_DETACH

---

An

atd run script

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec atd -d

---

### [A `bind9` run script]{#bind9}

(*Linux*)

     #!/bin/sh
     MEM="`head -1 ./env/MEM`"
     CHROOT="`head -1 ./env/CHROOT`"
     exec softlimit -m "${MEM}" \
       named -u bind -t "${CHROOT}" -g 2>&1

(*HP-UX/GNU*, *Linux*)

     #!/bin/sh
     exec named -f -t /var/spool/named/ -u named

---

### [A `boa` run script]{#boa}

(*SunOS*)

     #!/bin/sh
     exec 2>&1
     ulimit -n 1024
     exec /pack/boa/current/boa -c /pack/boa/current -d 2>&1

---

### [A `cfengine` run script]{#cfengine}

(*SunOS*)

     #!/bin/sh
     /pack/cfengine/sbin/cfagent --file /pack/cfengine/etc/cfagent.conf \
       -L -v -q exec sleep 3600

---

### [A `cfsd` run script]{#cfsd}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     
     set -e
     
     PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
     DAEMON=cfsd
     CONFIG_FILE=/etc/cfs.conf
     
     # source config file
     test ! -r "$CONFIG_FILE" || . "$CONFIG_FILE"
     
     test -n "$CRYPT_ROOT" || exit 0
     test -n "$NULL_EXPORT" || exit 0
     test -n "$CFS_MOUNT" || exit 0
     test -n "$CFS_UMOUNT" || exit 0
     
     sv start portmap || exit 1
     
     exec env \
        NODAEMON=1 CFS_MOUNT="$CFS_MOUNT" CFS_UMOUNT="$CFS_UMOUNT" $DAEMON

---

### [A `chrony` run script]{#chrony}

(*LFS*)

     #!/bin/sh
     exec 2>&1
     exec chronyd -d -r -s

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `clamd` run script]{#clamav}

(*RedHat 7*)

     #!/bin/sh
     exec 2>&1
     exec setuidgid clamav softlimit -a 40000000 clamd

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `clamsmtpd` run script]{#clamsmtpd}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec chpst -uclamav clamsmtpd -d 3

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `courier-imap` run script]{#courier-imap}

(*SunOS*)

     #!/bin/sh
     exec 2>&1
     PREFIX="/usr/lib/courier-imap"
     exec envdir ./env \
       tcpserver -v -R 0 143 \
       $PREFIX/sbin/imaplogin \
       $PREFIX/libexec/authlib/authshadow \
       $PREFIX/libexec/authlib/authvchkpw \
       $PREFIX/bin/imapd Maildir

---

### [A `cron` run script]{#cron}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec cron -f -l

---

### [A `cups` run script]{#cups}

(*Debian woody*)

     #!/bin/sh
     exec 2>&1
     exec cupsd -f

---

### [A `dhclient` run script]{#dhclient}

(*Debian woody*)

     #!/bin/sh
     exec dhclient -e -d -cf ./config -lf ./leases -pf ./pid eth1

---

### [A `dhcpcd` run script]{#dhcpcd}

(*Linux*, DHCP Client Daemon v.scriptconfig-0.1)

     #!/bin/sh
     exec dhcpcd -a -d -D -H eth0

---

### [A `dhcpd` run script]{#dhcpd}

(*Debian woody*)

     #!/bin/sh
     exec 2>&1
     exec dhcpd-2.2.x -f -d -cf ./config eth0

This service needs a [log service](faq.html#createlog) to be set up.

(*Solaris 8*, uses additional dsvclockd service, would get fired off
automatically, but then it\'s not supervised)

     #!/bin/sh
     exec 2>&1
     exec envdir ./env /usr/lib/inet/dsvclockd -f

     #!/bin/sh
     if svok /service/dsvclockd; then
       sleep 2 # wait to make sure dsvclockd is initialized
       exec 2>&1
       exec envdir ./env /usr/lib/inet/in.dhcpd -b manual -d # -v
     fi
     echo dsvclockd is not running - aborting
     exec /usr/bin/sleep 5

The in.dhcpd service needs a [log service](faq.html#createlog) to be set
up.

---

### [A `dictd` run script]{#dictd}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec dictd -d nodetach

---

### [A `dropbear` run script]{#dropbear}

(*Linux*)

     #!/bin/sh
     exec 2>&1
     exec dropbear -F -E -p 22

This service needs a [log service](faq.html#createlog) to be set up.

(*Linux*, running under
[tcpsvd](https://smarden.org/ipsvd/tcpsvd.8.html))

     #!/bin/sh
     exec 2>&1
     exec tcpsvd -v -i./peers 0 22 dropbear -i -E

This service needs a [log service](faq.html#createlog) to be set up.

---

### [An `exim` run script]{#exim}

(*Linux*, *FreeBSD*)

     #!/bin/sh
     exec /usr/local/sbin/exim -bdf -q30m

---

### [`expireproctitle` run script examples]{#expireproctitle}

(*Linux*)

     #!/bin/sh
     echo -n .
     exec sleep 300

---

### [A `fam` run script]{#fam}

(*Slackware Linux 9.0*)

     #!/bin/sh
     exec 2>&1
     exec fam -L -f -v

This service needs a [log service](faq.html#createlog) to be set up.

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     sv start /service/portmap || exit 1
     exec famd -T 0 -f

---

### [A postfix `master` run script]{#postfix}

(*Debian etch*)

     #!/bin/sh
     exec 1>&2
     
     daemon_directory=/usr/lib/postfix \
     command_directory=/usr/sbin \
     config_directory=/etc/postfix \
     queue_directory=/var/spool/postfix \
     mail_owner=postfix \
     setgid_group=postdrop \
       /etc/postfix/postfix-script check || exit 1
     
     exec /usr/lib/postfix/master

---

### [A `fcron` run script]{#fcron}

(*LFS*)

     #!/bin/sh
     exec 2>&1
     exec fcron -f -y

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `fetchmail` run script]{#fetchmail}

(*Linux*)

     #!/bin/sh
     INTERVAL=551
     exec 2>&1
     echo "*** Starting fetchmail service..."
     exec env FETCHMAILHOME="./pid" \
       chpst -u fetchmail fetchmail -v \
         -f ./fetchmail.conf \
         --nodetach \
         --daemon ${INTERVAL}

---

### [A `gdm` run script]{#gdm}

(*Debian woody*)

     #!/bin/sh
     exec gdm -nodaemon

---

### [A `mingetty` run script]{#getty}

(*Debian*)

     #!/bin/sh
     exec mingetty tty5

The [runit](index.html) package does not care about utmp records for
getties. You should choose a getty that handles its own utmp and wtmp
records. Debian\'s `mingetty` creates its own utmp record.

### An `agetty` run script

(*Slackware Linux 9.0*)

     #!/bin/sh
     exec agetty 38400 tty1 linux

### A `fgetty` run script

(*Linux*)

     #!/bin/sh
     exec chpst -P fgetty tty4

The [runsv](runsv.8.html) program does not automatically create a new
session and separate process group for run scripts, which can cause some
*getties* to fail due to limited permissions. The [chpst](chpst.8.html)
program can be used to alter the process state for those *getties*.

---

### [A `gpm` run script]{#gpm}

(*LFS*)

     #!/bin/sh
     exec 2>&1
     . /etc/sysconfig/mouse
     exec gpm -D -m $MDEVICE -t $PROTOCOL

---

### [A `hotwayd` run script]{#hotwayd}

(*LFS*)

     #!/bin/sh
     exec 2>&1
     exec tcpsvd -l0 -u nobody 127.0.0.1 110 hotwayd

---

### [An `oidentd` run script]{#identd}

(*Linux*, oidentd version 2.0.6)

     #!/bin/sh
     exec 2>&1
     exec oidentd -i -S -t 10 -u daemon -g daemon

This service needs a [log service](faq.html#createlog) to be set up.

(*Debian*)

     #!/bin/sh
     exec /usr/local/sbin/oidentd --nosyslog -i -u ident -g ident -l 15 -m \
       -C /etc/oidentd/oidentd.conf 2>&1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [An `innd` run script]{#inn}

(*Linux*)

     #!/bin/sh
     MEM="`head -1 ./env/MEM`"
     exec softlimit -m "${MEM}" \
       setuidgid news \
       /usr/sw/bin/news/inndstart -f -r

---

### [`jabberd` run scripts]{#jabberd}

(*RedHat 7*)

     #!/bin/sh
     exec 2>&1
     if [ -f /usr/local/jabber/jabber.pid ]; then
       rm /usr/local/jabber/jabber.pid
     fi
     exec setuidgid jabberd \
       /usr/local/jabber/jabberd/jabberd -D -c /etc/jabber.xml

This service needs a [log service](faq.html#createlog) to be set up.

     #!/bin/sh
     exec 2>&1
     sleep 5 # so that jit starts after jabberd is up
     exec setuidgid jabberd \
       /usr/local/jabber/jit/jabberd/jabberd -c /etc/jit.xml

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `junkbuster` run script]{#junkbuster}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     DAEMON=junkbuster
     exec "$DAEMON" /etc/junkbuster/config

---

### [A `kdm` run script]{#kdm}

(*Debian sid*, *FreeBSD 5.3-REL*)

     #!/bin/sh
     sv start /service/getty-* || exit 1
     exec kdm -nodaemon

---

### [A `keepalived` run script]{#keepalived}

(*Linux*)

     #!/bin/sh
     exec 2>&1
     exec keepalived -n

---

### [A `klogd` run script]{#klogd}

(*Debian woody*)

     #!/bin/sh
     exec klogd -n

---

### [A `leafnode` run script]{#leafnode}

(*Linux*)

     #!/bin/sh
     exec 2>&1
     exec envuidgid news \
       tcpserver -v -x rules.cdb -c 10 -U 0 119 \
       leafnode

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A run script for `logging Linux kernel messages with multilog`]{#loglinuxkernel}

(*Linux*)

     #!/bin/sh -e
     exec < /proc/kmsg \
     setuidgid loguser \
     multilog t n64 ./main

---

### [A `mdadm` run script]{#mdadm}

(*Debian sarge*)

     #!/bin/sh
     DEBIANCONFIG=/etc/default/mdadm
     MAIL_TO=root
     test -f $DEBIANCONFIG && . $DEBIANCONFIG
     exec mdadm --monitor --scan --mail $MAIL_TO

---

### [A `minidentd` run script]{#minidentd}

(*Linux*)

     #!/bin/sh
     exec 2>&1
     exec envuidgid nobody \
       tcpserver -vUR 0 113 \
       timeoutafter 60 \
       minidentd -v

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `mpd` run script]{#mpd}

(*Debian sid*, *FreeBSD 5.3-REL*)

     #!/bin/sh
     MPDCONF=/etc/mpd.conf
     exec mpd --stdout --no-daemon $MPDCONF

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `nscd` run script]{#nscd}

(*Debian sarge*)

     #!/bin/sh
     secure=""
     for table in passwd group
       do
         if egrep '^'$table':.*nisplus' /etc/nsswitch.conf >/dev/null
           then
             nscd_nischeck $table || secure="$secure -S $table,yes"
         fi
     done
     exec nscd -d -- $secure

---

### [A `ntpd` run script]{#ntpd}

(*Linux*)

     #!/bin/sh
     MEM=`head -1 ./env/MEM`
     exec softlimit -m "${MEM}" \
       ntpd -n

(*OpenNTPD on OpenBSD/Linux*)

     #!/bin/sh
     exec /usr/sbin/ntpd -s -d 2>&1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `nullidentd` run script]{#nullidentd}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec tcpsvd -u nobody -x nullidentd-cdb -t 60 0 113 nullidentd

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `polipo` run script]{#polipo}

(*Linux*)

     #!/bin/sh
     exec 2>&1
     exec setuidgid polipo \
       polipo -c config forbiddenFile="`pwd`"/forbidden diskCacheRoot="`pwd`"/cache

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `pop3-ssl` run script]{#pop3-ssl}

(*BSD*)

     #!/bin/sh
     exec tcpserver -R -v -c 50 0 995 /usr/local/sbin/stunnel
     -f  -p /etc/ssl/stunnel.pem \
     -l /var/qmail/bin/qmail-popup -- /var/qmail/bin/qmail-popup
     "`cat /var/qmail/control/me`" vchkpw /var/qmail/bin/qmail-pop3d Maildir 2>&1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `pop3vscan` run script]{#pop3vscan}

(*RedHat 7*)

     #!/bin/sh
     exec 2>&1
     exec setuidgid pop3vscan pop3vscan -d

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `portmap` run script]{#portmap}

(*LFS*, *Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec portmap -d

---

### [A `postgresql` run script]{#postgresql}

(*Debian*, *SunOS*)

     #!/bin/sh
     exec setuidgid postgres /usr/lib/postgresql/bin/postmaster \
       -D /var/lib/postgres/data 2>&1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `powernowd` run script]{#powernowd}

(*Debian*)

     #!/bin/sh
     set -e  # barf if modprobe fails
     modprobe cpufreq-userspace
     test ! -f /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ||
       exec /usr/sbin/powernowd -d
     
     echo "required sysfs objects not found!"
     echo "Read /usr/share/doc/powernowd/README.Debian for more information."
     sv down "$(pwd)"

---

### [A `ppp` run script]{#ppp}

(*FreeBSD*)

     #!/bin/sh
     exec 2>&1
     echo "ppp -ddial dsl "
     exec ppp -unit0 -foreground dsl

### A `pppd` run script

(*Linux*)

     #!/bin/sh
     exec 2>&1
     exec chpst -P pppd call isp nodetach

---

### [A `privoxy` run script]{#privoxy}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec chpst -u privoxy:nogroup privoxy --no-daemon /etc/privoxy/config

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `proftpd` run script]{#proftpd}

(*Linux*, ProFTPD Version 1.2.8rc1, configure it to use \'ErrorLog
\"/dev/stdout\"\')

     #!/bin/sh
     exec 2>&1
     exec proftpd -n -d 1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `pure-ftpd` run script]{#pure-ftpd}

(*Debian woody*)

     #!/bin/sh
     exec 2>&1
     exec pure-ftpd -a 50 -E -l pam -u 100

---

### [A `radiusd` run script]{#radiusd}

(*Slackware Linux 9.0*, cistron radius 1.6.1)

     #!/bin/sh
     exec 2>&1
     exec radiusd -f -y -z -lstdout

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `rmrtg` run script]{#rmrtg}

()

     #!/bin/sh
     MRTGUID=`id -u mrtg`
     MRTGGID=`id -g mrtg`
     exec 2>&1
     exec tcpserver -l 0 -R -H -q \
       -u"${MRTGUID}" -g"${MRTGGID}" 0 5660 /home/mrtg/ext/rmrtg

---

### [A `rsyncd` run script]{#rsyncd}

(*SunOS*)

     #!/bin/sh
     exec 2>&1
     exec softlimit -d 100000000 tcpserver \
       -x /pack/rsync/rsync.service/tcp.rsync.cdb -v -c 100 -U -H -l 0 -R \
       1.2.3.4 873 nice -2 /pack/rsync/bin/rsync --daemon --no-detach \
       --config /pack/rsync/etc/rsyncd.conf

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `smbd` run script]{#samba}

(*Linux*)

     #!/bin/sh
     PATH="/usr/local/samba/bin"
     exec 2>&1
     exec smbd -F -S -d3

This service needs a [log service](faq.html#createlog) to be set up.

### A `nmbd` run script

(*Linux*)

     #!/bin/sh
     PATH="/usr/local/samba/bin"
     exec 2>&1
     exec nmbd -F -S -d1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `shoutcast` run script]{#shoutcast}

(*RedHat 7*)

     #!/bin/sh
     exec 2>&1
     exec setuidgid sc_serv \
       /usr/local/sc_serv/bin/sc_serv /etc/sc_serv.conf

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `spamd` run script]{#spamd}

(*Debian woody*)

     #!/bin/sh
     exec spamd -m 20 -a -H -s stderr 2>&1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `squid` run script]{#squid}

(*Debian woody*)

     #!/bin/sh
     exec squid -f ./config -sN

---

### [A `sshd` run script]{#sshd}

(*Debian*)

     #!/bin/sh
     exec 2>&1
     exec /usr/sbin/sshd -D -e

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `sshd (under tcpserver)` run script]{#sshdtcpserver}

(*Linux*, sshd version OpenSSH_3.4p1)

     #!/bin/sh -e
     exec 2>&1
     exec \
     tcpserver -1vpdl0 -Xxtcp.cdb 0 ssh \
     sshd -ief config

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `statd` run script]{#statd}

(*Debian woody*)

     #!/bin/sh
     exec 2>&1
     NEED_LOCKD=yes
     if test -f /proc/ksyms; then
       # We need to be conservative and run lockd,
       # unless we can prove that it isn't required.
       grep -q lockdctl /proc/ksyms || NEED_LOCKD=no
     fi
     if [ "$NEED_LOCKD" = yes ]; then
       rpc.lockd
     fi
     exec rpc.statd -F -d

This service needs a [log service](faq.html#createlog) to be set up.

### [A `mountd` run script]{#mountd}

(*Debian*)

     #!/bin/sh
     sv start portmap statd || exit 1
     RPCNFSDCOUNT=8  # Number of servers to be started up by default
     RPCMOUNTDOPTS=
     
     trap '/usr/bin/killall -2 nfsd' 0
     trap 'exit 2' 1 2 3 15
     
     exportfs -r
     rpc.nfsd -- $RPCNFSDCOUNT
     rpcinfo -u localhost nfs 3 >/dev/null 2>&1 ||
       RPCMOUNTDOPTS="$RPCMOUNTDOPTS --no-nfs-version 3"
     exec rpc.mountd -F $RPCMOUNTDOPTS

---

### [A `stunnel` run script]{#stunnel}

(*BSD*)

     #!/bin/sh
     exec /usr/local/sbin/stunnel -f -d 1234 -r 25 -v3 -a /etc/ssl/mailcerts 2>&1

---

### [A `svnserve` run script]{#subversion}

(*Debian*)

     #!/bin/sh
     exec 2>&1
     exec /usr/bin/svnserve -d --foreground

---

### [A `swat` run script]{#swat}

     #!/bin/sh
     exec 2>&1
     exec tcpsvd -l0 127.0.0.1 901 swat

---

### [A `syslogd` run script]{#syslogd}

(*Debian sarge*)

     #!/bin/sh
     exec syslogd -n

---

### [A `taiclockd` run script]{#taiclockd}

     #!/bin/sh
     exec 2>&1
     exec setuidgid taiclock /usr/local/clockspeed/bin/taiclockd

---

### [A `tmda-ofmipd` run script]{#tmda-ofmipd}

(*RedHat 7*)

     #!/bin/sh
     exec /usr/local/tmda/bin/tmda-ofmipd -f -d -R pop3 -p my.host.name:8025 2>&1

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `tomcat` run script]{#tomcat}

(*SunOS*)

     #!/bin/sh
     exec 2>&1
     exec \
     setuidgid apache \
     /pack/tomcat/current/bin/catalina.sh run

---

### [A `tor` run script]{#tor}

(*Debian sarge*)

     #!/bin/sh
     exec 2>&1
     exec chpst -P -u debian-tor:debian-tor tor

---

### [A `vsftpd` run script]{#vsftpd}

(*RedHat 7*)

     #!/bin/sh
     exec 2>&1
     exec tcpserver -c30 -Xv -llocalhost \
       -x/etc/tcp.ftp.cdb -uvsftpd -gvsftpd 0 ftp \
         softlimit -d300000 /var/vsftpd/bin/vsftpd

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `wvdial` run script]{#wvdial}

(*Slackware Linux 9.0*, the service should have a `down` file, use
`svc -o /service/wvdial` to bring up a ppp connection)

     #!/bin/sh
     exec 2>&1
     exec wvdial ISP

This service needs a [log service](faq.html#createlog) to be set up.

---

### [A `xdm` run script]{#xdm}

(*Linux*)

     #!/bin/sh
     sv start /service/vc-* || exit 1
     exec xdm -nodaemon

---

### [A `xfs` run script]{#xfs}

(*Debian sarge*)

     #!/bin/sh
     SOCKET_DIR=/tmp/.font-unix
     mkdir -p $SOCKET_DIR
     chown 0:0 $SOCKET_DIR
     chmod 1777 $SOCKET_DIR
     exec /usr/bin/X11/xfs -nodaemon

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
