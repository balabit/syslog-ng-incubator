

# Websocket source and destination

## Introduction
 The Websocket destination can send log messages to an WebSocket Server directly. In addition, this destination can also act as a WebSocket server, so the WebSocket client (such as a javascript client in a browser) could subscribe directly to it to get log messages. These will make sending log message privately or publicly much easier.
A WebSocket source is also included.  It can receive log messages directly from WebSocket clients.


## Installation
If you want to compile syslog-ng-incubator with the websocket source and destination, you must have libwebsockets installed in your system.

Please go to libwebsockets [official website](https://libwebsockets.org/) to download v2.0-stable and install it.


## Configuration
Here is an example of an destination example which act as a client
```
@version: 3.7
@include "scl.conf"

source      s_system { system(); internal();};

destination ws_client_des {
    websocket(
        # mode("client")
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

`protocol`:          websocket protocol
`address`:           websocket server listening address
`port`:              websocket server listening port
`path`:              the path of the websocket server service
`ssl`:               If this config appears, ssl will be used.
`allow_self_signed`: If you want to allow self-signed certificate when ssl is enabled, plese use this configue.
`cert`:              When it act as an websocket client it is optional. If the server wants us to present a valid SSL client, certificate, you can the filepath of it
`key`:               the private key for the cert
`cacert`             (optional) When it act as an client, it is a CA cert and CRL can be used to validate the cert send by the server.



## The client and server example
I write a api wrapper for libwebsockets, so using libwebsockets will be much easier.

Simple example can be found in `client_example.c` and `server_example.c`(TODO: this program is not provided so far.).

The server example also include a http server by which users can send and view
messages.  The server can act like a chat room.

First run code below to start the test server
```
make test-server
```

And open another terminal and run test client
```
make test-client
```

Open your browser and open the link http://localhost:8000/ and click the "Connect"
button to begin receive log.  Typing some messages in the first editable
textarea and click the "send" button, you will send message to other clients.

Then you can find that the test-client is interacting with the browser client.

