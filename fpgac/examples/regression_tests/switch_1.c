struct test {
    volatile _Bool in;
    volatile int out:2;
} io;

main() {
    switch(io.in) {
    case 0:    io.out = 2;
    }
}
