struct test {
    volatile int  a:2;
    volatile int  b:2;
    volatile int out:2;
} io;

main() {
    int loopvar;
        
    loopvar = 0; 
    while(1) {
        io.out = loopvar & io.a;
        loopvar++;
    }
}
