/* clear_cpuqdata.c - clear_cpuqdata */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  clear_cpuqdata - reset all process state counters to 0, even if SJF
 *  scheduling is active, in which case all processes will revert to nearly
 *  even ground for a moment
 *------------------------------------------------------------------------
 */
syscall clear_cpuqdata() {
	int32 i,j,k;

	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	// for each time value in each counter in the 2D array in each process
	// set it to zero
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

	// set the global counters to zero as well
	for(i=0; i<QTYPE_VALS; i++) {
		for(j=0; j<PR_STATES; j++) {
			state_times[i][j] = ztime;
		}
		procs_finished[i] = 0;
	}

	restore(mask);

	return 0;
}
