/* setenv.c - setenv */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  setenv  -  Set environment variables that control dynamic system parameters
 *------------------------------------------------------------------------
 */
syscall	setenv(uint32 var, uint32 val) {

	// if the var value is invalid return error
	if(var < 0 || var >= ENV_VARS) return SYSERR;

	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	struct envvar *varptr = &envtab[var];

	long int retval=OK;

	// if the custom set function exists, then use it
	if((varptr->set) != NULL) {

		// capture the env var's set function return value for better error detection
		retval = varptr->set(varptr, val);

	} else {

		//otherwise just do a simple integer assignment
		varptr->val = val;

	}

	restore(mask);
	return retval;
}
