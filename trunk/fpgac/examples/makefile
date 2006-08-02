all:	testit

testit:
	cd pipelines; make -s
	cd tmcc; make -s
	cd regression_tests; make -s

clean:
	cd pipelines; make clean -s
	cd tmcc; make clean -s
	cd regression_tests; make clean -s
	cd pci; make clean -s
	cd crypto; make clean -s

ref: testit
	cd pipelines; make ref -s
	cd tmcc; make orig -s
	cd regression_tests; make ref -s
