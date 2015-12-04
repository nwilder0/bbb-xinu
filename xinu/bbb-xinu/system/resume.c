/* resume.c - resume */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  resume  -  Unsuspend a process, making it ready
 *------------------------------------------------------------------------
 */
pri16	resume(
	  pid32		pid		/* ID of process to unsuspend	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	pri16	prio;			/* Priority to return		*/

	mask = disable();
	LOG2(DEBUG_INFO,DEBUG_SCHEDULER,"\nResume: mask disabled,now proceeding with error checking\n");
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16)SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prstate != PR_SUSP) {
		LOG2(DEBUG_ERROR, DEBUG_SCHEDULER, "\nResume: Error: pid is not in a PR_SUSP state\n");
		restore(mask);
		return (pri16)SYSERR;
	}
	LOG2(DEBUG_INFO,DEBUG_SCHEDULER,"\nResume: No errors, now proceeding with making process ready\n");
	prio = prptr->prprio;		/* Record priority to return	*/
	ready(pid);
	LOG2(DEBUG_INFO,DEBUG_SCHEDULER,"\nResume: process is ready, restoring mask and exiting function\n");
	restore(mask);
	return prio;
}
