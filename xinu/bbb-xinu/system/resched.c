/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	//LOG("\nResched() step 1\n");
	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		/* ADDDD */

		ptold->prprio = setprio(currpid);

		if (ptold->prprio > firstkey(readylist)) {
			//LOG("\nResched(): top ps remains\n");
			return;
		}

		/* Old process will no longer remain current */
		LOG("\nResched(): ready to record qdata\n");

		record_cpuqdata(currpid);  /* ADDDD */
		ptold->prstate = PR_READY;
		readycount++;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */
	currpid = dequeue(readylist);
	LOG("\nResched(): new curr ps is pid = %d\n",currpid);
	readycount--;
	ptnew = &proctab[currpid];

	record_cpuqdata(currpid);  /* ADDDD */
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
