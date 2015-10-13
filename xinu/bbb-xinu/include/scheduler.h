/* ADDDD */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#ifndef PR_STATES
#define	PR_STATES		8
#endif

#ifndef QTYPE_VALS
#define	QTYPE_VALS 		3
#endif

#define QTYPE_INVALID 	-1
#define QTYPE_DEFAULT 	0
#define QTYPE_PRIORITY 	0
#define QTYPE_SJF 		1
#define QTYPE_RAND 		2

#define CPUQDATA_DEFAULT FALSE

#define MASK_32to16 0x0000FFFF

#define INT16(u) (u%32768)

#define zerotime(t) do{t.secs = 0; t.ms = 0;} while(0)
#define addtime(t1,t2) do{t1.secs += t2.secs + (t1.ms + t2.ms)/(60); t1.ms = (t1.ms + t2.ms)%60;} while(0)
#define negtime(t1,t2) do{if(t1.ms < t2.ms) {t1.secs--; t1.ms += 1000;} t1.secs -= t2.secs; t1.ms -= t2.ms; } while(0)
#define prttime(t1) (t1.secs * 1000 + t1.ms)
#define eqtime(t1,t2) ((t1.secs == t2.secs) && (t1.ms == t2.ms))

#define NOW (cputime())?(_mstime_now):(_mstime_now)

extern  qid16 	scratchlist;
extern  uint32  *clktimems;  /* ADDDD */
extern  mstime state_times[QTYPE_VALS][PR_STATES]; /* ADDDD */
extern  uint32  procs_finished[QTYPE_VALS]; /* ADDDD */
extern  uint16  readycount;
extern	mstime  _mstime_now;
extern	mstime	ztime;

#endif /* INCLUDE_SCHEDULER_H_ */
