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

/*------------------------------------------------------------------------
 *  set_dblevel - sets the debug level env var and updates the global
 *  debug_mask variable to use the new corresponding value as it's four
 *  most significant bits; this value controls the severity level of
 *  displayed debug messages ranging from most restrictive (none - 0) to
 *  least restrictive (verbose - 0xF) (other values are error (1), warning
 *  (3), and info (7)); values are inclusive to more restrictive settings
 *------------------------------------------------------------------------
 */
syscall set_dblevel(struct envvar *varptr, uint32 newval) {

	uint32 i;
	uint16 lvlmask = 0;

	uint32 oldval = varptr->val;

	/* if a change was actually made */
	if(oldval != newval) {
		/* since values are inclusive build the total based on the 2's
		 * exponent provided from the setenv function */
		for(i=1; i<=newval; i++)
		{
			/* since level is the most significant 4 bits, shift left */
			lvlmask = lvlmask + DEBUG_L1 * (0x0001<<i);
		}

		/* mask off group bits, and combine them with the new level ones */
		debug_mask = (debug_mask & ~(DEBUG_VERBOSE)) | lvlmask;

		varptr->val = newval;
	}

	return OK;
}

/*------------------------------------------------------------------------
 *  set_dbgroup - sets the debug group env var and updates the global
 *  debug_mask variable to use the new corresponding value as it's 12
 *  least significant bits; this value controls the category of
 *  displayed debug messages; values include none, scheduler, memalloc,
 *  shell, rwb, and all
 *------------------------------------------------------------------------
 */
syscall set_dbgroup(struct envvar *varptr, uint32 newval) {

	uint16 grpmask = (uint16)newval;

	uint32 oldval = varptr->val;

	/* if a change was actually made */
	if(oldval != newval) {
		/* if < 2, then grpmask == newval */
		if(newval>2) {
			/* if newval is the highest value for this env var */
			/* then it must correspond to DEBUG_ALL (0xFFF) */
			if(newval>=(DBGROUP_STRNUM-1)) {
				grpmask = DEBUG_ALL;
			} else {
				/* else correct grpmask is newval-1 left shifts */
				grpmask = 1 << (newval-1);
			}
		}
		/* mask off the level bits and combine with new grpmask */
		debug_mask = (debug_mask & ~(DEBUG_ALL)) | grpmask;

		varptr->val = newval;
	}

	return OK;
}
