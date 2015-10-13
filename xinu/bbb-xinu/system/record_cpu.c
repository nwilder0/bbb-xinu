#include <xinu.h>

/* assume interrupts disabled ADDDD */

mstime _mstime_now;

syscall record_cpuqdata(pid32 pid) {

	if(envtab[EV_CPUQDATA].val || envtab[EV_SCHEDULER].val == QTYPE_SJF) {

		intmask		mask;		/* Saved interrupt mask		*/
		mask = disable();

		LOG("\nEntering record_cpuqdata, pid = %u\n",pid);
		if(isbadpid(pid)) {
			restore(mask);
			return SYSERR;
		}

		uint32 schedval = envtab[EV_SCHEDULER].val;
		LOG("\nrecord_cpuqdata step 1a, schedval = %u\n",schedval);

		//if((schedval <= EV_VALUE_INVALID) || (schedval >= QTYPE_VALS)) return SYSERR;

		mstime timein = proctab[pid].timestatein;
		uint16 pstate = proctab[pid].prstate;

		LOG("\nrecord_cpuqdata step 2\n");

		//if(pstate <= PR_FREE  || pstate >= PR_STATES) return SYSERR;

		//LOG("\nrecord_cpuqdata step 3\n");
		mstime timenow = NOW;
		if(timein.secs || timein.ms) {
			mstime tmptime = timenow;
			negtime(tmptime,timein);
			if((tmptime.secs <= 0) && (tmptime.ms <= 0)) tmptime.ms = 1;
			LOG("\nProcess %u spent %u time in %s",pid,prttime(tmptime),envtab[EV_SCHEDULER].vals[schedval]);
			if(pid!=0) addtime(state_times[schedval][pstate],tmptime);
			mstime *pstatetime = &(proctab[pid].statetimes[schedval][pstate]);
			addtime((*pstatetime),tmptime);
		}
		proctab[pid].timestatein = timenow;
		restore(mask);
	}
	return OK;
}

uint8 cputime() {
	LOG("\nClock time: %u\n",clktime);
	_mstime_now.secs = clktime;
	if(clktimems) {
		_mstime_now.ms = 1000 - *clktimems;
		return TRUE;
	} else {
		_mstime_now.ms = 0;
		return FALSE;
	}
}

void dtimer() {

	struct	procent	*prptr;		/* pointer to process		*/
	int32	i;

	char *pstate[]	= {		/* names for process states	*/
			"free ", "curr ", "ready", "recv ", "sleep", "susp ",
			"wait ", "rtime"};

	kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s\n",
		   "Pid", "Name", "State", "Prio", "Ppid", "Stack Base",
		   "Stack Ptr", "Stack Size");

	kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s\n",
		   "---", "----------------", "-----", "----", "----",
		   "----------", "----------", "----------");

	/* Output information for each process */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		kprintf("%3d %-16s %s %4u %4d 0x%08X 0x%08X %8d\n",
			i, prptr->prname, pstate[(int)prptr->prstate],
			prptr->prprio, prptr->prparent, prptr->prstkbase,
			prptr->prstkptr, prptr->prstklen);
	}



}
