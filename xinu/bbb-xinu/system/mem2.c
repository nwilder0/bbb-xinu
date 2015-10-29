#include <xinu.h>

syscall printfreemem() {

	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

	struct	memblk	*curr;

	int blkcount = 0;
	int totalsz = 0;

	curr = memlist.mnext;
	if(curr == &memlist) {
		printf("\nThere is no free memory\n");
	}

	while (curr != &memlist) {			/* Search free list	*/
		printf("\nFree memory block: Address - %x, Size - %u", curr, curr->mlength);
		totalsz += curr->mlength;
		blkcount++;

		curr = curr->mnext;
	}

	printf("\n\nTotal blocks: %d,  Total size: %d bytes\n\n", blkcount, totalsz);

	restore(mask);
	return OK;
}

