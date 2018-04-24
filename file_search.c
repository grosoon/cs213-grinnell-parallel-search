include "search.h"
include "queue.h"

int main(int argc, char** argv){
  if(argc != 2){
    fprintf(stderr, "Usage: %s <search term>", argv[0]);
    exit(EXIT_FAILURE);
  }

  
  start_search(".", argv[1]);

}


