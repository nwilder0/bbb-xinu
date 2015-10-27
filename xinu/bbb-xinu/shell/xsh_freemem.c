/* xsh_freemem.c - xsh_freemem */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_freemem - print out the list of free memory blocks
 *------------------------------------------------------------------------
 */

shellcmd xsh_freemem(int nargs, char *args[]) {

	printfreemem();

	return OK;
}
