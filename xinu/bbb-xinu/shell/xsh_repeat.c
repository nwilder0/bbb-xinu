/* xsh_repeat.c - xsh_repeat */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_repeat - echo the input from stdin until -exit is entered
 *------------------------------------------------------------------------
 */
shellcmd xsh_repeat(int nargs, char *args[])
{

	char *input[nargs];
	input[0] = "repeat\0";
	input[1] = NULL;
    char buffer[256];
    buffer[0] = '\0';
    int tmpargs = nargs;
    int i = 0;

    if (nargs > 1) {

    	input[0] = "repeat\0";
    	for(i=1; i<nargs; i++) input[i]=args[i];
        strncpy(buffer,input[1],strlen(input[1]));

    } else {
    	return -1;
    }

	while(strncmp(buffer,"-exit",5)) {

		if(input[1]) {

			// basically the same as the xsh_echo code here

			int32	i;

			if (tmpargs > 1) {
				printf("%s", input[1]);

				for (i = 2; i < tmpargs; i++) {
					printf(" %s", input[i]);
				}
			}
			printf("\n");

		}

		fgets(buffer,sizeof(buffer),stdin);

		// reset the 'args' and input array to use one string instead of many in the echo'ing steps
		input[1] = buffer;
		tmpargs = 2;

	}

    return 0;
}
