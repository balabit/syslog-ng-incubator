grok parser
===========

grok parser can parse log messages using grok patterns.

Example config:

```
parser p_grok {
  grok( 
       pattern_directory("/etc/syslog-ng/grok.d") 
       match("%{STRING:field}" tags("matched" "string") ) 
       match("%{NUMBER:field}" tags("matched" "number") )
  );
};
```

How to compile: you should have libgrok (https://github.com/jordansissel/grok) installed on your system.
--with-libgrok in not implemented yet, but I should implement it or include libgrok as a git submodule.

You can alternatively compile libgrok from my repository: https://github.com/talien/grok. It has an autotools based Makefile, so can configure it like this:
autoreconf -i && ./configure && make && make install. It also installs a .pc file for pkg-config.

It's not a filter like in logstash, instead a syslog-ng parser (eg. it does not drop the message if it does not match)

The parsed subpatterns are available as message fields.
