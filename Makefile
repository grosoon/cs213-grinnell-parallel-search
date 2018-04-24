CC = clang
CFLAGS = -g

all: search test file-search

clean: 
	rm -f search
	rm -f test
	rm -f file-search

search: search.c search.h queue.c queue.h 
	$(CC) $(CLFAGS) -o search search.c queue.c 
test: test.c file-search.c search.c file-search.h
	$(CC) $(CFLAGS) -o test test.c file-search.c
file-search: file-search.c search.c queue.c queue.h file-search.h search.h
	$(CC) $(CFLAGS) -o file-search file-search.c queue.c queue.h search.c search.h 
