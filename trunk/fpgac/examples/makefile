all:	testit testlcd

testit:
	tmcc -target xnf -S counter.c
	tmcc -target xnf -S countup.c
	@echo 'Only the date should change in the following two diffs:'
	-diff counter.xnf.orig counter.xnf
	-diff countup.xnf.orig countup.xnf

testlcd:
	cd lcd; make

bits:	countup.bit counter.bit

counter.bit:	counter.c
	tmcc counter.c

countup.bit:	countup.c
	tmcc countup.c

clean:
	rm -f *.xnf *.bit
	cd lcd; make clean
