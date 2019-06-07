CC=gcc
CFLAGS=-I.

all : server client

server : server.c
	$(CC) server.c -o server  -lpthread  -L. -lmultmod $(CFLAGS)

client : client.c
	$(CC) client.c -o client  -L. -lmultmod $(CFLAGS)

clean :
	rm -f client server
	
