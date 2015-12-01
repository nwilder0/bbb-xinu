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
struct  rwbentry rwbtab[RWB_COUNT]; /* Read-Write Blocker table */
struct	memblk	memlist;	/* List of free memory blocks		*/
signed char rwbflags[NPROC];

/* Active system status */

int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/

mstime state_times[QTYPE_VALS][PR_STATES]; /* global process state counters to track time spent in a state */
uint32 procs_finished[QTYPE_VALS]; /* global counters of processes that have completed execution */
uint16 readycount; /* counter to track the current number in the readyqueue */

mstime ztime = {0,0}; /* mstime zero element */

/* the environment variables table */
struct envvar envtab[ENV_VARS] =
{
		{ 0, "environment\0", 0, NULL, ENV_VARS, NULL },
		{ 1, "scheduler\0", QTYPE_VALS, (char *[]){"priority\0","shortest-job-first\0","random\0"},
				QTYPE_DEFAULT, (void *)set_scheduler },
		{ 2, "cpuqdata\0", 2, (char *[]){"no\0", "yes\0"}, CPUQDATA_DEFAULT, (void *)set_cpuqdata },
		{ 3, "debug\0", 2, (char *[]){"no\0", "yes\0"}, DEBUG_DEFAULT, NULL },
		{ 4, "dtimer\0", 0, NULL, DTIMER_DEFAULT, NULL },
		{ 5, "memalloc\0", 2, (char *[]){"first-fit\0","best-fit\0"}, MEMALLOC_DEFAULT, NULL },
		{ 6, "cmdhistory\0", 0, NULL, CMDHIST_DEFAULT, (void *)set_cmdhistory }
};

/* temporary queue used to reorganize other queues */
qid16 scratchlist;

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
	for (memptr = memlist.mnext; memptr != &memlist;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=&memlist;memptr = memptr->mnext) {
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
	int32	i,j;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Ptr to semaphore table entry	*/
	struct  rwbentry *rwbptr;   /* Ptr to read-write blocker table entry */

	/* Platform Specific Initialization */

	platinit();

	LOG("initevec()\n");
	/* Initialize the interrupt vectors */

	initevec();
	
	/* Initialize free memory list */
	LOG("meminit()\n");

	meminit();
	LOG("init system vars\n");

	/* Initialize system variables */

	/* intialize the readylist count tracker */
	readycount = 0;

	/* Count the Null process as the first process in the system */

	prcount = 1;

	LOG("defer is next\n");
	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	LOG("init state_times\n");

	/* initialize the global scheduler data counters */
	for(i = 0; i < QTYPE_VALS; i++) {
		for(j = 0; j < PR_STATES; j++) {
			zerotime(state_times[i][j]);
		}
		procs_finished[i] = 0;
	}


	/* Initialize process table entries free */
	LOG("init proctab\n");

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
	}

	/* Initialize the Null process entry */	
	LOG("init null proc()\n");

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->timestatein = ztime;		/* initialize the state entered timestamp to 0 */

	LOG("init null proc state times\n");

	/* initialize all the null process' state counters to 0 */
	for(i = 0; i < QTYPE_VALS; i++) {
		for(j = 0; j < PR_STATES; j++) {
			prptr->statetimes[i][j] = ztime;
		}
	}
	prptr->prprio = 0;
	prptr->prprio0 = 0; /* set initial provided priority, used when moving back to priority queuing */
	strncpy(prptr->prname, "prnull", 7);
	prptr->prstkbase = getstk(NULLSTK);
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	currpid = NULLPROC;
	
	/* Initialize semaphores */
	LOG("init sem queues()\n");

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	for(i=0; i<RWB_COUNT; i++) {
		rwbptr = &rwbtab[i];
		rwbptr->rwcount = 0;
		rwbptr->nextw = 0;
		rwbptr->rwqueue = newqueue();
		rwbptr->rwstate = S_FREE;
	}
	for(i=0; i<NPROC; i++) {
		rwbflags[i] = 0;
	}

	/* Initialize buffer pools */
	LOG("bufinit()\n");

	bufinit();

	/* Create a ready list for processes */
	LOG("readylist\n");
	readylist = newqueue();

	/* Create a scratch queue for temporary purposes, should only be used inside a syscall with interrupts disabled */
	scratchlist = newqueue();

	/* Initialize the real time clock */
	LOG("clkinit()\n");
	clkinit();

	LOG("init devs\n");
	for (i = 0; i < NDEVS; i++) {
		LOG("init dev %d\n",i);
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
