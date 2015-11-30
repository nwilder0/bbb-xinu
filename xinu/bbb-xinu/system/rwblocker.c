#include <xinu.h>

syscall rwb_init(struct rwblocker *rwb)
{
	rwb->semrwb = semcreate();
	rwb->rwcount = 0;
	rwb->rwqueue = newqueue();

	return 0;
}


syscall rwb_trywrite()
{
	// behaves just like semaphore wait
	return 0;
}


syscall rwb_tryread(uint32 rwbnum)
{
	// a writer is active, then...
	waitrwb(rwbnum);
	// else
	return 0;
}

syscall waitrwb(uint32 rwbnum) {

	return 0;
}

syscall signalrwb(uint32 rwbnum) {

	// dec count
	// if count = 0 and writer in queue then pop queue and start writer
	// else pop all readers
	return 0;
}

