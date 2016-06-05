.PHONY: clean all test-server test-client
all: server client

server: server.c server.h
	gcc server.c -o server -lwebsockets
client: client.c client.h
	gcc client.c -g  -o client -lwebsockets -lpthread
clean:
	rm -f server
	rm -rf server.dSYM
	rm -f client
test-server: server
	./server

test-client: client
	./client
