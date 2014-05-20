#define _GNU_SOURCE

#include "brk.h"
#include <unistd.h>
#include <string.h> 
#include <errno.h> 
#include <sys/mman.h>
#include <limits.h>

#define NALLOC 1024                                     /* minimum #units to request */


typedef long Align;                                     /* for alignment to long boundary */

union header {                                          /* block header */
  struct {
    union header *ptr;                                  /* next block if on free list */
    unsigned size;                                      /* size of this block  - what unit? */ 
  } s;
  Align x;                                              /* force alignment of blocks */
};

typedef union header Header;

static Header base;                                     /* empty list to get started */
static Header *freep = NULL;                            /* start of free list */

void *firstfit(unsigned, Header *, Header *);
void *bestfit(unsigned, Header *, Header *);


/* free: put block ap in the free list */

void free(void * ap)
{
  Header *bp, *p;

  if(ap == NULL) return;                                /* Nothing to do */

  bp = (Header *) ap - 1;                               /* point to block header */
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;                                            /* freed block at atrt or end of arena */

  if(bp + bp->s.size == p->s.ptr) {                     /* join to upper nb */
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  }
  else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp) {                             /* join to lower nbr */
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

/* morecore: ask system for more memory */

#ifdef MMAP

static void * __endHeap = 0;

void * endHeap(void)
{
  if(__endHeap == 0) __endHeap = sbrk(0);
  return __endHeap;
}
#endif


static Header *morecore(unsigned nu)
{
  void *cp;
  Header *up;
#ifdef MMAP
  unsigned noPages;
  if(__endHeap == 0) __endHeap = sbrk(0);
#endif

  if(nu < NALLOC)
    nu = NALLOC;
#ifdef MMAP
  noPages = ((nu*sizeof(Header))-1)/getpagesize() + 1;
  cp = mmap(__endHeap, noPages*getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  nu = (noPages*getpagesize())/sizeof(Header);
  __endHeap += noPages*getpagesize();
#else
  cp = sbrk(nu*sizeof(Header));
#endif
  if(cp == (void *) -1){                                 /* no space at all */
    perror("failed to get more memory");
    return NULL;
  }
  up = (Header *) cp;
  up->s.size = nu;
  free((void *)(up+1));
  return freep;
}

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
#ifdef STRATEGY
  if(STRATEGY == 1){
   	return (void *)firstfit(nunits, p, prevp);
  }else if(STRATEGY == 2){
  	return (void *)bestfit(nunits, p, prevp);  	
  }
#else 
  return (void *)firstfit(nunits, p, prevp);
#endif
}


void *firstfit(unsigned nunits, Header *p, Header *prevp){
	for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
		if(p->s.size >= nunits) {			/* big enough */
			if (p->s.size == nunits)		/* exactly */
				prevp->s.ptr = p->s.ptr;
			else {                     		/* allocate tail end */
				p->s.size -= nunits;		/* update free size */
				p += p->s.size;				/* define a new header for allocated space */
				p->s.size = nunits;			/* update a new header allocated size */
			}
			freep = prevp;
			return (void *)(p+1);			/* return allocated space pointer, header excluded */
		}
		if(p == freep)                    	/* wrapped around free list */
			if((p = morecore(nunits)) == NULL)
				return NULL;              	/* none left */
	}
}

void *bestfit(unsigned nunits, Header *p, Header *prevp){
	Header *bestp = NULL;
	Header *bestprevp = NULL;
	unsigned min_size = INT_MAX;
	for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
		if(p->s.size >= nunits) {			/* big enough */
			if (p->s.size == nunits){		/* exactly */
				prevp->s.ptr = p->s.ptr;
				freep = prevp;
				return (void *)(p+1);		/* return allocated space pointer, header excluded */
			}else if(bestp == NULL || p->s.size < min_size){
				bestp = p;
				bestprevp = prevp;
				min_size = p->s.size;
			}
		}
		if(p == freep){                    	/* wrapped around free list */
			if(bestprevp != NULL){
				bestp->s.size -= nunits;		/* update free size of best candidate */
				bestp += bestp->s.size;			/* define a new header for allocated space */
				bestp->s.size = nunits;			/* update a new header allocated size */
				freep = bestprevp;
				return (void *)(bestp+1);		/* return allocated space pointer, header excluded */
			}

			if((p = morecore(nunits)) == NULL)
				return NULL;              	/* none left */
		}
	}
}

