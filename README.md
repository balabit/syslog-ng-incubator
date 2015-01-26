syslog-ng module incubator
==========================

The [syslog-ng][sng] module incubator (Incubator henceforth) is a
collection of tools and modules for [syslog-ng][sng] that for one
reason or the other, are not part of the official repository. This
serves both as a staging ground for experimental modules, and as a
repository of plugins that are not aimed at upstream inclusion. It's
also an example of a third party syslog-ng module.

 [sng]: https://github.com/balabit/syslog-ng

**NOTE**: The Incubator requires syslog-ng 3.6.1 or newer.
Contents
--------

 * [Lua destination][sng:lua]: This destination is really just a
   wrapper, that allows one to write destination drivers in Lua, with
   some limitations.

   [sng:lua]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/lua/

 * [Perl destination][sng:perl]: This destination allows one to write
   destination plugins in Perl.

   [sng:perl]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/perl/

 * [Python destination][sng:python]: This destination allows one to write
   destination plugins in Python.

   [sng:python]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/python/

 * [Trigger source][sng:trigger]: A very simple example source that
   periodically generates a message. Useful mostly for debugging
   purposes.

   [sng:trigger]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/trigger-source/

 * [Monitor source][sng:monitor]: A module similar to the trigger
   source, except it dispatches to a Lua function to generate the
   message.

   [sng:monitor]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/monitor-source/

 * [Extra template functions][sng:bf+]: Extra template functions, such
   as `$(//)` which is floating-point division, as opposed to the
   built-in `$(/)` (integer division).
 
   Functions:
    * // : floating point division
    * or : returns first non-empty string
    * state : gets or sets global state from template function.
    * padding : pads a string to a specified width with specified chars.

   [sng:bf+]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/basicfuncs-plus/

 * [$(getent) template function][sng:tfgetent]: A template function to
   look up users (by uid or name), groups or services, and retrieve
   some of the found properties.

   [sng:tfgetent]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/getent/

 * [RSS destination][sng:rss]: A very simple destination module that
   allows one to offer log messages as an RSS feed.

   [sng:rss]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/rss/

 * [logmongource][sng:mongource]: A log visualisation tool that
   extracts messages from a MongoDB collection, and visualises them
   with [Gource](https://code.google.com/p/gource/).

   [sng:mongource]: https://github.com/balabit/syslog-ng-incubator/tree/master/tools/visualize/

 * [Kafka destination][sng:kafka]: A simple, work in progress
   destination that allows syslog-ng to send events to the
   [Apache Kafka](http://kafka.apache.org/) distributed queue.

   [sng:kafka]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/kafka/

 * [Grok parser][sng:grok]: Grok is an advanced pattern format (like PatternDB) used primarily by LogStash, 
   which allows users to parse unstructured data into a structured format. This module 
   allows syslog-ng users to use Grok patterns, too.

   [Grok](https://github.com/jordansissel/grok/) C parser for grok.

   [sng:grok]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/grok/

 * [0MQ source/destination][sng:zmq]: ZeroMQ is a simple, high-speed messaging protocol.
   These drivers allows syslog-ng to send/receive logs from ZeroMQ message brokers.

   [0MQ](http://zeromq.org/) message protocol.

   [sng:zmq]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/zmq/

 * [Java destination][sng:java]: Java destination drivers enables the users to write destinations
   in Java or in any JVM compatible language.
  
   [sng:java]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/java/

 * [Date parser][sng:date]: A parser for arbitrary date formats which
   may be contained in non-syslog messages.
  
   [sng:date]: https://github.com/balabit/syslog-ng-incubator/tree/master/modules/date/

Installation
------------

Installing the modules and tools follows the usual autotools way:

    $ git clone git://github.com/balabit/syslog-ng-incubator.git
    $ cd syslog-ng-incubator
    $ autoreconf -i
    $ ./configure && make && make install

Of course, one will need all the dependencies ([syslog-ng][sng],
bison, flex, [riemann-c-client][lrc], [libmongo-client][lmc],
[lua][lua], [perl][perl], [python][python], [rdkafka][kafka];
of which the latter six are optional) installed too.

 [lrc]: https://github.com/algernon/riemann-c-client
 [lmc]: https://github.com/algernon/libmongo-client
 [lua]: http://www.lua.org/
 [perl]: http://www.perl.org/
 [python]: http://www.python.org/
 [kafka]: https://github.com/edenhill/librdkafka
 
 
Docker container
----------------

An automatically built docker container is available at https://registry.hub.docker.com/u/talien/syslog-ng-incubator/
from the latest development snapshot.

License
-------

Copyright (C) 2011-2014 BalaBit IT Security Ltd., Gergely Nagy
<algernon@balabit.hu>, Viktor Tusa <tusa@balabit.hu>,
Viktor Juhasz <viktor.juhasz@balabit.com>, and other
contributors; released under the terms of the
[GNU General Public License][gpl], version 2 (or later).

 [gpl]: http://www.gnu.org/licenses/gpl-2.0.html
