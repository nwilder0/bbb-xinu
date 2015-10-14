/* record_cpuqdata.c - record_cpuqdata */

#include <xinu.h>

mstime _mstime_now;

/*------------------------------------------------------------------------
 *  record_cpuqdata  - called when a process transitions from one state to
 *  another.  This function updates the appropriate process counter with
 *  the since the process entered the state (the old state that it is now
 *  leaving).  The function also adds the state time to the global totals
 *------------------------------------------------------------------------
 */
syscall record_cpuqdata(pid32 pid) {

	// if cpuqdata env var is off, then don't collect this data
	// unless the scheduler is in SJF mode, in that case it will
	// need this data to make a well formed decision on who is
	// monopolizing the cpu
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

		mstime timein = proctab[pid].timestatein;
		uint16 pstate = proctab[pid].prstate;

		LOG("\nrecord_cpuqdata step 2\n");

		/* note that NOW should not be directly modified as it is a global available to all procs */
		/* instead just use it in a by-value assignment */
		mstime timenow = NOW;

		/* as long as there is any time in the state start timestamp assume it is good */
		/* absolute zero is assumed to be an erroneous value, though, as it should only be attainable */
		/* prior to clkinit, at which time any code involving time may not behave predictably */
		if(timein.secs || timein.ms) {
			mstime tmptime = timenow;
			negtime(tmptime,timein);

			// highly I/O processes were consistently reporting 0 time (presumeably less than 1 ms) in PR_CURR,
			// and thus several of these processes always had the same priority under SJF
			// the fix is to require every call of record_cpuqdata to log at least 1 ms
			// and then the processes will deviate in priority (though maybe not greatly)
			if((tmptime.secs <= 0) && (tmptime.ms <= 0)) tmptime.ms = 1;

			LOG("\nProcess %u spent %u time in %s",pid,prttime(tmptime),envtab[EV_SCHEDULER].vals[schedval]);

			// update the counters here
			if(pid!=0) addtime(state_times[schedval][pstate],tmptime);
			mstime *pstatetime = &(proctab[pid].statetimes[schedval][pstate]);
			addtime((*pstatetime),tmptime);
		}

		// whether or not timein was a good value this time, update the timein for the next state
		proctab[pid].timestatein = timenow;
		restore(mask);
	}
	return OK;
}

/*----------------------------------------------------------------------------------
 *  cputime  -  return the current system time in milliseconds (via a mstime struct)
 *----------------------------------------------------------------------------------
 */
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
/*------------------------------------------------------------------------
 *  dtimer - outputs data similar to the 'ps' cmd to kprintf on a timer
 *  during the clock interrupt to increase the likelihood of reporting
 *  if a process has taken over the system
 *------------------------------------------------------------------------
 */
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
