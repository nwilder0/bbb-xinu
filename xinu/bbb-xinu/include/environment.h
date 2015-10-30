/* environment.h - includes definitions related to environment variables */

#ifndef INCLUDE_ENVIRONMENT_H_
#define INCLUDE_ENVIRONMENT_H_

/* current number of vars defined */
#define ENV_VARS 7

/* max name and value (string alias) lengths */
#define EV_MAX_NAME_LEN 30
#define EV_MAX_VAL_LEN 50

/* standard integer values applicable to many different vars */
#define EV_VALUE_INVALID -1
#define EV_VALUE_NO 0
#define EV_VALUE_YES 1

/* the actual env vars, as identified by integer number */
#define EV_BLANK 0
#define EV_SCHEDULER 1
#define EV_CPUQDATA 2
#define EV_DEBUG 3
#define EV_DTIMER 4
#define EV_MEMALLOC 5		/* controls the memory allocation algorithm, first fit or best fit */
#define EV_CMDHIST 6		/* controls the storing of shell command history */

/* default values for vars that do not have another header to be defined in */
#define DEBUG_DEFAULT 0
#define DTIMER_DEFAULT 0

#define MEMALLOC_FIRSTFIT 0
#define MEMALLOC_BESTFIT  1
#define MEMALLOC_DEFAULT  1

#define CMDHIST_DEFAULT	  5			/* default number of past commands to save */
#define CMDHIST_MAX		  100		/* max number of past commands that can be saved */

/* macro to write to stdin via kprintf some debugging message if env var EV_DEBUG is set to true */
#define LOG(...) if(envtab[EV_DEBUG].val) kprintf( __VA_ARGS__)

/* struct that defines the table entries in the env vars table */
struct envvar {

	uint32 evid;
	char name[EV_MAX_NAME_LEN];
	uint32 valcount;	/* number of defined value aliases, if no aliases only integers then this should be 0 */
	char **vals;		/* array of strings of the value aliases */
	uint32 val;			/* the actual env var's integer value */
	/* function pointer to the custom set function if one exists */
	/* if one doesn't exist then setenv() will just set the val integer in the envvar entry */
	syscall (*set) (struct envvar *, uint32);

};

/* linked list of strings; used to store history of shell commands */
struct strlist {
	char *str;
	struct strlist *prev;
	struct strlist *next;
};

/* extern for the env vars table */
extern struct envvar envtab[];

extern struct strlist *cmdhistory;

#endif /* INCLUDE_ENVIRONMENT_H_ */
