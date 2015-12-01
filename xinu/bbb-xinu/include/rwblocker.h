
#ifndef INCLUDE_RWBLOCKER_H_
#define INCLUDE_RWBLOCKER_H_

#define RWB_COUNT 10

struct rwbentry {

	uint32 ptr_data;

	byte  rwstate;		/* Whether entry is S_FREE or S_USED	*/
	sid32 semrwb;

	int32 rwcount;
	int32 nextw;

	qid16 rwqueue;

};

struct rwbqentry {

	pid32 pid;

	uint8 is_writer;

};

extern struct rwbentry rwbtab[];
extern signed char rwbflags[];

#define	isbadrwb(s)	((int32)(s) < 0 || (s) >= RWB_COUNT)

#endif /* INCLUDE_RWBLOCKER_H_ */
