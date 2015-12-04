/* shell.c  -  shell */

#include <xinu.h>
#include <stdio.h>
#include "shprototypes.h"

/************************************************************************/
/* Table of Xinu shell commands and the function associated with each	*/
/************************************************************************/
const	struct	cmdent	cmdtab[] = {
	{"argecho",	TRUE,	xsh_argecho},
	{"arp",		FALSE,	xsh_arp},
	{"cat",		FALSE,	xsh_cat},
	{"clear",	TRUE,	xsh_clear},
	{"date",	FALSE,	xsh_date},
	{"devdump",	FALSE,	xsh_devdump},
	{"echo",	FALSE,	xsh_echo},
	{"exit",	TRUE,	xsh_exit},
	{"help",	FALSE,	xsh_help},
	{"ipaddr",	FALSE,	xsh_ipaddr},
	{"kill",	TRUE,	xsh_kill},
	{"memdump",	FALSE,	xsh_memdump},
	{"memstat",	FALSE,	xsh_memstat},
	{"ping",	FALSE,	xsh_ping},
	{"ps",		FALSE,	xsh_ps},
	{"sleep",	FALSE,	xsh_sleep},
	{"udp",		FALSE,	xsh_udpdump},
	{"udpecho",	FALSE,	xsh_udpecho},
	{"udpeserver",	FALSE,	xsh_udpeserver},
	{"uptime",	FALSE,	xsh_uptime},
	{"?",		FALSE,	xsh_help},
	{"factorial", FALSE, xsh_factorial},
	{"repeat", FALSE, xsh_repeat},
	{"cpu_use", FALSE, xsh_cpu_use},
	{"factwait", FALSE, xsh_factwait},
	{"set",FALSE, xsh_set},
	{"rwb_test_init",FALSE, xsh_rwb_test_init},
	{"rwb_add_tester",FALSE, xsh_rwb_add_tester},
	{"rwb_test_init",FALSE, xsh_rwb_test_stop}
};

uint32	ncmd = sizeof(cmdtab) / sizeof(struct cmdent);

struct strlist *cmdhistory = NULL;

/*------------------------------------------------------------------------
 *  cmdhistoryinit  -  initializes the list that will store previously
 *  issued commands
 *------------------------------------------------------------------------
 */
void cmdhistoryinit() {

	uint32 histval = envtab[EV_CMDHIST].val;
	int i;

	LOG("\n cmdhistoryinit: starting \n");
	if(histval > 0 && histval <= CMDHIST_MAX) {

		LOG("\n cmdhistoryinit: alloc mem for %d cmds \n", histval);
		cmdhistory = (struct strlist *)getmem(sizeof(struct strlist)*histval);
		if(cmdhistory == -1) {
			cmdhistory = NULL;
		} else {
			struct strlist *tmpcmdhist = cmdhistory;
			LOG("\n cmdhistoryinit: *tmpcmdhist starts at addr: %x \n",tmpcmdhist);
			struct strlist *next=NULL;
			tmpcmdhist->next = NULL;
			tmpcmdhist->prev = NULL;
			tmpcmdhist->str = NULL;

			LOG("\n cmdhistoryinit: setting initial vals, loop \n");
			for(i = 1; i<histval; i++) {
				LOG("\n cmdhistoryinit: for loop %d: set tmp->str (%x) to NULL \n", i, &(tmpcmdhist->str));
				tmpcmdhist->str = NULL;
				LOG("\n cmdhistoryinit: for loop %d: set tmp->next (%x) to next ptr (val: %x) \n",
						 i, &(tmpcmdhist->next),next);
				tmpcmdhist->next = next;
				LOG("\n cmdhistoryinit: for loop %d: set tmp->prev (%x) to tmp+12 (val: %x) \n",
						 i, &(tmpcmdhist->prev),tmpcmdhist + 1);
				tmpcmdhist->prev = tmpcmdhist + 1;
				LOG("\n cmdhistoryinit: for loop %d: set next to tmp (%x) \n", i, tmpcmdhist);
				next = tmpcmdhist;
				LOG("\n cmdhistoryinit: for loop %d: set tmp to tmp->prev (%x) (val: %x) \n",
						 i, &(tmpcmdhist->prev),tmpcmdhist->prev);
				tmpcmdhist = tmpcmdhist->prev;
			}
			LOG("\n cmdhistoryinit: for loop %d: set tmp->next (%x) to next ptr (val: %x) \n",
					 i, &(tmpcmdhist->next),next);
			tmpcmdhist->next = next;
			LOG("\n cmdhistoryinit: for loop %d: set tmp->str (%x) to NULL \n", i, &(tmpcmdhist->str));
			tmpcmdhist->str = NULL;
			LOG("\n cmdhistoryinit: for loop %d: set tmp->prev (%x) to NULL \n", i, &(tmpcmdhist->prev));
			tmpcmdhist->prev = NULL;
		}
	}
	LOG("\n cmdhistoryinit: finished \n");
}

/*------------------------------------------------------------------------
 *  cmdhistorydel  -  free the list that hold the shell command history
 *------------------------------------------------------------------------
 */
void cmdhistorydel() {

	struct strlist *tmpcmdhist = cmdhistory;

	if(cmdhistory!=NULL) {
		LOG("\n cmdhistorydel: starting \n");
		while(tmpcmdhist != NULL) {
			if(tmpcmdhist->str != NULL) {
				LOG("\n cmdhistorydel: freeing str \n");
				freemem(tmpcmdhist->str,strlen(tmpcmdhist->str)+1);
				tmpcmdhist->str = NULL;
			}
			tmpcmdhist = tmpcmdhist->prev;
		}

		LOG("\n cmdhistorydel: freeing cmd list \n");

		freemem((char *)cmdhistory, sizeof(struct strlist)*(envtab[EV_CMDHIST].val));
		cmdhistory = NULL;
	}
	LOG("\n cmdhistorydel: finished \n");
}

/*------------------------------------------------------------------------
 *  cmdhistoryadd  -  add a cmd to the history
 *------------------------------------------------------------------------
 */
void cmdhistoryadd(char *buf, uint32 len) {

	struct strlist *tmpcmd = cmdhistory;
	int i = 0;

	LOG("\n cmdhistoryadd: starting \n");

	char *newcmd = getmem(len+1);
	strncpy(newcmd,buf,len);
	*(newcmd + len) = '\0';

	char *prevstr = NULL;

	LOG("\n cmdhistoryadd: set tmpcmd to cmdhistory (val: %x) \n",cmdhistory);
	LOG("\n cmdhistoryadd: alloc newcmd (%x) to len: %d and copy in str: %s \n",newcmd,len+1,newcmd);

	LOG("\n cmdhistoryadd: loop \n");

	/* loop through the history list, bumping each cmd string to a previous slot */
	while(tmpcmd != NULL && newcmd != NULL) {
		LOG("\n cmdhistoryadd: for loop %d: set prevstr to tmpcmd->str (%x) (val: %x) \n",
				i, &(tmpcmd->str), tmpcmd->str);
		LOG("\n \t tmp->str: str val = %s", tmpcmd->str);
		prevstr = tmpcmd->str;
		LOG("\n cmdhistoryadd: for loop %d: set tmp->str (%x) to newcmd (val: %x) \n",
				i, &(tmpcmd->str), newcmd);
		LOG("\n \t newcmd: str val = %s", newcmd);
		tmpcmd->str = newcmd;
		LOG("\n cmdhistoryadd: for loop %d: set newcmd to prevstr (val: %x) \n",
				i, prevstr);
		LOG("\n \t prevstr: str val = %s", prevstr);
		newcmd = prevstr;
		LOG("\n cmdhistoryadd: for loop %d: set tmpcmd to tmpcmd->prev (%x) (val: %x) \n",
				i, &(tmpcmd->prev), tmpcmd->prev);
		tmpcmd = tmpcmd->prev;
		LOG("\n cmdhistoryadd: loop iteration %d complete",i++);
	}

	/* if one cmd string fell off the back of the list, then free it */
	if(newcmd) {
		LOG("\n cmdhistoryadd: about to free last used cmd \n");
		LOG("\n cmdhistoryadd: str size is %d bytes", strlen(newcmd)+1);
		LOG("\n cmdhistoryadd: str ptr address is %x", newcmd);
		LOG("\n cmdhistoryadd: str val is %s", newcmd);
		if(envtab[EV_DEBUG].val == EV_VALUE_YES) sleepms(500);
		freemem(newcmd,strlen(newcmd)+1);
	}
	LOG("\n cmdhistoryadd: finished \n");
}




/************************************************************************/
/* shell  -  Provide an interactive user interface that executes	*/
/*	     commands.  Each command begins with a command name, has	*/
/*	     a set of optional arguments, has optional input or		*/
/*	     output redirection, and an optional specification for	*/
/*	     background execution (ampersand).  The syntax is:		*/
/*									*/
/*		   command_name [args*] [redirection] [&]		*/
/*									*/
/*	     Redirection is either or both of:				*/
/*									*/
/*				< input_file				*/
/*			or						*/
/*				> output_file				*/
/*									*/
/************************************************************************/

process	shell (
		did32	dev		/* ID of tty device from which	*/
	)				/*   to accept commands		*/
{
	char	buf[SHELL_BUFLEN];	/* Input line (large enough for	*/
					/*   one line from a tty device	*/
	int32	len;			/* Length of line read		*/
	char	tokbuf[SHELL_BUFLEN +	/* Buffer to hold a set of	*/
			SHELL_MAXTOK];  /* Contiguous null-terminated	*/
					/* Strings of tokens		*/
	int32	tlen;			/* Current length of all data	*/
					/*   in array tokbuf		*/
	int32	tok[SHELL_MAXTOK];	/* Index of each token in	*/
					/*   array tokbuf		*/
	int32	toktyp[SHELL_MAXTOK];	/* Type of each token in tokbuf	*/
	int32	ntok;			/* Number of tokens on line	*/
	pid32	child;			/* Process ID of spawned child	*/
	bool8	backgnd;		/* Run command in background?	*/
	char	*outname, *inname;	/* Pointers to strings for file	*/
					/*   names that follow > and <	*/
	did32	stdinput, stdoutput;	/* Descriptors for redirected	*/
					/*   input and output		*/
	int32	i;			/* Index into array of tokens	*/
	int32	j;			/* Index into array of commands	*/
	int32	msg;			/* Message from receive() for	*/
					/*   child termination		*/
	int32	tmparg;			/* Address of this var is used	*/
					/*   when first creating child	*/
					/*   process, but is replaced	*/
	char	*src, *cmp;		/* Pointers used during name	*/
					/*   comparison			*/
	bool8	diff;			/* Was difference found during	*/
					/*   comparison			*/
	char	*args[SHELL_MAXTOK];	/* Argument vector passed to	*/
					/*   builtin commands		*/

	/* Print shell banner and startup message */

	fprintf(dev, "\n\n%s%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
		SHELL_BAN0,SHELL_BAN1,SHELL_BAN2,SHELL_BAN3,SHELL_BAN4,
		SHELL_BAN5,SHELL_BAN6,SHELL_BAN7,SHELL_BAN8,SHELL_BAN9);

	fprintf(dev, "%s\n\n", SHELL_STRTMSG);

	cmdhistoryinit();

	/* Continually prompt the user, read input, and execute command	*/

	while (TRUE) {

		/* Display prompt */

		fprintf(dev, SHELL_PROMPT);

		/* Read a command */

		LOG("\n Shell: about to read \n");
		len = read(dev, buf, sizeof(buf));

		srand(cputime());	/* ADDDDD */

		/* Exit gracefully on end-of-file */

		if (len == EOF) {
			break;
		}

		/* If line contains only NEWLINE, go to next line */

		if (len <= 1) {
			continue;
		}

		LOG("\n Shell: adding cmd of len %d \n",len);
		cmdhistoryadd(buf, len);

		buf[len] = SH_NEWLINE;	/* terminate line */

		/* Parse input line and divide into tokens */


		ntok = lexan(buf, len, tokbuf, &tlen, tok, toktyp);

		/* Handle parsing error */

		if (ntok == SYSERR) {
			fprintf(dev,"%s\n", SHELL_SYNERRMSG);
			continue;
		}

		/* If line is empty, go to next input line */

		if (ntok == 0) {
			fprintf(dev, "\n");
			continue;
		}

		/* If last token is '&', set background */

		if (toktyp[ntok-1] == SH_TOK_AMPER) {
			ntok-- ;
			tlen-= 2;
			backgnd = TRUE;
		} else {
			backgnd = FALSE;
		}


		/* Check for input/output redirection (default is none) */

		outname = inname = NULL;
		if ( (ntok >=3) && ( (toktyp[ntok-2] == SH_TOK_LESS)
				   ||(toktyp[ntok-2] == SH_TOK_GREATER))){
			if (toktyp[ntok-1] != SH_TOK_OTHER) {
				fprintf(dev,"%s\n", SHELL_SYNERRMSG);
				continue;
			}
			if (toktyp[ntok-2] == SH_TOK_LESS) {
				inname =  &tokbuf[tok[ntok-1]];
			} else {
				outname = &tokbuf[tok[ntok-1]];
			}
			ntok -= 2;
			tlen = tok[ntok];
		}


		if ( (ntok >=3) && ( (toktyp[ntok-2] == SH_TOK_LESS)
				   ||(toktyp[ntok-2] == SH_TOK_GREATER))){
			if (toktyp[ntok-1] != SH_TOK_OTHER) {
				fprintf(dev,"%s\n", SHELL_SYNERRMSG);
				continue;
			}
			if (toktyp[ntok-2] == SH_TOK_LESS) {
				if (inname != NULL) {
				    fprintf(dev,"%s\n", SHELL_SYNERRMSG);
				    continue;
				}
				inname = &tokbuf[tok[ntok-1]];
			} else {
				if (outname != NULL) {
				    fprintf(dev,"%s\n", SHELL_SYNERRMSG);
				    continue;
				}
				outname = &tokbuf[tok[ntok-1]];
			}
			ntok -= 2;
			tlen = tok[ntok];
		}

		/* Verify remaining tokens are type "other" */

		for (i=0; i<ntok; i++) {
			if (toktyp[i] != SH_TOK_OTHER) {
				break;
			}
		}
		if ((ntok == 0) || (i < ntok)) {
			fprintf(dev, SHELL_SYNERRMSG);
			continue;
		}

		stdinput = stdoutput = dev;

		/* Lookup first token in the command table */

		for (j = 0; j < ncmd; j++) {
			src = cmdtab[j].cname;
			cmp = tokbuf;
			diff = FALSE;
			while (*src != NULLCH) {
				if (*cmp != *src) {
					diff = TRUE;
					break;
				}
				src++;
				cmp++;
			}
			if (diff || (*cmp != NULLCH)) {
				continue;
			} else {
				break;
			}
		}

		/* Handle command not found */

		if (j >= ncmd) {
			fprintf(dev, "command %s not found\n", tokbuf);
			continue;
		}

		/* Handle built-in command */

		if (cmdtab[j].cbuiltin) { /* No background or redirect. */
			if (inname != NULL || outname != NULL || backgnd){
				fprintf(dev, SHELL_BGERRMSG);
				continue;
			} else {
				/* Set up arg vector for call */

				for (i=0; i<ntok; i++) {
					args[i] = &tokbuf[tok[i]];
				}

				/* Call builtin shell function */

				if ((*cmdtab[j].cfunc)(ntok, args)
							== SHELL_EXIT) {
					break;
				}
			}
			continue;
		}

		/* Open files and redirect I/O if specified */

		if (inname != NULL) {
			stdinput = open(NAMESPACE,inname,"ro");
			if (stdinput == SYSERR) {
				fprintf(dev, SHELL_INERRMSG, inname);
				continue;
			}
		}
		if (outname != NULL) {
			stdoutput = open(NAMESPACE,outname,"w");
			if (stdoutput == SYSERR) {
				fprintf(dev, SHELL_OUTERRMSG, outname);
				continue;
			} else {
				control(stdoutput, F_CTL_TRUNC, 0, 0);
			}
		}

		/* Spawn child thread for non-built-in commands */
		LOG("\n Shell: creating child 4 cmd \n");

		child = create(cmdtab[j].cfunc,
			SHELL_CMDSTK, SHELL_CMDPRIO,
			cmdtab[j].cname, 2, ntok, &tmparg);

		/* If creation or argument copy fails, report error */

		if ((child == SYSERR) ||
		    (addargs(child, ntok, tok, tlen, tokbuf, &tmparg)
							== SYSERR) ) {
			fprintf(dev, SHELL_CREATMSG);
			continue;
		}

		/* Set stdinput and stdoutput in child to redirect I/O */

		proctab[child].prdesc[0] = stdinput;
		proctab[child].prdesc[1] = stdoutput;

		msg = recvclr();
		resume(child);
		if (! backgnd) {
			msg = receive();
			while (msg != child && msg != NULLPROC) {
				msg = receive();
			}
		}
    }

    /* Terminate the shell process by returning from the top level */

    fprintf(dev,SHELL_EXITMSG);

	LOG("\n Shell: cmdhistory delete \n");

    cmdhistorydel();

    return OK;
}

