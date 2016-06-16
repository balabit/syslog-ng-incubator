# About the test
This folder contains some configure and Makefile to demonstrate how to use the websocket plugin.


# Presumption
I presume you followed the [README](../README.md) installation section and have installed libwebsocket, syslog-ng and syslog-ng-incubator with libwebsocket correctly.


# Test connection from websocket destination as a client to a websocket server

Make sure you are in the test directory and your 8000 port is available.

Open terminal A to run the command blew to start a websocket server
```
make test-server
```

Open terminal B to run the command below to start websocket destination as a client to connect to the websocket server in terminal A
```
make test-client-dest
```


Open terminal C to run the command below to send the log message
```
logger YOUR_LOG_MESSAGE
```

You will see messages are sent from the websocket destination and received in the websocket server.

## test secure connection

First you should generate the keys you want. You can generate the `server.crt` and `server.key` with one command `openssl req -nodes -new -x509 -keyout server.key -out server.crt`

Edit `test_client_dest.conf`, change this part
```
# ssl(
#     allow_self_signed
#     cert("/root/tmp/keys/server.crt")
#     key("/root/tmp/keys/server.key")
# )
```
to
```
ssl(
    allow_self_signed
    cert("<your-path-to-server.crt>")
    key("<your-path-to-server.key>")
)
```

At last you should uncomment the content below the comment `if you want to the server serve as ssl, uncomment this line` and replace the path in `sample_server/server.c`

Then you can test with the same way last section mentioned.
