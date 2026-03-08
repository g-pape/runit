% svlogd(8)

# NAME

svlogd - runit\'s service logging daemon

# SYNOPSIS

**svlogd** \[-tttvL\] \[-r *c*\] \[-R *xyz*\] \[-l *len*\] \[-b
*buflen*\] *logs*

# DESCRIPTION

*logs* consists of one or more arguments, each specifying a directory.

**svlogd** continuously reads log data from its standard input,
optionally filters log messages, and writes the data to one or more
automatically rotated *logs*.

Recent log files can automatically be processed by an arbitrary
processor program when they are rotated, and **svlogd** can be told to
alert selected log messages to standard error, and through udp.

**svlogd** runs until it sees end-of-file on standard input or is sent a
TERM signal, see below.

## LOG DIRECTORY

A log directory *log* contains some number of old log files, and the
current log file *current*. Old log files have a file name starting with
*\@* followed by a precise timestamp (see the daemontools\' **tai64n**
program), indicating when *current* was rotated and renamed to this
file.

A log directory additionally contains the lock file *lock*, maybe
*state* and *newstate*, and optionally the file *config*. **svlogd**
creates necessary files if they don\'t exist.

If **svlogd** has trouble opening a log directory, it prints a warning,
and ignores this log directory. If **svlogd** is unable to open all log
directories given at the command line, it exits with an error. This can
happen on start-up or after receiving a HUP signal.

## LOG FILE ROTATION

**svlogd** appends selected log messages to the *current* log file. If
*current* has *size* bytes or more (or there is a new-line within the
last *len* of *size* bytes), or is older than a specified amount of
*time*, *current* is rotated:

**svlogd** closes *current*, changes permission of *current* to 0755,
renames *current* to \@*timestamp*.s, and starts with a new empty
*current*. If **svlogd** sees *num* or more old log files in the log
directory, it removes the oldest one. Note that this doesn\'t decrease
the number of log files if there are already more than *num* log files,
this must be done manually, e.g. for keeping 10 log files:

`ls -1 \@* |sort |sed -ne '10,$p' |xargs rm`

## PROCESSOR

If **svlogd** is told to process recent log files, it saves *current* to
\@*timestamp*.u, feeds \@*timestamp*.u through "sh -c \'*processor*\'"
and writes the output to \@*timestamp*.t. If the *processor* finishes
successfully, \@*timestamp*.t is renamed to \@*timestamp*.s, and
\@*timestamp*.u is deleted; otherwise \@*timestamp*.t is deleted and the
*processor* is started again. **svlogd** also saves any output that the
*processor* writes to file descriptor 5, and makes that output available
on file descriptor 4 when running *processor* on the next log file
rotation.

A *processor* is run in the background. If **svlogd** sees a previously
started *processor* still running when trying to start a new one for the
same *log*, it blocks until the currently running *processor* has
finished successfully. Only the HUP signal works in that situation. Note
that this may block any program feeding its log data to **svlogd.**

## CONFIG

On startup, and after receiving a HUP signal, **svlogd** checks for each
log directory *log* if the configuration file *log/config* exists, and
if so, reads the file line by line and adjusts configuration for *log*
as follows:

If the line is empty, or starts with a "#", it is ignored. A line of the
form

s*size*
:   sets the maximum file size of *current* when **svlogd** should
    rotate the current log file to *size* bytes. Default is 1000000. If
    *size* is zero, **svlogd** doesn\'t rotate log files. You should set
    *size* to at least (2 \* *len*).

n*num*
:   sets the number of old log files **svlogd** should maintain to
    *num*. If **svlogd** sees more that *num* old log files in *log*
    after log file rotation, it deletes the oldest one. Default is 10.
    If *num* is zero, **svlogd** doesn\'t remove old log files.

N*min*
:   sets the minimum number of old log files **svlogd** should maintain
    to *min*. *min* must be less than *num*. If *min* is set, and
    **svlogd** cannot write to *current* because the filesystem is full,
    and it sees more than *min* old log files, it deletes the oldest
    one.

t*timeout*
:   sets the maximum age of the *current* log file when **svlogd**
    should rotate the current log file to *timeout* seconds. If
    *current* is *timeout* seconds old, and is not empty, **svlogd**
    forces log file rotation.

!*processor*
:   tells **svlogd** to feed each recent log file through *processor*
    (see above) on log file rotation. By default log files are not
    processed.

u*a.b.c.d\[:port\]*
:   tells **svlogd** to transmit the first *len* characters of selected
    log messages to the IP address *a.b.c.d*, port number *port*. If
    *port* isn\'t set, the default port for syslog is used (514). *len*
    can be set through the -l option, see below. If **svlogd** has
    trouble sending udp packets, it writes error messages to the log
    directory. Attention: logging through udp is unreliable, and should
    be used in private networks only.

U*a.b.c.d\[:port\]*
:   is the same as the *u* line above, but the log messages are no
    longer written to the log directory, but transmitted through udp
    only. Error messages from **svlogd** concerning sending udp packages
    still go to the log directory.

p*prefix*
:   tells **svlogd** to prefix each line to be written to the log
    directory, to standard error, or through UDP, with *prefix*.

If a line starts with a *-*, *+*, *e*, or *E*, **svlogd** matches the
first *len* characters of each log message against *pattern* and acts
accordingly:

\-*pattern*
:   the log message is deselected.

\+*pattern*
:   the log message is selected.

e*pattern*
:   the log message is selected to be printed to standard error.

E*pattern*
:   the log message is deselected to be printed to standard error.

Initially each line is selected to be written to *log/current*.
Deselected log messages are discarded from *log*. Initially each line is
deselected to be written to standard err. Log messages selected for
standard error are written to standard error.

# PATTERN MATCHING

**svlogd** matches a log message against the string *pattern* as
follows:

*pattern* is applied to the log message one character by one, starting
with the first. A character not a star ("\*") and not a plus ("+")
matches itself. A plus matches the next character in *pattern* in the
log message one or more times. A star before the end of *pattern*
matches any string in the log message that does not include the next
character in *pattern*. A star at the end of *pattern* matches any
string.

Timestamps optionally added by **svlogd** are not considered part of the
log message.

An **svlogd** pattern is not a regular expression. For example consider
a log message like this

`2005-12-18_09:13:50.97618 tcpsvd: info: pid 1977 from 10.4.1.14`

The following pattern doesn\'t match

`-*pid*`

because the first star matches up to the first p in tcpsvd, and then the
match fails because i is not s. To match this log message, you can use a
pattern like this instead

`-*: *: pid *`

# OPTIONS

**-t**
:   timestamp. Prefix each selected line with a precise timestamp (see
    the daemontools\' **tai64n** program) when writing to *log* or to
    standard error.

**-tt**
:   timestamp. Prefix each selected line with a human readable, sortable
    UTC timestamp of the form YYYY-MM-DD_HH:MM:SS.xxxxx when writing to
    *log* or to standard error.

**-ttt**
:   timestamp. Prefix each selected line with a human readable, sortable
    UTC timestamp of the form YYYY-MM-DDTHH:MM:SS.xxxxx when writing to
    *log* or to standard error.

**-r** *c*
:   replace. *c* must be a single character. Replace non-printable
    characters in log messages with *c*. Characters are replaced before
    pattern matching is applied.

**-R** *xyz*
:   replace charset. Additionally to non-printable characters, replace
    all characters found in *xyz* with *c* (default "\_").

**-l** *len*
:   line length. Pattern matching applies to the first *len* characters
    of a log message only. Default is 1000.

**-b** *buflen*
:   buffer size. Set the size of the buffer **svlogd** uses when reading
    from standard input and writing to *logs* to *buflen*. Default
    is 1024. *buflen* must be greater than *len*. For **svlogd**
    instances that process a lot of data in short time, the buffer size
    should be increased to improve performance.

**-L**
:   lossy. If *current* can not be written due to *ENOSPC* and other
    remedies have failed, discard log lines. If the option is given
    twice, **svlogd** prints a warning to standard error whenever log
    data is lost.

**-v**
:   verbose. Print verbose messages to standard error.

# SIGNALS

If **svlogd** is sent a HUP signal, it closes and reopens all *logs*,
and updates their configuration according to *log/config*. If **svlogd**
has trouble opening a log directory, it prints a warning, and discards
this log directory. If **svlogd** is unable to open all log directories
given at the command line, it exits with an error.

If **svlogd** is sent a TERM signal, or if it sees end-of-file on
standard input, it stops reading standard input, processes the data in
the buffer, waits for all *processor* subprocesses to finish if any, and
exits 0 as soon as possible.

If **svlogd** is sent an ALRM signal, it forces log file rotation for
all *logs* with a non empty *current* log file.

# SEE ALSO

sv(8), runsv(8), chpst(8), runit(8), runit-init(8), runsvdir(8),
runsvchdir(8)

https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
