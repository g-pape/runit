% runit - service dependencies

[G. Pape](https://smarden.org/pape/)\
[runit](index.html)

---

# runit - service dependencies

---

*runit*'s service supervision resolves dependencies automatically for
many or even most service daemons, those that exit if another service
they need to function properly is not or no longer available. The
[runsv](https://smarden.org/runit/runsv.8.html) supervisor will then
start them again, until everything is in shape.

---

For service daemons with different behavior, or to arrange services more
structurally, the [sv](https://smarden.org/runit/sv.8.html) program can
help. A few examples:

For the service daemon *floyd*, that needs the *pinkd* service up and
running, add `sv start pinkd` to *floyd*'s `run` script, right after
`set -e`.

     #!/bin/sh
     set -e
     sv start pinkd
     exec floyd

When the *pinkd* service daemon crashes or is restarted for some other
reason and the *floyd* daemon is unable to handle this, add
`sv hup floyd` (or whatever the *floyd* daemon understands) to *pinkd*'s
`finish` script to force its reload or restart.

     #!/bin/sh
     sv hup floyd

If the *pinkd* service shall be stopped when the *floyd* service is
stopped gracefully, add `test "$1" != 0 || sv down pinkd` to *floyd*'s
`finish` script.

     #!/bin/sh
     test "$1" != 0 || sv down pinkd

Or vice versa.

Explore the [sv](https://smarden.org/runit/sv.8.html) program for more
options.

---

For service daemons that do not make their service available immediately
after starting, a `check` script can be added. The
[sv](https://smarden.org/runit/sv.8.html) program will use this `check`
script to better evaluate whether a service is fully started and
available, for example:

For a mail service listening on localhost:25, this `check` script could
be used

     #!/bin/sh
     exec nc -z localhost 25

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
