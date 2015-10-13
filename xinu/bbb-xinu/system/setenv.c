/* setenv.c - setenv */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  setenv  -  Set environment variables that control dynamic system parameters
 *------------------------------------------------------------------------
 */
syscall	setenv(uint32 var, uint32 val) {

	LOG("\nEntering setenv step 1\n");
	if(var < 0 || var >= ENV_VARS) return SYSERR;

	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	struct envvar *varptr = &envtab[var];

	long int retval=OK;
	LOG("\nsetenv step 2\n");
	if((varptr->set) != NULL) {

		retval = varptr->set(varptr, val);

	} else {

		varptr->val = val;

	}

	restore(mask);
	return retval;
}
