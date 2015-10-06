/* watchdog.c */

#include <xinu.h>

/*------------------------------------------------------------------------
 * wddisable  -  disable the watchdog timer on BeagleBone Black
 *------------------------------------------------------------------------
 */
void wddisable() {
        volatile struct am335x_wdt *wdt_ptr =
          (volatile struct am335x_wdt *)AM335X_WDT_ADDR;

        /* execute the start/stop sequence for watchdog timers
           according to AM335X TRM instructions in 20.4.3.8 */
        wdt_ptr->wdt_wspr = 0x0000AAAA;
        while (wdt_ptr->wdt_wwps & W_PEND_WSPR);
        wdt_ptr->wdt_wspr = 0x00005555;
        while (wdt_ptr->wdt_wwps & W_PEND_WSPR);
}
