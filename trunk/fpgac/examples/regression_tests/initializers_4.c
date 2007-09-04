/*
 * this isn't working yet. jbass
 */

struct test {
    volatile int out:4;
} io;

//unsigned char abc[16] = {1, 2, 3, 5, 7, 11, 13, 17};
unsigned char abc[16];

main() {
	io.out = abc[3];
	io.out = abc[4];
	io.out = abc[6];
}
