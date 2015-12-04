/* xsh_rwb_test.c - rwb_test_init, rwb_add_tester */

#include <xinu.h>
#include <stdlib.h>

/* the two arrays that serve as shared data elements */
int32 shared_data1[10];
int32 shared_data2[10];

/* array of the two corresponding rwb IDs assigned to each of the arrays */
rwb32 rwbtestid[2];

/*------------------------------------------------------------------------
 * xsh_rwb_test_init - get ready testing rwb by running rwbcreate and
 * storing the RWB IDs in the rwbtestid array
 *------------------------------------------------------------------------
 */
shellcmd xsh_rwb_test_init(int nargs, char *args[])
{
	int i;

	for(i=0; i<10; i++) {
		shared_data1[i]=0;
		shared_data2[i]=0;
	}

	rwbtestid[0] = rwbcreate(0);

	rwbtestid[1] = rwbcreate(0);

	printf("\nread-write blocker 1 created with ID: %d",rwbtestid[0]);
	printf("\nread-write blocker 2 created with ID: %d\n", rwbtestid[1]);

	/* print rwb state debug output, to check everything before actual readers/writers are run */
	print_rwb_debug();

	return OK;
}

/*------------------------------------------------------------------------
 * xsh_rwb_test_stop - delete the two test RWBs
 *------------------------------------------------------------------------
 */
shellcmd xsh_rwb_test_stop(int nargs, char *args[])
{
	int32 retval1, retval2;

	retval1 = rwbdelete(rwbtestid[0]);
	retval2 = rwbdelete(rwbtestid[1]);

	return retval1+retval2;
}

/*------------------------------------------------------------------------
 * xsh_rwb_add_tester - takes one parameter, which if 'w' makes process a
 * writer, otherwise, it's a reader; this function runs infinitely looping
 * in and out of each of the two test RWBs; random work time of 2-8 secs
 * is done in the shared data area each loop, plus any wait on the
 * trywrite or tryread calls in each loop.
 *------------------------------------------------------------------------
 */
shellcmd xsh_rwb_add_tester(int nargs, char *args[]) {

	signed char is_writer = 1;
	int32 r,j;
	int32 wtime;
	int32 ridx;
	int32 read_num;

	// check the parameter and make sure it makes sense
	// rwb_test [processes per blocker] [how many are writers]

	if(nargs>1) {
		if(!strncmp(args[1],"w",2)) {
			is_writer = -1;
		}
	}

	while(1) {

		// randomly select 0 or 1 for which array and which rwb
		r = rand() % 2;
		// randomly select a work time from 2 to 8 secs
		wtime = (rand()%6+2)*1000;

		if(rwbtab[rwbtestid[r]].rwstate==S_FREE) return SYSERR;

		if(is_writer == -1) {
			LOG2(DEBUG_INFO,DEBUG_RWB,"\nPID: %d - writer about to try to enter the work loop\n",currpid);
			rwb_trywrite(rwbtestid[r]);
			LOG2(DEBUG_INFO,DEBUG_RWB,"\nPID: %d - starting work loop\n",currpid);
			for(j=0; j<5; j++) {
				//chose a random index of the shared_data to work on
				ridx = rand() % 10;
				if(r) shared_data2[ridx] = shared_data2[ridx] + wtime/1000;
				else shared_data1[ridx] = shared_data1[ridx] + wtime/1000;

				LOG2(DEBUG_INFO,DEBUG_RWB,
						"\nPID: %d - Wrote to shared_data %d: index %d; added value of %d\n",
						currpid,r,ridx,(wtime/1000));
				sleepms(wtime/5);
			}
		} else {
			LOG2(DEBUG_INFO,DEBUG_RWB,"\nPID: %d - reader about to try to enter the work loop\n",currpid);
			rwb_tryread(rwbtestid[r]);
			LOG2(DEBUG_INFO,DEBUG_RWB,"\nPID: %d - starting work loop\n",currpid);
			for(j=0; j<5; j++) {
				// chose random index
				ridx = rand() % 10;
				if(r) read_num = shared_data2[ridx];
				else read_num = shared_data1[ridx];
				LOG2(DEBUG_INFO,DEBUG_RWB,
						"\nPID: %d - Read value of %d from shared_data %d at index %d \n",
						currpid,read_num,r,ridx);
				sleepms(wtime/5);
			}
		}
		LOG2(DEBUG_INFO,DEBUG_RWB,"\nPID: %d - work loop finished, signaling\n",currpid);
		signalrwb(rwbtestid[r]);

	}

	return OK;
}
