

typedef struct to_recurse {
  char *file_name;
  struct to_recurse *next;
} to_recurse_t;


typedef struct queue {
  to_recurse_t *front;
  to_recurse_t *back;
}queue_t;

queue_t* init_queue();
void add_to_queue(queue_t *queue, char* file_name);
to_recurse_t* get_next(queue_t *queue);
void free_queue(queue_t *queue);
