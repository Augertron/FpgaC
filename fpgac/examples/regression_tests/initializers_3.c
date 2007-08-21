struct test {
    volatile int  out:1;
} io;

main() {
    static int gvar = 1;

    io.out = gvar;
}
