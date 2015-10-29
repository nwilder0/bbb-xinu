/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
int32 cpudelay;

pid32 shellpid = NULL;

volatile uint32	gcounter = 400000000;
process	counterproc() {

	while(gcounter > 0) {
		gcounter--;
	}
	return OK;
}

syscall reshell()
{
	kprintf("\n\nattempting restore of shell..\n\n");
	send(shellpid,(umsg32)NULLPROC);
	return OK;
}

process	main(void)
{
	/* Start the network */

	//ethinit(&devtab[ETHER0]);

	uint32	*macaddr = 0x44e10630;
	kprintf("mac addr: %x %x\n", *macaddr, *(macaddr+1));
	int32 i;
/*	for(i = 0; i < 6; i++) {
		NetData.ethucast[i] = i;
		NetData.ethbcast[i] = 0xff;
	}*/

	//kprintf("reading packet from ETHER0\n");
	//struct netpacket pkt;
	//read(ETHER0, &pkt, 1518);

	//resume(create(counterproc, 8192, 19, "counter proc", 0, NULL));
	//sleep(10);
	//kprintf("Counter value: %d\n", gcounter);

	//DELAY(5000000);
	//netstart();

	kprintf("\n...creating a shell\n");
	recvclr();
	shellpid = create(shell, 8192, 50, "shell", 1, CONSOLE);
	resume(shellpid);

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		shellpid = create(shell, 4096, 20, "shell", 1, CONSOLE);
		resume(shellpid);
	}
	return OK;
}
