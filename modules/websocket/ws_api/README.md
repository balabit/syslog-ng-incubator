# Description
This folder contains wrappers of libwebsockets apis which will make it much easier to write libwebsocets clients and servers.

This folder includes two examples:
* A websocket server example which accept websocket client connections and broadcast messages every 2 seconds.  It also include a http server by which users can send and view messages.
* A websocket client example which connect to the websocket server and send messages every 2 seconds.

The code of the examples can be found in `ws_api/client_example.c` and `ws_api/server_example.c`


# How to run the examples

First run code below to start the server example.
```
make test-server
```

And open another terminal and run client example.
```
make test-client
```


Open your browser and open the link http://localhost:8000/ and click the
"Connect" button to begin receive log.  Typing some messages in the first
editable textarea and click the "send" button, you will send message to the
server. The messages from the server will be displayed on your screen.

Then you can find that the test-client is interacting with the browser client.
