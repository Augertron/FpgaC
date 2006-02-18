/* $Id$ */

/*
 * PROJECT  : TM-1<->LCD
 * MODULE   : sun.c -- TOYC
 * VERSION  : $Revision$ $Date$
 * AUTHOR(S): David Galloway, James Duhault
 * CREATED  : 18 July 1994
 *
 * DESCRIPTION:
 *
 * This is a tmcc program to run a SEIKO G321 LCD module in BLACK&WHITE
 * mode, i.e., 1 bit per pixel.  It is 320x240.
 *
 * It is part of a multi-threaded tmcc circuit.  This thread talks to
 * the SUN, and allows it to read from and write to the frame buffer in
 * the sram.  The SRAM is managed by a different thread.
 *
 * LOCKED BY: $Locker:  $
 *
 */
 
/*
 * MODIFICATION HISTORY:
 *
 *
 * Revision 1.1  94/09/16  09:59:11  drg
 * Initial revision
 * 
 * Revision 1.8  94/07/22  11:32:46  drg
 * Fix comments.
 * 
 * Revision 1.7  94/07/22  11:29:24  drg
 * Put the sram access into a routine, and call it three times.  Cuts the
 * size of the circuit from 885 luts to 688.  A very big incrementer, I
 * guess.
 * 
 * Revision 1.6  94/07/22  11:14:23  drg
 * Add P1_PUTBYTES to put a stream of bytes.
 * 
 * Revision 1.5  94/07/22  10:20:25  drg
 * James' modifications to support the Seiko G321 320x240 display.  Seems
 * to work.
 * 
 * Revision 1.3  94/07/21  17:00:34  duhault
 * Added autoincrement of addresses during read/write operations to increase
 * speed during sequential multibyte accesses -- it is now only necessary
 * to set the address once.
 * 
 * Revision 1.2  94/07/21  12:38:40  duhault
 * First pass... seems to work according to analysis using the Logic analyzer
 * Almost a duplicate of the DMF666 code but the Seiko does not have the
 * display split into upper and lower halves.
 * 
 *
 */

/*
 * Communication between the SRAM and sun threads is handled through port
 * variables that have no pins, ie PORT_PIN is not set
 */

/* The interface from the SUN to this circuit */
#include "sunlcd.h"

/* The interface to the SRAM thread */
#include "sramsun.h"

/*
 * Address 0 in the SRAM is the top left corner of the screen, and addresses
 * increase to the right and down the screen.
 */

/* the SUN <-> TM-1 ports */
int sun_p0:12, sun_p1:12, sun_p2:12;


#define TICK	while(0) {}

/* Ask the sram thread to do a read or a write operation.  You must
 * set sram_sun_write appropriately before calling this routine.
 */

do_sram_access()
	{
	sram_sun_request = 1;
	sram_fromsun = (sun_p2 & P2_FROMSUNMASK);
	while(!sram_sun_done)
		;
	sram_sun_request = 0;
	/* increment the address automatically for fast */
	/* SRAM writes/reads using the SUN. i.e. this   */
	/* way we do not have to keep setting the       */
	/* address for sequential multibyte accesses    */
	sram_sun_address = sram_sun_address + 1;
	}

/*
 * FUNCTION: main()
 *
 *  The main function!
 *
 */

main()
	{
	int sunflag:1, myflag:1;
	char tosun;
	int latchcommand:12;
	int addressdiff:20;
	int latchaddress:20;

	/* Connections to the I/O board leading to the SUN
	 * sun_p1 is put through a register first, because
	 * the SUN is on a different clock than us, and we
	 * may have races unless everyone sees sun_p1&P1_GO
	 * change at the same time.
	 * Similarly for sun_p2, and the P2_HS_SUN and P2_STOP flags.
	 */
#pragma	fpgac_outputport(sun_p0)
#pragma	fpgac_inputport(sun_p1)
#pragma	fpgac_inputport(sun_p2)
#pragma	fpgac_portflags(sun_p1, PORT_REGISTERED_AND_PIN)
#pragma	fpgac_portflags(sun_p2, PORT_REGISTERED_AND_PIN)

	/* Connections to the SRAM thread - call fpgac_portflags to turn off PORT_PIN
	 */
#pragma	fpgac_outputport(sram_sun_address)
#pragma	fpgac_outputport(sram_fromsun)
#pragma	fpgac_outputport(sram_sun_request)
#pragma	fpgac_outputport(sram_sun_write)
#pragma	fpgac_inputport(sram_tosun)
#pragma	fpgac_inputport(sram_sun_done)
#pragma	fpgac_portflags(sram_sun_address,	PORT_REGISTERED)
#pragma	fpgac_portflags(sram_fromsun,		PORT_REGISTERED)
#pragma	fpgac_portflags(sram_sun_request,	PORT_REGISTERED)
#pragma	fpgac_portflags(sram_sun_write,	PORT_REGISTERED)
#pragma	fpgac_portflags(sram_tosun,		0)
#pragma	fpgac_portflags(sram_sun_done,	0)

	while(1) {
		sun_p0 = P0_READY;

		/* Wait for the SUN to tell us to do something
		 */
		while(!(sun_p1 & P1_GO))
			;
		latchcommand = (sun_p1 & P1_COMMASK);
		latchaddress = ((sun_p1&P1_TOPADDRMASK)<<7)
				| (sun_p2&P2_LOWADDRMASK);

		sun_p0 = 0;	/* Turn off P0_READY */

		if((latchcommand&P1_COMMASK) == P1_SETADDR) {
				sram_sun_address = latchaddress;
			}
		else if((latchcommand&P1_COMMASK) == P1_PUTDATA) {
			sram_sun_write = 1;
			do_sram_access();
			}
		else if((latchcommand&P1_COMMASK) == P1_PUTBYTES) {
			/* This is a faster way of transferring a stream of
			 * data from the SUN to the sram.  The SUN puts
			 * one byte at a time into sun_p2.  With each new
			 * byte, it inverts the P2_HS_SUN hand shaking line.
			 * This circuit waits for that bit to change, then
			 * sends the byte to the sram thread to be stored.
			 * It then inverts the P0_HS_TM1 handshaking line
			 * to tell the SUN that it has accepted the byte
			 * and the cycle continues.  The process ends when
			 * the sun sets P2_STOP.
			 */
			myflag = 0;
			while(!(sun_p2 & P2_STOP)) {
				sunflag = ((sun_p2 & P2_HS_SUN) != 0);
				sram_sun_write = 1;
				do_sram_access();
				myflag = !myflag;
				if(myflag)
					sun_p0 = P0_HS_TM1;
				else
					sun_p0 = 0;
				while(!(sun_p2 & P2_STOP)
					&& (sunflag ==
						((sun_p2 & P2_HS_SUN) != 0)))
					;
				}
			}
		else if((latchcommand&P1_COMMASK) == P1_GETDATA) {
			sram_sun_write = 0;
			do_sram_access();
			}

		TICK;

		/* Tell the SUN we are done */
		sun_p0 = (sram_tosun & P0_TOSUNMASK) | P0_DONE;

		/* Wait for the SUN to see the DONE signal, and drop GO
		 */
		while(sun_p1 & P1_GO)
			;
	}
} /* end of main() */

