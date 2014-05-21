#define _GNU_SOURCE

#include "malloc.h"

static Header base;                                     /* empty list to get started */
static Header *freep = NULL;                            /* start of free list */

static Header *morecore(unsigned);


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
static Header *morecore(unsigned nu)
{
  void *cp;
  Header *up;

  if(nu < NALLOC)
    nu = NALLOC;

#ifdef MMAP
  unsigned noPages;
  if(__endHeap == 0) __endHeap = sbrk(0);

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

  return (void *)fit_strategy(nunits, p, prevp);   
}

void *realloc(void * ap, size_t nbytes)
{
    Header *bp, *p;
    unsigned nunits;

    if (ap == NULL) {
      return malloc(nbytes);
    } else if (nbytes <= 0) {
        free(ap);
        return NULL;
    }

    bp = (Header*) ap - 1;
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) +1;
            
    if (nunits == bp->s.size){
      return ap; 
    }

    p = malloc(nbytes);
    if (p == NULL) return NULL;

    if (nunits < bp->s.size){
      memcpy(p, ap, nbytes);
    }else{
      memcpy(p, ap, sizeof(Header) * (bp->s.size - 1));
    }

    free(ap);

    return (void*)p;
}


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

void *best_fit(unsigned nunits, Header *p, Header *prevp)
{
	Header *bestprevp = NULL;
	unsigned min_size = INT_MAX;
	for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
		if(p->s.size >= nunits) {			/* big enough */
      if(bestprevp == NULL || p->s.size < min_size){
				bestprevp = prevp;
				min_size = p->s.size;
			}
		}
		if(p == freep){                    	/* wrapped around free list */
			if(bestprevp != NULL){
        /* return allocated space pointer, header excluded */
				return (void *)allocate(nunits, bestprevp->s.ptr, bestprevp);		
			}

			if((p = morecore(nunits)) == NULL){
				return NULL;              	/* none left */
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
