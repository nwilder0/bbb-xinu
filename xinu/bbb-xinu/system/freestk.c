/* freestk.c - freestk */

#include <xinu.h>

/*------------------------------------------------------------------------
 * freestk - Free the stack memory beginning at the provided pointer
 *------------------------------------------------------------------------
 */
syscall	freestk(char *stkaddr, uint32 nbytes)
{

	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*next, *block, *blkaddr, *curr;
	uint32	top;

	LOG("\n freestk: starting for stkaddr %x, nbytes %d \n", stkaddr, nbytes);
	mask = disable();

	blkaddr = ((struct memblk *)(stkaddr - nbytes + sizeof(uint32)));

	LOG("\n freestk: blkaddr is %x \n", blkaddr);
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) minheap)
			  || ((uint32) blkaddr > (uint32) maxheap)) {
		LOG("\n freestk: SYSERR 1 \n");
		restore(mask);
		return SYSERR;
	}


	LOG("\n freestk: blkaddr (block) is %x \n", blkaddr);
	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = blkaddr;

	LOG("\n freestk: memlist %x, mprev %x \n", &memlist, memlist.mprev);
	next = &memlist;			/* Walk along free list	*/
	curr = memlist.mprev;
	while ((curr != &memlist) && (curr > block)) {
		next = curr;
		curr = curr->mprev;
		LOG("\n freestk: looping next(last) %x, curr(next) %x \n", next, curr);
	}

	if (curr == &memlist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) curr + curr->mlength;
	}
	LOG("\n freestk: top %x, curr %x \n", top, curr);

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((curr != &memlist) && (uint32) block < top)
	    || ((next != &memlist)	&& (uint32) block+nbytes>(uint32)next)) {
		LOG("\n freestk: syserr 2 \n");
		restore(mask);
		return SYSERR;
	}

	memlist.mlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		LOG("\n freestk: coalescing with prev block \n");
		curr->mlength += nbytes;
		block = curr;
	} else {			/* Link into list as new node	*/
		block->mnext = next;
		next->mprev = block;
		block->mlength = nbytes;
		block->mprev = curr;
		curr->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if ((((uint32) block) + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
		next = block->mnext;
		next->mprev = block;
	}
	restore(mask);
	return OK;

}
