CC=gcc
CFLAGS=-I.

all: sequential_min_max1 parallel_min_max1 test_exec par_dop

par_dop : utils.o find_min_max.o utils.h find_min_max.h par_dop.c
	$(CC) -o par_dop utils.o find_min_max.o par_dop.c $(CFLAGS)

test1:
	$(CC) find_min_max.h find_min_max.c utils.h utils.c sequential_min_max.c -o sequential_min_max2
	$(CC) find_min_max.h find_min_max.c utils.h utils.c parallel_min_max.c -o parallel_min_max2

test_exec: test_exec.c
	$(CC) test_exec.c -o test_exec

sequential_min_max1 : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o sequential_min_max1 find_min_max.o utils.o sequential_min_max.c $(CFLAGS)

parallel_min_max1 : utils.o find_min_max.o utils.h find_min_max.h parallel_min_max.c
	$(CC) -o parallel_min_max1 utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o sequential_min_max1 parallel_min_max1
