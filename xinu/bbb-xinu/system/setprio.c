#include <xinu.h>
#include <limits.h>

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
	if(pid==NULLPROC) return prio;

	mstime timediff, tmptime;
	switch(envtab[EV_SCHEDULER].val) {

	case QTYPE_SJF:
		record_cpuqdata(pid);
		tmptime = proctab[pid].statetimes[QTYPE_SJF][PR_CURR];
		prio = SHRT_MAX - INT16(prttime(tmptime));

		if(prio>=(proctab[pid].prprio)) {
			LOG("\nsetprio(): QTYPE_SJF - old prio: %u, new prio: %u\n",proctab[pid].prprio,prio);
			LOG("  timediff = [%u, %u]  tmptime = [%u, %u]\n", timediff.secs, timediff.ms,tmptime.secs, tmptime.ms);
		}
		break;

	case QTYPE_RAND:
		prio = (rand() % 99) + 1;
		break;

	case QTYPE_PRIORITY:
		prio = proctab[pid].prprio0;
		break;
	}

	restore(mask);
	return INT16(prio);
}
