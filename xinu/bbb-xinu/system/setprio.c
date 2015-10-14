/* setprio.c - setprio */

#include <xinu.h>
#include <limits.h>

/*------------------------------------------------------------------------
 *  setprio - this function encapsulates the code that determines a process
 *  priority; scheduling algorithm is of course the main determinant of
 *  how the priority will be determined; the priority is returned here
 *  rather than assigning it to the process entry within the function
 *------------------------------------------------------------------------
 */
syscall	setprio(
	  pid32		pid		/* Process ID			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	uint32	prio=0;			/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}
	// don't mess with the null process priority
	// or bad things will happen
	if(pid==NULLPROC) return prio;

	// declare SJF's time vars here since the declarations can't be done inside the case statement
	mstime timediff, tmptime;

	switch(envtab[EV_SCHEDULER].val) {

	case QTYPE_SJF:

		//record time in state even though no transition so that time already spent on the cpu
		//will count against the current process
		record_cpuqdata(pid);

		//after record take the time spent so far on the CPU
		tmptime = proctab[pid].statetimes[QTYPE_SJF][PR_CURR];

		//convert the time value into a non-negative 16-bit integer
		//and then subtract that cpu used value from the max to get
		//cpu unused, this is due to the ready queue putting largest
		//priority at the front of the queue
		prio = SHRT_MAX - INT16(prttime(tmptime));

		// keep an eye on increasing priority in debug mode as it was frequently a problem
		if(prio>=(proctab[pid].prprio)) {
			LOG("\nsetprio(): QTYPE_SJF - old prio: %u, new prio: %u\n",proctab[pid].prprio,prio);
			LOG("  timediff = [%u, %u]  tmptime = [%u, %u]\n", timediff.secs, timediff.ms,tmptime.secs, tmptime.ms);
		}
		break;

	case QTYPE_RAND:
		// give the process a random priority in this method
		prio = (rand() % 99) + 1;
		break;

	case QTYPE_PRIORITY:
		// in this method, just refresh the priority from the backup value
		prio = proctab[pid].prprio0;
		break;
	}

	restore(mask);
	// return the priority and make sure it will fit into a 16 bit integer
	return INT16(prio);
}
