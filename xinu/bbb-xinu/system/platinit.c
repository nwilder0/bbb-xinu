/* platinit.c - platinit */

#include <xinu.h>

/*------------------------------------------------------------------------
 * platinit - platform specific initialization for BeagleBone Black
 *------------------------------------------------------------------------
 */
void	platinit(void)
{

	struct	uart_csreg *uptr;	/* address of UART's CSRs	*/
	struct  dentry *devptr;

	//kprintf("initintc()\n");
	/* Initialize the Interrupt Controller */

	initintc();

	/* Pad control for CONSOLE */
	//kprintf("padctl()\n");
	am335x_padctl(UART0_PADRX_ADDR,
			AM335X_PADCTL_RXTX | UART0_PADRX_MODE);
	am335x_padctl(UART0_PADTX_ADDR,
			AM335X_PADCTL_TX | UART0_PADTX_MODE);

	/* Reset the UART device */
	//kprintf("uart reset\n");
	//kprintf("CONSOLE = %d; NPROC = %d, devtab = %x \n", CONSOLE, NPROC, devtab);
	//kprintf("proctab = %x, devtab[0] = %x\n", proctab, &devtab[0]);
	devptr = &devtab[CONSOLE];
	uptr = (struct uart_csreg *) devptr->dvcsr;
	//kprintf("about to do soft reset()\n");
//	uptr->sysc |= UART_SYSC_SOFTRESET;
	//kprintf("wait for uart reset()\n");
//	while((uptr->syss & UART_SYSS_RESETDONE) == 0);

	//kprintf("returning from platinit()\n");
}
