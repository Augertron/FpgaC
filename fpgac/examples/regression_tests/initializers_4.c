struct test {
    volatile int out:4;
} io;

unsigned char abc[16] = {1, 2, 3, 5, 7, 11, 13, 17};

main() {
	out = abc[3];
	out = abc[4];
	out = abc[6];
}
