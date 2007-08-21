struct test {
    volatile int out:3;
} io;

enum abc {a, b, c};
enum abc eval;

main() {
	eval = b;
	eval += 1;
	io.out = eval;
}



