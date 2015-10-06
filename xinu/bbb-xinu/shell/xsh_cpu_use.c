#include <xinu.h>
#include <stdio.h>

/* ADDDD */

shellcmd xsh_cpu_use(int nargs, char *args[]) {
	int i,j;
	struct	procent	*prptr;
	char *pstate[]	= {		/* names for process states	*/
		"free ", "curr ", "ready", "recv ", "sleep", "susp ",
		"wait ", "rtime"};
	uint32 statetimes[PR_STATES];
	for (i = 0; i<PR_STATES; i++) statetimes[i] = state_times[i];

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		printf("%3d %-16s %s %4d %4d 0x%08X 0x%08X %8d\n",
			i, prptr->prname, pstate[(int)prptr->prstate],
			prptr->prprio, prptr->prparent, prptr->prstkbase,
			prptr->prstkptr, prptr->prstklen);
		printf("    Time spent in state: ");
		for(j=0; j<PR_STATES; j++) {
			uint32 statetime = prptr->statetimes[j];
			if(j==prptr->prstate) {
				uint32 timefragment = cputime()-(prptr->timestatein);
				statetime += timefragment;
				if(i) statetimes[j] += timefragment;
			}
			if(!(j%4)) printf("\n");
			printf("    %s: %8d  ",pstate[j],statetime);
		}
		printf("\n\n");
	}

	printf("\n\nSystem CPU Totals: ");
	for(i = 0; i<PR_STATES; i++) {
		if(!(i%4)) printf("\n");
		printf("%s: %d  ",pstate[i],statetimes[i]);
	}
	printf("\n\nSystem CPU Averages per Process:");
	for(i = 0; i<PR_STATES; i++) {
		if(!(i%4)) printf("\n");
		printf("%s: %u  ",pstate[i],statetimes[i]/(1.0*(procs_finished+prcount-1)));
	}
	printf("\n");
	return 0;
}
