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

	record_cpuqdata(pid);  /* call function to record process state time data */
							   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
	prptr->prstate = PR_READY;
	readycount++;  /* increase the readylist count tracker */

	/* assigns a new adjusted priority based on the scheduler currently active */
	prptr->prprio = setprio(pid);

	insert(pid, readylist, prptr->prprio);
	resched();

	return OK;
}
