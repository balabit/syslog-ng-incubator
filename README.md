syslog-ng module incubator
==========================

The [syslog-ng][sng] module incubator (Incubator henceforth) is a
collection of tools and modules for [syslog-ng][sng] that for one
reason or the other, are not part of the official repository. This
serves both as a staging ground for experimental modules, and as a
repository of plugins that are not aimed at upstream inclusion. It's
also an example of a third party syslog-ng module.

 [sng]: https://github.com/balabit/syslog-ng-3.5

Contents
--------

 * [Riemann destination][sng:riemann]: A simple, work in progress
   destination that allows syslog-ng to send events to the
   [Riemann](http://riemann.io/) network monitoring system.

   [sng:riemann]: https://github.com/algernon/syslog-ng-incubator/tree/master/modules/riemann/

 * [logmongource][sng:mongource]: A log visualisation tool that
   extracts messages from a MongoDB collection, and visualises them
   with [Gource](https://code.google.com/p/gource/)

   [sng:mongource]: https://github.com/algernon/syslog-ng-incubator/tree/master/tools/visualize/

Installation
------------

Installing the modules and tools follows the usual autotools way:

    $ git clone git://github.com/algernon/syslog-ng-incubator.git
    $ cd syslog-ng-incubator
    $ autoreconf -i
    $ ./configure && make && make install

Of course, one will need all the dependencies ([syslog-ng][sng],
[riemann-c-client][lrc], and [libmongo-client][lmc]; of which the
latter two are optional) installed too.

 [lrc]: https://github.com/algernon/riemann-c-client
 [lmc]: https://github.com/algernon/libmongo-client

License
-------

Copyright (C) 2011-2013 BalaBit IT Security Ltd. and Gergely Nagy
<algernon@balabit.hu>, released under the terms of the
[GNU General Public License][gpl], version 2 (or later).

 [gpl]: http://www.gnu.org/licenses/gpl-2.0.html
