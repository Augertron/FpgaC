all:	testit clean

install: clean
	cd src; make install
	cd doc; make install

testit:
	cd src; make install
	cd doc; make install
	cd examples; make -s

clean:
	cd src; make clean -s
	cd doc; make clean -s
	cd examples; make clean -s

orig: testit
	cd examples; make orig; make clean
