# Monitor source: Execute a Lua function periodically #

The monitor source is combines the capabilities of a Lua-based driver and the [trigger source](/source-trigger/) . The monitor source periodically executes a Lua function.
 * If the function returns a string, a new message is created which contains the returned string as the ${MESSAGE} part.
 * If the function returns a (non-nested) dict, the key/value pairs of the dict are added as name-value pairs to the newly created message.

**WARNING**: The monitor source works only if you enable Lua support when compiling syslog-ng incubator.

Synopsis:
```
source s_monitor {
    monitor(
        monitor-freq(10) # The source executes the specified Lua script and generates a log message every 'monitor-freq' seconds. Default value (in seconds): 10
        monitor-script("/etc/syslog-ng/monitor.lua") # The Lua script to execute. For a sample script that returns health information about the host, see the /source-monitor/monitor-source-example.lua file
   );
};```

For further examples, see the following blog post: https://talien.blogs.balabit.com/2014/03/monitoring-with-syslog-ng/
