// cnf output for this is wrong, xnf is correct. missing last t = i    (jbass])
fpgac_tristate  t:1;
fpgac_input   i:1;
fpgac_output  o:1;

main() {

    while(i) {
    o = t;
    }
    t = i;
}
