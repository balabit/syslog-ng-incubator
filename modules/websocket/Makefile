.PHONY: clean all test-client
all: client_example

client.o: client.c client.h
	gcc client.c -g  -o client.o -c

client_example.o: client_example.c
	gcc client_example.c -g  -o client_example.o  -c

client_example: client.o client_example.o
	gcc -o client_example client_example.o client.o -lwebsockets -lpthread

test-client: client_example
	./client_example

clean:
	rm client.o client_example.o client_example
