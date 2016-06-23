

# Websocket source and destination

## Introduction
 The Websocket destination can send log messages to an WebSocket Server directly. In addition, this destination can also act as a WebSocket server, so the WebSocket client (such as a javascript client in a browser) could subscribe directly to it to get log messages. These will make sending log message privately or publicly much easier.
A WebSocket source is also included.  It can receive log messages directly from WebSocket clients.


## Installation
If you want to compile syslog-ng-incubator with the websocket source and destination, you must have libwebsockets installed in your system.

Please go to libwebsockets [official website](https://libwebsockets.org/) to download v2.0-stable and install it.


## Test helper scripts
There is some scripts which can help you test the module easier. Please refer to [README](test/README.md)


## Configuration
Here is an example of an destination example which act as a client
```
@version: 3.7
@include "scl.conf"

source      s_system { system(); internal();};

destination ws_client_des {
    websocket(
        mode("client")
        protocol("example-protocol")
        address("127.0.0.1")
        path("/")
        port(8000)
        template("$HOST $ISODATE $MSG")
        ssl(
            allow_self_signed
            cert("/root/tmp/keys/server.crt")
            key("/root/tmp/keys/server.key")
        )
    );
};

log { source(s_system); destination(ws_client_des); };
```


## Options explanation
This section will explain the options

`mode`:              The mode of the destination. Only server mode and client mode are supported
`protocol`:          websocket protocol
`address`:           websocket server listening address
`port`:              websocket server listening port
`path`:              the path of the websocket server service
`ssl`:               If this config appears, ssl will be used.
`allow_self_signed`: If you want to allow self-signed certificate when ssl is enabled, plese use this configue.
`cert`:              When it act as an websocket client it is optional. If the server wants us to present a valid SSL client, certificate, you can the filepath of it
`key`:               the private key for the cert
`cacert`             (optional) When it act as an client, it is a CA cert and CRL can be used to validate the cert send by the server.



## The websocket apis and examples
I write a api wrapper for libwebsockets, so using libwebsockets will be much easier.

More detailed description can be found in the [README](ws_api/README.md).
