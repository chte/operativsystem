#include "tstutility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_SIZE 512

void *getbreak(void){
    #ifdef MMAP
      return endHeap();
    #else
      return (void *)sbrk(0);
    #endif
};


int main(int argc, char *argv[]) {
    int epochs = (argc > 1) ? atoi(argv[1]) : 0;


    int i;
    void *lowbreak, *highbreak;
    memblock_t block[epochs];
    srand(SEED);

    /* Initialize structure */
    for (i = 0; i < epochs; ++i) {
        block[i].ptr = NULL;
    }

    lowbreak = getbreak();


    for (i = 0; i < epochs; ++i) {
        int index = rand() % epochs;
        if(rand() % 2 == 0){
            free(block[index].ptr);
            block[index].ptr = NULL;

        }else{

            /* Malloc with a random size between 1 and MAX_SIZE */
            block[index].size = rand() % MAX_SIZE + 1;
            block[index].ptr = malloc(block[index].size);

            if (block[index].ptr == NULL) {
                perror("Error: Insufficient memory when mallocing.");
                exit(1);
            }
            
            /* Realloc with a random size between 1 and MAX_SIZE */
            int REALLOC_SIZE = rand() % MAX_SIZE + 1;
            block[index].ptr = realloc( block[index].ptr, REALLOC_SIZE );
            block[index].size = REALLOC_SIZE;
            if (block[index].ptr == NULL) {
                perror("Error: Insufficient memory when reallocing.");
                exit(1);
            }            
        }
        
    }


    for (i = 0; i < epochs; ++i) {
        free(block[i].ptr);
    }

    highbreak = getbreak();

    fprintf(stderr, "Memory usage: %u b\n", highbreak-lowbreak);

    return 0;
}