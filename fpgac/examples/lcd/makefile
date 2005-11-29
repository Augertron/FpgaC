testit:	lcd.xnf
	@echo 'Only the dates should change in the following diff:'
	-diff lcd.xnf.orig lcd.xnf

lcd.xnf:	lcd.c sram.xnf sun.xnf myclock.xnf debug.xnf
	tmcc -target xnf -S lcd.c sram.xnf sun.xnf myclock.xnf debug.xnf

sram.xnf:	sram.c
	tmcc -target xnf -S -c -T1 sram.c

sun.xnf:	sun.c
	tmcc -target xnf -S -c -T2 sun.c

clean:
	rm -f lcd.xnf sram.xnf sun.xnf
