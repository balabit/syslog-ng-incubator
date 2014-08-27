# Generating periodic log messages with the trigger source #

The 'trigger' source is a very simple plugin that periodically generates a message. You can use this plugin:
 * to generate messages on a timer to aid debugging, or
 * as a source plugin example to get to know the syslog-ng code base.

You can find the source code of this module in the `modules/trigger-source` directory of [syslog-ng-incubator](https://github.com/balabit/syslog-ng-incubator).

Synopsis:
```
source s_trigger {
    trigger(
        trigger-freq(10) # The source generates a log message every 'trigger-freq' seconds. Default value (in seconds): 10
        trigger-message("This is the text of the generated message") # Default value: "Trigger source is trigger happy."
        tags("add-a-tag-if-you-need")
        program-override("trigger") # Specifies the ${PROGRAM} field of the generated log message
        host-override("trigger-happy") # Specifies the ${HOST} field of the generated log message
   );
};```
