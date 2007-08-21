struct test {
    volatile int  out:1;
} io;

int gvar = 1;

main() {
    io.out = gvar;
}
