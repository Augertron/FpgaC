/* The interface variables used by the sun thread to talk to the sram
 * thread.
 */

int sram_sun_address:17;	/* byte address */

char sram_fromsun;		/* 8 bit word to be written to SRAM */
char sram_tosun;		/* 8 bit word to be read from SRAM */

int sram_sun_request:1;		/* SUN wants SRAM access */
int sram_sun_done:1;		/* SRAM has completed SUN access */
int sram_sun_write:1;		/* SUN wants a write, rather than a read */
