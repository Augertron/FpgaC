/* This file defines the interface between the SUN and the TM-1 running
 * LCD controller application.
 * 
 * There are 3 registers, each 12 bits long.
 */

/* Bits in Register 0 (port0p) */
#define P0_TOSUNMASK	0x0FF	/* TM-1 returns 8 bit data item */
#define P0_HS_TM1	0x100	/* TM-1 toggles to indicate new data item */
#define P0_READY	0x200	/* TM-1 is ready for a new command */
#define P0_DONE		0x400	/* TM-1 has finished current command */

/* bits in Register 1 (port1p) */
#define P1_GO		0x001	/* SUN tells TM-1 to look at command */
#define P1_COMMASK	0x01E	/* SUN tells TM-1 what to do */
#define   P1_SETADDR	0x002	/* SUN sets address for next command */
#define   P1_PUTDATA	0x004	/* SUN sends one byte */
#define   P1_GETDATA	0x008	/* SUN reads one byte */
#define   P1_PUTBYTES	0x010	/* SUN sends multiple bytes */
#define P1_TOPADDRMASK	0xFE0	/* SUN sets top bits of frame buffer address */

/* Bits in Register 2 (port2p) */
#define P2_LOWADDRMASK	0xFFF	/* SUN sets bottom bits of frame buffer addr */
#define P2_FROMSUNMASK	0x0FF	/* SUN sends 8 bit data item */
#define P2_HS_SUN	0x100	/* SUN toggles to indicate new data item */
#define P2_STOP		0x200	/* SUN sets to end data transfer */
