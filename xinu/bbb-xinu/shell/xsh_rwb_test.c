#include <xinu.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xsh_factwait - this shell command runs as an infinite loop with a sleep
 * timer specified by the cmd line parameter. A parameter of 0 will never
 * sleep (just actively loop), one with a parameter of 10 will sleep for
 * 10ms every loop.
 *------------------------------------------------------------------------
 */

int32 shared_data1[10];
int32 shared_data2[10];

rwb32 rwbtestid[2];

shellcmd xsh_rwb_test(int nargs, char *args[])
{
	uint32 max_read_time = 8000;		// in ms, so 10 secs
	uint32 write_time    = 5000;		// writers take 5 secs
	int i;

	for(i=0; i<10; i++) {
		shared_data1[i]=0;
		shared_data2[i]=0;
	}

	rwbtestid[0] = rwbcreate(0);

	rwbtestid[1] = rwbcreate(0);




	printf("\nread-write blocker 1 created with ID: %d",rwbtestid[0]);
	printf("\nread-write blocker 2 created with ID: %d", rwbtestid[1]);

	print_rwb_debug();

	return OK;
}

shellcmd xsh_add_rwb_tester(int nargs, char *args[]) {

	unsigned char is_writer = 1;
	int32 r,j;
	int32 wtime;
	int32 ridx;
	int32 read_num;

	// check the parameter and make sure it makes sense
	// rwb_test [processes per blocker] [how many are writers]

	if(nargs>1) {
		if(!strncmp(args[1],"w",1)) {
			is_writer = -1;
		}
	}

	while(1) {

		// randomly select 0 or 1 for which array and which rwb
		r = rand() % 2;
		// randomly select a work time from 2 to 8 secs
		wtime = (rand()%6+2)*1000;

		if(is_writer == -1) {
			printf("\nPID: %d - writer about to try to enter the work loop\n",currpid);
			rwb_trywrite(rwbtestid[r]);
			printf("\nPID: %d - starting work loop\n",currpid);
			for(j=0; j<5; j++) {
				//chose a random index of the shared_data to work on
				ridx = rand() % 10;
				if(r) shared_data2[ridx] = shared_data2[ridx] + wtime/1000;
				else shared_data1[ridx] = shared_data1[ridx] + wtime/1000;

				printf("\nPID: %d - Wrote to shared_data %d: index %d; added value of %d\n",currpid,r,ridx,(wtime/1000));
				sleepms(wtime/5);
			}
		} else {
			printf("\nPID: %d - reader about to try to enter the work loop\n",currpid);
			rwb_tryread(rwbtestid[r]);
			printf("\nPID: %d - starting work loop\n",currpid);
			for(j=0; j<5; j++) {
				// chose random index
				ridx = rand() % 10;
				if(r) read_num = shared_data2[ridx];
				else read_num = shared_data1[ridx];
				printf("\nPID: %d - Read value of %d from shared_data %d at index %d \n",currpid,read_num,r,ridx);
				sleepms(wtime/5);
			}
		}
		printf("\nPID: %d - work loop finished, signaling\n",currpid);
		signalrwb(rwbtestid[r]);

	}

	return OK;
}
