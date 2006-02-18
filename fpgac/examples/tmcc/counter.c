/* counter.c -- Will make the 8 LEDs on the Xilinx XC4000 demo board
 *		count up.
 *
 *	Compile as:
 *
 *		tmcc counter.c
 */

main() {
	fpgac_output lights:8;
	char count;

//#pragma	fpgac_outputport(lights, 60, 59, 58, 57, 66, 65, 62, 61)

	count = 0;
	while(1) {
		count = count + 1;
		lights = ~count;
		}
	}
