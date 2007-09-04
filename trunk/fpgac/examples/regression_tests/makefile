tests: loops ioports initializers enum volatile sixtyfourbit expr cond regression

clean:
	rm -f *.cnf *.xnf *.ucf test.c

loops: loops4 loops7 loops8 loops9

loops4: loops_4_for.c loops_4_while.c
	cp loops_4_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_4_for.cnf
	cp loops_4_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_4_while.cnf
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_4*.cnf

loops7: loops_7_for.c loops_7_while.c
	cp loops_7_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_7_for.cnf
	cp loops_7_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_7_while.cnf
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_7*.cnf

loops8: loops_8_for.c loops_8_while.c
	cp loops_8_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_8_for.cnf
	cp loops_8_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_8_while.cnf
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_8*.cnf

loops9: loops_9_for.c loops_9_while.c
	cp loops_9_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_9_for.cnf
	cp loops_9_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_9_while.cnf
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_9*.cnf

ioports: ioports1

ioports1: ioports_1.c
	-fpgac -target cnf ioports_1.c

initializers: initializers1 initializers2 initializers3 initializers4

initializers1: initializers_1.c
	-fpgac -target cnf initializers_1.c

initializers2: initializers_2.c
	-fpgac -target cnf initializers_2.c

initializers3: initializers_3.c
	-fpgac -target cnf initializers_3.c

initializers4: initializers_4.c
	-fpgac -target cnf initializers_4.c

enum: enum1 enum2 enum3 enum4

enum1: enum_1.c
	-fpgac -target cnf enum_1.c

enum2: enum_2.c
	-fpgac -target cnf enum_2.c

enum3: enum_3.c
	-fpgac -target cnf enum_3.c

enum4: enum_4.c
	-fpgac -target cnf enum_4.c

volatile: volatile1 volatile2 volatile3

volatile1: volatile_1.c
	-fpgac -target cnf volatile_1.c

volatile2: volatile_2.c
	-fpgac -target cnf volatile_2.c

volatile3: volatile_3.c
	-fpgac -target cnf volatile_3.c

sixtyfourbit: sixtyfourbit1

sixtyfourbit1: sixtyfourbit_1.c
	-fpgac -target cnf sixtyfourbit_1.c

expr:	expr1

expr1:	expr_1.c
	-fpgac -target cnf expr_1.c

cond:	cond1 cond2 cond3

cond1:	cond_1.c
	-fpgac -target cnf cond_1.c

cond2:	cond_2.c
	-fpgac -target cnf cond_2.c

cond3:	cond_3.c
	-fpgac -target cnf cond_3.c

regression:
	-diff -c -I "^\/\/ fpgac," loops_4_for.cnf loops_4_for.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_4_while.cnf loops_4_while.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_7_for.cnf loops_7_for.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_7_while.cnf loops_7_while.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_8_for.cnf loops_8_for.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_8_while.cnf loops_8_while.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_9_for.cnf loops_9_for.cnf.ref
	-diff -c -I "^\/\/ fpgac," loops_9_while.cnf loops_9_while.cnf.ref
	-diff -c -I "^\/\/ fpgac," ioports_1.cnf ioports_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," initializers_1.cnf initializers_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," initializers_2.cnf initializers_2.cnf.ref
	-diff -c -I "^\/\/ fpgac," initializers_3.cnf initializers_3.cnf.ref
	-diff -c -I "^\/\/ fpgac," initializers_4.cnf initializers_4.cnf.ref
	-diff -c -I "^\/\/ fpgac," enum_1.cnf enum_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," enum_2.cnf enum_2.cnf.ref
	-diff -c -I "^\/\/ fpgac," enum_3.cnf enum_3.cnf.ref
	-diff -c -I "^\/\/ fpgac," enum_4.cnf enum_4.cnf.ref
	-diff -c -I "^\/\/ fpgac," volatile_1.cnf volatile_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," volatile_2.cnf volatile_2.cnf.ref
	-diff -c -I "^\/\/ fpgac," volatile_3.cnf volatile_3.cnf.ref
	-diff -c -I "^\/\/ fpgac," sixtyfourbit_1.cnf sixtyfourbit_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," expr_1.cnf expr_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," cond_1.cnf cond_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," cond_2.cnf cond_2.cnf.ref
	-diff -c -I "^\/\/ fpgac," cond_3.cnf cond_3.cnf.ref

ref:  tests
	mv loops_4_for.cnf loops_4_for.cnf.ref
	mv loops_4_while.cnf loops_4_while.cnf.ref
	mv loops_7_for.cnf loops_7_for.cnf.ref
	mv loops_7_while.cnf loops_7_while.cnf.ref
	mv loops_8_for.cnf loops_8_for.cnf.ref
	mv loops_8_while.cnf loops_8_while.cnf.ref
	mv loops_9_for.cnf loops_9_for.cnf.ref
	mv loops_9_while.cnf loops_9_while.cnf.ref
	mv ioports_1.cnf ioports_1.cnf.ref
	mv initializers_1.cnf initializers_1.cnf.ref
	mv initializers_2.cnf initializers_2.cnf.ref
	mv initializers_3.cnf initializers_3.cnf.ref
	mv initializers_4.cnf initializers_4.cnf.ref
	mv enum_1.cnf enum_1.cnf.ref
	mv enum_2.cnf enum_2.cnf.ref
	mv enum_3.cnf enum_3.cnf.ref
	mv enum_4.cnf enum_4.cnf.ref
	mv volatile_1.cnf volatile_1.cnf.ref
	mv volatile_2.cnf volatile_2.cnf.ref
	mv volatile_3.cnf volatile_3.cnf.ref
	mv sixtyfourbit_1.cnf sixtyfourbit_1.cnf.ref
	mv expr_1.cnf expr_1.cnf.ref
	mv cond_1.cnf cond_1.cnf.ref
	mv cond_2.cnf cond_2.cnf.ref
	mv cond_3.cnf cond_3.cnf.ref
