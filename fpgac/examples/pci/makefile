build:	pci.c pci.ucf
	-fpgac -target xnf -S -p xcv2000e-bg560-8 pci.c
	-xnf2edf pci.xnf
	-ngdbuild -intstyle ise -dd ../pci -uc pci.ucf -p xcv2000e-bg560-8 pci.edf pci.ngd
	-map -intstyle ise -p xcv2000e-bg560-8 -cm area -pr b -k 6 -c 100 -tx off -o pci.ncd pci.ngd pci.pcf
	-par -w -intstyle ise -ol high -t 1 pci.ncd pci.ncd pci.pcf
	-trce -intstyle ise -e 3 -l 3 -xml pci pci.ncd -o pci.twr pci.pcf

clean:
	-rm -f netlist.lst pci.bld pci.log pci.mrp pci.ncd pci.ngd pci.ngm pci.ngo
	-rm -f pci.pad pci_pad.csv pci_pad.txt pci.par pci.pcf pci.twx pci.xpi xnf2edf.log
