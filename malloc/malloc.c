/*
 *  An implementation of malloc, realloc and free 
 *  which can be used instead of the predefined 
 *  functions in the standard library libc.
 *   
 *  Uses MMAP(3) to request memory, if does not 
 *  exists on the system it fallbacks to sbrk(2).
 *
 *  This is an extension of Kernigan and Ritchie's 
 *  code provided in book The C Programming Language.
 *
 *  Authors:
 *  Christopher Teljstedt, chte@kth.se
 *  Carl Eriksson, carerik@kth.se
*/

#define _GNU_SOURCE

#include "malloc.h"

static Header base;                                     /* empty list to get started */
static Header *freep = NULL;                            /* start of free list */

static Header *morecore(unsigned);


/* switch function depending on which strategy 
   that was provided at compile time */

#if   STRATEGY == 1
  #define fit_strategy first_fit
#elif STRATEGY == 2    
  #define fit_strategy best_fit
#else
  exit(1); 
#endif


/* free: put block ap in the free list */
void free(void * ap){
  Header *bp, *p;

  if(ap == NULL) return;                                /* Nothing to do */

  bp = (Header *) ap - 1;                               /* point to block header */

  /* point to block header */
  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
      if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
          break; /* freed block at start or end of arena */

  if (bp + bp->s.size == p->s.ptr) {
      /* join to upper nbr */
      bp->s.size += p->s.ptr->s.size;
      bp->s.ptr = p->s.ptr->s.ptr;
  } else
      bp->s.ptr = p->s.ptr;

  if (p + p->s.size == bp) {
      /* join to lower nbr */
      p->s.size += bp->s.size;
      p->s.ptr = bp->s.ptr;
  } else
      p->s.ptr = bp;

  freep = p;
}

/* morecore: ask system for more memory */
static Header *morecore(unsigned nu)
{
  void *cp;
  Header *up;

  /* we do not want to ask for memory
     in every call to malloc, so request
     atleast NALLOC units (1024) */
  if(nu < NALLOC) 
    nu = NALLOC;

/* UNIX system call sbrk(n) returns a
   pointer to n more bytes of storage */
#ifdef MMAP
  unsigned noPages;
  /* set endHeap to current location of program break */
  if(__endHeap == 0) __endHeap = sbrk(0);

  /* get no. pages convert from header units to bytes */
  noPages = ((nu*sizeof(Header))-1)/getpagesize() + 1;

  /* request more memory with MMAP */
  cp = mmap(__endHeap, noPages*getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  
  /* convert the requested size back to header units */
  nu = (noPages*getpagesize())/sizeof(Header);

  /* update endHeap with new program break*/
  __endHeap += noPages*getpagesize();
#else
  cp = sbrk(nu*sizeof(Header));
#endif

  if(cp == (void *) -1){                                 /* no space at all */
    perror("failed to get more memory");
    return NULL;
  }

  /* cp points to the requested memory */
  up = (Header *) cp;

  /* update header with the new size */
  up->s.size = nu;

  /* find a place, starting at freep,
     to insert the free block */
  free((void *)(up+1));

  /* return the pointer */
  return freep;
}

/* 
 * realloc(): a dynamic general-purpose storage reallocator. 
 *
 * Reallocates a memory block, changes the size of the memory
 * block pointed to by pointer (ap).
 * 
 * The function may move the memory block to a new location,
 * if the requested size is to large.
 *
 * The content of the memory block is preserved up to the lesser
 * of the old and new sizes.
 *
 * If the pointer (ap) is NULL realloc() behaves like malloc()
 * and assigns a new memory block and return a pointer to the
 * beginning.
 *
 * malloc(), realloc() and free() may be called in any order.
 */
void *realloc(void * ap, size_t nbytes)
{
    Header *bp, *p;
    unsigned nunits;

    /* Allocate a new memory block if NULL */ 
    if (ap == NULL) {
      return malloc(nbytes);
    } 

    /* If size is zero memory previously
       allocated is deallocated and a
       NULL pointer is returned */ 
    if (nbytes <= 0) {
        free(ap);
        return NULL;
    }

    bp = (Header*) ap - 1;
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header)+1;
    
    /* If size is equal to allocated block
       then just return the pointer */         
    if (nunits == bp->s.size){
      return ap; 
    }

    /* Otherwise request a new memory block,
       if system is out of memory return NULL */         
    p = malloc(nbytes);
    if (p == NULL) return NULL;

    /* Perserve as much allocated data
       as possible, by copying to the
       newly allocated space */         
    if (nunits < bp->s.size){
      memcpy(p, ap, nbytes);
    }else{
      memcpy(p, ap, sizeof(Header) * (bp->s.size - 1));
    }

    /* Lastly free the old allocated space */
    free(ap);

    return (void*)p;
}


/* 
 * malloc(): a dynamic general-purpose storage allocator. 
 *
 *  
 * Allocates a memory block, returning a pointer to the beginning
 * of the block. The content of the newly allocated block is not
 * initialized remaining with indeterminate values.
 *
 * Instead of allocating from a compiled-in fixed-size array, malloc() 
 * will request space from OS as needed. The free storage is kept as 
 * a list of free blocks.
 *
 * If freep is NULL it is initialized in the first call of malloc()  
 * to the base of the degenerate free list, i.e. one block of size
 * zero. The search for a free block begins at freep which is where
 * the last free block was found.
 *
 * A pointer is returned from malloc() that points to the free space 
 * within the block, which begins one unit beyond the header.
 *
 * malloc(), realloc() and free() may be called in any order.
 */
void * malloc(size_t nbytes)
{
  Header *p, *prevp;
  Header * morecore(unsigned);
  unsigned nunits;

  if(nbytes == 0) return NULL;
  nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) +1;

  if((prevp = freep) == NULL) {
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }

  return (void *)fit_strategy(nunits, p, prevp);   
}


/* First fit strategy; walk through the chain of free blocks
   and find any free block which fits the requested size */
void *first_fit(unsigned nunits, Header *p, Header *prevp)
{
  for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
    if(p->s.size >= nunits) {     /* big enough */
        return (void *)allocate(nunits, prevp->s.ptr, prevp);   
		}
		if(p == freep){                 /* wrapped around free list */
			if((p = morecore(nunits)) == NULL){
				return NULL;              	/* none left */
      }
    }
	}
}

/* Best fit strategy; walk through the whole chain of free blocks
   and find the smallest free block which fits the requested size */
void *best_fit(unsigned nunits, Header *p, Header *prevp)
{
	Header *bestprevp = NULL;
	unsigned min_size = INT_MAX;
	for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
		if(p->s.size >= nunits) {			/* big enough */
      /* is it smaller than best candidate, if so
         update bestprevp pointer and minimum size,
         else continue walking */
      if(bestprevp == NULL || p->s.size < min_size){
				bestprevp = prevp;
				min_size = p->s.size;
			}
		}

		if(p == freep){                 /* wrapped around free list */
			if(bestprevp != NULL){
        /* return allocated space pointer, header excluded */
				return (void *)allocate(nunits, bestprevp->s.ptr, bestprevp);		
			}

      /* wrapped around free list without finding
         a candidate, request more free memory */
			if((p = morecore(nunits)) == NULL){
				return NULL;              	/* no memory left */
      }
		}
	}
}

void *allocate(unsigned nunits, Header *p, Header *prevp){
      if (p->s.size == nunits)    /* exactly */
        prevp->s.ptr = p->s.ptr;
      else {                        /* allocate tail end */
        p->s.size -= nunits;    /* update free size */
        p += p->s.size;       /* define a new header for allocated space */
        p->s.size = nunits;     /* update a new header allocated size */
      }
      freep = prevp;
      return (void *)(p+1);     /* return allocated space pointer, header excluded */
}
