/* getstk.c - getstk */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getstk  -  Allocate stack memory, returning highest word address
 *------------------------------------------------------------------------
 */
char  	*getstk(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*prev, *curr, *next;	/* Walk through memory list	*/
	struct	memblk	*fits, *fitsprev; /* Record block that fits	*/

	LOG("\n getstk: starting, size = %d \n", nbytes);

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use mblock multiples	*/

	LOG("\n getstk: rounded size is now: %d",nbytes);

	prev = &memlist;
	curr = memlist.mprev;
	fits = NULL;
	fitsprev = NULL;  /* Just to avoid a compiler warning */

	LOG("\n getstk: about to enter loop: curr: %x, prev: %x",curr, prev);
	while (curr != &memlist) {			/* Scan entire list	*/
		if (curr->mlength >= nbytes) {
			if((envtab[EV_MEMALLOC].val == MEMALLOC_FIRSTFIT) || (fits == NULL) ||
					(curr->mlength == nbytes) || (fits->mlength > curr->mlength)) {
				fits = curr;		/*   when request fits	*/
				fitsprev = prev;
			}
		}
		LOG("\n getstk: next iteration: curr = curr->mprev (%x)",curr->mprev);
		prev = curr;
		curr = curr->mprev;
	}

	if (fits == NULL) {			/* No block was found	*/
		restore(mask);
		return (char *)SYSERR;
	}
	if (nbytes == fits->mlength) {		/* Block is exact match	*/
		fitsprev->mnext = fits->mnext;
		next = fits->mnext;
		next->mprev = fitsprev;
	} else {				/* Remove top section	*/

		fits->mlength -= nbytes;
		fits = (struct memblk *)(((char *)fits) + fits->mlength);
	}
	memlist.mlength -= nbytes;
	restore(mask);
	LOG("\n getstk: returning stack block of %d bytes at addr %x", nbytes,
			((uint32) ((char *)fits + nbytes - sizeof(uint32))));
	return (char *)((uint32)( ((char *)fits) + nbytes - sizeof(uint32)));
}
