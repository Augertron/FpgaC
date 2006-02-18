/* $Id$ */

/*
 * PROJECT  : TM-1<->LCD
 * MODULE   : sram.c -- TOYC
 * VERSION  : $Revision$ $Date$
 * AUTHOR(S): David Galloway, James Duhault
 * CREATED  : 15 July 1994
 *
 * DESCRIPTION:
 *
 * This is a tmcc program to run a SEIKO G321 LCD module in BLACK&WHITE
 * mode, i.e., 1 bit per pixel.  It is 320x240.
 *
 * It is one thread of a multi-threaded program.  This one talks to the
 * SRAM, and provides access to the SRAM to the sun and lcd threads
 *
 * LOCKED BY: $Locker:  $
 *
 */
 
/*
 * MODIFICATION HISTORY:
 *
 *
 *
 *
 * 
 * Revision 1.6  94/07/22  10:20:22  drg
 * James' modifications to support the Seiko G321 320x240 display.  Seems
 * to work.
 * 
 * Revision 1.3  94/07/21  16:59:12  duhault
 * Fixed mistake in the order in which bytes are stored in the 4 SRAMS.
 * 
 * Revision 1.2  94/07/21  12:35:57  duhault
 * First pass... seems to work according to analysis using the Logic analyzer
 * A few changes from the DMF666 code but very similiar in operation.
 * 
 *
 */

#include "sramsun.h"
#include "sramlcd.h"

/*
 * Interface to the SRAM
 * ---------------------
 *
 * Address 0 in the SRAM is the top left corner of the screen
 * and addresses increase to the right and down the screen.
 *
 * All four SRAMS are used to store pixels so that one 32 bit read
 * (4 SRAMS x 1 byte per SRAM) will give 32 sequential pixels:
 *	chip 1: 1st block of 8 pixels, 5th block, ...
 *	chip 2: 2nd block of 8 pixels, 6th block, ...
 *	chip 3: 3rd block of 8 pixels, 7th block, ...
 *	chip 4: 4th block of 8 pixels, 8th block, ...
 *
 * We write to the "processor port" on each SRAM,
 * and read from the "system port".
 *
 */

int sram_addr:15;
int sram_k:1;		/* SRAM clock */
int sram_siebar:1;	/* system input enable (inverted) */
int sram_poebar:1;	/* processor output enable */
int sram_soebar:1;	/* system output enable */
int sram_wbar:4;		/* write enables, low order bit = low address */
int sram_piebar:4;	/* processor input enables */
long sram_toram;		/* processor data port */
long sram_fromram;	/* system data port */

#define TICK	while(0) {}

/*
 * FUNCTION: main()
 *
 *  The main function! (what else could it be?)
 *
 */

main()
	{
	int doinglcd:1, write:1, lcd_has_acked:1, request:1;

#pragma	fpgac_outputport(sram_k)
#pragma	fpgac_outputport(sram_piebar)
#pragma	fpgac_outputport(sram_siebar)
#pragma	fpgac_outputport(sram_poebar)
#pragma	fpgac_outputport(sram_soebar)
#pragma	fpgac_outputport(sram_addr)
#pragma	fpgac_outputport(sram_toram)
#pragma	fpgac_inputport(sram_fromram)
#pragma	fpgac_outputport(sram_wbar)

	/* Interface to sun thread, using ports with PORT_PIN turned off */

#pragma	fpgac_inputport(sram_sun_address)
#pragma	fpgac_inputport(sram_fromsun)
#pragma	fpgac_outputport(sram_tosun)
#pragma	fpgac_inputport(sram_sun_request)
#pragma	fpgac_outputport(sram_sun_done)
#pragma	fpgac_inputport(sram_sun_write)
#pragma	fpgac_portflags(sram_sun_address,	0)
#pragma	fpgac_portflags(sram_sun_request,	0)
#pragma	fpgac_portflags(sram_sun_write,		0)
#pragma	fpgac_portflags(sram_fromsun,		0)
#pragma	fpgac_portflags(sram_tosun,		PORT_REGISTERED)
#pragma	fpgac_portflags(sram_sun_done,		PORT_REGISTERED)

	/* Interface to lcd thread, using ports with PORT_PIN turned off */

#pragma	fpgac_inputport(sram_lcd_address)
#pragma	fpgac_outputport(sram_tolcd)
#pragma	fpgac_inputport(sram_lcd_request)
#pragma	fpgac_outputport(sram_lcd_done)
#pragma	fpgac_portflags(sram_lcd_address,	0)
#pragma	fpgac_portflags(sram_lcd_request,	0)
#pragma	fpgac_portflags(sram_tolcd,		PORT_REGISTERED)
#pragma	fpgac_portflags(sram_lcd_done,		PORT_REGISTERED)

	/* Read from "system" ports, and write to "processor" ports */
	sram_k = 1;
	sram_piebar = 0xF;
	sram_siebar = 1;
	sram_poebar = 1;
	sram_soebar = 0;


	/* initialize LCD acknowledgement signal */
	lcd_has_acked = 1;

	/* enter main loop for continuous operation */
	while(1) {

		/* first check if the LCD thread has dropped its request    */
		/* flag in acknowledgement and, if so, set the LCD ack flag */
		if(sram_lcd_request == 0)
			lcd_has_acked = 1;

		/* set the done flags for the SUN and the LCD */
		sram_lcd_done = 0;
		sram_sun_done = 0;

		/* initialize internal request flag and wait for the SUN or */
		/* the LCD threads to raise there own request flags, then   */
                /* process the request */ 
		request = 0;
		while(!request) {
			/* keep checking to see if the LCD thread has */
			/* dropped its request flag in acknowledgment */
			if(sram_lcd_request == 0)
				lcd_has_acked = 1;
			/* The LCD module has priority over the SUN */
			if(sram_lcd_request && lcd_has_acked) {
				doinglcd = 1;
				write = 0;
				/* set the SRAM address -- it is shifted  */
				/* by two as we use the 2 lower bits for  */
				/* decoding and selecting the appropriate */
				/* SRAM chip. See below.                  */
				sram_addr = sram_lcd_address>>2;
				request = 1;
				}
			else if(sram_sun_request) {
				doinglcd = 0;
				write = sram_sun_write;
				/* set the SRAM address -- it is shifted  */
				/* by two as we use the 2 lower bits for  */
				/* decoding and selecting the appropriate */
				/* SRAM chip. See below.                  */
				sram_addr = sram_sun_address>>2;
				request = 1;
				}
			}

		/* keep checking to see if the LCD thread has */
		/* dropped its request flag in acknowledgment */
		if(sram_lcd_request == 0)
			lcd_has_acked = 1;

		/* drop the SRAM clock */
		sram_k = 0;

		/* set up a READ or WRITE; note that WRITE's only originate */
		/* from the SUN thread                                      */
		if(write) {
			/* The SUN writes 8 bits at a time.
			 * Put the data on all 4 SRAM chips, then
			 * use piebar and wbar to pick which one
			 * does the write.
			 */
			sram_toram = (sram_fromsun<<24)
					| ((sram_fromsun&0xFF)<<16)
					| ((sram_fromsun&0xFF)<<8)
					| (sram_fromsun&0xFF);

			/* this is where we control the SRAM storage so      */
			/* that a 32bit read is possible as mentioned above. */
			/* we need to check the 2 lowest bits of the address */
			/* in order to select the appropriate SRAM:          */
			/* 	00 (0): SRAM 4                               */
			/*	01 (1): SRAM 3                               */
			/*	10 (2): SRAM 2                               */
			/*	11 (3): SRAM 1                               */

			if(sram_sun_address & 0x1) {
				if(sram_sun_address & 0x2) {
					/* 11 -> SRAM 1 */
					/* ~0xE = ~1110 = 0001   */
					sram_piebar = 0xE;
					sram_wbar = 0xE;
					}
				else
					{
					/* 01 -> SRAM 3 */
					/* ~0xB = ~1011 = 0100   */
					sram_piebar = 0xB;
					sram_wbar = 0xB;
					}
				}
			else	{
				if(sram_sun_address & 0x2) {
					/* 10 -> SRAM 2 */
					/* ~0xD = ~1101 = 0010   */
					sram_piebar = 0xD;
					sram_wbar = 0xD;
					}
				else
					{
					/* 00 -> SRAM 4 */
					/* ~0x7 = ~0111 = 1000   */
					sram_piebar = 0x7;
					sram_wbar = 0x7;
					}
				}
			}
		else { /* its a read so all 4 chips will be used (32 bits) */
			sram_piebar = 0xF;
			sram_wbar = 0xF;
			}
		TICK;

		/* keep checking to see if the LCD thread has */
		/* dropped its request flag in acknowledgment */
		if(sram_lcd_request == 0)
			lcd_has_acked = 1;

		/* raise the SRAM clock -- READ or WRITE will be processed */
		sram_k = 1;

		TICK;

		/* keep checking to see if the LCD thread has */
		/* dropped its request flag in acknowledgment */
		if(sram_lcd_request == 0)
			lcd_has_acked = 1;

		/* now get data regardless of whether its a READ or WRITE */
		if(doinglcd) {
			sram_tolcd = sram_fromram;
			sram_lcd_done = 1;
			lcd_has_acked = 0;
			}
		else {
			/* remember, the SUN only reads 8 bits at a time so */
			/* we must select the correct byte                  */
			if(sram_sun_address & 0x1) {
				if(sram_sun_address & 0x2)
					sram_tosun = sram_fromram & 0xFF;
				else
					sram_tosun = (sram_fromram>>16) & 0xFF;
				}
			else
				{
				if(sram_sun_address & 0x2)
					sram_tosun = (sram_fromram>>8) & 0xFF;
				else
					sram_tosun = (sram_fromram>>24) & 0xFF;
				}

			/* tell the SUN thread that we are done */
			sram_sun_done = 1;

			TICK;

			/* keep checking to see if the LCD thread has */
			/* dropped its request flag in acknowledgment */
			if(sram_lcd_request == 0)
				lcd_has_acked = 1;

			/* We know that the SUN thread is waiting for us,
			 * so we can assume that sram_sun_request will drop
			 * next tick.
			 */
			}
		}
	}
