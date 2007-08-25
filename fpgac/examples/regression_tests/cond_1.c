struct test {
    volatile _Bool in;
    volatile int out:2;
} io;

main() {
    io.out = io.in ? 2 : 1;
}
