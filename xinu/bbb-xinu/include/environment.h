/* environment.h - includes definitions related to environment variables */

#ifndef INCLUDE_ENVIRONMENT_H_
#define INCLUDE_ENVIRONMENT_H_

/* current number of vars defined */
#define ENV_VARS 9

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
#define EV_DBGROUP 7		/* controls LOG2 message display based on topical category */
#define EV_DBLEVEL 8		/* controls LOG2 message display based on severity */

/* default values for vars that do not have another header to be defined in */
#define DEBUG_DEFAULT 0
#define DTIMER_DEFAULT 0

#define DBGROUP_DEFAULT 0x0000	/* initial value is to display nothing */
#define DBLEVEL_DEFAULT 0x7000	/* initial value is info; will work only if group value set */

#define DBGROUP_STRNUM			6	/* number of different strings to use with the set cmd */
#define DBLEVEL_STRNUM			5	/* number of different strings to use with the set cmd */

#define DEBUG_NONE				0x0000	/* display nothing */
#define DEBUG_SCHEDULER 		0x0001	/* display messages related to CPU, procs, scheduling */
#define DEBUG_MEMALLOC	 		0x0002	/* display messages related to memory, allocation */
#define DEBUG_SHELL 			0x0004	/* display messages related to shell cmds/operation */
#define DEBUG_RWB 				0x0008  /* display messages related to rwb operation */
#define DEBUG_ALL				0x0FFF  /* display messages from all categories */
#define DEBUG_ERROR 			0x1000  /* display only error messages */
#define DEBUG_WARN		 		0x3000  /* display error, warning messages */
#define DEBUG_INFO 				0x7000  /* display error, warning, info messages */
#define DEBUG_VERBOSE 			0xF000  /* display error, warning, info, verbose messages */
#define DEBUG_L1				0x1000	/* levels not currently used */
#define DEBUG_L2				0x2000
#define DEBUG_L3				0x4000
#define DEBUG_L4				0x8000

/* set the provided bits in the debug mask */
#define set_dbmask(x)			(do {debug_mask = debug_mask | (uint16)x;} while(0))
/* clear the provided bits in the debug mask */
#define clr_dbmask(x)			(do {debug_mask = debug_mask & ~(uint16)x;} while(0))
/* check if the provided bits are set */
#define isset_dbmask(x)			((debug_mask & (uint16)x) == (uint16)x)

extern uint16 debug_mask;

#define MEMALLOC_FIRSTFIT 0
#define MEMALLOC_BESTFIT  1
#define MEMALLOC_DEFAULT  0

#define CMDHIST_DEFAULT	  5			/* default number of past commands to save */
#define CMDHIST_MAX		  100		/* max number of past commands that can be saved */

/* macro to write to stdin via kprintf some debugging message if env var EV_DEBUG is set to true */
#define LOG(...) if(envtab[EV_DEBUG].val) kprintf( __VA_ARGS__)
/* new version of LOG that supresses message if level, group bits are not on */
#define LOG2(lvl,grp,...) if(isset_dbmask((lvl+grp))) kprintf( __VA_ARGS__)

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
