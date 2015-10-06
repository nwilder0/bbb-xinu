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
	record_cpu(pid); /* ADDDD */
	prptr->prstate = PR_READY;
	readycount++;  /* ADDDD */

	/* ADDDD */
	switch(scheduler) {

	case QTYPE_SJF:
		prptr->prprio = (uint16)((cputime() - prptr->statetimes[PR_CURR]) & MASK_32to16);
		break;

	case QTYPE_RAND:
		prptr->prprio = (rand() % 99) + 1;
		break;

	case QTYPE_DEFAULT:
		break;
	}
	insert(pid, readylist, prptr->prprio);
	resched();

	return OK;
}
