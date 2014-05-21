#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <string.h> 
#include <unistd.h>
#include <stdio.h>
#include <errno.h> 
#include <sys/mman.h>
#include <limits.h> 

#define NALLOC 1024                                     /* minimum #units to request */

#ifndef STRATEGY
	#define STRATEGY 1
#endif

typedef long Align;                                     /* for alignment to long boundary */

union header {                                          /* block header */
  struct {
    union header *ptr;                                  /* next block if on free list */
    unsigned size;                                      /* size of this block  - what unit? */ 
  } s;
  Align x;                                              /* force alignment of blocks */
};

typedef union header Header;

void free(void *);
void *realloc(void *, size_t);
void *first_fit(unsigned, Header *, Header *);
void *best_fit(unsigned, Header *, Header *);
void *allocate(unsigned, Header *, Header *);

#endif /* __MALLOC_H__ */
