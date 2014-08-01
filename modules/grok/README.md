grok parser
===========

grok parser can parse log messages using grok patterns.

Example config:

```
parser p_grok {
  grok( grok_pattern_directory("/etc/syslog-ng/grok.d") grok_pattern("pattern1") grok_pattern("pattern2") );
};
```

How to compile: you should have libgrok (https://github.com/jordansissel/grok) installed on your system.
--with-libgrok in not implemented yet, but I should implement it or include libgrok as a git submodule.

It's not a filter like in logstash, instead a syslog-ng parser (eg. it does not drop the message if it does not match)

The parsed subpatterns are available as message fields.
