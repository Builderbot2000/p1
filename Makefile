# compiler to use
CC=gcc

# enable debug symbols and warnings
CFLAGS=-c -Wall

all: server client tunnel

server: server.o
	$(CC) server.o -o server

client: client.o
	$(CC) client.o -o client

tunnel: tunnel.o
	$(CC) tunnel.o -o tunnel

server.o: server.c
	$(CC) $(CFLAGS) server.c

client.o: client.c
	$(CC) $(CFLAGS) client.c

tunnel.o: tunnel.c
	$(CC) $(CFLAGS) tunnel.c
clean:
	rm -rf *.o client server tunnel

