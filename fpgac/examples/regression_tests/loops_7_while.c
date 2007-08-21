struct test {
    volatile int    a:2;
    volatile int   out1:2;
    volatile int   out2:2;
} io;

main() {
    char loopvar1;
    char loopvar2;
        
    loopvar1 = 0;
    loopvar2 = 0;
    while(loopvar1 < 2) {
        io.out1 = loopvar1 & io.a;
        io.out2 = loopvar2 & io.a;
        loopvar1++;
        loopvar2++;
     }
}
