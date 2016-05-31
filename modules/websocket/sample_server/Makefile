.PHONY: clean all test
all: server

server: server.c server.h
	gcc  server.c -o server -lwebsockets
clean:
	rm -f server
	rm -rf server.dSYM
test: server
	./server
