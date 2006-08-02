tests: loops ioports initializers enum volatile sixtyfourbit expr regression

clean:
	rm -f *.cnf *.xnf *.ucf *.vhd test.c

loops: loops4 loops7 loops8 loops9

loops4: loops_4_for.c loops_4_while.c
	cp loops_4_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_4_for.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_4_for.vhd
	cp loops_4_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_4_while.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_4_while.vhd
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_4*.cnf
	-diff -c -I "^-- fpgac " loops_4*.vhd

loops7: loops_7_for.c loops_7_while.c
	cp loops_7_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_7_for.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_7_for.vhd
	cp loops_7_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_7_while.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_7_while.vhd
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_7*.cnf
	-diff -c -I "^-- fpgac " loops_7*.vhd

loops8: loops_8_for.c loops_8_while.c
	cp loops_8_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_8_for.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_8_for.vhd
	cp loops_8_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_8_while.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_8_while.vhd
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_8*.cnf
	-diff -c -I "^-- fpgac " loops_8*.vhd

loops9: loops_9_for.c loops_9_while.c
	cp loops_9_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_9_for.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_9_for.vhd
	cp loops_9_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_9_while.cnf
	                           fpgac -target vhd test.c; mv test.vhd loops_9_while.vhd
	rm -f test.c
	-diff -c -I "^\/\/ fpgac," loops_9*.cnf
	-diff -c -I "^-- fpgac " loops_9*.vhd

ioports: ioports1

ioports1: ioports_1.c
	-fpgac -target cnf ioports_1.c
	-fpgac -target vhd ioports_1.c

initializers: initializers1 initializers2 initializers3 initializers4

initializers1: initializers_1.c
	-fpgac -target cnf initializers_1.c
	-fpgac -target vhd initializers_1.c

initializers2: initializers_2.c
	-fpgac -target cnf initializers_2.c
	-fpgac -target vhd initializers_2.c

initializers3: initializers_3.c
	-fpgac -target cnf initializers_3.c
	-fpgac -target vhd initializers_3.c

initializers4: initializers_4.c
	-fpgac -target cnf initializers_4.c
	-fpgac -target vhd initializers_4.c

enum: enum1 enum2 enum3 enum4

enum1: enum_1.c
	-fpgac -target cnf enum_1.c
	-fpgac -target vhd enum_1.c

enum2: enum_2.c
	-fpgac -target cnf enum_2.c
	-fpgac -target vhd enum_2.c

enum3: enum_3.c
	-fpgac -target cnf enum_3.c
	-fpgac -target vhd enum_3.c

enum4: enum_4.c
	-fpgac -target cnf enum_4.c
	-fpgac -target vhd enum_4.c

volatile: volatile1

volatile1: volatile_1.c
	-fpgac -target cnf volatile_1.c
	-fpgac -target vhd volatile_1.c

sixtyfourbit: sixtyfourbit1

sixtyfourbit1: sixtyfourbit_1.c
	-fpgac -target cnf sixtyfourbit_1.c
	-fpgac -target vhd sixtyfourbit_1.c

expr:	expr1

expr1:	expr_1.c
	-fpgac -target cnf expr_1.c
	-fpgac -target vhd expr_1.c

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
	-diff -c -I "^\/\/ fpgac," sixtyfourbit_1.cnf sixtyfourbit_1.cnf.ref
	-diff -c -I "^\/\/ fpgac," expr_1.cnf expr_1.cnf.ref

	-diff -c -I "^-- fpgac " loops_4_for.vhd loops_4_for.vhd.ref
	-diff -c -I "^-- fpgac " loops_4_while.vhd loops_4_while.vhd.ref
	-diff -c -I "^-- fpgac " loops_7_for.vhd loops_7_for.vhd.ref
	-diff -c -I "^-- fpgac " loops_7_while.vhd loops_7_while.vhd.ref
	-diff -c -I "^-- fpgac " loops_8_for.vhd loops_8_for.vhd.ref
	-diff -c -I "^-- fpgac " loops_8_while.vhd loops_8_while.vhd.ref
	-diff -c -I "^-- fpgac " loops_9_for.vhd loops_9_for.vhd.ref
	-diff -c -I "^-- fpgac " loops_9_while.vhd loops_9_while.vhd.ref
	-diff -c -I "^-- fpgac " ioports_1.vhd ioports_1.vhd.ref
	-diff -c -I "^-- fpgac " initializers_1.vhd initializers_1.vhd.ref
	-diff -c -I "^-- fpgac " initializers_2.vhd initializers_2.vhd.ref
	-diff -c -I "^-- fpgac " initializers_3.vhd initializers_3.vhd.ref
	-diff -c -I "^-- fpgac " initializers_4.vhd initializers_4.vhd.ref
	-diff -c -I "^-- fpgac " enum_1.vhd enum_1.vhd.ref
	-diff -c -I "^-- fpgac " enum_2.vhd enum_2.vhd.ref
	-diff -c -I "^-- fpgac " enum_3.vhd enum_3.vhd.ref
	-diff -c -I "^-- fpgac " enum_4.vhd enum_4.vhd.ref
	-diff -c -I "^-- fpgac " volatile_1.vhd volatile_1.vhd.ref
	-diff -c -I "^-- fpgac " sixtyfourbit_1.vhd sixtyfourbit_1.vhd.ref
	-diff -c -I "^-- fpgac " expr_1.vhd expr_1.vhd.ref
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
	mv sixtyfourbit_1.cnf sixtyfourbit_1.cnf.ref
	mv expr_1.cnf expr_1.cnf.ref

	mv loops_4_for.vhd loops_4_for.vhd.ref
	mv loops_4_while.vhd loops_4_while.vhd.ref
	mv loops_7_for.vhd loops_7_for.vhd.ref
	mv loops_7_while.vhd loops_7_while.vhd.ref
	mv loops_8_for.vhd loops_8_for.vhd.ref
	mv loops_8_while.vhd loops_8_while.vhd.ref
	mv loops_9_for.vhd loops_9_for.vhd.ref
	mv loops_9_while.vhd loops_9_while.vhd.ref
	mv ioports_1.vhd ioports_1.vhd.ref
	mv initializers_1.vhd initializers_1.vhd.ref
	mv initializers_2.vhd initializers_2.vhd.ref
	mv initializers_3.vhd initializers_3.vhd.ref
	mv initializers_4.vhd initializers_4.vhd.ref
	mv enum_1.vhd enum_1.vhd.ref
	mv enum_2.vhd enum_2.vhd.ref
	mv enum_3.vhd enum_3.vhd.ref
	mv enum_4.vhd enum_4.vhd.ref
	mv volatile_1.vhd volatile_1.vhd.ref
	mv sixtyfourbit_1.vhd sixtyfourbit_1.vhd.ref
	mv expr_1.vhd expr_1.vhd.ref


