all:	testit testlcd

testit:
	fpgac -target xnf -S -c counter.c
	fpgac -target xnf -S -c countup.c
	-diff -c -I "^PROG, fpgac," counter.xnf.orig counter.xnf
	-diff -c -I "^PROG, fpgac," countup.xnf.orig countup.xnf

testlcd:
	cd lcd; make -s

bits:	countup.bit counter.bit

counter.bit:	counter.c
	fpgac counter.c

countup.bit:	countup.c
	fpgac countup.c

clean:
	rm -f *.xnf *.bit *.cnf
	cd lcd; make clean

orig: testit
	-mv counter.xnf counter.xnf.orig
	-mv countup.xnf countup.xnf.orig
	cd lcd; make orig
