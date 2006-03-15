build:	aes.c aes.ucf
	-fpgac -target xnf -S -p xcv2000e-bg560-8 aes.c
	-xnf2edf aes.xnf
	-ngdbuild -intstyle ise -dd ../aes -uc aes.ucf -p xcv2000e-bg560-8 aes.edf aes.ngd
	-map -intstyle ise -p xcv2000e-bg560-8 -cm area -pr b -k 6 -c 100 -tx off -o aes.ncd aes.ngd aes.pcf
	-par -w -intstyle ise -ol high -t 1 aes.ncd aes.ncd aes.pcf
	-trce -intstyle ise -e 3 -l 3 -xml aes aes.ncd -o aes.twr aes.pcf

clean:
	-rm -f netlist.lst aes.bld aes.log aes.mrp aes.ncd aes.ngd aes.ngm aes.ngo
	-rm -f aes.pad aes_pad.csv aes_pad.txt aes.par aes.pcf aes.twx aes.xpi xnf2edf.log
