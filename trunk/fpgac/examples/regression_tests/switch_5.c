struct test {
    volatile _Bool in;
    volatile int out:2;
} io;

main() {
    _Bool a, b, c;

    switch(io.in) {
    case 0:    a = 1; break;
    case 1:    b = 1; break;
    default:   c = 1; break;
    }
    io.out = a | b<<1 | c<<1 | c;
}
