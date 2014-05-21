#ifndef __TSTUTILITY_H__
#define __TSTUTILITY_H__

#include <string.h> 
#include <unistd.h>
#include <stdio.h>
#include <errno.h> 
#include <sys/mman.h>
#include <limits.h> 

#define MAX_SIZE 512
#define SEED 5

/*
 * Representation of block in memory.
 */
typedef struct memblock_s {
    void * ptr;
    int size;
} memblock_t;

#endif /* __TSTUTILITY_H__ */
