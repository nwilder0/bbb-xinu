

#ifndef INCLUDE_ENVIRONMENT_H_
#define INCLUDE_ENVIRONMENT_H_

#define ENV_VARS 5

#define EV_MAX_NAME_LEN 30
#define EV_MAX_VAL_LEN 50

#define EV_VALUE_INVALID -1
#define EV_VALUE_NO 0
#define EV_VALUE_YES 1

#define EV_BLANK 0
#define EV_SCHEDULER 1
#define EV_CPUQDATA 2
#define EV_DEBUG 3
#define EV_DTIMER 4

#define DEBUG_DEFAULT 0
#define DTIMER_DEFAULT 0

#define LOG(...) if(envtab[EV_DEBUG].val) kprintf( __VA_ARGS__)

struct envvar {

	uint32 evid;
	char name[EV_MAX_NAME_LEN];
	uint32 valcount;
	char **vals;
	uint32 val;
	syscall (*set) (struct envvar *, uint32);

};

extern struct envvar envtab[];


#endif /* INCLUDE_ENVIRONMENT_H_ */
