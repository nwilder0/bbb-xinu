/* meminit.c - meminit */

#include <xinu.h>

void	*minheap;	/* Start address of heap	*/
void	*maxheap;	/* End address of heap		*/

/*------------------------------------------------------------------------
 * meminit - Initialize the free memory list for BeagleBone Black
 *------------------------------------------------------------------------
 */
void	meminit(void)
{
	struct	memblk *memptr;	/* Memory block pointer	*/

	/* Initialize the minheap and maxheap variables */

	minheap = (void *)&end;
	maxheap = (void *)MAXADDR;

	/* Initialize the memory list as one big block */

	/* adjust the next few lines to handle double linked memlist */
	memlist.mnext = (struct memblk *)minheap;
	memlist.mprev = (struct memblk *)minheap;
	memptr = memlist.mnext;

	memptr->mprev = &memlist;
	memptr->mnext = &memlist;
	memptr->mlength = (uint32)maxheap - (uint32)minheap;

	memlist.mnextsz = memptr;
	memptr->mnextsz = (struct memblk *)NULL;

}
