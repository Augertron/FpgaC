struct test {
	volatile int a:1;
	volatile int o:2;
} io;

main() {
    if(io.a) {
        io.o = 0;
	io.o = 1;
	io.o = 0;
	io.o = 1;
        io.o = 0;
    }
    io.o = 2;
}
