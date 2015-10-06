/* ADDDD */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#define PR_STATES 8

#define QTYPE_DEFAULT 0
#define QTYPE_SJF 1
#define QTYPE_RAND 2

#define MASK_32to16 0x0000FFFF

extern  uint32  *clktimems;  /* ADDDD */
extern  uint32  state_times[PR_STATES]; /* ADDDD */
extern  uint32  procs_finished; /* ADDDD */
extern  uint16	scheduler; /* ADDDD */
extern  uint16  readycount;



#endif /* INCLUDE_SCHEDULER_H_ */
