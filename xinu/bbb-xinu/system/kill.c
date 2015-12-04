/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i,j;			/* Index into descriptors	*/
	rwb32   rwbid;

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

	record_cpuqdata(pid);  /* call function to record process state time data */
						   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */

	/* check the process state counters and update the finished procs global counter for the current scheduler */
	/* if and only if the process actually spent any time in any of the states (i.e. moving among any of the states */
	/* while this scheduler was active */
	for(i=0;i<QTYPE_VALS;i++) {
		for(j=0;j<PR_STATES;j++) {
			mstime *tmptime = &(prptr->statetimes[i][j]);
			if(tmptime->secs || tmptime->ms) {
				procs_finished[i]++;
				break;
			}

		}
	}

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();
		break;

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		/* rwb IDs are encoded in prsem field as negatives below -1 (-1 = null) */
		if(prptr->prsem < -1) {
			/* get the actual rwb ID */
			rwbid = -1 * (prptr->prsem + 2);
			/* reset the process' rwb flag */
			rwbflags[pid] = 0;
			/* decrement the queue count for the queue the process will be removed from */
			rwbtab[rwbid].qcount--;
		} else {
			semtab[prptr->prsem].scount++;
		}
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* if actively using rwb, then exit by calling signalrwb */
		if(rwbflags[pid] != 0  && prptr->prsem < -1) _signalrwb(pid, -1*(prptr->prsem+2));
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
