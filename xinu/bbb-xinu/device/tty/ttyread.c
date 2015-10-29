/* ttyread.c - ttyread */

#include <xinu.h>

uint8 ttychkspecial(char *ch) {
	LOG("\n ttychkspecial: Checking char: %c", *ch);
	static uint8	chesc=0,arrow=0;

	switch(*ch) {
	case TY_ESC:
		chesc = 1;
		arrow = 0;

		break;

	case TY_ARROW:
		if(chesc) arrow = 1;
		else arrow = 0;

		break;

	case TY_UP:
		LOG("\n ttychkspecial: Found up arrow press \n");
		LOG("\n ttychkspecial: shellpid = %d, shell prstate = %d \n", shellpid, proctab[shellpid].prstate);
		LOG("\n ttychkspecial: arrow = %d, chesc = %d \n",arrow, chesc);
		if((arrow && chesc) && (shellpid && proctab[shellpid].prstate == PR_WAIT)) {
			arrow = 0;
			chesc = 0;
			LOG("\n ttychkspecial: Using up arrow press \n");
			return 1;
		}

		break;

	case TY_DOWN:
		LOG("\n ttychkspecial: Found down arrow press \n");
		if((arrow && chesc) && (shellpid && proctab[shellpid].prstate == PR_WAIT)) {
			arrow = 0;
			chesc = 0;
			LOG("\n ttychkspecial: Using down arrow press \n");
			return -1;
		}

		break;


	default:
		chesc = 0;
		arrow = 0;

		break;
	}
	return 0;
}

/*------------------------------------------------------------------------
 *  ttyread  -  Read character(s) from a tty device (interrupts disabled)
 *------------------------------------------------------------------------
 */
devcall	ttyread(
	  struct dentry	*devptr,	/* Entry in device switch table	*/
	  char	*buff,			/* Buffer of characters		*/
	  int32	count 			/* Count of character to read	*/
	)
{
	struct	ttycblk	*typtr;		/* Pointer to tty control block	*/
	int32	avail;			/* Characters available in buff.*/
	int32	nread;			/* Number of characters read	*/
	int32	firstch;		/* First input character on line*/
	char	ch;			/* Next input character		*/
	static struct strlist *tmpcmd = NULL;
	uint8	chkarrow=0;

	if(tmpcmd == NULL) { tmpcmd = cmdhistory;}
	if (count < 0) {
		return SYSERR;
	}
	typtr= &ttytab[devptr->dvminor];

	if (typtr->tyimode != TY_IMCOOKED) {

		/* For count of zero, return all available characters */

		if (count == 0) {
			avail = semcount(typtr->tyisem);
			if (avail == 0) {
				return 0;
			} else {
				count = avail;
			}
		}
		for (nread = 0; nread < count; nread++) {
			*buff++ = (char) ttygetc(devptr);
		}
		return nread;
	}

	/* Block until input arrives */

	firstch = ttygetc(devptr);

	/* Check for End-Of-File */

	if (firstch == EOF) {
		return EOF;
	}

	/* Read up to a line */

	ch = (char) firstch;
	*buff++ = ch;
	ttychkspecial(buff-1);

	nread = 1;
	while ( (nread < count) && (ch != TY_NEWLINE) &&
			(ch != TY_RETURN) ) {
		ch = ttygetc(devptr);
		*buff++ = ch;
		nread++;
		chkarrow = ttychkspecial(buff-1);
		if(chkarrow != 0) {

			if(tmpcmd != NULL && tmpcmd->str != NULL) {
				LOG("\nttyread: Erasing %d characters\n",nread);
				while(nread--) {
					ttyputc(devptr,typtr->tyierasec);
					buff--;
				}
				if(chkarrow == -1 && tmpcmd->next != NULL) {
					LOG("\nttyread: Doing down arrow\n");
					tmpcmd = tmpcmd->next;
					buff = tmpcmd->str;
					nread = strlen(tmpcmd->str);
				}

				if(chkarrow == 1 && tmpcmd->prev != NULL) {
					LOG("\nttyread: Doing up arrow\n");
					buff = tmpcmd->str;
					nread = strlen(tmpcmd->str);
					tmpcmd = tmpcmd->prev;
				}

				if(nread > 0) {
					LOG("\nttyread: Echoing historical cmd of length: %d\n", nread);
					ttywrite(devptr, buff, nread);
				}

			}

		}

	}
	return nread;
}

