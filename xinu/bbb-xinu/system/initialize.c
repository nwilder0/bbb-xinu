/* initialize.c - nulluser, sysinit, sizmem */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>

extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
extern	void xdone(void);	/* System "shutdown" procedure		*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/

/* Declarations of major kernel variables */

struct	procent	proctab[NPROC];	/* Process table			*/
struct	sentry	semtab[NSEM];	/* Semaphore table			*/
struct	memblk	memlist;	/* List of free memory blocks		*/

/* Active system status */

int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/

uint32 state_times[PR_STATES]; /* ADDDD */
uint32 procs_finished; /* ADDDD */
uint16 scheduler; /*  ADDDD */
uint16 readycount; /*  ADDDD */

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{	
	//kprintf("nulluser()\n");

	struct	memblk	*memptr;	/* Ptr to memory block		*/
	uint32	free_mem;		/* Total amount of free memory	*/
	
	/* Initialize the system */

	sysinit();

	kprintf("\n\r%s\n\n\r", VERSION);
	
	/* Output Xinu memory layout */
	free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=NULL;memptr = memptr->mnext) {
	    kprintf("           [0x%08X to 0x%08X]\r\n",
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Enable interrupts */

	enable();

	/* Disable the watchdog */

    wddisable();

	/* Create a process to execute function main() */

	resume (
	   create((void *)main, INITSTK, INITPRIO, "Main process", 0,
           NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {
		;		/* Do nothing */
	}

}

/*------------------------------------------------------------------------
 *
 * sysinit - intialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	//kprintf("sysinit()\n");
	int32	i;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Prr to semaphore table entry	*/

	/* Platform Specific Initialization */

	platinit();

	kprintf("initevec()\n");
	/* Initialize the interrupt vectors */

	initevec();
	
	/* Initialize free memory list */
	kprintf("meminit()\n");

	meminit();
	kprintf("init system vars\n");

	/* Initialize system variables */

	/*  ADDDD */
	scheduler = QTYPE_DEFAULT;

	/* ADDDD */
	readycount = 0;

	/* Count the Null process as the first process in the system */

	prcount = 1;

	kprintf("defer is next\n");
	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	kprintf("init state_times\n");

	/* ADDDD */
	for(i = 0; i < PR_STATES; i++) {
		state_times[i] = 0;
	}
	procs_finished = 0;

	/* Initialize process table entries free */
	kprintf("init proctab\n");

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
	}

	/* Initialize the Null process entry */	
	kprintf("init null proc()\n");

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->timestatein = 0;		/* ADDDD */

	kprintf("init null proc state times\n");
	for(i=0; i<PR_STATES; i++) prptr->statetimes[i] = 0;
	prptr->prprio = 0;
	strncpy(prptr->prname, "prnull", 7);
	prptr->prstkbase = getstk(NULLSTK);
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	currpid = NULLPROC;
	
	/* Initialize semaphores */
	kprintf("init sem queues()\n");

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */
	kprintf("bufinit()\n");

	bufinit();

	/* Create a ready list for processes */
	kprintf("readylist\n");
	readylist = newqueue();

	/* Initialize the real time clock */
	kprintf("clkinit()\n");
	clkinit();

	kprintf("init devs\n");
	for (i = 0; i < NDEVS; i++) {
		kprintf("init dev %d\n",i);
		init(i);
	}
	return;
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}
