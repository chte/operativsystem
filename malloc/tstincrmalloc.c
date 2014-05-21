#include "tstutility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "brk.h"

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

    int i;
    unsigned *start, *end;
    memblock_t block[epochs];
    srand(SEED);

    /* Initialize structure */
    for (i = 0; i < epochs; ++i) {
        block[i].ptr = NULL;
    }

    start = getmem();

    for (i = 0; i < epochs; ++i) {
        block[i].size = 2 << (rand() % RNG_SPAN + EXPONENT - 1);
        block[i].ptr = malloc(block[i].size);

        if (block[i].ptr == NULL) {
            perror("Error: Insufficient memory.");
            exit(1);
        }
    }

    for (i = 0; i < epochs; ++i) {
        free(block[i].ptr);
    }

    end = getmem();

    fprintf(stderr, "Memory usage: %u b\n", end - start);

    return 0;
}