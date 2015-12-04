/* rwblocker.c - rwbcreate, newrwb, rwbreset, rwbdelete, rwb_tryread, rwb_trywrite,
 * signalrwb, _signalrwb, print_rwb_debug
 * */

#include <xinu.h>

local	rwb32	newrwb(void);

/*------------------------------------------------------------------------
 *  rwbcreate  -  Set a new rw blocker and return the ID to the caller
 *------------------------------------------------------------------------
 */
rwb32	rwbcreate(
	  int32		count		/* Initial rwb count	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	rwb32	rwb;			/* RWB ID to return	*/

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
 *  newrwb  -  Allocate an unused RW blocker and return its index
 *------------------------------------------------------------------------
 */
local	rwb32	newrwb(void)
{
	static	rwb32	nextrwb = 0;	/* Next rwb index to try	*/
	rwb32	rwb;			/* RWB ID to return	*/
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
 * rwbdelete  -  Delete a rwb by releasing its table entry
 *------------------------------------------------------------------------
 */
syscall	rwbdelete(
		rwb32		rwb		/* ID of rwb to delete	*/
	)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	rwbentry *rwbptr;		/* Ptr to rwb table entry	*/
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

	/* clear out the wait queue and reset their rwb flags */
	resched_cntl(DEFER_START);
	while (rwbptr->qcount-- > 0) {
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
 *  rwbreset  -  Reset a RWB's count and release waiting processes
 *------------------------------------------------------------------------
 */
syscall	rwbreset(
	  rwb32		rwb,		/* ID of rwb to reset	*/
	  int32		count		/* New count, probably only 0 is appropriate */
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	rwbentry *rwbptr;		/* Ptr to rwb table entry */
	qid16	rwbqueue;		/* RWB's process queue ID	*/
	pid32	pid;			/* ID of a waiting process	*/

	mask = disable();

	if (count < 0 || isbadrwb(rwb) || rwbtab[rwb].rwstate==S_FREE) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	rwbqueue = rwbptr->rwqueue;
	resched_cntl(DEFER_START);
	while ((pid=getfirst(rwbqueue)) != EMPTY)
		rwbptr->qcount--;
		if (!isbadpid(pid)) {
			rwbflags[pid] = 0;
			ready(pid);
		}
	rwbptr->rwcount = count;
	resched_cntl(DEFER_STOP);
	restore(mask);

	return OK;
}

/*------------------------------------------------------------------------
 *  rwb_trywrite  -  Attempt to write to an RWB controlled shared area,
 *  similar to calling wait on a semaphore
 *------------------------------------------------------------------------
 */
syscall rwb_trywrite(rwb32 rwb)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	struct	rwbentry *rwbptr;		/* Ptr to rwb table entry	*/

	mask = disable();
	print_rwb_debug();		/* if debug is on, this shows state of this rwb */

	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	/* set the flag to show this PID is now a writer */
	rwbflags[currpid] = -1;
	/* if rwcount != 0 then either a writer or reader(s) are currently active */
	/* if qcount > 0, then other procs are waiting to access the data, good   */
	/*    for the case where active proc just left, but scheduler hasn't 	  */
	/*    given cpu time to the newly released next proc in the queue but	  */
	/*    shouldn't be necessary as rwcount is set in signalrwb after dequeue */
	if (((rwbptr->rwcount) != 0) || (rwbptr->qcount > 0)) {		/* If caller must block	*/

		prptr = &proctab[currpid];

		record_cpuqdata(currpid);  /* call function to record process state time data */
								   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = -1*(rwb+2);		/* Record rwb ID (-1 taken by null)	*/

		rwbptr->qcount++;
		enqueue(currpid,rwbptr->rwqueue);/* Enqueue on rwb	*/
		resched();			/*   and reschedule	*/

	} else {
		rwbptr->rwcount--;			/* if not waiting and moving to active */
	}

	restore(mask);
	return OK;
}

/*------------------------------------------------------------------------
 *  rwb_tryread  -  Attempt to read to an RWB controlled shared area
 *------------------------------------------------------------------------
 */
syscall rwb_tryread(rwb32 rwb)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	struct	rwbentry *rwbptr;		/* Ptr to rwb table entry	*/

	mask = disable();
	print_rwb_debug();	/* if debug is on, this shows state of this rwb */

	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}

	rwbptr = &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	rwbflags[currpid] = 1;				/* mark this PID as a reader */
	/* don't jump queue if queue isn't empty, and don't enter if writer working */
	if ((rwbptr->rwcount < 0) || (rwbptr->qcount > 0)) {		/* If caller must block	*/
		prptr = &proctab[currpid];

		record_cpuqdata(currpid);  /* call function to record process state time data */
								   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = -1*(rwb+2);		/* Record RWB ID: -2 to -(RWB_COUNT+1) is valid (-1 is used by Xinu to indicate NULL here */

		rwbptr->qcount++;
		enqueue(currpid,rwbptr->rwqueue);/* Enqueue on rwb	*/
		resched();			/*   and reschedule	*/

	} else {
		rwbptr->rwcount++;		/* if not waiting and going straight to active */
	}

	restore(mask);

	return OK;
}

/*------------------------------------------------------------------------
 *  signalrwb  -  Normal call to signal on an RWB, wrapper that inserts
 *  currpid into _signalrwb as most signal calls will use
 *------------------------------------------------------------------------
 */
syscall signalrwb(rwb32 rwb) {
	return _signalrwb(currpid, rwb);
}

/*------------------------------------------------------------------------
 *  _signalrwb  -  signal on the provided rwb, using the provided PID that
 *  should be currpid except when signal called by kill to remove a PID
 *------------------------------------------------------------------------
 */
syscall _signalrwb(pid32 thisid, rwb32 rwb) {

	intmask mask;			/* Saved interrupt mask		*/
	struct	rwbentry *rwbptr;		/* Ptr to rwb table entry	*/

	/* is_writer holds -1, 0, or 1 for thisid to indicate role */
	/* next_is_writer holds the same but for PIDs in the wait queue */
	int32 is_writer, next_is_writer;
	pid32 nextpid = -1;				/* initialize the next PID in queue */

	mask = disable();
	print_rwb_debug();		/* if debug is on, this shows state of this rwb */

	if (isbadrwb(rwb)) {
		restore(mask);
		return SYSERR;
	}
	if (isbadpid(thisid)) {
		restore(mask);
		return SYSERR;
	}
	is_writer = rwbflags[thisid];	/* is thisid a writer or reader */
	rwbflags[thisid] = 0;			/* this PID is leaving the rwb, so reset the flag */

	rwbptr= &rwbtab[rwb];
	if (rwbptr->rwstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	/* if this PID is a writer then increment (-1 -> 0) */
	if (is_writer==-1) {
		rwbptr->rwcount++;
	/* if this PID is a reader then decrement (>=1 -> 0+) */
	} else {
		rwbptr->rwcount--;
	}

	if(isempty(rwbptr->rwqueue)) {
		LOG2(DEBUG_INFO,DEBUG_RWB,"rwsignal: PID %d wants to signal on %d but found it empty",
				thisid,rwb);
		next_is_writer = 0;
	} else {
		/* if there is anything in the wait queue then figure out if it should be released */
		nextpid = firstid(rwbptr->rwqueue);
		if (!isbadpid(nextpid)) {
			/* get the role (writer, reader) of first queued PID */
			next_is_writer = rwbflags[nextpid];
			if(next_is_writer == -1) {
				/* if first queued PID is a writer then release it only if all readers are finished */
				if(rwbptr->rwcount == 0) {
					rwbptr->qcount--;
					ready(dequeue(rwbptr->rwqueue));
					/* decrement rwcount here instead of in new writer's trywrite, */
					/* so a new process can't slip in ahead of it */
					rwbptr->rwcount--;
				}
			/* if not a writer then it should be a reader */
			} else {
				if(next_is_writer != 1) {
					LOG2(DEBUG_INFO,DEBUG_RWB,
							"rwsignal: PID %d wants to dequeue PID %d but rwbflag is not 1 (%d)",
							thisid,nextpid,next_is_writer);
				}
				/* next is a reader, then release it from the wait queue as well as any subsequent readers */
				while(next_is_writer == 1) {
					rwbptr->qcount--;
					ready(dequeue(rwbptr->rwqueue));
					/* increment here so a new writer can't slip in ahead of this PID */
					rwbptr->rwcount++;
					nextpid = firstid(rwbptr->rwqueue);
					if (!isbadpid(nextpid)) {
						next_is_writer = rwbflags[nextpid];
					} else {
						next_is_writer = 0;
					}
				}
			}
		} else {
			LOG2(DEBUG_INFO,DEBUG_RWB,
					"rwsignal: PID %d wants to signal on %d queue, but first PID %d is bad",
					thisid,rwbptr->rwqueue,nextpid);
		}
	}

	proctab[thisid].prsem = -1;

	restore(mask);

	return OK;

}

/*------------------------------------------------------------------------
 *  print_rwb_debug - print out the state of all RWBs in rwbtab, including
 *  active process type, and wait queue contents; this output only displays
 *  if LOG2 debugging is active.
 *------------------------------------------------------------------------
 */
syscall print_rwb_debug() {

	intmask mask;
	int i = 0;
	struct rwbentry *rwb;
	qid16 qpid;
	signed char is_writer;

	/* disable interrupts since system variables are going to be read/iterated over */
	mask = disable();

	LOG2(DEBUG_L3,DEBUG_RWB,"\nPrinting Read-Write Blocker Debug data:\n");

	LOG2(DEBUG_L3,DEBUG_RWB,"\nRWB Table Limit: %d\n", RWB_COUNT);

	LOG2(DEBUG_L3,DEBUG_RWB,"Table State by Entry: ");

	for(i=0;i<RWB_COUNT; i++) {				/* for each read-write blocker entry */

		rwb = &rwbtab[i];

		LOG2(DEBUG_L3,DEBUG_RWB,"\n%d. Entry state = ",i);
		if(rwb->rwstate == S_FREE) {
			LOG2(DEBUG_L3,DEBUG_RWB,"Free");
		} else {
			LOG2(DEBUG_L3,DEBUG_RWB,"In Use\n    ");
			LOG2(DEBUG_L3,DEBUG_RWB,"Current Activity: ");
			if(rwb->rwcount == 0) {
				LOG2(DEBUG_L3,DEBUG_RWB,"      None (%d)", rwb->rwcount);
			} else {
				if(rwb->rwcount > 0) {
					LOG2(DEBUG_L3,DEBUG_RWB,"      Reading (%d readers)",rwb->rwcount);

				} else {
					LOG2(DEBUG_L3,DEBUG_RWB,"      Writing (%d)",rwb->rwcount);
				}
			}
			if(! isempty(rwb->rwqueue) ) {
				LOG2(DEBUG_L3,DEBUG_RWB,"    Queue:      %d waiting [", rwb->qcount);
				qpid = firstid(rwb->rwqueue);

					while(! isbadpid(qpid) ) {			/* iterate through the queue */
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
					LOG2(DEBUG_L3,DEBUG_RWB,"]\n");
			}

		}


	}
	LOG2(DEBUG_L3,DEBUG_RWB,"\n\n\n");
	restore(mask);

	return 0;
}
