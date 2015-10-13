/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	LOG("\nReady(): ready to record qdata; flag is %d\n",envtab[EV_CPUQDATA].val);
	record_cpuqdata(pid); /* ADDDD */
	prptr->prstate = PR_READY;
	readycount++;  /* ADDDD */

	/* ADDDD */
	prptr->prprio = setprio(pid);

	insert(pid, readylist, prptr->prprio);
	resched();

	return OK;
}
