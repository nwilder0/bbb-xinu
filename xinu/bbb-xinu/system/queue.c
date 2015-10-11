/* queue.c - enqueue, dequeue */

#include <xinu.h>

struct qentry	queuetab[NQENT];	/* Table of process queues	*/

/*------------------------------------------------------------------------
 *  enqueue  -  Insert a process at the tail of a queue
 *------------------------------------------------------------------------
 */
pid32	enqueue(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q		/* ID of queue to use		*/
	)
{
	qid16	tail, prev;		/* Tail & previous node indexes	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	tail = queuetail(q);
	prev = queuetab[tail].qprev;

	queuetab[pid].qnext  = tail;	/* Insert just before tail node	*/
	queuetab[pid].qprev  = prev;
	queuetab[prev].qnext = pid;
	queuetab[tail].qprev = pid;
	return pid;
}

/*------------------------------------------------------------------------
 *  dequeue  -  Remove and return the first process on a list
 *------------------------------------------------------------------------
 */
pid32	dequeue(
	  qid16		q		/* ID queue to use		*/
	)
{
	pid32	pid;			/* ID of process removed	*/

	if (isbadqid(q)) {
		return SYSERR;
	} else if (isempty(q)) {
		return EMPTY;
	}

	pid = getfirst(q);
	queuetab[pid].qprev = EMPTY;
	queuetab[pid].qnext = EMPTY;
	return pid;
}

/*------------------------------------------------------------------------
 *  copyqueue  -  Move the internal queue linked list of pids from one
 *  queue to another by pointing the internal list at the other queue's
 *  first and last id's.  Any contents in the destination queue will be
 *  dequeued first, and the source queue head and tail will be linked to
 *  make the source queue empty.
 *------------------------------------------------------------------------
 */
void copyqueue(qid16 srcq, qid16 destq) {

	while(!isempty(destq)) dequeue(destq);

	qid16 firstid = firstid(srcq);
	qid16 lastid = lastid(srcq);
	firstid(srcq) = queuetail(srcq);
	lastid(srcq) = queuehead(srcq);

	firstid(destq) = firstid;
	queuetab[firstid].qprev = queuehead(destq);
	lastid(destq) = lastid;
	queuetab[lastid].qnext = queuetail(destq);
}
