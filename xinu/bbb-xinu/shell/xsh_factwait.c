/* xsh_factwait.c - xsh_factwait */

#include <xinu.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xsh_factwait - this shell command runs as an infinite loop with a sleep
 * timer specified by the cmd line parameter. A parameter of 0 will never
 * sleep (just actively loop), one with a parameter of 10 will sleep for
 * 10ms every loop.
 *------------------------------------------------------------------------
 */

shellcmd xsh_factwait(int nargs, char *args[])
{
	uint32 wait_t = 100; /* 100 ms default */

	uint32 n = 10;
	uint32 f = 1;
	uint32 i = 0;

	// check the parameter and make sure it makes sense
	if(nargs>1) {
		wait_t = atol(args[1]);
		if(wait_t<0 || wait_t>(60*1000)) wait_t = 100;
	}

	/* start the loop */
	while(1) {
		for(i=n;i>0;i--) f*=i;
		if(wait_t) sleepms(wait_t);
	}

	return 0;
}
