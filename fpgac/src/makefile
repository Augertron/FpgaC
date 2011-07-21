# FPGA C (fpgac) makefile

HOME=/usr/local
BIN=$(HOME)/bin
LIB=$(HOME)/lib
MAN=$(HOME)/man/man1

#CFLAGS= -O -ggdb -pg
CFLAGS= -O -ggdb
YFLAGS= -d
YLIBS= -ly -ll

# If you want to use flex and bison, try these instead:
LEX = flex
YACC = bison
YFLAGS = -d --yacc
YLIBS = -lfl

# If you want to use lex and yacc, use these:
# YFLAGS= -d
# YLIBS= -ly -ll

OBJ= fpgac.o adders.o output_xilinx.o output_netlist.o minimize_lut.o syntax.o output_vhdl.o \
	output_altera_vqm.o output_edif.o

fpgac.compiler:	$(OBJ)
	$(CC) $(CFLAGS) -o fpgac.compiler $(OBJ) $(YLIBS)

lex.yy.c:	syntax.l names.h y.tab.h
	$(LEX) syntax.l

syntax.o:	lex.yy.c
	$(CC) -c $(CFLAGS) -o syntax.o lex.yy.c

$(OBJ):	names.h
output_xilinx.o output_netlist.o output_edif.o fpgac.o:	outputvars.h

fpgac.sh:	fpgac.skel makefile
	sed 's"LIB_defined_in_the_makefile"$(LIB)"' fpgac.skel > fpgac.sh
	chmod a+x fpgac.sh

clean:
	rm -f *.o *.xnf *.cnf fpgac.compiler y.tab.h lex.yy.c fpgac.sh y.tab.c make.out

lint:	lex.yy.c
	$(YACC) $(YFLAGS) fpgac.y
	lint y.tab.c lex.yy.c adders.c output_xilinx.c minimize_lut.c
	rm -f y.tab.c

install: fpgac.compiler	fpgac.sh
	cp fpgac.sh $(BIN)/fpgac
	cp ../doc/fpgac.1 $(MAN)/fpgac.1
	cp fpgac.compiler $(LIB)/fpgac.compiler
#	ssh -n $(X86HOST) "cd $(SRCDIR); make clean; make fpgac.compiler"
#	cp fpgac.compiler $(BIN)/x86/fpgac.compiler
#	ssh -n $(SUN4HOST) "cd $(SRCDIR); make clean; make fpgac.compiler"
#	cp fpgac.compiler $(BIN)/sun4/fpgac.compiler

newpatchlevel:
	grep '"\$$Revision: ' fpgac.y > patchlevel.h
