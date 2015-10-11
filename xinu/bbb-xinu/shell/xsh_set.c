/* xsh_set.c - xsh_set */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_set - display or set environment / system variables
 *------------------------------------------------------------------------
 */
shellcmd xsh_set(int nargs, char *args[])
{
	int32	i;

	if(nargs==2) {
		if(!strncmp(args[1],"--help",6)) {
			printf("This command prints or sets the system environment values that are dynamically changeable.\n\n");
			printf("Display all values syntax: set\n");
			printf("Display one value syntax: set [setting name]\n");
			printf("Set one value syntax: set [setting name] = [setting value]\n\n");
			return OK;
		} else {
			uint32 ivar = -1;
			char strval[50];
			for(i=0;i<ENV_VARS;i++) {
				if(!strncmp(env_vars[i],args[1],25)) {
					ivar = i;
				}
			}

			if(ivar!=-1) {
				uint32 valcount = env_valcounts[ivar];
				uint32 ival = environment[ivar];

				if(valcount) {
					if((ival>=0) && (environment[ival]<valcount)) {
						strncpy(strval,env_vals[ivar][ival],50);
						strval[49]='\0';
					} else {
						strncpy(strval,"INVALID\0",8);
					}
					printf("\n  %s = %s (%u)\n\n", args[1], strval, ival);
				} else {
					printf("\n  %s = %u\n\n", args[1], ival);
				}
			}
		}
	}

	// set or...
	if(nargs == 4) {
		if(!strncmp(args[2],"=",1)) {
			char strvar[25], strval[50];
			strncpy(strvar,args[1],24);
			strvar[24]='\0';
			strncpy(strval,args[3],49);
			strval[49]='\0';

			uint32 ivar = -1;
			for(i=0;i<ENV_VARS;i++) {
				if(!strncmp(env_vars[i],strvar,25)) {
					ivar = i;
				}
			}

			if(ivar!=-1) {
				uint32 intval = atol(strval);
				uint32 valcount = env_valcounts[ivar];
				if(strncmp(strval,"0",1) && intval==0) intval = -1;

				if(valcount && (intval == -1)) {
					// loop thru and find the value
					for(i=0;i<valcount;i++) {
						if(!strncmp(strval,env_vals[ivar][i],50)) intval = i;
					}
				}

				if(intval < 0 || intval >= valcount) intval = -1;

				setenv(ivar,intval);

			}
		}
	}

	// display
	printf("\n");

	for(i=0; i<ENV_VARS; i++) {
		char strvar[25], strval[50];
		uint32 ival = environment[i];

		strncpy(strvar,env_vars[i],24);
		strvar[24]='\0';

		if((ival>env_valcounts[i])||(ival<0)) {
			strncpy(strval,"INVALID\0",8);
		} else {
			strncpy(strval,env_vals[i][ival],49);
		}
		printf("  %s = %s (%u)\n",strvar,strval,ival);
	}
	printf("\n");

	return OK;
}


