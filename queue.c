#include <stdio.h>
#include "queue.h"


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
    queue->first = queue->front->next;
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

