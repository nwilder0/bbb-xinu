
#ifndef RWB_COUNT
#define RWB_COUNT 10
#endif

struct rwbentry {

	byte  rwstate;		/* Whether entry is S_FREE or S_USED	*/

	int32 rwcount;

	qid16 rwqueue;
	int32 qcount;

};

extern struct rwbentry rwbtab[];
extern int32 rwbflags[];

#define	isbadrwb(s)	((int32)(s) < 0 || (s) >= RWB_COUNT || rwbtab[s].rwstate == S_FREE)

