all:	testit

testit:
	fpgac -target cnf -S BitSer.c
	@echo 'Only the date should change in the following two diffs:'
	-diff BitSer.cnf.orig BitSer.cnf

bits:	countup.bit

BitSer.bit:	BitSer.c
	fpgac BitSer.c

clean:
	rm -f *.xnf *.bit *.cnf

orig: testit
	-mv BitSer.cnf BitSer.cnf.orig
