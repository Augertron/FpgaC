/* countup.c -- will make the two 7 segment displays on the Xilinx XC4000
 * 	demo board count from 00 to 99 repeatedly.  Use the switches to
 *	control how fast it counts.
 *
 *	tmcc countup.c
 */

char seven_seg(int x:4)
	{
	char result;

	x = x & 0xf; result = 0;
	if(x == 0x0) result = 0xfc;
	if(x == 0x1) result = 0x60;
	if(x == 0x2) result = 0xda;
	if(x == 0x3) result = 0xf2;
	if(x == 0x4) result = 0x66;
	if(x == 0x5) result = 0xb6;
	if(x == 0x6) result = 0xbe;
	if(x == 0x7) result = 0xe0;
	if(x == 0x8) result = 0xfe;
	if(x == 0x9) result = 0xf6;
	if(x == 0xa) result = 0xee;
	if(x == 0xb) result = 0x3e;
	if(x == 0xc) result = 0x9c;
	if(x == 0xd) result = 0x7a;
	if(x == 0xe) result = 0x9e;
	if(x == 0xf) result = 0x8e;
	return(~result);
	}



void delay(char n)
	{
	while(n != 0)
		n = n - 1;
	}



void twodigit(char y)
	{
	char tens;
	char leftdigit, rightdigit;

#pragma	outputport(leftdigit, 37, 44, 40, 29, 35, 36, 38, 39);
#pragma	outputport(rightdigit, 41, 51, 50, 45, 46, 47, 48, 49);

	tens = 0;
	while(y >= 10) {
		tens = tens + 1;
		y = y - 10;
		}
	leftdigit = seven_seg(tens);
	rightdigit = seven_seg(y);
	}



void main()
	{
	char count;
	char switches;

#pragma	inputport(switches, 28, 27, 26, 25, 24, 23, 20, 19);

	count = 0;
	while(1) {
		twodigit(count);
		count = count + 1;
		if(count >= 100)
			count = 0;
		delay(switches);
		}
	}
