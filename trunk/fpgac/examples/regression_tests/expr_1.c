/*
 * This isn't working yet. jbass
 */

struct test {
    volatile int out:8;
} io;

main() {
    int i;

    for(i=0; i<12; i++)
        io.out = "Hello World\n"[i];
}
