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
	LOG("\n getmem: starting \n");
	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

	LOG("\n getmem: set prev to memlist (%x) \n", &memlist);
	prev = &memlist;

	LOG("\n getmem: set curr to memlist.mnext (val: %x) \n", memlist.mnext);
	curr = memlist.mnext;
	best = NULL;

	while (curr != &memlist) {			/* Search free list	*/

		if (curr->mlength == nbytes) {
			/* if block is exact match, go ahead and return with it	*/
			LOG("\n getmem: exact match \n");
			prev->mnext = curr->mnext;
			next = curr->mnext;
			next->mprev = prev;
			memlist.mlength -= nbytes;
			LOG("\n getmem: returning exact match block of %d bytes at addr %x", nbytes, curr);
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* if block is bigger than needed	*/

			LOG("\n getmem: block is bigger than needed (need: %d, block (%x) is %d) \n",nbytes,curr,curr->mlength);
			// if no best block has been found or this one is better
			if((best == NULL) || (best->mlength > curr->mlength)) {
				LOG("\n getmem: setting best to this block (%x) \n",curr);
				best = curr;
			}
			// if the memory allocation algorithm is first fit then go ahead and return with 'best'
			if(envtab[EV_MEMALLOC].val == MEMALLOC_FIRSTFIT) {
				break;
			}

		} 			/* Move to next block	*/

		LOG("\n getmem: set prev to curr (val: %x) \n", curr);
		prev = curr;
		LOG("\n getmem: set curr to curr->mnext (val: %x) \n", curr->mnext);
		curr = curr->mnext;

	}

	// if something was found (not an exact fit)
	if(best != NULL) {
		prev = best->mprev;
		leftover = (struct memblk *)(((char *)best) +
				nbytes);
		prev->mnext = leftover;
		leftover->mprev = prev;
		leftover->mnext = best->mnext;
		next = best->mnext;
		next->mprev = leftover;
		leftover->mlength = best->mlength - nbytes;
		memlist.mlength -= nbytes;
		LOG("\n getmem: returning block of %d bytes at addr %x", nbytes, best);
		restore(mask);
		return (char *)(best);
	}

	restore(mask);
	return (char *)SYSERR;
}
