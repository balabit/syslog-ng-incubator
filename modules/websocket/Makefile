.PHONY: clean all test-client
all: client_example

# client related
client.o: client.c client.h
	gcc client.c -g  -o client.o -c

client_example.o: client_example.c
	gcc client_example.c -g  -o client_example.o  -c

client_example: client.o client_example.o
	gcc -o client_example client_example.o client.o -lwebsockets -lpthread

test-client: client_example
	./client_example

# server related
server.o: server.c server.h
	gcc server.c -g -o server.o -c

server_example.o: server_example.c
	gcc server_example.c -g  -o server_example.o  -c

server_example: server.o server_example.o
	gcc -o server_example server_example.o server.o -lwebsockets -lpthread

test-server: server_example
	./server_example

clean:
	rm client.o client_example.o client_example
	rm server.o server_example.o server_example
