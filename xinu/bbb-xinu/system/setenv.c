/* setenv.c - setenv */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  setenv  -  Set environment variables that control dynamic changeable system parameters
 *------------------------------------------------------------------------
 */
syscall	setenv(uint32 var, uint32 val) {

	if(var < 0 || var > ENV_VARS) return SYSERR;

	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();
	// error checking in certain cases?
	environment[var] = val;

	if(!isempty(scratchlist)) {
		restore(mask);
		return SYSERR;
	}

	if(var == EV_SCHEDULER) {
		uint32 i,count;
		pid32 pid;

		count = readycount;
		for(i=0;i<count;i++) {
			pid32 tmppid = dequeue(readylist);
			readycount--;
			if(!isbadpid(tmppid)) {
				register struct procent *prptr = &proctab[tmppid];
				switch(environment[EV_SCHEDULER]) {

				case QTYPE_SJF:
					prptr->prprio = (uint16)((cputime() - prptr->statetimes[PR_CURR]) & MASK_32to16);
					break;

				case QTYPE_RAND:
					prptr->prprio = (rand() % 99) + 1;
					break;

				case QTYPE_PRIORITY:
					prptr->prprio = prptr->prprio0;
					break;
				}

				insert(tmppid, scratchlist, prptr->prprio);
				readycount++;
			}
		}
		queuecopy(scratchlist,readylist);
	}

	restore(mask);
	return OK;
}
