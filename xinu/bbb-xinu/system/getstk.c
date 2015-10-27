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

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use mblock multiples	*/

	prev = &memlist;
	curr = memlist.mprev;
	fits = NULL;
	fitsprev = NULL;  /* Just to avoid a compiler warning */

	while (curr != &memlist) {			/* Scan entire list	*/
		if (curr->mlength >= nbytes) {	/* Record block address	*/
			if((envtab[EV_MEMALLOC].val == MEMALLOC_FIRSTFIT) || (fits == NULL) ||
					(curr->mlength == nbytes) || (fits->mlength > curr->mlength)) {
				fits = curr;		/*   when request fits	*/
				fitsprev = prev;
			}
		}
		next = curr;
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
		fits = (struct memblk *)((uint32)fits + fits->mlength);
	}
	memlist.mlength -= nbytes;
	restore(mask);
	return (char *)((uint32) fits + nbytes - sizeof(uint32));
}
