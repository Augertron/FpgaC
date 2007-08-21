struct test {
    volatile int  a:3;
    volatile int  b:3;
    volatile int out:3;
} io;

main() {
    char loopvar;
        
    loopvar = 0;
    for(loopvar++;0;);
    io.out = loopvar & io.a;
    for(loopvar++;0;);
    io.out = loopvar & io.b;
}
