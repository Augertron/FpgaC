all:	fpgac.man

fpgac.man:	fpgac.1
	nroff -man fpgac.1 | col -b > fpgac.man

clean:

install:
	cp fpgac.1 /usr/local/man/man1/fpgac.1
