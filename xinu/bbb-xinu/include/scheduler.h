/* scheduler.h - includes definitions related to scheduling algorithms */
/* 		as well as macros used for mstime operations 				   */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#ifndef PR_STATES
#define	PR_STATES		8
#endif

/* the number of scheduling algorithms */
#ifndef QTYPE_VALS
#define	QTYPE_VALS 		3
#endif

#define QTYPE_INVALID 	-1		/* placeholder for a bad value */
#define QTYPE_DEFAULT 	0		/* default is currently set to priority */
#define QTYPE_PRIORITY 	0		/* priority scheduling */
#define QTYPE_SJF 		1		/* shortest job first */
#define QTYPE_RAND 		2		/* random job next */

#define CPUQDATA_DEFAULT FALSE	/* turn off cpuqdata by default */

#define MASK_32to16 0x0000FFFF	/* used in troubleshooting 32 to 16 bit integer issues */

#define INT16(u) (u%32768)		/* macro to safely convert a larger time integer to a priority integer */

/* macros to allow basic operations on mstime structs */

/* set to zero */
#define zerotime(t) do{t.secs = 0; t.ms = 0;} while(0)
/* addition with assignment */
#define addtime(t1,t2) do{t1.secs += t2.secs + (t1.ms + t2.ms)/(60); t1.ms = (t1.ms + t2.ms)%60;} while(0)
/* negation with assignment */
#define negtime(t1,t2) do{if(t1.ms < t2.ms) {t1.secs--; t1.ms += 1000;} t1.secs -= t2.secs; t1.ms -= t2.ms; } while(0)
/* equality check */
#define eqtime(t1,t2) ((t1.secs == t2.secs) && (t1.ms == t2.ms))

/* combine the two integers into one for printing, risk of overflow will be smaller */
/* if this just includes state time deltas rather than absolute timestamps */
#define prttime(t1) (t1.secs * 1000 + t1.ms)

/* run cputime to get the current mstime and put the values into the global */
/* which will be returned for by value assignment */
#define NOW (cputime())?(_mstime_now):(_mstime_now)


extern  qid16 	scratchlist;						/* temporary list for moving processes to and from queues */
extern  uint32  *clktimems;  						/* pointer to current millisecond offset in clock interrupt handler */
extern  mstime state_times[QTYPE_VALS][PR_STATES];  /* global sums of state time counters */

/* processes that have already been killed (sorted by running scheduler at the time) */
/*  to allow for per process state time assessment */
extern  uint32  procs_finished[QTYPE_VALS];
/* tracker of number of process in the readyqueue, is easier than traversing the queue each time */
extern  uint16  readycount;
/* global to hold current time */
extern	mstime  _mstime_now;
/* global to hold zero value time struct */
extern	mstime	ztime;

#endif /* INCLUDE_SCHEDULER_H_ */
