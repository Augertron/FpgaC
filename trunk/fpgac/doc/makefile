all:	fpgac.ps fpgac.man

fpgac.man:	fpgac.1
	nroff -man fpgac.1 | col -b > fpgac.man

fpgac.ps:	fpgac.r
	groff -Tps -ms -t fpgac.r > fpgac.ps

showit:
	troff -ms fpgac.r | xtroff

printit:	fpgac.ps
	lpr fpgac.ps
