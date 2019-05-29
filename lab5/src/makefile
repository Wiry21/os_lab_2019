CC=gcc
CFLAGS=-I.

all : mutex modfact deadlock mutex_s
 
deadlock : deadlock.c
	$(CC) deadlock.c -o deadlock -lpthread -L. $(CFLAGS)

modfact : modfact.c
	$(CC) modfact.c -o modfact -lpthread -L. $(CFLAGS)

mutex : mutex.c
	$(CC) mutex.c -o mutex -lpthread -L. $(CFLAGS)
	
mutex : mutex_s.c
	$(CC) mutex_s.c -o mutex_s -lpthread -L. $(CFLAGS)
	
clean :
	rm -f mutex modfact deadlock mutex_s