struct test {
    volatile int  a:1;
    volatile int  b:1;
    volatile int  c:1;
    volatile int out1:1;
    volatile int out2:1;
} io;

main() {
    for( ;io.b|io.c; ) {
        io.out1 = (io.b & io.c);
        for( ;io.a; ) {
            io.out2 = (io.a & io.c);
        }
    }
    for( ;io.a|io.b; ) {
        io.out1 =  (io.a & io.b);
    }
}



