/*
 * struct_1.c and struct_2.c are a simple test that struct tags work
 */

struct test {
    volatile _Bool in;
    volatile _Bool out;
};

struct test io;

main() {
    io.out = io.in;
}
