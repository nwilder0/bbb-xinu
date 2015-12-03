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

	LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: starting\n");

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: deferring\n");
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */

		LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: current remains eligible\n");

		ptold->prprio = setprio(currpid); /* assign a new priority appropriate for the current scheduler */
		LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: current process got new priority of %d\n",ptold->prprio);

		if (ptold->prprio > firstkey(readylist)) {
			LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: current remains current\n");
			return;
		}

		/* Old process will no longer remain current */


		LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: recording CPUQ data, then putting the old proc back in the ready list\n");
		record_cpuqdata(currpid);  /* call function to record process state time data */
								   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
		ptold->prstate = PR_READY;
		readycount++;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */
	currpid = dequeue(readylist);
	LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: new current proc is %d\n",currpid);

	/* LOG("\nResched(): new curr ps is pid = %d\n",currpid); */
	readycount--;
	ptnew = &proctab[currpid];

	LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: recording CPUQ data (if active) then we'll reset the quantum\n");
	record_cpuqdata(currpid);  /* call function to record process state time data */
							   /* (actual recording is controlled by EV_CPUQDATA env var and choice of scheduler) */
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: last step: ARM context switch\n");
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */
	LOG2(DEBUG_INFO, DEBUG_SCHEDULER, "\nResched: returning\n");

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
