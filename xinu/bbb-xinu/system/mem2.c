#include <xinu.h>

syscall printfreemem() {

	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

	struct	memblk	*prev, *curr;

	prev = &memlist;
	curr = memlist.mnext;
	if(curr == NULL) {
		printf("\nThere is no free memory\n");
	}

	while (curr != NULL) {			/* Search free list	*/
		printf("\nFree memory block: Address - %x, Size - %u", curr, curr->mlength);
		prev = curr;
		curr = curr->mnext;
	}

	restore(mask);
	return OK;
}

