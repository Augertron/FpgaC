// cnf output for this is wrong, xnf is correct. missing last t = i    (jbass])

struct test {
    volatile int t:1;
    volatile int i:1;
    volatile int out:1;
} io;

main() {
    while(io.i) {
        io.out = io.t;
    }
    io.t = io.i;
}
