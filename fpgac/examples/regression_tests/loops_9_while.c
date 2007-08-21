struct test {
    volatile int  a:3;
    volatile int  b:3;
    volatile int out:3;
} io;

main() {
    char loopvar;

    loopvar = 0;
    loopvar++;
    while (0) ;
    io.out = loopvar & io.a;
    loopvar++;
    while (0) ;
    io.out = loopvar & io.b;
}
