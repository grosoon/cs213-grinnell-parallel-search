#include <stdio.h>
#include <stdbool.h>
#include "queue.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//Set up search
void start_search(char* file_name, char* str);

//Threaded search 
void* search_dir(void* args);