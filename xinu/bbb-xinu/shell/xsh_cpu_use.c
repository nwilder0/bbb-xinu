#include <xinu.h>
#include <stdio.h>

/* ADDDD */

shellcmd xsh_cpu_use(int nargs, char *args[]) {
	int i,j,k;
	struct	procent	*prptr;
	char *pstate[]	= {		/* names for process states	*/
		"free ", "curr ", "ready", "recv ", "sleep", "susp ",
		"wait ", "rtime"};
	mstime statetimes[QTYPE_VALS][PR_STATES];
	uint32 existprocs = 0;

	if(envtab[EV_CPUQDATA].val == EV_VALUE_NO) {
		printf("\nCollection of CPU queue and process state data is not enabled.\n");
		printf("Type 'set cpuqdata = yes' to enable.\n\n");
		return 0;
	}

	if(nargs > 1) {
		if(!strncmp(args[1],"clear",5)) {
			clear_cpuqdata();
			printf("CPU Queue and Process State Data has been cleared.\n");
			return 0;
		}
	}


	for (i = 0; i<QTYPE_VALS; i++) {
		for(j=0; j<PR_STATES; j++) {
			statetimes[i][j]=state_times[i][j];
		}
	}

	if(nargs==1) {

		for(k=0;k<QTYPE_VALS;k++) {

			printf("\nScheduler: %s\n\n", envtab[EV_SCHEDULER].vals[k]);

			printf("%3s %-16s %5s %5s %10s %10s %10s %10s %10s \n",
				   "Pid", "Name", "State", "Prio", pstate[1],
				   pstate[2], pstate[3], pstate[4], pstate[6]);

			printf("%3s %-16s %5s %5s %10s %10s %10s %10s %10s \n",
				   "---", "----------------", "-----", "-----", "----------",
				   "----------", "----------", "----------", "----------");

			/* Output information for each process */

			for (i = 0; i < NPROC; i++) {
				prptr = &proctab[i];
				if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
					continue;
				}
				printf("%3d %-16s %s %5d ",
					i, prptr->prname, pstate[(int)prptr->prstate],
					prptr->prprio);
				for(j=1; j<PR_STATES; j++) {
					if(j!=5 && j!=7) {
						mstime tmptime = prptr->statetimes[k][j];
						if((prptr->prstate == j) && (envtab[EV_SCHEDULER].val == k)) {
							mstime tfrag = NOW;
							negtime(tfrag,prptr->timestatein);
							addtime(tmptime,tfrag);
						}
						printf("%10d ",prttime(tmptime));
					}
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("\n");


	} else if(!strncmp(args[1],"all\0",3)) {

		for (i = 0; i < NPROC; i++) {
			prptr = &proctab[i];
			if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
				continue;
			}
			existprocs++;
			printf("%3d %-16s %s %4d %4d 0x%08X 0x%08X %8d\n",
				i, prptr->prname, pstate[(int)prptr->prstate],
				prptr->prprio, prptr->prparent, prptr->prstkbase,
				prptr->prstkptr, prptr->prstklen);
			for(k=0; k<QTYPE_VALS; k++) {
				printf("  For %s scheduler\n",envtab[EV_SCHEDULER].vals[k]);
				printf("    Time spent in state: ");
				for(j=0; j<PR_STATES; j++) {
					mstime statetime = prptr->statetimes[k][j];
					if(j==prptr->prstate && k==envtab[EV_SCHEDULER].val) {
						mstime timefragment = NOW;
						mstime timein = prptr->timestatein;
						negtime(timefragment, timein);
						addtime(statetime, timefragment);
						if(i) addtime(statetimes[k][j], timefragment);
					}
					if(!(j%3)) printf("\n");
					printf("    %s: %8u  ",pstate[j],prttime(statetime));
				}
				printf("\n");
			}

			printf("\n\n");
		}

		printf("\n\nSystem CPU Totals: ");
		for(k=0; k<QTYPE_VALS; k++) {
			printf("\n For %s scheduler ",envtab[EV_SCHEDULER].vals[k]);
			for(i = 0; i<PR_STATES; i++) {
				if(!(i%4)) printf("\n");
				printf("%s: %u  ",pstate[i],prttime(statetimes[k][i]));
			}
			printf("\n");
		}

		printf("\n\nSystem CPU Averages per Process: ");
		printf("\n (processes existing = %u)",existprocs);
		for(k=0; k<QTYPE_VALS; k++) {
			printf("\n For %s scheduler ",envtab[EV_SCHEDULER].vals[k]);
			printf("\n  (processes finished = %u)",procs_finished[k]);
			for(i = 0; i<PR_STATES; i++) {
				if(!(i%4)) printf("\n");
				printf("%s: %u  ",pstate[i],prttime(statetimes[k][i])/(procs_finished[k]+existprocs-1));
			}
			printf("\n");
		}

	}

	return 0;
}
