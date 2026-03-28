% runit - installation

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - installation

---

*runit* by default installs into the [/package
hierarchy](https://cr.yp.to/slashpackage.html). To install as non-root
user and without the [/package](https://cr.yp.to/slashpackage.html)
directory, make the programs and documentation available manually.

---

[Install into /package](#package)\
[Install programs and documentation manually](#manually)

---

[]{#package}

## Install runit into /package

If you don\'t have a `/package` directory, create it now:

    mkdir -p /package
    chmod 1755 /package

Download [runit-2.3.1.tar.gz](runit-2.3.1.tar.gz) into `/package`
([sha256sum](https://smarden.org/runit/sha256sum.asc)) and unpack the
archive

    cd /package
    gunzip runit-2.3.1.tar
    tar -xpf runit-2.3.1.tar
    rm runit-2.3.1.tar
    cd admin/runit-2.3.1

On MacOSX, do

    echo 'cc -Xlinker -x' >src/conf-ld
    cp src/Makefile src/Makefile.old
    sed -e 's/ -static//' <src/Makefile.old >src/Makefile

Now compile and install the *runit* programs

    package/install

If you want to make the man pages available in the `/usr/local/man/`
hierarchy, do:

    package/install-man

To report success:

    mail pape-runit-2.3.1@xxiv.smarden.org <compile/sysdeps

If you use *runit* regularly, please
[contribute](https://smarden.org/pape/#contribution) to the project.

Refer to [replacing init](replaceinit.html) for replacing *init* with
*runit*, or to [use with traditional init](useinit.html) for running
*runit*\'s service supervision with your system\'s current *init*
scheme.

---

[]{#manually}

## Install runit programs and documentation manually

Download [runit-2.3.1.tar.gz](runit-2.3.1.tar.gz) into the current
directory ([sha256sum](https://smarden.org/runit/sha256sum.asc)) and
unpack the archive

    gunzip runit-2.3.1.tar
    tar -xpf runit-2.3.1.tar
    cd admin/runit-2.3.1

Run `pwd` and note the working directory. When following the
documentation onwards, you need to replace `/package/admin/runit`
accordingly.

On MacOSX, do

    echo 'cc -Xlinker -x' >src/conf-ld
    cp src/Makefile src/Makefile.old
    sed -e 's/ -static//' <src/Makefile.old >src/Makefile

Compile and check the *runit* programs

    package/compile
    package/check

The *runit* programs are available in the `command/` directory. You
probably want to install the `command/runit*` programs into `/sbin`, and
the other programs from `command/` into `/bin`. As non-root user you
probably want to install them into `$HOME/bin`.

The documentation is available in the `doc/` directory, and the man
pages in the `man/` directory.

To report success:

    mail pape-runit-2.3.1@xxiv.smarden.org <compile/sysdeps

If you use *runit* regularly, please
[contribute](https://smarden.org/pape/#contribution) to the project.

Refer to [replacing init](replaceinit.html) for replacing *init* with
*runit*, or to [use with traditional init](useinit.html) for running
*runit*\'s service supervision with your system\'s current *init*
scheme.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
