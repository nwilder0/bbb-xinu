#include <xinu.h>

syscall clear_cpuqdata() {
	int32 i,j,k;

	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	for(i=0; i<NPROC; i++) {
		for(j=0; j<QTYPE_VALS; j++) {
			for(k=0; k<PR_STATES; k++) {
				if(proctab[i].prstate!=PR_FREE) {
					proctab[i].statetimes[j][k] = ztime;
				}
			}
		}
		proctab[i].timestatein = ztime;
	}

	for(i=0; i<QTYPE_VALS; i++) {
		for(j=0; j<PR_STATES; j++) {
			state_times[i][j] = ztime;
		}
		procs_finished[i] = 0;
	}


	restore(mask);

	return 0;
}
