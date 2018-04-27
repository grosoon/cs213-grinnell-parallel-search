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

to_recurse_t* new_to_recurse(char* file_name) {
	to_recurse_t *new = malloc(sizeof(to_recurse_t));
	new->file_name = file_name;
	new->next = NULL;
	return new;
}


queue_t* init_queue(){
	queue_t* ret = malloc(sizeof(queue_t));
	ret->front = NULL;
	ret->back = NULL;
	return ret;
}

void add_to_queue(queue_t *queue, char* file_name){
	to_recurse_t *new = new_to_recurse(file_name); 
	if(queue->front == NULL){
		queue->front = new;
		queue->back = new;
	} else {
		queue->back->next = new;
	}
}

to_recurse_t* get_next(queue_t *queue){
	to_recurse_t* ret = queue->front;
	if(ret != NULL){
		queue->front = queue->front->next;
	}
	return ret;
}

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


//Threaded searching function
void* search_dir(void* args){
	//Unpack args
	thread_args_t* thread_args = (thread_args_t*) args;
	char* dir_name = thread_args-> file_name;
	//free(args);  MIght need to be freeing thread_args instead

	//Open a directory from the file name
	DIR* cur_dir = opendir(dir_name);
	if (cur_dir == NULL){
		fprintf(stderr,"Unable to open directory %s \n", dir_name);
		exit (EXIT_FAILURE);
	}
	//Get a directory entry  
	struct dirent* cur_file = (struct dirent*)malloc(sizeof(struct dirent));
	cur_file = readdir(cur_dir);
  
	//Access and store file metadata
	struct stat* file_stat = (struct stat*) malloc (sizeof(struct stat));

	//Loop our way through the directory
	while(cur_file != NULL){
    
		//Prevent searching in .. and .git
		//CURRENTLY RUNNING INTO ISSUES AND CAUSING INFINITE LOOPING
		/*if (strstr(cur_file->d_name, "..") == NULL){
		continue;
		}*/ 
    
		//Store the info about the file in a stat
		stat(cur_file->d_name, file_stat);

		//Get whether or not file is a dir
		bool dir = false; //Initially assume all are not

		//If it is a directory set dir to true
		if ((file_stat->st_mode &S_IFDIR)){
			dir = true;
		}
  
		//Check if the current file/directory name contains the
		//search string
		// printf ("Current entry is %s\n", cur_file->d_name);
		//printf("Search string is %s\n", search_str);
		if (strstr (cur_file->d_name,search_str) != NULL){
			//Just print the file name for now.
			printf("%s \n", cur_file->d_name); 
		}

		//If the element is a directory do the thread check

		if (dir){
			//    printf("Cur_file is a directory\n");
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
				pthread_t dir_thread;
      
				thread_args_t* dir_thread_args = malloc (sizeof(thread_args_t));
				dir_thread_args-> file_name = cur_file->d_name;
				printf("New thread to search %s\n", cur_file->d_name);
				//Create the new thread to run search_dir
				pthread_create (&dir_thread, NULL, search_dir, dir_thread_args);
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

//Set up and run the search
void start_search(char* file_name, char* str){

	//init stuff
	queue = init_queue();

	//add in code to get num of cpus
	//Eventually replace with calcuation result
	max_threads = 2 * sysconf(_SC_NPROCESSORS_ONLN); 
	cur_threads = 0;

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

