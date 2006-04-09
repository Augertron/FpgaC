fpgac_output  o:1;

main() {
    static int gvar:1 = 1;

    o = gvar;
}
