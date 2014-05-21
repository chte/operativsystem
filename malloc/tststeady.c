#include "tstutility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_SIZE 32

void *getmem(void){
    #ifdef MMAP
      return endHeap();
    #else
      return (void *)sbrk(0);
    #endif
};


int main(int argc, char *argv[]) {
    int epochs = (argc > 1) ? atoi(argv[1]) : 0;


    int epoch, i;
    void *lowbreak, *highbreak;
    memblock_t block[epochs];
    srand(5);

    /* Initialize structure */
    for (i = 0; i < epochs; ++i) {
        block[i].ptr = NULL;
    }

    lowbreak = getmem();


    for (i = 0; i < epochs; ++i) {
        block[i].size = MAX_SIZE;
        block[i].ptr = malloc(block[i].size);

        if (block[i].ptr == NULL) {
            perror("Error: Insufficient memory when mallocing.");
            exit(1);
        }    
    }


    for (i = 0; i < epochs; ++i) {
        free(block[i].ptr);
    }

    highbreak = getmem();

    fprintf(stderr, "Memory usage: %u b\n", highbreak-lowbreak);

    return 0;
}