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
    start_l_search(".", argv[1]);
    clock_t diff = clock() - start;
    one += diff * 1000 / CLOCKS_PER_SEC;
    /* start = clock(); */
    /* start_search(".", argv[1], 64); */
    /* diff = clock() - start; */
    /* two += diff * 1000 / CLOCKS_PER_SEC; */
    /* start = clock(); */
    /* start_search(".", argv[1], 4); */
    /* diff = clock() - start; */
    /* four += diff * 1000 / CLOCKS_PER_SEC; */
    /* start = clock(); */
    /* start_search(".", argv[1], 8); */
    /* diff = clock() - start; */
    /* eight += diff * 1000 / CLOCKS_PER_SEC; */
    /* start = clock(); */
    /* start_search(".", argv[1], 16); */
    /* diff = clock() - start; */
    /* sixteen += diff * 1000 / CLOCKS_PER_SEC; */
    /* start = clock(); */
    /* start_search(".", argv[1], 32); */
    /* diff = clock() - start; */
    /* thirty_two += diff * 1000 / CLOCKS_PER_SEC; */
    /* start = clock(); */
    /*  start_search(".", argv[1], 64); */
    /* diff = clock() - start; */
    /* sixty_four += diff * 1000 / CLOCKS_PER_SEC; */
    /* start = clock(); */
    /* start_l_search(".", argv[1]); */
    /* diff = clock() - start; */
    /* linear += diff * 1000 / CLOCKS_PER_SEC; */
    
  }
  printf("one: %d\n", one/50);
  printf("two: %d\n", two/50);
  printf("four: %d\n", four/50);
  printf("eight: %d\n", eight/50);
  printf("sixteen: %d\n", sixteen/50);
  printf("thirty_two: %d\n", thirty_two/50);
  printf("sixty_four: %d\n", sixty_four/50);
  printf("linear: %d\n", linear/50);


  //Start the search in the current directory
  return 0;
}
