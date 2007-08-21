struct test {
    volatile int in:1;
    volatile int a:2;
    volatile int b:2;
    volatile int out:2;
} io;

main() {
    io.out = io.in ? io.a : io.b;
}
