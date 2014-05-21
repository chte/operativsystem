#include "tstutility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void *getmem(void){
    #ifdef MMAP
      return endHeap();
    #else
      return (void *)sbrk(0);
    #endif
};


int main(int argc, char *argv[]) {
    int epochs = (argc > 1) ? atoi(argv[1]) : MAX_EPOCHS;

    if (epochs > MAX_EPOCHS) {
        fprintf(stderr, "Test set to default EPOCH, set it to lower than %d.\n", MAX_EPOCHS);
        epochs = MAX_EPOCHS;
    }

    int epoch, i;
    void *lowbreak, *highbreak;
    memblock_t block[epochs];
    srand(SEED);

    /* Initialize structure */
    for (i = 0; i < epochs; ++i) {
        block[i].ptr = NULL;
    }

    lowbreak = getmem();


    for (i = 0; i < epochs; ++i) {
           
            /* Malloc between 0 and MAX_SIZE */
            block[i].size = 512;
            block[i].ptr = malloc(block[i].size);

            if (block[i].ptr == NULL) {
                perror("Error: Insufficient memory when mallocing.");
                exit(1);
            }
            
            /* Realloc between 0 and 2*MAX_SIZE */
            /*int REALLOC_SIZE = 32;
            block[i].ptr = realloc( block[i].ptr, REALLOC_SIZE );
            block[i].size = REALLOC_SIZE;
            if (block[i].ptr == NULL) {
                perror("Error: Insufficient memory when reallocing.");
                exit(1);
            }*/

        
    }


    for (i = 0; i < epochs; ++i) {
        free(block[i].ptr);
    }

    highbreak = getmem();

    fprintf(stderr, "Memory usage: %u b\n", highbreak-lowbreak);

    return 0;
}