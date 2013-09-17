Visualisation tools for syslog-ng
=================================

One way to visualize the history of log messages that pass through
syslog-ng is to wrap them into a format that gource[0] understands:

    destination d_gource {
       file("/var/log/gource/logs.src"
            template("$UNIXTIME|$HOST@$SOURCE|M|$HOST/$PROGRAM/messages"));
    };

Once this is done, we only need to call gource appropriately, to
produce spectacular results:

    $ gource --log-format custom --auto-skip-seconds 3 \
             --seconds-per-day 1 --file-idle-time 1 \
             --hide files \
             --bloom-intensity 0.25 --bloom-multiplier 0.25 \
             --user-friction 0.25 --highlight-all-users \
             /var/log/gource/logs.src

Of course, one can tweak the options, or even use completely different
ones - the possibilities are endless!

And if one just so happens to have their logs stored in MongoDB, the
logmongource tool in this directory can be used to export the data
into a format suitable for gource. For added amusement, it also colors
each host differently - something which the destination above cannot
do (yet).

The tool assumes that the DATE key in the database is a UNIXTIME, but
other than that, it uses the standard mongodb() destination defaults.

 [0]: https://code.google.com/p/gource/
