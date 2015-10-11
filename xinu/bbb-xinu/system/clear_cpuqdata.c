#include <xinu.h>

syscall clear_cpuqdata() {
	int32 i,j;

	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	for(i=0; i<NPROC; i++) {
		for(j=0; j<PR_STATES; j++) {
			proctab[i].statetimes[j] = 0;
		}
		proctab[i].timestatein = 0;
	}

	for(i=0; i<PR_STATES; i++) {
		state_times[i] = 0;
	}
	procs_finished = 0;


	restore(mask);

	return 0;
}
