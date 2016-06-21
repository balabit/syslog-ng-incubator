# Description
This folder contains wrappers of libwebsockets apis which will make it much
easier to write libwebsocets clients and servers. There is a sample websocket
server which accept websocket client connections. It also include a http server
by which users can send and view messages.


# Test

First run code below to start the test server
```
make test-server
```

And open another terminal and run test client
```
make test-client
```


Open your browser and open the link http://localhost:8000/ and click the
"Connect" button to begin receive log.  Typing some messages in the first
editable textarea and click the "send" button, you will send message to the
server. The messages from the server will be displayed on your screen.


Then you can find that the test-client is interacting with the browser client.
