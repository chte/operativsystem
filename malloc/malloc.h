#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <string.h> 
#include <unistd.h>
#include <stdio.h>
#include <errno.h> 
#include <sys/mman.h>
#include <limits.h> 

#define NALLOC 1024         /* minimum #units to request */

#ifndef STRATEGY
	#define STRATEGY 1
#endif

typedef long Align;         /* for alignment to long boundary */

/*
 * Header for each free block, contains
 * a record of the block size and a pointer
 * to the next free block.
 */
union header {   /* block header */
  struct {
    union header *ptr;      /* next block if on free list */
    unsigned size;          /* size of this block + header, measured 
    						   in multiples of headervsize */ 
  } s;
  Align x;                  /* force alignment of blocks */
};

typedef union header Header;

/*void free(void *);*/
void *realloc(void *, size_t);
void *first_fit(unsigned, Header *, Header *);
void *best_fit(unsigned, Header *, Header *);
void *allocate(unsigned, Header *, Header *);

#endif /* __MALLOC_H__ */
