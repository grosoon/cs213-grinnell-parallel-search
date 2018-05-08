#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pfs.h"


//--------------------------------------------------------
typedef struct to_recurse {
  char *file_name;
  struct to_recurse *next;
} to_recurse_t;


typedef struct queue {
  to_recurse_t *front;
  to_recurse_t *back;
}queue_t;

//Set up the to_recurse_t structs (queue nodes)
to_recurse_t* new_to_recurse(char* file_name) {
  to_recurse_t *new = malloc(sizeof(to_recurse_t));
  new->file_name = file_name;
  new->next = NULL;
  return new;
}

//Set up a queue
queue_t* init_queue(){
  queue_t* ret = malloc(sizeof(queue_t));
  ret->front = NULL;
  ret->back = NULL;
  return ret;
}

//Add a string to a node in the queue
void add_to_queue(queue_t *queue, char* file_name){
  to_recurse_t *new = new_to_recurse(file_name); 
  if(queue->front == NULL){
    queue->front = new;
    queue->back = new;
  } else {
    queue->back->next = new;
    queue->back = queue->back->next;
    queue->back->next = NULL;
  }
}

//Get the next element in the queue
to_recurse_t* get_next(queue_t *queue){
  to_recurse_t* ret = queue->front;
  if(ret != NULL){
    queue->front = ret->next;
  }
  return ret;
}

//Free the queue
void free_queue(queue_t *queue){
  to_recurse_t* cur = queue->front;
  to_recurse_t* next = queue->front;
  while(cur != NULL){
    next = cur->next;
    free(cur);
    cur = next;
  }
  free(queue);
}
//Print the queue
void print_queue(queue_t* queue){
  to_recurse_t* cur = queue->front;
  printf("Queue Elements:\n");
  while (cur != NULL){
    printf("\t%s\n", cur->file_name);
    cur = cur->next;
  }
}
//----------------------------------------------------------------
queue_t* queue; //The backlog queue
pthread_mutex_t q_lock; //The queue lock

int max_threads; //the thread limit

pthread_mutex_t count_lock; //Lock to protect # of threads running
int cur_threads; //# of threads running

//Args for each search thread
typedef struct thread_args{
  char* file_name;
}thread_args_t;

char* search_str; //The search string

#define MAX_NAME 500 //Maximum number of chars in a file name

//Threaded searching function
void* search_dir(void* args){

  
  pthread_t new_thread[100];//Array of threads
  thread_args_t* dir_thread_args[100]; //Array of thread arguments
  int cur_thread = 0; //The index of the current thread

  //Create a thread_args_t from the input param
  thread_args_t *arg = (thread_args_t*) args;

  //Open the directory
  DIR* cur_dir = opendir(arg->file_name);

  //If the cur_dir is not a directory that can be opened exit.
  if (cur_dir == NULL){
    fprintf(stderr,"Unable to open directory %s \n", arg->file_name);
    exit (EXIT_FAILURE);
  }
  struct stat file_stat; //the stat structure

  //Start reading the directory
  struct dirent* cur_file = readdir(cur_dir);
  
  //While there are still files in the directory
  while(cur_file != NULL){
    //If the file is not one of the specified ones we aren't searching
    if(!((strcmp(cur_file->d_name, arg->file_name) == 0)||
         (strcmp(cur_file->d_name, "..") == 0) ||
         (strcmp(cur_file->d_name, ".") == 0))){

      //If we find something containing the file name
      if(strstr(cur_file->d_name, search_str) != NULL){
        printf("%s in %s\n", cur_file->d_name, arg->file_name);
      }
      //Create the file path
      char* cur_path = malloc(sizeof(char)*MAX_NAME);
      strcpy(cur_path, arg->file_name);
      strcat(cur_path, "/");
      strcat(cur_path, cur_file->d_name);

      //Get the stat for the file
      if(stat(cur_path, &file_stat) != 0){
        fprintf(stderr,"Stat failed: %s\n", cur_path);
        //exit(2);
      }else if (S_ISDIR(file_stat.st_mode)){
        //intf("%s is a directory\n", cur_file->d_name);

        //Lock the count
        pthread_mutex_lock(&count_lock);

        //Check if we have any threads left before hitting the max
        if (cur_threads >= max_threads || cur_thread >= 100){
          //Lock the queue
          pthread_mutex_lock(&q_lock);

          add_to_queue (queue, cur_path); //Add the directory to the queue

          //Unlock the queue
          pthread_mutex_unlock(&q_lock);
        } else {
          //Create a new set of thread args for the new thread
          dir_thread_args[cur_thread]  = malloc(sizeof(thread_args_t));
          dir_thread_args[cur_thread]->file_name = cur_path;

          //Create a new thread for the directory
          pthread_create(&(new_thread[cur_thread]), NULL, search_dir, dir_thread_args[cur_thread]);
          cur_threads ++; 
          cur_thread ++;
        }//Close the if/else on count check

        //Unlock the count
        pthread_mutex_unlock(&count_lock);
      } else {

        //Free the cur_path allocation
        free(cur_path);
   
      }//Close the directory check
      
    }//Close the file name check

    //Continue through the directory
    cur_file = readdir(cur_dir);

  }//Close the while loop

  //Close the now finished directory
  closedir(cur_dir);

  //Lock the queue
  pthread_mutex_lock(&q_lock);

  //Get the next element from the queue
  to_recurse_t* next_dir = get_next(queue);

  //Unlock the queue
  pthread_mutex_unlock(&q_lock);

  //If the next element is not null
  if(next_dir != NULL){
   
    //Create a new thread args with the file name
    thread_args_t* next_args = malloc(sizeof(thread_args_t));
    next_args->file_name = next_dir->file_name;

    //Rerun the search on the new directory
    search_dir(next_args);

    //When that search finishes, free everything 
    free(next_args);
    free(next_dir->file_name);
    free(next_dir);
  }//Close the next_dir check

  //Join all of the running threads and clean up
  for(int i = 0; i < cur_thread; i++){
    pthread_join(new_thread[i], NULL);
    free(dir_thread_args[i]->file_name);
    free(dir_thread_args[i]);
  }
  
  //Lock the count
  pthread_mutex_lock(&count_lock);

  //Thread is now dead, decrement the count
  cur_threads --;

  //Unlock the count
  pthread_mutex_unlock(&count_lock);

  return NULL;
    

}

void start_search(char* file_name, char* str, int mult){

  //init stuff
  queue = init_queue();

  //Set max threads to be a multiple of the number of processors
  max_threads = mult * sysconf(_SC_NPROCESSORS_ONLN); 
 
  //Start with 1 thread
  cur_threads = 1;

  //Init the locks
  pthread_mutex_init(&q_lock, NULL);
  pthread_mutex_init(&count_lock, NULL);
  
  //Initialize search string global
  search_str = str;

  //Set up first directory search
  thread_args_t* args = malloc (sizeof (thread_args_t));
  args->file_name = file_name;

  //Search the directory
  search_dir (args);

  //Clean up
  free(args); 
  free_queue(queue);
  pthread_mutex_destroy(&q_lock);
  pthread_mutex_destroy(&count_lock);
}

void start_l_search(char* file_name, char* str){

  //init stuff
  queue = init_queue();

  //Linear search uses at most 1 thread
  max_threads = 1;

  //Start with 1 thread
  cur_threads = 1;

  //Init the locks
  pthread_mutex_init(&q_lock, NULL);
  pthread_mutex_init(&count_lock, NULL);
  
  //Initialize search string global
  search_str = str;

  //Set up first directory search
  thread_args_t* args = malloc (sizeof (thread_args_t));
  args->file_name = file_name;

  //Search the directory
  search_dir (args);

  //Clean up
  free(args);
  free_queue(queue);
  pthread_mutex_destroy(&q_lock);
  pthread_mutex_destroy(&count_lock);
}


/*
//------------------------------------------------------
Main function
int main(int argc, char** argv){

  
  if(argc != 2){
    fprintf(stderr, "Usage: %s <search term>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  
  // printf("In main\n");
  int one = 0;
  int two = 0;
  int four = 0;
  int eight = 0;
  int linear = 0;
  int sixteen = 0;
  int thirty_two = 0;
  int sixty_four = 0;
  
  for(int i = 0; i < 50; i++){
    
    clock_t start = clock();
    start_search(".", argv[1], 1);
    clock_t diff = clock() - start;
    one += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
    start_search(".", argv[1], 2);
    diff = clock() - start;
    two += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
    start_search(".", argv[1], 4);
    diff = clock() - start;
    four += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
    start_search(".", argv[1], 8);
    diff = clock() - start;
    eight += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
    start_search(".", argv[1], 16);
    diff = clock() - start;
    sixteen += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
    start_search(".", argv[1], 32);
    diff = clock() - start;
    thirty_two += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
     start_search(".", argv[1], 64);
    diff = clock() - start;
    sixty_four += diff * 1000 / CLOCKS_PER_SEC;
    start = clock();
    start_l_search(".", argv[1]);
    diff = clock() - start;
    linear += diff * 1000 / CLOCKS_PER_SEC;
    
  }
  printf("one: %d\n", one/50);
  printf("two: %d\n", two/50);
  printf("four: %d\n", four/50);
  printf("eight: %d\n", eight/50);
  printf("sixteen: %d\n", sixteen/50);
  printf("thirty_two: %d\n", thirty_two/50);
  printf("sixty_four: %d\n", sixty_four/50);
  printf("linear: %d\n", linear/50);

  start_search(".", argv[1], 64);


  //Start the search in the current directory
  return 0;
  
  }*/
