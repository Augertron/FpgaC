all:	testit

testit:
	cd pipelines; make
	cd tmcc; make
	cd regression_tests; make

clean:
	cd pipelines; make clean
	cd tmcc; make clean
	cd regression_tests; make clean
	cd pci; make clean

orig: testit
	cd pipelines; make orig
	cd tmcc; make orig
	cd regression_tests; make orig
