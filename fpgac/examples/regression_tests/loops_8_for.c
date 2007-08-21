struct test {
    volatile int  a:2;
    volatile int  b:2;
    volatile int out:2;
} io;

main() {
    int loopvar;
        
    for(loopvar=0; ; loopvar++) {
        io.out = loopvar & io.a;
    }
}
