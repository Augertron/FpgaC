struct test {
    volatile int  out:1;
} io;

main() {
    int gvar = 1;

    io.out = gvar;
}
