#include <xinu.h>

/* assumes interrupts are disabled */

syscall set_scheduler(struct envvar *varptr, uint32 newval) {

	uint32 i,count;

	if(!isempty(scratchlist)) {
		kprintf("\nerror: scratchlist is not empty\n");
		return SYSERR;
	}

	uint32 oldval = varptr->val;
	varptr->val = newval;

	if(oldval != newval) {
		count = readycount;
		for(i=0;i<count;i++) {
			pid32 tmppid = dequeue(readylist);
			readycount--;
			if(!isbadpid(tmppid)) {
				register struct procent *prptr = &proctab[tmppid];

				prptr->prprio = setprio(tmppid);

				insert(tmppid, scratchlist, prptr->prprio);
				readycount++;
			}
		}
		copyqueue(scratchlist,readylist);

		if(envtab[EV_CPUQDATA].val==EV_VALUE_YES) {
			for(i=0;i<NPROC;i++) {
				struct procent *prptr = &proctab[i];
				if(prptr->prstate != PR_FREE) record_cpuqdata(i);
			}
		}
	}

	return OK;
}


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
