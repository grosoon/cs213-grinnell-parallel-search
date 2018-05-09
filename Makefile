CC = clang
CFLAGS = -Wall -g

all: pfs #tests

clean:
	rm -f pfs
pfs: pfs.c
	$(CC) $(CFLAGS) -o pfs pfs.c -lpthread
#tests : test.c pfs.c pfs.h
#	$(CC) $(CFLAGS) pfs.c test.c -o tests -lpthread
