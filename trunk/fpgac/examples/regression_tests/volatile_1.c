struct test {
    volatile int out:1;
} io;

/*
 * Test for new clock state for each assignment
 */

main()
{
	io.out = 1;
	io.out = 0;
	io.out = 1;
	io.out = 0;
}



