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
  // new->file_name = malloc(sizeof(char)*500);
  new->file_name = file_name;
  //strcpy(new->file_name, file_name);
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
    //printf("First Element in queue\n");
    queue->front = new;
    queue->back = new;
  } else {
    //printf("Last element in queue is %s\n", queue->back->file_name);
    queue->back->next = new;
    queue->back = queue->back->next;
    queue->back->next = NULL;
  }
  //printf("Queue front is %s\n", queue->front->file_name);
  //printf("Queue back is %s\n", queue->back->file_name);
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

char* search_str;

#define MAX_NAME 500

//Threaded searching function
void* search_dir(void* args){
  pthread_t new_thread[max_threads];
  thread_args_t* dir_thread_args[max_threads];
  int cur_thread = 0;
  
  thread_args_t *arg = (thread_args_t*) args;
  DIR* cur_dir = opendir(arg->file_name);
  if (cur_dir == NULL){
    fprintf(stderr,"Unable to open directory %s \n", arg->file_name);
    exit (EXIT_FAILURE);
  }
  struct stat file_stat;

  struct dirent* cur_file = readdir(cur_dir);
  while(cur_file != NULL){
    if(!((strcmp(cur_file->d_name, arg->file_name) == 0)||
         (strcmp(cur_file->d_name, "..") == 0) ||
         (strcmp(cur_file->d_name, ".") == 0))){
      if(strstr(cur_file->d_name, search_str) != NULL){printf("%s in %s\n", cur_file->d_name, arg->file_name);}
      char* cur_path = malloc(sizeof(char)*MAX_NAME);
      strcpy(cur_path, arg->file_name);
      strcat(cur_path, "/");
      strcat(cur_path, cur_file->d_name);
      
      if(stat(cur_path, &file_stat) != 0){
        fprintf(stderr,"Stat failed: %s\n", cur_path);
        exit(2);
      }
      if (S_ISDIR(file_stat.st_mode)){
        //printf("%s is a directory\n", cur_file->d_name);
        pthread_mutex_lock(&count_lock);
        if (cur_threads >= max_threads){
          pthread_mutex_lock(&q_lock);
          add_to_queue (queue, cur_path);
          pthread_mutex_unlock(&q_lock);
        } else {
          dir_thread_args[cur_thread]  = malloc(sizeof(thread_args_t));
          dir_thread_args[cur_thread]->file_name = cur_path;
          pthread_create(&(new_thread[cur_thread]), NULL, search_dir, dir_thread_args[cur_thread]);
          cur_threads ++;
          cur_thread ++;
        }
        pthread_mutex_unlock(&count_lock);
      }
    }
    cur_file = readdir(cur_dir);
  }
  closedir(cur_dir);

  pthread_mutex_lock(&q_lock);
  to_recurse_t* next_dir = get_next(queue);
  pthread_mutex_unlock(&q_lock);
  if(next_dir != NULL){
    thread_args_t* next_args = malloc(sizeof(thread_args_t));
    next_args->file_name = next_dir->file_name;
    search_dir(next_args);
    free(next_args);
    free(next_dir->file_name);
    free(next_dir);
  }

  //printf("ENDING_THREAD\n");
  pthread_mutex_lock(&count_lock);
  cur_threads --;
  pthread_mutex_unlock(&count_lock);

  return NULL;
    

}

void start_search(char* file_name, char* str){

  //init stuff
  queue = init_queue();

  //add in code to get num of cpus
  max_threads = 1;
  //max_threads = 2 * sysconf(_SC_NPROCESSORS_ONLN); 
  printf("Max threads: %d\n", max_threads);
	
  cur_threads = 1;

  //Init the locks
  pthread_mutex_init(&q_lock, NULL);
  pthread_mutex_init(&count_lock, NULL);
  
  //Initialize search string global
  search_str = str;
  
  thread_args_t* args = malloc (sizeof (thread_args_t));
  args->file_name = file_name;
	
  search_dir (args);

}
//------------------------------------------------------
//Main function
int main(int argc, char** argv){
	printf("In main\n");
	if(argc != 2){
		fprintf(stderr, "Usage: %s <search term>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	start_search(".", argv[1]);
	
	return 0;
}
