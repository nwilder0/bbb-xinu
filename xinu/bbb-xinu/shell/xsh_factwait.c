/* xsh_factorial.c - xsh_factorial */

#include <xinu.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xsh_factorial - calculate the factorial of the provided integer (0-500)
 *------------------------------------------------------------------------
 */
shellcmd xsh_factwait(int nargs, char *args[])
{
	uint32 wait_t = 100; /* 100 ms default */

	uint32 n = 10;
	uint32 f = 1;
	uint32 i = 0;

	if(nargs>1) {
		wait_t = atol(args[1]);
		if(wait_t<1 || wait_t>(60*1000)) wait_t = 100;
	}

	while(1) {
		for(i=n;i>0;i--) f*=i;
		sleepms(wait_t);
	}

	return 0;
}
