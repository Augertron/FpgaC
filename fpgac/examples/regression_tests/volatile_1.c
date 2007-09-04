struct test {
	volatile int o:2;
} io;

main() {
    io.o = 0;
    io.o = 1;
    io.o = 0;
    io.o = 1;
    io.o = 0;
    io.o = 2;
}
