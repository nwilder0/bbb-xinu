#include <xinu.h>

/* assume interrupts disabled ADDDD */

syscall record_cpuqdata(pid32 pid) {

	if(!(proctab[pid].timestatein)) {
		if(pid!=0) state_times[proctab[pid].prstate] += (cputime() - (proctab[pid].timestatein));
		proctab[pid].statetimes[proctab[pid].prstate] += (cputime() - (proctab[pid].timestatein));
	}
	proctab[pid].timestatein = cputime();

	return 0;
}

uint32 cputime() {
	if(clktimems)
		return ((clktime+1)*1000 - *clktimems);
	else
		return 0;
}


