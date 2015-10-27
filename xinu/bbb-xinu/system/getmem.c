/* getmem.c - getmem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getmem  -  Allocate heap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char  	*getmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*prev, *next, *curr, *leftover, *best;

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

	prev = &memlist;

	curr = memlist.mnext;
	best = NULL;

	while (curr != &memlist) {			/* Search free list	*/

		if (curr->mlength == nbytes) {
			/* if block is exact match, go ahead and return with it	*/
			prev->mnext = curr->mnext;
			next = curr->mnext;
			next->mprev = prev;
			memlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* if block is bigger than needed	*/

			// if no best block has been found or this one is better
			if((best == NULL) || (best->mlength > curr->mlength)) {
				best = curr;
			}
			// if the memory allocation algorithm is first fit then go ahead and return with 'best'
			if(envtab[EV_MEMALLOC].val == MEMALLOC_FIRSTFIT) {
				break;
			}

		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}

	// if something was found (not an exact fit)
	if(best != NULL) {

		leftover = (struct memblk *)((uint32) curr +
				nbytes);
		prev->mnext = leftover;
		leftover->mprev = prev;
		leftover->mnext = curr->mnext;
		next = curr->mnext;
		next->mprev = leftover;
		leftover->mlength = curr->mlength - nbytes;
		memlist.mlength -= nbytes;
		restore(mask);
		return (char *)(best);
	}

	restore(mask);
	return (char *)SYSERR;
}
