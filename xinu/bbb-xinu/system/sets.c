/* sets.c - set_scheduler, set_cpuqdata */

#include <xinu.h>

/* assumes interrupts are disabled */

/*------------------------------------------------------------------------
 *  set_scheduler - sets a new value for the scheduler env var, after
 *  changing the value, the function goes through all readylist processes
 *  and resets their priorities to an appropriate value for the new
 *  scheduling method, as part of the readylist reset, all processes are
 *  removed, re-prioritized, and then re-queued to ensure proper sorting
 *------------------------------------------------------------------------
 */
syscall set_scheduler(struct envvar *varptr, uint32 newval) {

	uint32 i,count;

	// scratchlist is used reseting the readylist
	if(!isempty(scratchlist)) {
		LOG("\nerror: scratchlist is not empty\n");
		return SYSERR;
	}

	// preserve the oldval for a little bit and set the new val
	uint32 oldval = varptr->val;
	varptr->val = newval;

	// if there was no change, don't reset anything
	if(oldval != newval) {
		count = readycount;
		for(i=0;i<count;i++) {

			// take everything off the readylist
			pid32 tmppid = dequeue(readylist);
			readycount--;

			if(!isbadpid(tmppid)) {
				register struct procent *prptr = &proctab[tmppid];

				// reset its priority
				prptr->prprio = setprio(tmppid);

				// and put it on the scratchlist
				insert(tmppid, scratchlist, prptr->prprio);
				readycount++;
			}
		}
		// now move all the inner contents of the scratchlist to the readylist
		copyqueue(scratchlist,readylist);

		// restart everyone's cpuqdata counters since the scheduler has changed
		if(envtab[EV_CPUQDATA].val==EV_VALUE_YES) {
			for(i=0;i<NPROC;i++) {
				struct procent *prptr = &proctab[i];
				if(prptr->prstate != PR_FREE) record_cpuqdata(i);
			}
		}
	}

	return OK;
}

/*------------------------------------------------------------------------
 *  set_cpuqdata - sets the cpuqdata env var flag, and preps or clears the
 *  cpuqdata counters
 *------------------------------------------------------------------------
 */
syscall set_cpuqdata(struct envvar *varptr, uint32 newval) {

	uint32 i;

	uint32 oldval = varptr->val;

	if(oldval != newval) {
		if(newval) {
			for(i=0;i<NPROC;i++) {
				struct procent *prptr = &proctab[i];
				if(prptr->prstate != PR_FREE) record_cpuqdata(i);
			}
		} else {
			clear_cpuqdata();
		}
		varptr->val = newval;
	}

	return OK;
}

/*------------------------------------------------------------------------
 *  set_cmdhistory - sets the cmd history env var and resets the linked
 *  list of cmds with the size appropriate for the new value of the var
 *------------------------------------------------------------------------
 */
syscall set_cmdhistory(struct envvar *varptr, uint32 newval) {

	uint32 oldval = varptr->val;

	if(oldval != newval) {
		cmdhistorydel();
		if(newval) {
			cmdhistoryinit();
		}
		varptr->val = newval;
	}

	return OK;
}
