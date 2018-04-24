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


queue_t* queue; //The backlog queue
pthread_mutex_t q_lock; //The queue lock

int max_threads; //the thread limit

pthread_mutex_t count_lock; //Lock to protect # of threads running
int cur_threads; //# of threads running

//Args for each search thread
typedef struct thread_args{
  char* file_name;
}thread_args_t;

char* search_str;


//Set up and run the search
void start_search(char* file_name, char* str){

  //init stuff
  queue = init_queue();

  //add in code to get num of cpus

  max_threads = 2 * sysconf(_SC_NPROCESSORS_ONLN); //Eventually replace with calcuation result
  cur_threads = 0;

  //Init the locks
  pthread_mutex_init(&q_lock, NULL);
  pthread_mutex_init(&count_lock, NULL);
  
  //Initialize search string global
  search_str = str;
  

}

//Threaded searching function
void* search_dir(void* args){
  //Unpack args
  thread_args_t* thread_args = (thread_args_t*) args;
  char* file_name = thread_args-> file_name;
  //free(args);  MIght need to be freeing thread_args instead

  //Open a directory from the file name
  DIR* cur_dir = opendir(file_name); //Add some error checking 
  
  //Get a directory entry  
  struct dirent* cur_file = readdir(cur_dir); //Add error checking
  
  //Access file metadata
  struct stat file_info;

  //Loop our way through the directory
  while(cur_file != NULL){
    stat(cur_file->d_name, &file_info);

    //Get whether or not file is a dir
    bool dir = false; //Initially assume all are not

    //If it is a directory set dir to true
    if (S_ISDIR(file_info.st_mode)){
      dir = true;
    }
  
    //Check if the current file/directory name contains the search string
    if (strstr (cur_file->d_name,search_str) != NULL){
      printf("%s \n", cur_file->d_name); //Just print the file name for now.
    }

    //If the element is a directory do the thread check

    if (dir){
      //Lock the count
      pthread_mutex_lock (&count_lock);


      if (cur_threads >= max_threads){
        //Lock the queue
        pthread_mutex_lock (&q_lock);

        //Add file name to queue
        add_to_queue (queue, cur_file->d_name);

        //Unlock queue
        pthread_mutex_unlock (&q_lock);
      } else{
        //Create a new thread and execute the search function
        pthread_t* dir_thread;
      
        thread_args_t* dir_thread_args = malloc (sizeof(thread_args_t));
        dir_thread_args-> file_name = cur_file->d_name;
      
        //Create the new thread to run search_dir
        pthread_create (dir_thread, NULL, search_dir, dir_thread_args);
        cur_threads++;
      }

      //Unlock the count
      pthread_mutex_unlock (&count_lock);
    }
    // Continue on our way through the directory
    cur_file = readdir(cur_dir);
  }
  //Close the current directory when done
  closedir(cur_dir);

  //Check if the queue is empty
  pthread_mutex_lock(&q_lock);
  to_recurse_t* next_dir = get_next(queue);
  pthread_mutex_unlock(&q_lock);
  if(next_dir != NULL){
    thread_args_t* next_args = malloc(sizeof(thread_args_t));
    next_args->file_name = next_dir->file_name;
    search_dir(next_args);
    free(next_args);
    free(next_dir);
  }

  //The thread is dead now
  
  free(args);

  //Decrement the number of threads running
  pthread_mutex_lock(&count_lock);
  cur_threads --;
  pthread_mutex_unlock(&count_lock);

  return NULL;
}
