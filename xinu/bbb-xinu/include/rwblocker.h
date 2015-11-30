
#ifndef INCLUDE_RWBLOCKER_H_
#define INCLUDE_RWBLOCKER_H_

#define RWB_COUNT 10

struct rwblocker {

	uint32 ptr_data;

	sid32 semrwb;

	uint32 rwcount;

	qid16 rwqueue;

};

struct rwbqentry {

	pid32 pid;

	uint8 is_writer;

};

extern struct rwbentry rwbtab[];

#endif /* INCLUDE_RWBLOCKER_H_ */
