CC = clang
CFLAGS = -g

all: queue search file_search test

clean:
	rm -f queue 
	rm -f search
	rm -f test
	rm -f file_search	

queue: queue.h queue.c
	$(CC) $(CFLAGS) -o queue queue.c
search: search.c queue.c #queue.h 
	$(CC) $(CLFAGS) -o search search.c queue.c 
file_search: file_search.c search.c search.h queue.h
	$(CC) $(CFLAGS) -o file_search file_search.c #search.c queue.c
test: test.c file-search.c #search.c file_search.h
	$(CC) $(CFLAGS) -o test test.c file_search.c 

