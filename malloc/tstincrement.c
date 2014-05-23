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
        block[i].size = 0;
    }

    lowbreak = getbreak();


    for (i = 0; i < epochs; ++i) {
        /* Malloc 2, 4, ... up to 1024 and reset */
        block[i].size = (2 << (i % 10)); //Up to 1022
        block[i].ptr = malloc(block[i].size);

        if (block[i].ptr == NULL) {
            perror("Error: Insufficient memory when mallocing.");
            exit(1);
        }    
    }


    for (i = 0; i < epochs; ++i) {
        free(block[i].ptr);
    }

    highbreak = getbreak();

    fprintf(stderr, "Memory usage: %u b\n", highbreak-lowbreak);

    return 0;
}