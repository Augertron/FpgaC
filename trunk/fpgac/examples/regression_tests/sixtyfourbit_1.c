struct test {
    volatile int port:64;
} io;

main(){
    io.port = 0xfedcba9876543210;
}
