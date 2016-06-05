# Description
This folder contains a sample websocket server which accept websocket client
connections. It also include a http server by which users can send and view
messages.  The server can act like a chat room.


# Test

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
