/* clkhandler.c - clkhandler */

#include <xinu.h>

/*-----------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *-----------------------------------------------------------------------
 */

uint32 *clktimems=NULL; /* a pointer to the static millisecond tracker inside the clock handler */

void	clkhandler()
{
	static uint32 count1000 = 1000;	/* variable to count 1000ms */

	volatile struct am335x_timer1ms *csrptr = 0x44E31000;
					/* Pointer to timer CSR	    */

	/* If there is no interrupt, return */

	if((csrptr->tisr & AM335X_TIMER1MS_TISR_OVF_IT_FLAG) == 0) {
		return;
	}

	LOG2(DEBUG_INFO,DEBUG_SCHEDULER,
			"\nClkInt: a clock tick is being handled, ms was %d, secs were %d\n", 1000-count1000,clktime);

	/* Acknowledge the interrupt */

	csrptr->tisr = AM335X_TIMER1MS_TISR_OVF_IT_FLAG;

	/* Decrement 1000ms counter */

	count1000--;

	/* After 1 sec, increment clktime */

	if(count1000 == 0) {
		clktime++;
		count1000 = 1000;

		/* if EV_DTIMER env var is turned on then run the associated debugging output on a psuedo timer */
		if(envtab[EV_DTIMER].val && !(clktime%(envtab[EV_DTIMER].val))) {
			dtimer();
		}
	}

	/* if still NULL, update the pointer to the millisecond tracker so millisecond timestamps can be generated */
	if(!clktimems) {
		clktimems = &count1000;
	}

	/* check if sleep queue is empty */

	if(!isempty(sleepq)) {

		/* sleepq nonempty, decrement the key of */
		/* topmost process on sleepq		 */

		if((--queuetab[firstid(sleepq)].qkey) == 0) {

			wakeup();
		}
	}

	/* Decrement the preemption counter */
	/* Reschedule if necessary	    */

	if((--preempt) == 0) {
		preempt = QUANTUM;
		resched();
	}
}
