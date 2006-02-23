all:	testit clean

install: clean
	cd src; make install
	cd doc; make install

testit:
	cd src; make install
	cd doc; make install
	cd examples; make

clean:
	cd src; make clean
	cd doc; make clean
	cd examples; make clean

orig: testit
	cd examples; make orig; make clean
