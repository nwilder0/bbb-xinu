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
	pid32 pid;

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
	while (rwbptr->qcount-- > 0) {	/* Free all waiting processes	*/
		pid = getfirst(rwbptr->rwqueue);
		if (!isbadpid(pid)) {
			rwbflags[pid] = 0;
			ready(pid);
		}
	}

	rwbptr->rwcount = 0;

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
		rwbptr->qcount--;
		if (!isbadpid(pid)) {
			rwbflags[pid] = 0;
			ready(pid);
		}
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
	print_rwb_debug();

	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	rwbflags[currpid] = -1;
	if ((rwbptr->rwcount) != 0) {		/* If caller must block	*/
		prptr = &proctab[currpid];

		record_cpuqdata(currpid);  /* call function to record process state time data */
								   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = -1*(rwb+2);		/* Record semaphore ID	*/

		rwbptr->qcount++;
		enqueue(currpid,rwbptr->rwqueue);/* Enqueue on semaphore	*/
		resched();			/*   and reschedule	*/
	} else {
		rwbptr->rwcount--;
	}

	restore(mask);
	return OK;
}


syscall rwb_tryread(rwb32 rwb)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	struct	rwbentry *rwbptr;		/* Ptr to sempahore table entry	*/

	mask = disable();
	print_rwb_debug();

	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	rwbflags[currpid] = 1;
	if ((rwbptr->rwcount) < 0) {		/* If caller must block	*/
		prptr = &proctab[currpid];

		record_cpuqdata(currpid);  /* call function to record process state time data */
								   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = -1*(rwb+2);		/* Record RWB ID: -2 to -(RWB_COUNT+1) is valid (-1 is used by Xinu to indicate NULL here	*/

		rwbptr->qcount++;
		enqueue(currpid,rwbptr->rwqueue);/* Enqueue on semaphore	*/
		resched();			/*   and reschedule	*/
	} else {
		rwbptr->rwcount++;
	}

	restore(mask);
	return OK;
}

syscall signalrwb(rwb32 rwb) {
	return _signalrwb(currpid, rwb);
}


syscall _signalrwb(pid32 thisid, rwb32 rwb) {

	intmask mask;			/* Saved interrupt mask		*/
	struct	rwbentry *rwbptr;		/* Ptr to sempahore table entry	*/
	signed char is_writer, next_is_writer;
	pid32 nextpid = -1;

	mask = disable();
	print_rwb_debug();

	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}
	if (isbadpid(thisid)) {
		restore(mask);
		return SYSERR;
	}
	is_writer = rwbflags[thisid];
	rwbflags[thisid] = 0;

	rwbptr= &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}


	if (is_writer) {
		rwbptr->rwcount++;
	} else {
		rwbptr->rwcount--;
	}

	if(isempty(rwbptr->rwqueue)) {
		next_is_writer = 0;
	} else {
		nextpid = firstkey(rwbptr->rwqueue);
		if (!isbadpid(nextpid)) {
			next_is_writer = rwbflags[nextpid];
			if(next_is_writer == -1) {
				if(rwbptr->rwqueue == 0) {
					rwbptr->qcount--;
					ready(dequeue(rwbptr->rwqueue));
				}
			} else {
				while(next_is_writer == 1) {
					rwbptr->qcount--;
					ready(dequeue(rwbptr->rwqueue));
					nextpid = firstkey(rwbptr->rwqueue);
					if (!isbadpid(nextpid)) {
						next_is_writer = rwbflags[nextpid];
					} else {
						next_is_writer = 0;
					}
				}
			}
		}
	}

	proctab[thisid].prsem = -1;

	restore(mask);
	return OK;

}

syscall print_rwb_debug() {

	int i = 0;
	struct rwbentry *rwb;
	qid16 qpid;
	signed char is_writer;

	LOG2(DEBUG_L3,DEBUG_RWB,"\nPrinting Read-Write Blocker Debug data:\n");

	LOG2(DEBUG_L3,DEBUG_RWB,"\nRWB Table Limit: %d\n", RWB_COUNT);

	LOG2(DEBUG_L3,DEBUG_RWB,"Table State by Entry: ");

	for(i=0;i<RWB_COUNT; i++) {

		rwb = &rwbtab[i];

		LOG2(DEBUG_L3,DEBUG_RWB,"\n%d. Entry state = ");
		if(rwb->rwstate == S_FREE) {
			LOG2(DEBUG_L3,DEBUG_RWB,"Free");
		} else {
			LOG2(DEBUG_L3,DEBUG_RWB,"In Use\n    ");
			LOG2(DEBUG_L3,DEBUG_RWB,"Current Activity: ");
			if(rwb->rwcount == 0) {
				LOG2(DEBUG_L3,DEBUG_RWB,"      None");
			} else {
				if(rwb->rwcount > 0) {
					LOG2(DEBUG_L3,DEBUG_RWB,"      Reading (%d readers)",rwb->rwcount);

				} else {
					LOG2(DEBUG_L3,DEBUG_RWB,"      Writing");
				}
				if(! isempty(rwb->rwqueue) ) {
					LOG2(DEBUG_L3,DEBUG_RWB,"    Queue:      %d waiting [", rwb->rwcount);
					qpid = firstid(rwb->rwqueue);

					while(! isbadpid(qpid) ) {
						is_writer = rwbflags[qpid];
						LOG2(DEBUG_L3,DEBUG_RWB,"%d (", qpid);
						if(is_writer == -1) {
							LOG2(DEBUG_L3,DEBUG_RWB,"W");
						} else if(is_writer == 1) {
							LOG2(DEBUG_L3,DEBUG_RWB,"R");
						} else {
							LOG2(DEBUG_L3,DEBUG_RWB,"%d",is_writer);
						}
						LOG2(DEBUG_L3,DEBUG_RWB,"),");
						qpid = queuetab[qpid].qnext;

					}
					LOG2(DEBUG_L3,DEBUG_RWB,"\b]\n");
				}

			}
		}


	}
	LOG2(DEBUG_L3,DEBUG_RWB,"\n\n\n");

	return 0;
}
