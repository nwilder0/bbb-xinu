#include <xinu.h>

/* semcreate.c - semcreate, newsem */

local	rwb32	newrwb(void);

/*------------------------------------------------------------------------
 *  semcreate  -  Create a new semaphore and return the ID to the caller
 *------------------------------------------------------------------------
 */
rwb32	rwbcreate(
	  int32		count		/* Initial semaphore count	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	rwb32	rwb;			/* Semaphore ID to return	*/

	mask = disable();

	if (count < 0 || ((rwb=newrwb())==SYSERR)) {
		restore(mask);
		return SYSERR;
	}
	rwbtab[rwb].rwcount = count;	/* Initialize table entry	*/

	restore(mask);
	return rwb;
}

/*------------------------------------------------------------------------
 *  newsem  -  Allocate an unused semaphore and return its index
 *------------------------------------------------------------------------
 */
local	rwb32	newrwb(void)
{
	static	rwb32	nextrwb = 0;	/* Next semaphore index to try	*/
	rwb32	rwb;			/* Semaphore ID to return	*/
	int32	i;			/* Iterate through # entries	*/

	for (i=0 ; i<RWB_COUNT ; i++) {
		rwb = nextrwb++;
		if (nextrwb >= RWB_COUNT)
			nextrwb = 0;
		if (rwbtab[rwb].rwstate == S_FREE) {
			rwbtab[rwb].rwstate = S_USED;
			return rwb;
		}
	}
	return SYSERR;
}


/*------------------------------------------------------------------------
 * semdelete  -  Delete a semaphore by releasing its table entry
 *------------------------------------------------------------------------
 */
syscall	rwbdelete(
		rwb32		rwb		/* ID of semaphore to delete	*/
	)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	rwbentry *rwbptr;		/* Ptr to semaphore table entry	*/

	mask = disable();
	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}
	rwbptr->rwstate = S_FREE;

	resched_cntl(DEFER_START);
	while (rwbptr->rwcount++ < 0) {	/* Free all waiting processes	*/
		ready(getfirst(rwbptr->rwqueue));
	}
	resched_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}

/*------------------------------------------------------------------------
 *  semreset  -  Reset a semaphore's count and release waiting processes
 *------------------------------------------------------------------------
 */
syscall	rwbreset(
	  rwb32		rwb,		/* ID of semaphore to reset	*/
	  int32		count		/* New count (must be >= 0)	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	rwbentry *rwbptr;		/* Ptr to semaphore table entry */
	qid16	rwbqueue;		/* Semaphore's process queue ID	*/
	pid32	pid;			/* ID of a waiting process	*/

	mask = disable();

	if (count < 0 || isbadrwb(rwb) || rwbtab[rwb].rwstate==S_FREE) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	rwbqueue = rwbptr->rwqueue;	/* Free any waiting processes */
	resched_cntl(DEFER_START);
	while ((pid=getfirst(rwbqueue)) != EMPTY)
		ready(pid);
	rwbptr->rwcount = count;		/* Reset count as specified */
	resched_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}


syscall rwb_trywrite(rwb32 rwb)
{
	// behaves just like semaphore wait
	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	struct	rwbentry *rwbptr;		/* Ptr to sempahore table entry	*/

	mask = disable();
	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--(rwbptr->rwcount) < 0) {		/* If caller must block	*/
		prptr = &proctab[currpid];

		record_cpuqdata(currpid);  /* call function to record process state time data */
								   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = -1*(rwb+1);		/* Record semaphore ID	*/
		enqueue(currpid,rwbptr->rwqueue);/* Enqueue on semaphore	*/
		resched();			/*   and reschedule	*/
	}

	restore(mask);
	return OK;
}


syscall rwb_tryread(rwb32 rwbnum)
{
	// a writer is active, then...
	waitrwb(rwbnum);
	// else
	return 0;
}

syscall waitrwb(rwb32 rwbnum) {

	return 0;
}

syscall signalrwb(rwb32 rwbnum) {

	// dec count
	// if count = 0 and writer in queue then pop queue and start writer
	// else pop all readers
	return 0;
}

