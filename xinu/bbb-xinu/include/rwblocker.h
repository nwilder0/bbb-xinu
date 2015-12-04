/* rwblocker.h - includes definitions related to read-write blockers - isbadrwb */


/* number of read-write blockers to allocate on the system */
#ifndef RWB_COUNT
#define RWB_COUNT 10
#endif

/* main struct for read-write blockers, analogous to sentry with semaphores */
struct rwbentry {

	byte  rwstate;		/* Whether entry is S_FREE or S_USED	*/

	int32 rwcount;		/* count of active readers (> 0) or active writer (-1) */

	qid16 rwqueue;		/* xinu queue object */
	int32 qcount;		/* count of items in rwqueue */

};

extern struct rwbentry rwbtab[];	/* table of available/used read-write blockers */
extern int32 rwbflags[];			/* per process flags to indicate reader/writer */

/* as with isbadsem, does basic error checking on the ID */
#define	isbadrwb(s)	((int32)(s) < 0 || (s) >= RWB_COUNT || rwbtab[s].rwstate == S_FREE)

