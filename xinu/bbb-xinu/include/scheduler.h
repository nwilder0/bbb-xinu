/* ADDDD */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#define PR_STATES 8

#define QTYPE_VALS 3

#define QTYPE_INVALID -1
#define QTYPE_DEFAULT 0
#define QTYPE_SJF 1
#define QTYPE_RAND 2

#define MASK_32to16 0x0000FFFF

#define ENV_VARS 5

#define EV_VALUE_INVALID -1
#define EV_BLANK 0
#define EV_SCHEDULER 1

extern char* env_vars[];
extern char* env_vals[][];
extern  uint32  *clktimems;  /* ADDDD */
extern  uint32  state_times[PR_STATES]; /* ADDDD */
extern  uint32  procs_finished; /* ADDDD */
extern  uint16  readycount;

extern uint32 environment[ENV_VARS];



#endif /* INCLUDE_SCHEDULER_H_ */
