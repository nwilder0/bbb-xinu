/* memory.h - roundmb, truncmb, freestk */

#define	PAGE_SIZE		4096
#define MIN_BLOCKSIZE	16		/* use def for block size, 16 to allow for larger memblk struct */

/*----------------------------------------------------------------------
 * roundmb, truncmb - Round or truncate address to memory block size
 *----------------------------------------------------------------------
 */
#define	roundmb(x)	(char *)( ((MIN_BLOCKSIZE-1) + (uint32)(x)) & (~(MIN_BLOCKSIZE-1)) )
#define	truncmb(x)	(char *)( ((uint32)(x)) & (~(MIN_BLOCKSIZE-1)) )

/* expand memblk to allow for backward traversal */
struct	memblk	{			/* See roundmb & truncmb	*/
	struct	memblk	*mnext;		/* Ptr to next free memory blk	*/
	uint32	mlength;		/* Size of blk (includes memblk)*/
	struct 	memblk  *mprev;		/* ptr to previous free memory block */
	struct  memblk	*mnextsz;	/* ptr to next larger or equal sized memory block (not currently used) */
	};

extern	struct	memblk	memlist;	/* Head of free memory list	*/
extern	void	*minheap;		/* Start of heap		*/
extern	void	*maxheap;		/* Highest valid heap address	*/


/* Added by linker */

extern	int	text;			/* Start of text segment	*/
extern	int	etext;			/* End of text segment		*/
extern	int	data;			/* Start of data segment	*/
extern	int	edata;			/* End of data segment		*/
extern	int	bss;			/* Start of bss segment		*/
extern	int	ebss;			/* End of bss segment		*/
extern	int	end;			/* End of program		*/
