all: server.o client.o

server.o: server.c
	gcc server.c -lpthread -o server
client.o: client.c
	gcc client.c -lpthread -o client
clean:
	rm -f server client

