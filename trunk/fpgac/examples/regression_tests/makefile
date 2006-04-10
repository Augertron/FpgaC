tests: loops ioports initializers enum regression

clean:
	rm -f *.cnf *.xnf *.ucf test.c

loops: loops4 loops7 loops8 loops9

loops4: loops_4_for.c loops_4_while.c
	cp loops_4_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_4_for.cnf
	cp loops_4_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_4_while.cnf
	rm -f test.c
	-diff loops_4*.cnf

loops7: loops_7_for.c loops_7_while.c
	cp loops_7_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_7_for.cnf
	cp loops_7_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_7_while.cnf
	rm -f test.c
	-diff loops_7*.cnf

loops8: loops_8_for.c loops_8_while.c
	cp loops_8_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_8_for.cnf
	cp loops_8_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_8_while.cnf
	rm -f test.c
	-diff loops_8*.cnf

loops9: loops_9_for.c loops_9_while.c
	cp loops_9_for.c   test.c; fpgac -target cnf test.c; mv test.cnf loops_9_for.cnf
	cp loops_9_while.c test.c; fpgac -target cnf test.c; mv test.cnf loops_9_while.cnf
	rm -f test.c
	-diff loops_9*.cnf

ioports: ioports1

ioports1: ioports_1.c
	-fpgac -target cnf ioports_1.c

initializers: initializers1 initializers2 initializers3

initializers1: initializers_1.c
	-fpgac -target cnf initializers_1.c

initializers2: initializers_2.c
	-fpgac -target cnf initializers_2.c

initializers3: initializers_3.c
	-fpgac -target cnf initializers_3.c

enum: enum1 enum2 enum3 enum4

enum1: enum_1.c
	-fpgac -target cnf enum_1.c

enum2: enum_2.c
	-fpgac -target cnf enum_2.c

enum3: enum_3.c
	-fpgac -target cnf enum_3.c

enum4: enum_4.c
	-fpgac -target cnf enum_4.c

regression:
	-diff loops_4_for.cnf loops_4_for.cnf.orig
	-diff loops_4_while.cnf loops_4_while.cnf.orig
	-diff loops_7_for.cnf loops_7_for.cnf.orig
	-diff loops_7_while.cnf loops_7_while.cnf.orig
	-diff loops_8_for.cnf loops_8_for.cnf.orig
	-diff loops_8_while.cnf loops_8_while.cnf.orig
	-diff loops_9_for.cnf loops_9_for.cnf.orig
	-diff loops_9_while.cnf loops_9_while.cnf.orig
	-diff ioports_1.cnf ioports_1.cnf.orig
	-diff initializers_1.cnf initializers_1.cnf.orig
	-diff initializers_2.cnf initializers_2.cnf.orig
	-diff initializers_3.cnf initializers_3.cnf.orig
	-diff enum_1.cnf enum_1.cnf.orig
	-diff enum_2.cnf enum_2.cnf.orig
	-diff enum_3.cnf enum_3.cnf.orig
	-diff enum_4.cnf enum_4.cnf.orig

orig:  tests
	mv loops_4_for.cnf loops_4_for.cnf.orig
	mv loops_4_while.cnf loops_4_while.cnf.orig
	mv loops_7_for.cnf loops_7_for.cnf.orig
	mv loops_7_while.cnf loops_7_while.cnf.orig
	mv loops_8_for.cnf loops_8_for.cnf.orig
	mv loops_8_while.cnf loops_8_while.cnf.orig
	mv loops_9_for.cnf loops_9_for.cnf.orig
	mv loops_9_while.cnf loops_9_while.cnf.orig
	mv ioports_1.cnf ioports_1.cnf.orig
	mv initializers_1.cnf initializers_1.cnf.orig
	mv initializers_2.cnf initializers_2.cnf.orig
	mv initializers_3.cnf initializers_3.cnf.orig
	mv enum_1.cnf enum_1.cnf.orig
	mv enum_2.cnf enum_2.cnf.orig
	mv enum_3.cnf enum_3.cnf.orig
	mv enum_4.cnf enum_4.cnf.orig
