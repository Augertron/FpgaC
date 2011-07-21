struct test {
    volatile _Bool in;
    volatile int out:2;
} io;

main() {
    _Bool a, b, c;
    switch(io.in) {
    case 0:    a = 1;
    case 1:    b = 1;
               break;
    default:   c = 3;
    }
    io.out = (a+b) | c;
}
