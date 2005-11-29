/* The interface variables used by the lcd thread to talk to the sram
 * thread.
 */

int sram_lcd_address:17;	/* byte address */

long sram_tolcd;		/* 32 bit word to be read from SRAM */

int sram_lcd_request:1;		/* lcd wants SRAM access */
int sram_lcd_done:1;		/* SRAM has completed lcd access */
