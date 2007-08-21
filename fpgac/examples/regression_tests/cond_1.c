struct test {
    volatile int in:1;
    volatile int out:2;
} io;

main() {
    io.out = io.in ? 2 : 1;
}
