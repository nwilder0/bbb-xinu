/* watchdog.h */

struct am335x_wdt {
        uint32  wdt_widr;        /* watchdog identification register    */
        uint32  res1[3];         /* reserved                            */
        uint32  wdt_wdsc;        /* watchdog system control register    */
        uint32  wdt_wdst;        /* watchdog status register            */
        uint32  wdt_wisr;        /* watchdog interrupt status register  */
        uint32  wdt_wier;        /* watchdog interrupt enable register  */
        uint32  res2[1];         /* reserved                            */
        uint32  wdt_wclr;        /* watchdog control register           */
        uint32  wdt_wcrr;        /* watchdog counter register           */
        uint32  wdt_wldr;        /* watchdog load register              */
        uint32  wdt_wtgr;        /* watchdog trigger register           */
        uint32  wdt_wwps;        /* watchdog write posting bits reg.    */
        uint32  res3[3];         /* reserved.                           */
        uint32  wdt_wdly;        /* watchdog delay configuration reg.   */
        uint32  wdt_wspr;        /* watchdog start/stop register        */
        uint32  res4[2];         /* reserved                            */
        uint32  wdt_wirqstatraw; /* watchdog raw interrupt status reg.  */
        uint32  wdt_wirqstat;    /* watchdog interrupt status register  */
        uint32  wdt_wirqenset;   /* watchdog interrupt enable set reg.  */
        uint32  wdt_wirqenclr;   /* Watchdog Interrupt Enable Clear Reg.*/
};

#define AM335X_WDT_ADDR 0x44E35000  /* Watchdog reg base address        */
#define W_PEND_WSPR     0x00000010  /* Write pending for reg WSPR       */
