/* $Id$ */

/*
 * PROJECT  : TM-1<->LCD
 * MODULE   : lcd.c -- TOYC
 * VERSION  : $Revision$ $Date$
 * AUTHOR(S): David Galloway, James Duhault
 * CREATED  : 18 July 1994
 *
 * DESCRIPTION:
 *
 * This is a tmcc program to run a SEIKO G321 LCD module in BLACK&WHITE
 * mode, i.e., 1 bit per pixel.  It is 320x240.
 *
 * It is one thread of a multi-threaded program which copies data
 * to the LCD from the SRAM.  It gets data from the SRAM thread.
 *
 * LOCKED BY: $Locker:  $
 *
 */
 
/*
 * MODIFICATION HISTORY:
 *
 *
 * Revision 1.6  94/10/11  16:50:04  drg
 * Add more TICKs.  Display is still unhappy.
 * 
 * Revision 1.5  94/10/04  17:24:36  drg
 * Not working at all well at 10 MHz.  See if voltage stepping working.
 * 
 * Revision 1.4  94/09/23  15:16:19  drg
 * Change the timing sensitive parts to try out the 10 MHz crystal.
 * 
 * Revision 1.3  94/09/16  12:10:58  drg
 * Another try at fixing the flm pulse.  The previous one made the first
 * line very faint, since it didn't get the delay the others do.
 * 
 * Revision 1.2  94/09/16  10:36:14  drg
 * 
 * Delete lcd_m handling, since G321 doesn't look at it.  Fix lcd_flm
 * signal to come at the end of the first scanline, rather than the last.
 * 
 * Revision 1.1  94/09/16  09:59:08  drg
 * Initial revision
 * 
 * Revision 1.21  94/07/22  10:19:46  drg
 * James' modifications to support the Seiko G321 320x240 display.  Seems
 * to work.
 * 
 * Revision 1.2  94/07/21  12:37:18  duhault
 * First pass... seems to working according to analysis using the Logic analyzer
 * Operation is very close to the DMF666 code.
 * 
 *
 */

/* The interface to the LCD and the power control board */

/* These signals are all driven by this program.  All are negative edge
 * triggered.
 */
int lcd_flm:1;		/* Start of frame */
int lcd_lp:1;		/* End of scan line */
int lcd_cp:1;		/* Data clock */
int lcd_m:1;		/* Alternating 0 and 1, changes at top of frame */
			/* Note lcd_m not used by G321 */
int lcd_dispon:1;	/* Display on (1)/off (0) */
int lcd_data:4;		/* Data */

/* Power control  board */

/* TM-1 -> POWCON-MAX716EV control bits */
int max716ev_on:1, max716ev_v4on:1, max716ev_v6on:1, max716ev_v6step:1;
int max716ev_v5on:1, max716ev_v7on:1;

/* The interface between this thread and the SRAM thread */
#include "sramlcd.h"

#define NROWS	240
#define NCOLS	320

/* this display does 4 sequential pixels at a time
 * Address 0 in the SRAM is the top left corner of the screen, and addresses
 * increase to the right and down the screen.
 */

#define TICK	while(0) {}

/* Byte address of pixels currently being transferred to LCD */
int scan_addr:20;

/*
 * FUNCTION: doscanline()
 *
 *  'draws' one scan line
 *
 */

doscanline()
	{
	int stage:4;
	int column:10;
	long scan_data, new_scan_data;

	/* Connections to the LCD */
#pragma	fpgac_outputport(lcd_data)
#pragma	fpgac_outputport(lcd_cp)

	/* Connections to the SRAM thread.  Use fpgac_portflags to turn off
	 * PORT_PIN.  All communications are done through special port
	 * variables that do not have pins.
	 */
#pragma	fpgac_outputport(sram_lcd_address)
#pragma	fpgac_inputport(sram_tolcd)
#pragma	fpgac_outputport(sram_lcd_request)
#pragma	fpgac_inputport(sram_lcd_done)
#pragma	fpgac_portflags(sram_lcd_address, PORT_REGISTERED)
#pragma	fpgac_portflags(sram_lcd_request, PORT_REGISTERED)
#pragma	fpgac_portflags(sram_tolcd,	0)
#pragma	fpgac_portflags(sram_lcd_done,0)

	/* Read 32 bits from the RAMS to get the first set of pixels for
	 * this line.
	 */
	sram_lcd_address = scan_addr;
	sram_lcd_request = 1;
	scan_addr = scan_addr + 4;
	while(!sram_lcd_done)
		;
	new_scan_data = sram_tolcd;
	sram_lcd_request = 0;

	/* Copy an entire scanline of data from the SRAM to the LCD.
	 * Each time around the outer loop, we:
	 *	a) copy 32 columns of pixels from SRAM to LCD
	 *	b) read 32 bits from the SRAM, to use the next time around
	 */

	column = NCOLS;
	while(column != 0) {

		/* This display accepts 4 columns of pixels per access
		 * We do eight accesses in each loop
		 */
		column = column - 32;

		/* Take the 32 bits we read from the SRAM the last time
		 * around this loop, and start writing it to the LCD
		 */
		scan_data = new_scan_data;

		/* the 32 bits are written in 8 steps so loop */
		stage = 8;

		/* Meanwhile, start reading the next 32 bits */
                /* from the SRAM                             */

		sram_lcd_address = scan_addr;
		sram_lcd_request = 1;
		scan_addr = scan_addr + 4;

		while (stage != 0) {

			/* raise the LCD clock */
			lcd_cp = 1;

			/* get the 4 MSB bits and then shift them out    */
			/* of the original scan data to prepare for the  */
			/* next round -- the pixels are stored such that */
			/* MSB = leftmost pixel, LSB = rightmost pixel   */
			/* and we are scanning from left to right        */
			lcd_data = (scan_data >> 28) & 0xF;
			scan_data = scan_data << 4;

			/* The LCD data has a 80 ns setup time requirement, 
                 	 * so we'll double TICK here to be sure.
		 	 */
			TICK;
			TICK;
			TICK;

			/* Clock the LCD data */
			lcd_cp = 0;

			/* Now an 80 ns hold requirement on the LCD data */
			TICK;
			TICK;

			stage = stage - 1;
		}

		/* Finish the SRAM read */
		new_scan_data = sram_tolcd;
		sram_lcd_request = 0;

	}

	/* fix up the memory pointer which has gone too far */
	scan_addr = scan_addr - 4;

} /* end of doscanline() */



/*
 * FUNCTION: doframe()
 *
 * 'draws' one complete frame
 *
 */

doframe()
{
	int row:10;
	int delay:9;

#pragma	fpgac_outputport(lcd_lp)

	/* Set memory address pointer */
	scan_addr = 0;

	/* set frame pulse.  Will be set to zero at end of first scanline */
	lcd_flm = 1;

	row = NROWS;

	while(row != 0) {

		/* This display does 1 scanline at a time. */
		row = row - 1;

		doscanline();

		/* Indicate end of scanline
		 * Hold it for a while, since it seems to have a slow
		 * rise time
		 */

		lcd_lp = 1;
		TICK;
		TICK;
		TICK;
		lcd_lp = 0;
		TICK;
		TICK;
		TICK;
		lcd_flm = 0;

		/* this is a delay needed at the end of each scanline   */
		/* to slow things down a bit when using the external    */
		/* 10 MHz oscillator					*/
		/* It is necessary as the Seiko has specific scanline   */
		/* and frame rate timing requirements.  See Seiko docs  */
		delay = 90;
		while (delay != 0)
			delay = delay - 1;
		}


} /* end of doframe() */


/*
 * FUNCTION: main()
 *
 *  The main function!
 *
 */

main()
	{
	int count1:10, count2:10;

#pragma	fpgac_outputport(lcd_flm)
#pragma	fpgac_outputport(lcd_lp)
#pragma	fpgac_outputport(lcd_cp)
#pragma	fpgac_outputport(lcd_dispon)

	/* set up interface to power control board */

#pragma	fpgac_outputport(max716ev_on)
#pragma	fpgac_outputport(max716ev_v4on)
#pragma	fpgac_outputport(max716ev_v5on)
#pragma	fpgac_outputport(max716ev_v6on)
#pragma	fpgac_outputport(max716ev_v7on)
#pragma	fpgac_outputport(max716ev_v6step)

	/* activate the board */
	max716ev_on = 1;
	TICK;
	TICK;

	/* enable Vcc = +5V i.e. V4 then wait */
	max716ev_v4on = 1;

	/* Wait for approx. 10 ms. to ensure that supply is stable
	 * Assume a 10 MHz crystal.  100000 / 10000000 = .01
	 */
	count1 = 100;
	while(count1 != 0) {
		count1 = count1 - 1;
		count2 = 1000;
		while(count2 != 0) {
			count2 = count2 - 1;
			}
		}

	/* now apply Vee and Vadj, turn on the display
	 * and start sending the data
	 */

	max716ev_v6on = 1;
	/* note that v6 is initially -26V so we must 'step' it to ~-22V */
	/* for the G321                                                 */
	count1 = 12;
	while (count1 != 0) {
		max716ev_v6step = 0;
		TICK;
		TICK;
		max716ev_v6step = 1;
		count1 = count1 - 1;
		TICK;
	}
	max716ev_v6step = 0;
	lcd_dispon = 1;

	/* initialize all of the LCD control signals */
	lcd_flm = 0;
	lcd_lp = 0;
	lcd_cp = 0;

	/* start displaying data */
	while(1) {
		doframe();
	}
} /* end of main() */
