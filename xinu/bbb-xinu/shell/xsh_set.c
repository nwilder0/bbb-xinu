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

	/* if  1 arg and it is '--help' then print the help */
	if(nargs==2) {
		if(!strncmp(args[1],"--help",6)) {
			printf("This command prints or sets the system environment values that are dynamically changeable.\n\n");
			printf("Display all values syntax: set\n");
			printf("Display one value syntax: set [setting name]\n");
			printf("Set one value syntax: set [setting name] = [setting value]\n\n");
			return OK;
		/* else if one arg, then find and output just that env var */
		} else {
			uint32 ivar = -1;
			char strval[EV_MAX_VAL_LEN];
			for(i=0;i<ENV_VARS;i++) {
				if(!strncmp(envtab[i].name,args[1],EV_MAX_NAME_LEN)) {
					ivar = i;
				}
			}

			if(ivar!=-1) {
				uint32 valcount = envtab[ivar].valcount;
				uint32 ival = envtab[ivar].val;

				if(valcount) {
					if((ival>=0) && (ival<valcount)) {
						strncpy(strval,envtab[ivar].vals[ival],EV_MAX_VAL_LEN);
						strval[EV_MAX_VAL_LEN-1]='\0';
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

	// 4 args sounds like a set attempt
	// find the env var, find any alias and verify there's a equal sign in between
	// then if all is good, call setenv(var, val) to actually change things

	if(nargs == 4) {
		if(!strncmp(args[2],"=",1)) {
			LOG("\nSetting value of %s for %s\n",args[3],args[1]);
			char strvar[EV_MAX_NAME_LEN], strval[EV_MAX_VAL_LEN];
			strncpy(strvar,args[1],EV_MAX_NAME_LEN);
			strvar[EV_MAX_NAME_LEN-1]='\0';
			strncpy(strval,args[3],EV_MAX_VAL_LEN);
			strval[EV_MAX_VAL_LEN-1]='\0';

			uint32 ivar = -1;
			for(i=0;i<ENV_VARS;i++) {
				if(!strncmp(envtab[i].name,args[1],EV_MAX_NAME_LEN)) {
					ivar = i;
				}
			}

			LOG("\nivar = %u\n",ivar);
			if(ivar!=-1) {

				uint32 valcount = envtab[ivar].valcount;
				uint32 intval = atol(strval);

				if(valcount) {

					if(strncmp(strval,"0",1) && intval==0) {
						// loop thru and find the value
						for(i=0;i<valcount;i++) {
							if(!(strncmp(strval,envtab[ivar].vals[i],EV_MAX_VAL_LEN))) intval = i;
						}
					}

					if((intval < 0) || (intval >= valcount)) intval = -1;
				}
				LOG("\nnow setting setenv, intval = %u\n",intval);
				setenv(ivar,intval);

			}
		}
	}

	// no matter what parameters were provided lets display all the env vars
	// with their values
	printf("\n");

	for(i=0; i<ENV_VARS; i++) {
		char strvar[EV_MAX_NAME_LEN], strval[EV_MAX_VAL_LEN];
		uint32 ival = envtab[i].val;
		strval[0]='\0';

		strncpy(strvar,envtab[i].name,EV_MAX_NAME_LEN);
		strvar[EV_MAX_NAME_LEN-1]='\0';

		if((envtab[i].valcount<1) || !(envtab[i].vals)) {

		} else if((ival>envtab[i].valcount) || (ival<0)) {
			strncpy(strval,"INVALID\0",8);
		} else {
			strncpy(strval,envtab[i].vals[ival],EV_MAX_VAL_LEN);
			strval[EV_MAX_VAL_LEN-1]='\0';
		}
		printf("  %s = %u (%s)\n",strvar,ival,strval);
	}
	printf("\n");

	return OK;
}


