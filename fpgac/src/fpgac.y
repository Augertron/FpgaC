%{
/* fpgac.y - FPGA C - A hardware description language
 *		based on a subset of C.
 *
 *	TMCC by Dave Galloway, CSRI, University of Toronto
 */

/*
 * Copyright notice taken from BSD source, and suitably modified:
 *
 * Copyright (c) 1994, 1995, 1996 University of Toronto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	Toronto
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

/* Actually define all the variables in the include files */
#define EXTERN

#include "names.h"
#include "outputvars.h"
#include "output_vhdl.h"

%}

%token		IDENTIFIER LEFTPAREN RIGHTPAREN LEFTCURLY RIGHTCURLY SEMICOLON
%token		INT COMMA INTEGER EQUAL ILLEGAL EQUALEQUAL AND OR TILDE
%token		CHAR SHORT LONG UNSIGNED COLON VOID REGISTER
%token		NOTEQUAL XOR INPUTPORT OUTPUTPORT IF ELSE WHILE BREAK RETURN
%token		INTBITS ADD SHIFTRIGHT SHIFTLEFT SUB UNARYMINUS GREATEROREQUAL
%token		REPLAYSTART REPLAYEND NOT GREATER LESSTHAN LESSTHANOREQUAL
%token		ANDAND OROR BUS_PORT BUS_IDLE STRING PORTFLAGS PLUSEQUAL
%token		MINUSEQUAL SHIFTRIGHTEQUAL SHIFTLEFTEQUAL ANDEQUAL XOREQUAL
%token		OREQUAL LEFTBRACE RIGHTBRACE

%right	EQUAL PLUSEQUAL MINUSEQUAL SHIFTRIGHTEQUAL SHIFTLEFTEQUAL ANDEQUAL XOREQUAL OREQUAL

%left	OROR
%left	ANDAND
%left	OR
%left	XOR
%left	AND
%left	EQUALEQUAL NOTEQUAL
%left	GREATEROREQUAL GREATER LESSTHAN LESSTHANOREQUAL
%left	SHIFTRIGHT SHIFTLEFT
%left	ADD SUB
%left	TILDE UNARYMINUS NOT MINUSMINUS PLUSPLUS

%{

extern FILE *yyin;
char original_inputfilename[1024] = "";
char inputfilename[1024] = "";

char *possible_cpps[] = {
    "/usr/lib/cpp",
    "/usr/ccs/lib/cpp",
    "/lib/cpp",
    0
};

char cppname[1024];
char cppargs[1024];

int nocpp;

/* Architecture specific optimizations */

char *target_arch = "xnf";
int use_clock_enables = 0;
int ffs_zero_at_powerup = 0;

char *legal_arch[] = {
    "xnf",
    "xnf-gates",
    "xnf-roms",
    "xnf-eqns",
    "flex8000",
    "vhdl",
    "vhd",
    "stratix_vqm",
    (char *) 0
};

/* Other optimizations */

int optimization = 1;
int use_carry_select_adders = 1;
int use_dupcheck = 1;

struct variable *powerup_state;

int thread = 0;

int verbose = 0;

char *external_bus_name_format = "%s_v%d";

#define PORT_PIN	0x1
#define PORT_REGISTERED	0x2
#define PORT_PULLUP	0x4
#define PORT_PULLDOWN	0x8
#define PORT_MAXFLAG	0xF

main(int argc, char *argv[]) {
    int i;

    sprintf(cppargs,
	    " -DPORT_REGISTERED=0x%x -DPORT_PIN=0x%x -DPORT_REGISTERED_AND_PIN=0x%x -DPORT_WIRE=0x0 -DPORT_PULLUP=0x%x -DPORT_PULLDOWN=0x%x",
	    PORT_REGISTERED, PORT_PIN, (PORT_REGISTERED | PORT_PIN),
	    PORT_PULLUP, PORT_PULLDOWN);

    genclock = 1;
    output_format = XNFEQNS;
    debug = -1;
    clockname = "CLK";
    resetname = "RESET";

    while (argc > 1 && argv[1][0] == '-') {
	switch (argv[1][1]) {
	case 'p':
	    partname = argv[2];
	    --argc;
	    argv++;
	    break;

	case 'd':
	    if(argv[1][2] == '\0')
		debug = 1;
	    else if((argv[1][2] >= '0')
		     && (argv[1][2] <= '9'))
		debug = atoi(&argv[1][2]);
	    break;

	case 'f':
	    if(!strcmp(&argv[1][2], "no-carry-select"))
		use_carry_select_adders = 0;
	    else if(!strcmp(&argv[1][2], "carry-select"))
		use_carry_select_adders = 1;
	    else if(!strcmp(&argv[1][2], "no-dupcheck"))
		use_dupcheck = 0;
	    else {
		usage();
		exit(1);
	    }
	    break;

	case 'F':
	    if(!strstr(&argv[1][2], "%s")
		|| !strstr(&argv[1][2], "%d")) {
		fprintf(stderr, "fpgac: -Fformat" "string must contain %%s and " "%%d\n");
		exit(1);
	    }
	    external_bus_name_format = &argv[1][2];
	    break;

	case 'a':
	    nocpp = 1;
	    break;

	case 'c':
	    genclock = 0;
	    if(argv[1][2])
		clockname = &argv[1][2];
	    break;

	case 'r':
	    if(argv[1][2])
		resetname = &argv[1][2];
	    break;

	case 'D':
	case 'U':
	case 'I':
	    strncat(cppargs, " ", sizeof(cppargs));
	    strncat(cppargs, argv[1], sizeof(cppargs));
	    break;

	case 'T':
	    thread = atoi(&argv[1][2]);
	    break;

	case 'O':
	    optimization = atoi(&argv[1][2]);
	    break;

	case 's':
	    verbose = 1;
	    break;

	case 't':
	    if(!strcmp(argv[1], "-target")) {
		target_arch = argv[2];
		argv++;
		--argc;
		break;
	    }

	    /* Else fall through to usage message */

	default:
	    usage();
	    exit(1);
	    break;
	}
	--argc;
	argv++;
    }

    for(i = 0; legal_arch[i]; i++) {
	if(!strcmp(target_arch, legal_arch[i]))
	    break;
    }

    if(!legal_arch[i]) {
	error2(target_arch, "is not a supported architecture");
	fprintf(stderr, "use -target architecture_name where architecture_name is one of:\n");
	for(i = 0; legal_arch[i]; i++)
	    fprintf(stderr, "%s\n", legal_arch[i]);
	exit(1);
    }
    if(argc == 2) {
	strcat(original_inputfilename, argv[1]);
	strcat(inputfilename, argv[1]);
	if(freopen(inputfilename, "r", stdin) == (FILE *) NULL) {
	    perror(inputfilename);
	    exit(1);
	}
    }

    if(nocpp)
	yyin = stdin;
    else {
	/* Find out where cpp is on this machine */
	for(i = 0; possible_cpps[i]; i++) {
	    if(access(possible_cpps[i], F_OK) == 0)
		break;
	}
	if(!possible_cpps[i]) {
	    error2("Can't find the C preprocessor", "");
	    exit(1);
	}

	/* Newer Linux cpps won't do substitutions on #pragma lines,
	 * so hide them from cpp and put them back afterwards.
	 */
	strncat(cppname, "sed 's/^#pragma intbits/$pragma intbits/' |",
		sizeof(cppname));
	strncat(cppname, possible_cpps[i], sizeof(cppname));
	strncat(cppname, cppargs, sizeof(cppname));
	strncat(cppname, " | sed 's/^$pragma intbits/#pragma intbits/'",
		sizeof(cppname));
	yyin = popen(cppname, "r");
    }
    outputfile = stdout;

    if(!strcmp(target_arch, "xnf-gates")) {
	output_format = XNFGATES;
	target_arch = "xnf";
    }
    if(!strcmp(target_arch, "xnf-eqns")) {
	output_format = XNFEQNS;
	target_arch = "xnf";
    }
    if(!strcmp(target_arch, "xnf-roms")) {
	output_format = XNFROMS;
	target_arch = "xnf";
    }
    if(!strcmp(target_arch, "xnf")) {
	use_clock_enables = 1;
	ffs_zero_at_powerup = 1;
    } else if(!strcmp(target_arch, "flex8000")) {
	use_clock_enables = 0;
	ffs_zero_at_powerup = 1;
    } else if(!strcmp(target_arch, "vhdl") || !strcmp(target_arch, "vhd")) {
	output_format = VHDL;
	external_bus_name_format = "%s(%d)";
	use_clock_enables = 1;
	ffs_zero_at_powerup = 0;
    } else if(!strcmp(target_arch, "stratix_vqm")) {
	output_format = STRATIX_VQM;
	external_bus_name_format = "%s[%d]";
	use_clock_enables = 1;
	ffs_zero_at_powerup = 1;
    }
    init();
    if(yyparse() || (nerrors > 0))
	exit(1);
    else
	exit(0);
}

usage() {
    fprintf(stderr, "usage: fpgac [options] file.c [file2.xnf ...]\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "    %-20s %s\n", "-S",
	    "produce XNF file, but don't run ppr");
    fprintf(stderr, "    %-20s %s\n", "-O",
	    "optimize circuit for speed and size");
    fprintf(stderr, "    %-20s %s\n", "-p part",
	    "specify Xilinx part name");
    fprintf(stderr, "    %-20s %s\n", "-c",
	    "don't generate 15 Hz clock from internal OSC");
    fprintf(stderr, "    %-20s %s\n", "-D/-U/-I", "cpp arguments");
    fprintf(stderr, "    %-20s %s\n", "-s",
	    "give estimate of circuit size and depth");
    fprintf(stderr, "    %-20s %s\n", "-Tn",
	    "unique name prefix (multi-threaded circuits only)");
    fprintf(stderr, "    %-20s %s\n", "-Fformatstring",
	    "format string used for external bus names");
    fprintf(stderr, "    %-20s %s\n", "-fno-carry-select",
	    "use ripple carry adders and counters (smaller/slower)");
    fprintf(stderr, "    %-20s %s\n", "-fcarry-select",
	    "use carry select adders and counters (default)");
    fprintf(stderr, "    %-20s %s\n", "-target vhd",
	    "generate VHDL format");
    fprintf(stderr, "    %-20s %s\n", "-target stratix_vqm",
	    "generate Altera Stratix VQM format");
    fprintf(stderr, "    %-20s %s\n", "-target xnf-gates",
	    "generate XNF AND/OR/INV format");
    fprintf(stderr, "    %-20s %s\n", "-target xnf-roms",
	    "generate XNF ROM format");
    fprintf(stderr, "    %-20s %s\n", "-target xnf-eqns",
	    "generate XNF EQN format (default)");
    fprintf(stderr, "    %-20s %s\n", "-target flex8000",
	    "generate XNF AND/OR/INV format for Altera FLEX 8K");
    fprintf(stderr, "    %-20s %s\n", "-a", "don't run cpp");
    fprintf(stderr, "    %-20s %s\n", "-v",
	    "don't remove junk ppr output files");
    fprintf(stderr, "    %-20s %s\n", "-dn", "set debug level");
}

extern int inputlineno;

int tempchar = 0;

struct varlist *scopestack;

#define TICKMARK	"tm"
#define CURRENTSTATE	"curstate"

#define GLOBALSCOPE	((struct variable *) NULL)

struct variable *currentscope = GLOBALSCOPE;

struct varlist *breakstack;

int defaultwidth = 16;
int currentwidth = 1;

#define TYPE_INTEGER    0x0001
#define TYPE_SIGNED     0x0002
int currenttype = TYPE_INTEGER|TYPE_SIGNED;

char *bitname(struct bit *b) {
    if(b->name && !(b->flags & BIT_WORD))
	return (b->name);
    if(!b->name)
	b->name = (char *) malloc(MAXNAMELEN);
    if(b->variable->width == 1) {
	if(b->flags & SYM_VCC) {
	    if(output_format == VHDL)
		strncpy(b->name, "'1'", MAXNAMELEN);
	    else
		strncpy(b->name, b->variable->name, MAXNAMELEN);
	} else if(output_format == VHDL) {
	    if(b->variable->name[0] == '_') {
		sprintf(b->name, "T%d_%d%s", thread,
			b->variable->lineno, b->variable->name);
	    } else {
		sprintf(b->name, "T%d_%d_%s", thread,
			b->variable->lineno, b->variable->name);
	    }
	} else
	    sprintf(b->name, "%d_%d_%s", thread,
		    b->variable->lineno, b->variable->name);
    } else if(output_format == VHDL) {
	if(b->flags & BIT_WORD) {
	    if(b->variable->name[0] == '_') {
		sprintf(b->name, "T%d_%d%s(%d)", thread,
			b->variable->lineno,
			b->variable->name, b->bitnumber);
	    } else {
		sprintf(b->name, "T%d_%d_%s(%d)", thread,
			b->variable->lineno,
			b->variable->name, b->bitnumber);
	    }
	} else {
	    if(b->variable->name[0] == '_') {
		sprintf(b->name, "T%d_%d%s_%d", thread,
			b->variable->lineno,
			b->variable->name, b->bitnumber);
	    } else {
		sprintf(b->name, "T%d_%d_%s_%d", thread,
			b->variable->lineno,
			b->variable->name, b->bitnumber);
	    }
	}
    } else {
	sprintf(b->name, "%d_%d_%s_%d", thread,
		b->variable->lineno, b->variable->name, b->bitnumber);
    }
    return (b->name);
}

/* Return the name of a port, to be used in the output file
 * Skip the "_" on the front of the name.
 */

char *externalname(struct bit *b) {
    static char name[MAXNAMELEN];

    if(b->variable->width == 1)
	strncpy(name, b->variable->name + 1, MAXNAMELEN);
    else
	sprintf(name, external_bus_name_format, b->variable->name + 1, b->bitnumber);
    return (name);
}

struct bit *newbit() {
    struct bit *b;

    if(nbits >= NBITS) {
	fprintf(stderr, "fpgac: more than %d bit names\n", NBITS);
	exit(1);
    }
    b = &bits[nbits];
    b->primaries = (struct bitlist *) NULL;
    b->copyof = b;
    nbits++;
    return (b);
}

struct variable *ffoutput();

/* Flags for findvariable */
#define MUSTNOTEXIST	0
#define MUSTEXIST	1
#define MAYEXIST	2

struct variable *findvariable(char *s, int flag, int width) {
    int i, ticked;
    struct varlist *temp;
    struct variable *v;
    struct bit *b;

    ticked = 0;
    for(temp = scopestack; temp; temp = temp->next) {
	if(!strcmp(temp->variable->copyof->name, TICKMARK)) {
	    /* All variables above this point have been
	     * stored in flip flops
	     */
	    ticked = 1;
	    continue;
	}
	if(!strcmp(temp->variable->copyof->name, s)) {
	    if(flag == MUSTNOTEXIST)
		error2(s, "previously declared in this scope");
	    if(ticked) {

		/* The most recent version of the variable
		 * has been stored in a flipflop, and the
		 * clock has since ticked.
		 * Return a new version of the variable that
		 * points at the output of the FF
		 */

		makeff(temp->variable->copyof);
		v = ffoutput(temp->variable->copyof);
		return (v);
	    }
	    return (temp->variable);
	}
    }
    for(i = nvariables - 1; i >= 0; --i) {
	if(!strcmp(variables[i].name, s)) {
	    if(flag == MUSTNOTEXIST) {
		if(variables[i].scope == currentscope)
		    error2(s, "previously declared in this scope");
		else
		    continue;
	    }
	    if((variables[i].scope == currentscope)
		|| !variables[i].scope) {
		v = &variables[i];
		if(!(v->flags & SYM_FUNCTION)) {
		    /* The variable is either global, or uninitialized,
		     * and has not yet been modified in the routine
		     * we are compiling.
		     * Return a new version of the variable that
		     * points at the output of the FF
		     */

		    makeff(&variables[i]);
		    v = ffoutput(&variables[i]);
		    if(v->copyof->flags & SYM_INPUTPORT) {
			v->flags |= SYM_TEMP;
			modifiedvar(v);
		    }
		}
		return (v);
	    }
	}
    }

    if(flag == MUSTEXIST)
	error2(s, "has not been declared");

    if(nvariables >= NVARIABLES) {
	fprintf(stderr, "fpgac: more than %d variable names\n", NVARIABLES);
	exit(1);
    }
    v = &variables[nvariables];
    strncpy(v->name, s, MAXNAMELEN);
    v->width = width;
    v->lineno = inputlineno;
    v->flags = 0;
    v->type = 0;
    v->arraysize = 0;
    v->arrayaddrbits = 0;
    v->arrayref = 0;
    v->arraywrite = 0;
    v->scope = currentscope;
    v->copyof = v;
    nvariables++;
    v->bits = (struct bitlist *) NULL;
    for(i = 0; i < width; i++) {
	b = newbit();
	b->variable = v;
	b->bitnumber = i;
	addtolist(&v->bits, b);
    }
    return (v);
}

/* A parameter may have been seen before it is declared.  Make sure that
 * its width is the defaultwidth at the time of declaration, not the width
 * at the time the function header was encountered.
 *
 * It assumes that this is called before the variable has been used.
 */

changewidth(struct variable *v, int width) {
    struct bit *b;
    int i, saveflags;

    if(width == v->width)
	return;

    v->bits = NULL;
    v->width = width;

    if(debug == 3 || debug == 4)
	printf("Width of '%s' is now %d\n", v->name, width);

    for(i = 0; i < width; i++) {
	b = newbit();
	b->variable = v;
	b->bitnumber = i;
	addtolist(&v->bits, b);
    }

    if(v->flags & SYM_FF) {
	/* redo the makeff operation */
	saveflags = v->flags;
	v->flags &= ~(SYM_FF | SYM_TEMP);
	makeff(v);
	v->flags = saveflags;
    }
}

struct variable *intconstant(int value) {
    int i, temp, width;
    char buf[MAXNAMELEN];
    struct variable *v;
    struct bitlist *bl;
    struct bit *b;

    temp = (value >= 0) ? value : -value;
    for(width = 0; width < 64; width++) {
	if(temp == 0)
	    break;
	temp = temp >> 1;
    }

    /* Add one bit for the sign bit */
    width++;

    sprintf(buf, "constant_%d", value);
    v = findvariable(buf, MAYEXIST, width);
    v->flags |= SYM_LITERAL;
    bl = v->bits;
    temp = value;
    for(i = 0; i < width; i++) {
	b = bl->bit;
	bl = bl->next;
	b->flags |= SYM_KNOWNVALUE;
	b->truth[0] = temp & 0x1;
	temp = temp >> 1;
    }
    return (v);
}

clearvarflag(int bitmask) {
    int i;

    for(i = 0; i < nvariables; i++)
	variables[i].flags &= ~bitmask;
}

clearflag(int bitmask) {
    int i;

    for(i = 0; i < nbits; i++)
	bits[i].flags &= ~bitmask;
}

addtolist(struct bitlist **listp, struct bit *b) {
    struct bitlist *list;

    for(; *listp; listp = &((*listp)->next)) {
	if(b == (*listp)->bit)
	    return;
    }
    list = *listp;
    *listp = (struct bitlist *) malloc(sizeof(struct bitlist));
    (*listp)->bit = b;
    (*listp)->next = list;
}

addtolistwithduplicates(struct bitlist **listp, struct bit *b) {
    struct bitlist *list;

    for(; *listp; listp = &((*listp)->next)) {
    }
    list = *listp;
    *listp = (struct bitlist *) malloc(sizeof(struct bitlist));
    (*listp)->bit = b;
    (*listp)->next = list;
}

addtovlist(struct varlist **listp, struct variable *v) {
    struct varlist *list;

    for(; *listp; listp = &((*listp)->next)) {
	if(strcmp(v->name, (*listp)->variable->name) == 0)
	    return;
    }
    list = *listp;
    *listp = (struct varlist *) malloc(sizeof(struct varlist));
    (*listp)->variable = v;
    (*listp)->next = list;
}

addtovlistwithduplicates(struct varlist **listp, struct variable *v) {
    struct varlist *list;

    for(; *listp; listp = &((*listp)->next));
    list = *listp;
    *listp = (struct varlist *) malloc(sizeof(struct varlist));
    (*listp)->variable = v;
    (*listp)->next = list;
}

mergelists(struct bitlist **listp, struct bitlist *list) {
    for(; list; list = list->next)
	addtolist(listp, list->bit);
}

countlist(struct bitlist *list) {
    int i;

    i = 0;
    for(; list; list = list->next)
	i++;
    return (i);
}

and(int a, int b) {
    return (a & b);
}

or(int a, int b) {
    return (a | b);
}

xor(int a, int b) {
    return (a ^ b);
}

equal(int a, int b) {
    return (a == b);
}

notequal(int a, int b) {
    return (a != b);
}

struct variable *newtempvar(char *s, int width) {
    char buf[MAXNAMELEN];
    struct variable *temp;
    struct bitlist *bl;

    sprintf(buf, "L%d%s", tempchar++, s);
    temp = findvariable(buf, MUSTNOTEXIST, width);
    temp->flags |= SYM_TEMP;
    for(bl = temp->bits; bl; bl = bl->next)
	bl->bit->flags |= BIT_TEMP;
    return (temp);
}

struct variable *copyvar(struct variable *v) {
    char buf[MAXNAMELEN];
    struct variable *temp;
    struct bitlist *bl, *bl2;

    sprintf(buf, "L%d_%s", tempchar++, v->copyof->name);
    temp = findvariable(buf, MUSTNOTEXIST, v->width);
    temp->flags |= (v->flags & SYM_TEMP);
    temp->copyof = v->copyof;
    temp->type = v->type;
    temp->arraysize = v->arraysize;
    temp->arrayaddrbits = v->arrayaddrbits;
    temp->arrayref = v->arrayref;
    temp->arraywrite = v->arraywrite;
    bl2 = v->bits;
    for(bl = temp->bits; bl; bl = bl->next) {
	bl->bit->copyof = bl2->bit->copyof;
	bl2 = bl2->next;
    }
    return (temp);
}

struct variable *ffoutput(struct variable *v) {
    struct variable *result;
    struct bitlist *bl, *bl2;
    int i;

    /* Return a variable that is a reference to the output of
     * the given variable.
     */
    if(v->flags & SYM_LITERAL)
	return (v);
    result = copyvar(v);
    bl = v->bits;
    bl2 = result->bits;
    for(i = 0; i < v->width; i++) {
	bl2->bit->truth[0] = 0;
	bl2->bit->truth[1] = 1;
	addtolist(&bl2->bit->primaries, bl->bit);
	bl = bl->next;
	bl2 = bl2->next;
    }
    return (result);
}

struct bit *freezebit(struct bit *b) {
    struct bit *temp;
    int i;

    temp = newbit();
    temp->name = (char *) malloc(MAXNAMELEN);
    sprintf(temp->name, "T%d_%dL%d_%s", thread, inputlineno, tempchar++, bitname(b->copyof));
    if(b->flags & BIT_TEMP) {
	addtolist(&temp->primaries, b);
	temp->truth[0] = 0;
	temp->truth[1] = 1;
	return (temp);
    } else {
	/* Keep the original variable at the top of the tree, and
	 * push the new temporary down.
	 */
	temp->primaries = b->primaries;
	for(i = 0; i < 16; i++)
	    temp->truth[i] = b->truth[i];
	b->primaries = (struct bitlist *) NULL;
	addtolist(&b->primaries, temp);
	b->truth[0] = 0;
	b->truth[1] = 1;
	return (b);
    }
}

modifiedvar(struct variable *v) {
    struct varlist *temp;

    if(debug == 1 && v->bits && v->bits->bit) {
	printf("   push modified ");
	printbit(v->bits->bit);
    }
    temp = (struct varlist *) malloc(sizeof(struct varlist));
    temp->variable = v;
    temp->next = scopestack;
    scopestack = temp;
}

setbit(struct bit *b, struct bit *value) {
    int i;

    b->primaries = (struct bitlist *) NULL;
    mergelists(&b->primaries, value->primaries);
    b->flags &= ~(SYM_KNOWNVALUE);
    b->flags |= value->flags & (SYM_KNOWNVALUE);
    for(i = 0; i < 16; i++)
	b->truth[i] = value->truth[i];
    b->modifying_states = (struct bitlist *) NULL;
    mergelists(&b->modifying_states, value->modifying_states);
    b->suppressing_states = (struct bitlist *) NULL;
    mergelists(&b->suppressing_states, value->suppressing_states);
    b->modifying_values = (struct bitlist *) NULL;
    mergelists(&b->modifying_values, value->modifying_values);
    b->modifying_states_and_values = (struct bitlist *) NULL;
    mergelists(&b->modifying_states_and_values,
	       value->modifying_states_and_values);
}

bitequal(struct bit *x, struct bit *y) {
    struct bitlist *xbl, *ybl;
    int i, count;

    if(x == y)
	return (1);
    if((x->flags & SYM_INPUTPORT) || (y->flags & SYM_INPUTPORT))
	return (0);
    if((x->flags & SYM_FF) ^ (y->flags & SYM_FF))
	return (0);
    xbl = x->primaries;
    ybl = y->primaries;
    while (xbl && ybl) {
	if(xbl->bit != ybl->bit)
	    return (0);
	xbl = xbl->next;
	ybl = ybl->next;
    }
    if(xbl || ybl)
	return (0);
    count = 1 << countlist(x->primaries);
    for(i = 0; i < count; i++) {
	if(x->truth[i] != y->truth[i])
	    return (0);
    }
    return (1);
}

setvar(struct variable *v, struct variable *value) {
    int i;
    struct bitlist *bl, *bl2;

    bl = v->bits;
    bl2 = value->bits;
    for(i = 0; i < v->width; i++) {
	setbit(bl->bit, bl2->bit);
	bl = bl->next;
	if(bl2->next)
	    bl2 = bl2->next;
    }
}

struct variable *assignment(struct variable *v, struct variable *value) {
    v = copyvar(v);
    setvar(v, value);
    if((v->copyof->flags & SYM_INPUTPORT)
	&& !(v->copyof->flags & SYM_BUSPORT))
	error2("assignment made to input port:", v->copyof->name);
    if(v->copyof->flags & SYM_BUSPORT) {
	v->flags &= ~SYM_TEMP;
    }
    modifiedvar(v);
    return (v);
}

struct variable *assignmentstmt(struct variable *v, struct variable *value) {
    struct variable *result;

    poptargetwidth();
    result = assignment(v, value);
    if(v->copyof->flags & SYM_BUSPORT)
	assignment(v->copyof->enable, intconstant(1));
    return (result);
}

declarefunction(struct variable *v, int width) {
    if(v->returnvalue && (v->returnvalue->width != width) && strcmp(v->name, "_main"))
	error2(v->name, "returns different width than was assumed when it was first encountered");
    makefunction(v, width);
}

makefunction(struct variable *v, int width) {
    struct bitlist *bl;

    if(v->initialstate)
	return;
    v->flags |= SYM_FUNCTION;
    v->initialstate = newtempvar("init", 1);
    v->initialstate->flags = SYM_STATE;
    makeff(v->initialstate);
    v->returnvalue = newtempvar("retval", width);
    for(bl = v->returnvalue->bits; bl; bl = bl->next) {
	bl->bit->flags |= SYM_DONTPULLUP;
	bl->bit->flags &= ~BIT_TEMP;
    }
    v->finalstate = newtempvar("final", 1);
    v->finalstate->flags = SYM_STATE;
    v->finalstate->bits->bit->flags |= SYM_DONTPULLUP;
    v->finalstate->bits->bit->flags &= ~BIT_TEMP;
}

complementbit(struct bit *newbit, struct bit *oldbit) {
    int i;

    for(i = 0; i < 16; i++)
	newbit->truth[i] = !oldbit->truth[i];
    newbit->flags |= (oldbit->flags & SYM_KNOWNVALUE);
    mergelists(&newbit->primaries, oldbit->primaries);
}

struct variable *complement(struct variable *v) {
    int j;
    struct variable *newv;
    struct bitlist *bl, *bl2;

    newv = newtempvar("comp", v->width);
    bl = v->bits;
    bl2 = newv->bits;
    for(j = 0; j < v->width; j++) {
	complementbit(bl2->bit, bl->bit);
	bl = bl->next;
	bl2 = bl2->next;
    }
    modifiedvar(newv);
    return (newv);
}

twoop1bit(struct bit *temp, struct bit *left, struct bit *right, int (*func) ()) {
    struct bit *temp2;
    struct bitlist *r, *t;
    int i, j, k, n;
    int leftcount, rightcount, newcount;

    if(debug == 1) {
	printf("twoop1bit line %d\n", inputlineno);
	printbit(left);
	printbit(right);
    }
    temp->flags &= ~SYM_KNOWNVALUE;
    temp->primaries = 0;
    if((left->flags & SYM_KNOWNVALUE) && (right->flags & SYM_KNOWNVALUE)) {
	temp->truth[0] = (*func) (left->truth[0], right->truth[0]);
	temp->flags = SYM_KNOWNVALUE;
	if(debug == 1)
	    printbit(temp);
	return;
    }
    if(left->flags & SYM_KNOWNVALUE) {
	temp2 = left;
	left = right;
	right = temp2;
    }
    if(right->flags & SYM_KNOWNVALUE) {
	mergelists(&temp->primaries, left->primaries);
	for(i = 0; i < 16; i++)
	    temp->truth[i] = (*func) (left->truth[i], right->truth[0]);
	optimizebit(temp);
	if(debug == 1)
	    printbit(temp);
	return;
    }
    leftcount = countlist(left->primaries);
    rightcount = countlist(right->primaries);
    if(rightcount > leftcount) {
	temp2 = left;
	left = right;
	right = temp2;
	i = leftcount;
	leftcount = rightcount;
	rightcount = i;
    }
    mergelists(&temp->primaries, left->primaries);
    mergelists(&temp->primaries, right->primaries);
    newcount = countlist(temp->primaries);

    if(newcount > 4) {
	left = freezebit(left);
	leftcount = 1;
	temp->primaries = (struct bitlist *) NULL;
	mergelists(&temp->primaries, left->primaries);
	if(rightcount == 4) {
	    right = freezebit(right);
	    rightcount = 1;
	}
	mergelists(&temp->primaries, right->primaries);
	newcount = countlist(temp->primaries);
    }
    for(i = 0; i < (1 << newcount); i++) {
	j = i >> (newcount - leftcount);
	k = 0;
	for(r = right->primaries; r; r = r->next) {
	    k = k << 1;
	    n = newcount - 1;
	    for(t = temp->primaries; t; t = t->next) {
		if(r->bit == t->bit)
		    k |= ((i & (1 << n)) > 0);
		--n;
	    }
	}
	temp->truth[i] = (*func) (left->truth[j], right->truth[k]);
    }
    optimizebit(temp);
    if(debug == 1)
	printbit(temp);
}

struct variable *twoop(struct variable *left, struct variable *right, int (*func) ()) {
    struct variable *temp;
    struct bitlist *bl, *bl2, *bl3;
    int i, width;

    width = MAX(left->width, right->width);
    temp = newtempvar("twoop", width);
    bl = left->bits;
    bl2 = right->bits;
    bl3 = temp->bits;
    for(i = 0; i < width; i++) {
	twoop1bit(bl3->bit, bl->bit, bl2->bit, func);
	if(bl->next)
	    bl = bl->next;
	if(bl2->next)
	    bl2 = bl2->next;
	bl3 = bl3->next;
    }
    modifiedvar(temp);
    return (temp);
}

/* Good only for integers up to sizeof(int).  Used only in portflags() calls */

char *intop(char *left, char *right, int (*func) ()) {
    char temp[32], *result;

    sprintf(temp, "%d", func(atoi(left), atoi(right)));
    result = malloc(strlen(temp) + 1);
    strcpy(result, temp);
    return (result);
}

struct variable *thistick(struct variable *v) {
    struct variable *result;
    struct varlist *scope;

    /* Check to make sure we have a version of this variable that is
     * valid in the current clock period.
     */
    if(v->flags & SYM_LITERAL)
	return (v);
    for(scope = scopestack; scope; scope = scope->next) {
	if(!strcmp(scope->variable->copyof->name, TICKMARK))
	    break;
	if(scope->variable == v)
	    return (v);
    }
    if(v->flags & SYM_TEMP) {
	/* We want to use a temporary or an input from a previous
	 * clock tick.  This can happen in expressions like this:
	 *      r = (a&b) & func();
	 * Put the temporary into a flip flop, clocked by the state
	 * that existed when the temporary was created.  The state
	 * was saved in v->state by assertoutputs().
	 */
	if(!v->state)
	    error2("thistick finds no state in", v->name);
	result = newtempvar("now", v->width);
	result->flags &= ~SYM_TEMP;
	makeff(result);
	addtoff(result, v->state, v);
	result = ffoutput(result);
    } else {
	if(!(v->copyof->flags & SYM_FF))
	    error2("thistick handed non-flipflop", v->name);
	result = ffoutput(v->copyof);
    }
    return (result);
}

struct variable *twoopexpn(struct variable *left, struct variable *right, int (*func) ()) {
    left = thistick(left);
    right = thistick(right);
    return (twoop(left, right, func));
}

/* Remove any primaries that have no effect on the output of b */

optimizebit(struct bit *b) {
    int i, nprimaries, bit, j;
    struct bitlist **p;

    nprimaries = countlist(b->primaries);
    bit = 1 << (nprimaries - 1);
    for(p = &b->primaries; *p;) {
	for(i = 0; i < (1 << nprimaries); i++) {
	    if(b->truth[i] != b->truth[i ^ bit])
		break;
	}
	if(i == (1 << nprimaries)) {
	    if(debug == 1) {
		printf("optimizing %s out of %s\n",
		       bitname((*p)->bit), bitname(b));
		printf("nprimaries %d i %d bit 0x%x\n", nprimaries, i,
		       bit);
		printbit(b);
	    }
	    *p = (*p)->next;
	    for(i = 0; i < (1 << (nprimaries - 1)); i++) {
		switch (bit) {
                case 0x8:
		    j = i;
		    break;

                case 0x4:
		    j = (i & 0x3) | ((i << 1) & 0x8);
		    break;

                case 0x2:
		    j = (i & 0x1) | ((i << 1) & 0xC);
		    break;

                case 0x1:
		    j = i << 1;
		    break;
		}
		b->truth[i] = b->truth[j];
	    }
	    nprimaries--;
	} else {
	    p = &((*p)->next);
	}
	bit /= 2;
    }
    if(nprimaries == 0)
	b->flags |= SYM_KNOWNVALUE;
    else
	b->flags &= ~SYM_KNOWNVALUE;
    if(nprimaries == 1 && !(b->flags & (SYM_DONTPULLUP | SYM_FF))
	&& !(b->primaries->bit->flags
	     & (SYM_INPUTPORT | SYM_FF | SYM_DONTPULLUP))) {

	/* This bit is either the copy or the complement of some
	 * other bit.  Eliminate it altogether.
	 */

	if(b->truth[1])
	    setbit(b, b->primaries->bit);
	else {
	    setbit(b, b->primaries->bit);
	    for(i = 0; i < 16; i++)
		b->truth[i] = !b->truth[i];
	}
    }
}

struct variable *topbit(struct variable *v) {
    struct variable *result;
    struct bitlist *bl;

    result = newtempvar("topbit", 1);
    for(bl = v->bits; bl->next; bl = bl->next);
    setbit(result->bits->bit, bl->bit);
    modifiedvar(result);
    return (result);
}

/* Take all of the bits in v and func them in a balanced tree */

struct variable *treeop(struct variable *v, int (*func) ()) {
    struct variable *result, *temp, *temp2;
    struct bitlist *tbl, *rbl, *vbl, *t2bl;
    int i, j;
    int newwidth;

    newwidth = (v->width - 1) / 4 + 1;
    result = newtempvar("tree", newwidth);
    temp = newtempvar("tree2", v->width);
    temp2 = newtempvar("tree3", v->width + newwidth);
    v = ffoutput(v);
    tbl = temp->bits;
    rbl = result->bits;
    vbl = v->bits;
    t2bl = temp2->bits;
    for(i = 0; i < newwidth; i++) {
	for(j = 0; j < 4; j++) {
	    twoop1bit(tbl->bit, t2bl->bit, vbl->bit, func);
	    t2bl = t2bl->next;
	    t2bl->bit = tbl->bit;
	    tbl = tbl->next;
	    vbl = vbl->next;
	    if(!vbl)
		break;
	}
	rbl->bit = t2bl->bit;
	rbl = rbl->next;
	t2bl = t2bl->next;
    }
    if(newwidth == 1)
	return (result);
    else
	return (treeop(result, func));
}

/* Take all of the bits in bl and func them together */

struct variable *wordop(struct bitlist *bl, int (*func) ()) {
    char used[10000];
    int i, npacked, foundone, nresult, width;
    struct variable *packed, *tempvar, *temp2var;
    struct bitlist *vbl, *pbl, *tbl, *temp, *t2bl;

    width = countlist(bl);

    /* Use first fit bin packing to pack the bits into a small
     * number of 4 input ROMS.
     */
    for(i = 0; i < width; i++)
	used[i] = 0;
    npacked = 0;
    nresult = 0;
    packed = newtempvar("word", width);
    tempvar = newtempvar("word2", width);
    temp2var = newtempvar("word3", width * 2);
    pbl = packed->bits;
    tbl = tempvar->bits;
    t2bl = temp2var->bits;
    while (npacked < width) {
	foundone = 0;
	vbl = bl;
	for(i = 0; i < width; i++) {
	    if(!used[i]) {
		temp = (struct bitlist *) NULL;
		mergelists(&temp, t2bl->bit->primaries);
		mergelists(&temp, vbl->bit->primaries);
		if(countlist(temp) <= 4) {
		    twoop1bit(tbl->bit, t2bl->bit, vbl->bit, func);
		    t2bl = t2bl->next;
		    setbit(t2bl->bit, tbl->bit);
		    tbl = tbl->next;
		    used[i] = 1;
		    foundone = 1;
		    npacked++;
		    break;
		}
	    }
	    vbl = vbl->next;
	}
	if(!foundone) {
	    setbit(pbl->bit, t2bl->bit);
	    pbl = pbl->next;
	    t2bl = t2bl->next;
	    nresult++;
	}
    }
    setbit(pbl->bit, t2bl->bit);
    pbl->next = (struct bitlist *) NULL;
    packed->width = nresult + 1;

    return (treeop(packed, func));
}

/* Test this integer and return 1 or 0 */

struct variable *nonzero(struct variable *v) {
    struct variable *result;

    result = wordop(v->bits, or);
    modifiedvar(result);
    return (result);
}

makeff(struct variable *v) {
    struct bitlist *b;

    if(v->flags & (SYM_FF | SYM_TEMP | SYM_LITERAL))
	return;
    for(b = v->bits; b; b = b->next) {
	b->bit->flags |= SYM_FF;
	b->bit->flags &= ~BIT_TEMP;
	if(b->bit->primaries)
	    error2("This should not happen: makeff found inputs in", bitname(b->bit));
	if(ffs_zero_at_powerup) {
	    b->bit->flags |= SYM_KNOWNVALUE;
	    b->bit->truth[0] = 0;
	}
    }
    v->flags |= SYM_FF;
}

inputport(struct variable *v, struct varlist *vl) {
    struct bitlist *bl;
    struct bit *b;

    v->copyof->flags |= SYM_INPUTPORT;
    for(bl = v->bits; bl; bl = bl->next) {
	b = bl->bit->copyof;
	b->flags &= ~SYM_KNOWNVALUE;
	b->flags |= (SYM_INPUTPORT | BIT_HASPIN);
	if(vl)
	    b->pin = vl->variable->bits->bit->pin;
	else
	    b->pin = (char *) NULL;
	b->primaries = (struct bitlist *) NULL;
	addtolist(&b->primaries, b);
	b->truth[0] = 0;
	b->truth[1] = 1;
	if(vl)
	    vl = vl->next;
	if(vl && !bl->next)
	    error2("too many pin numbers for", v->name);
    }
}

outputport(struct variable *v, struct varlist *vl) {
    struct bitlist *bl;
    struct bit *b;

    makeff(v->copyof);
    v->copyof->flags |= SYM_OUTPUTPORT;
    for(bl = v->bits; bl; bl = bl->next) {
	b = bl->bit->copyof;
	b->flags |= (BIT_HASPIN | BIT_HASFF | SYM_OUTPUTPORT);
	if(vl)
	    b->pin = vl->variable->bits->bit->pin;
	else
	    b->pin = (char *) NULL;
	if(vl)
	    vl = vl->next;
	if(vl && !bl->next)
	    error2("too many pin numbers for", v->name);
    }
}

/* Declare a variable to be a bidirectional port.
 * Basically treat it as an input, but still collect all
 * assignments to it, and output a special version of the
 * FF and IOB at the end.
 */

busport(struct variable *v, struct varlist *vl) {
    struct bitlist *bl;
    struct variable *temp_scope;

    inputport(v, vl);
    makeff(v->copyof);
    if(currentscope != v->copyof->scope) {
	temp_scope = currentscope;
	currentscope = v->copyof->scope;
	v->copyof->enable = newtempvar("enable", 1);
	currentscope = temp_scope;
    } else
	v->copyof->enable = newtempvar("enable", 1);
    v->copyof->enable->flags &= ~SYM_TEMP;
    makeff(v->copyof->enable);
    v->copyof->flags |= SYM_BUSPORT;
    for(bl = v->bits; bl; bl = bl->next) {
	bl->bit->copyof->flags |= (SYM_BUSPORT | BIT_HASPIN | BIT_HASFF);
	bl->bit->copyof->enable = v->copyof->enable->bits->bit;
    }
}

busidle(struct variable *v) {
    if(v->copyof->enable)
	assignment(v->copyof->enable, intconstant(0));
    else
	error2(v->copyof->name, "not a bus_port");
}

/* Set the port flags on a port variable.  Let the program specify which
 * variables have pins, and which ones have FFs.  By default, both
 * inputports and outputports have pins, but only outputports have FFs.
 */

portflags(struct variable *v, char *s) {
    int flag, flag2;
    struct bitlist *bl;
    struct bit *b;

    if(!
	(v->copyof->
	 flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | SYM_BUSPORT))) {
	error2(v->copyof->name, "is not a port variable");
	return;
    }
    flag = atoi(s);
    if((flag < 0) || (flag > PORT_MAXFLAG)) {
	error2(v->copyof->name, "unrecognized flag bits");
	return;
    }
    flag2 = 0;
    if(flag & PORT_PIN)
	flag2 |= BIT_HASPIN;
    if(flag & PORT_REGISTERED)
	flag2 |= BIT_HASFF;
    if(flag & PORT_PULLUP)
	flag2 |= BIT_HASPULLUP;
    if(flag & PORT_PULLDOWN)
	flag2 |= BIT_HASPULLDOWN;
    for(bl = v->bits; bl; bl = bl->next) {
	b = bl->bit->copyof;
	b->flags &=
	    ~(BIT_HASPIN | BIT_HASFF | BIT_HASPULLUP | BIT_HASPULLDOWN);
	b->flags |= flag2;
    }
}

/* Make sure that ff is set to value at the end of state */

addtoff(struct variable *ff, struct variable *state, struct variable *value) {
    struct bitlist *fbl, *sbl, *vbl, *tbl;
    struct variable *temp;

    if(debug == 1) {
	printf("addtoff ff %s state %s value %s\n", ff->name,
	       state->name, value->name);
    }

    if(!(ff->flags & SYM_FF))
	error2("This should not happen: addtoff handed non-ff", ff->name);

    temp = twoop(value, state, and);
    fbl = ff->bits;
    sbl = state->bits;
    vbl = value->bits;
    tbl = temp->bits;
    while (fbl) {
	/* If we're not trying to set the FF back to itself,
	 * and we're not trying to initialize it to zero when
	 * we know that it powered up as zero, then ...
	 */
	if(!((countlist(vbl->bit->primaries) == 1)
	      && (vbl->bit->truth[1] == 1)
	      && (vbl->bit->primaries->bit == fbl->bit))
	    && !(ffs_zero_at_powerup
		 && bitequal(sbl->bit, powerup_state->bits->bit)
		 && (vbl->bit->flags & SYM_KNOWNVALUE)
		 && !vbl->bit->truth[0])
	    ) {
	    if(!(vbl->bit->flags & SYM_KNOWNVALUE)
		|| !vbl->bit->truth[0])
		addtolistwithduplicates(&fbl->bit->suppressing_states,
					sbl->bit);
	    addtolistwithduplicates(&fbl->bit->modifying_states, sbl->bit);
	    addtolistwithduplicates(&fbl->bit->modifying_values, vbl->bit);
	    addtolistwithduplicates(&fbl->bit->modifying_states_and_values,
				    tbl->bit);
	}
	fbl = fbl->next;
	if(vbl->next)
	    vbl = vbl->next;
	if(tbl->next)
	    tbl = tbl->next;
    }
}

tick(struct variable *state) {
    struct variable *temp;

    assertoutputs(state);

    temp = findvariable(TICKMARK, MAYEXIST, 1);
    modifiedvar(temp);
}

ifstmt(struct variable *expn, struct varlist *thenscope, struct varlist *elsescope) {
    struct variable *v, *altv, *temp;
    struct varlist *tempscope;
    struct variable *thenstate, *elsestate, *originalstate;
    struct variable *currentstate;
    struct variable *tempstate;
    int thenticked, elseticked;

    scopestack = thenscope;
    thenstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
    scopestack = elsescope;
    elsestate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
    scopestack = expn->junk;
    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
    originalstate = currentstate;

    thenticked = 0;
    for(tempscope = thenscope; tempscope != expn->junk;
	 tempscope = tempscope->next) {
	if(!strcmp(tempscope->variable->copyof->name, TICKMARK)) {
	    thenticked = 1;
	    break;
	}
    }

    elseticked = 0;
    for(tempscope = elsescope; tempscope != expn->junk;
	 tempscope = tempscope->next) {
	if(!strcmp(tempscope->variable->copyof->name, TICKMARK)) {
	    elseticked = 1;
	    break;
	}
    }

    if(thenticked && !elseticked) {
	tempscope = scopestack;
	scopestack = elsescope;
	tick(elsestate);
	elseticked++;
	tempstate = newtempvar("iftick", 1);
	tempstate->flags = SYM_STATE;
	makeff(tempstate);
	setvar(tempstate, elsestate);
	elsestate = assignment(elsestate, ffoutput(tempstate));
	elsescope = scopestack;
	scopestack = tempscope;
    }

    if(!thenticked && elseticked) {
	tempscope = scopestack;
	scopestack = thenscope;
	tick(thenstate);
	thenticked++;
	tempstate = newtempvar("iftick", 1);
	tempstate->flags = SYM_STATE;
	makeff(tempstate);
	setvar(tempstate, thenstate);
	thenstate = assignment(thenstate, ffoutput(tempstate));
	thenscope = scopestack;
	scopestack = tempscope;
    }

    if(!thenticked && !elseticked) {
	assignment(currentstate, originalstate);
	thenstate = expn;
	elsestate = complement(expn);
    } else {
	/* Record the fact that there was a tick during this if */

	temp = findvariable(TICKMARK, MAYEXIST, 1);
	modifiedvar(temp);
	assignment(currentstate, twoop(thenstate, elsestate, or));
    }

    for(; thenscope != expn->junk; thenscope = thenscope->next) {
	v = thenscope->variable;
	if(v->flags & SYM_TEMP)
	    continue;
	if(v->copyof->flags & (SYM_UPTODATE | SYM_STATE))
	    continue;
	if(!strcmp(v->copyof->name, TICKMARK))
	    continue;
	v->copyof->flags |= SYM_UPTODATE;
	tempscope = scopestack;
	scopestack = elsescope;
	altv = findvariable(v->copyof->name, MUSTEXIST, 0);
	scopestack = tempscope;
	ifmerge(v, altv, thenstate, elsestate, (thenticked || elseticked));
    }
    for(; elsescope != expn->junk; elsescope = elsescope->next) {
	v = elsescope->variable;
	if(v->flags & SYM_TEMP)
	    continue;
	if(v->copyof->flags & (SYM_UPTODATE | SYM_STATE))
	    continue;
	if(!strcmp(v->copyof->name, TICKMARK))
	    continue;
	v->copyof->flags |= SYM_UPTODATE;
	altv = findvariable(v->copyof->name, MUSTEXIST, 0);
	ifmerge(altv, v, thenstate, elsestate, (thenticked || elseticked));
    }
    clearvarflag(SYM_UPTODATE);
}

/* Called from ifstmt().  After an if statement, come up with an
 * expression for the value of any variable that was modified inside
 * one or both arms of the if.  Look at each bit of the variable.  If
 * it remains unchanged, leave it alone.  Otherwise, the new bit is:
 *	new = (thenstate&thenversion) | (elsestate&elseversion)
 */

ifmerge(v, altv, thenstate, elsestate, complexstate)
struct variable *v, *altv, *thenstate, *elsestate;
int complexstate;
{
    struct variable *temp, *temp2, *temp3, *temp4;
    struct bitlist *vbl, *altvbl, *bl, *bl2, *bl3, *bl4;
    int i;

    temp = newtempvar("if", v->width);
    temp2 = newtempvar("then", v->width);
    temp3 = newtempvar("else", v->width);
    temp4 = newtempvar("muxbit", v->width);

    vbl = v->bits;
    altvbl = altv->bits;
    bl = temp->bits;
    bl2 = temp2->bits;
    bl3 = temp3->bits;
    bl4 = temp4->bits;

    for(i = 0; i < v->width; i++) {
	if(bitequal(vbl->bit, altvbl->bit))
	    bl->bit = vbl->bit;
	else if(complexstate) {
	    twoop1bit(bl2->bit, vbl->bit, thenstate->bits->bit, and);
	    twoop1bit(bl3->bit, altvbl->bit, elsestate->bits->bit, and);
	    twoop1bit(bl->bit, bl2->bit, bl3->bit, or);
	} else {
	    muxbit(bl->bit, thenstate->bits->bit, vbl->bit,
		   altvbl->bit, bl2->bit, bl3->bit, bl4->bit);
	}
	vbl = vbl->next;
	altvbl = altvbl->next;
	bl = bl->next;
	bl2 = bl2->next;
	bl3 = bl3->next;
	bl4 = bl4->next;
    }
    assignment(v->copyof, temp);
}

/* Build a multiplexor that returns (condition ? a : b) */

muxbit(result, condition, a, b, tempbit1, tempbit2, tempbit3)
struct bit *result, *condition, *a, *b, *tempbit1, *tempbit2, *tempbit3;
{
    struct bitlist *tempbl;
    int shouldpush, count, mincount;

#define MUXBIT_A		1
#define MUXBIT_B		2
#define MUXBIT_CONDITION	3

    if(debug == 1) {
	printf("muxbit line %d\n", inputlineno);
	printbit(condition);
	printbit(a);
	printbit(b);
    }
    if(condition->flags & SYM_KNOWNVALUE) {
	if(condition->truth[0])
	    setbit(result, a);
	else
	    setbit(result, b);
	if(debug == 1)
	    printbit(result);
	return;
    }
    for(;;) {
	tempbl = (struct bitlist *) NULL;
	mergelists(&tempbl, condition->primaries);
	mergelists(&tempbl, a->primaries);
	mergelists(&tempbl, b->primaries);

	/* If the whole thing will fit into one LUT, then just
	 * call twoop1bit and complementbit to do the work
	 */
	if(countlist(tempbl) <= 4) {
	    twoop1bit(tempbit1, a, condition, and);
	    complementbit(tempbit3, condition);
	    twoop1bit(tempbit2, b, tempbit3, and);
	    twoop1bit(result, tempbit1, tempbit2, or);
	    if(debug == 1)
		printbit(result);
	    return;
	}

	/* If it won't fit, try pushing one of the 3 inputs down.
	 * Pick the one that leaves the fewest number of primaries
	 * left at the top level.  This is not the same as the biggest
	 * one, since the other two may have a lot in common.
	 */

	mincount = 10000;

	tempbl = (struct bitlist *) NULL;
	mergelists(&tempbl, a->primaries);
	mergelists(&tempbl, b->primaries);
	count = countlist(tempbl);
	if((count < mincount) && (countlist(condition->primaries) != 1)) {
	    mincount = count;
	    shouldpush = MUXBIT_CONDITION;
	}

	tempbl = (struct bitlist *) NULL;
	mergelists(&tempbl, condition->primaries);
	mergelists(&tempbl, a->primaries);
	count = countlist(tempbl);
	if((count < mincount) && (countlist(b->primaries) != 1)) {
	    mincount = count;
	    shouldpush = MUXBIT_B;
	}

	tempbl = (struct bitlist *) NULL;
	mergelists(&tempbl, condition->primaries);
	mergelists(&tempbl, b->primaries);
	count = countlist(tempbl);
	if((count < mincount) && (countlist(a->primaries) != 1)) {
	    mincount = count;
	    shouldpush = MUXBIT_A;
	}

	switch (shouldpush) {
	case MUXBIT_CONDITION:
	    condition = freezebit(condition);
	    break;
	case MUXBIT_A:
	    a = freezebit(a);
	    break;
	case MUXBIT_B:
	    b = freezebit(b);
	    break;
	}
    }
}

struct variable *whileloop(expn, initialstate, loopstate, endloopexpn)
struct variable *expn;
struct variable *initialstate, *loopstate, *endloopexpn;
{
    struct variable *currentstate, *endloopstate, *temp;
    struct variable *temp1, *temp2;

    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 0);
    tick(currentstate);

    temp1 = twoop(initialstate, expn, and);
    temp2 = twoop(currentstate, endloopexpn, and);
    setvar(loopstate, twoop(temp1, temp2, or));

    endloopstate = breakstack->variable;
    breakstack = breakstack->next;
    temp1 = twoop(initialstate, complement(expn), and);
    temp2 = twoop(currentstate, complement(endloopexpn), and);
    temp = twoop(temp1, temp2, or);
    setvar(endloopstate, twoop(endloopstate, temp, or));
    assignment(currentstate, ffoutput(endloopstate));
}

init() {
    struct variable *v, *running, *vcc, *myzeroff;
    char buf[128];

    /* Running is a FF whose output is 0 initially, and is 1 thereafter.
     * The initial state is the inverse of the FF output, so is 1 on the
     * first clock cycle, and 0 thereafter.
     */

    sprintf(buf, "%dRunning", thread);
    running = findvariable(buf, MUSTNOTEXIST, 1);
    makeff(running);
    sprintf(buf, "%dZero", thread);
    myzeroff = findvariable(buf, MUSTNOTEXIST, 1);
    makeff(myzeroff);
    setvar(myzeroff, ffoutput(myzeroff));
    vcc = findvariable("VCC", MUSTNOTEXIST, 1);
    vcc->bits->bit->flags |= SYM_VCC;
    running->flags |= SYM_STATE;
    setvar(running, complement(ffoutput(myzeroff)));
    running->bits->bit->flags |= SYM_STATE | SYM_DONTPULLUP;
    v = findvariable("_main", MUSTNOTEXIST, 1);
    declarefunction(v, defaultwidth);
    v->flags |= SYM_FUNCTIONEXISTS;
    v->initialstate->bits->bit->flags &= ~SYM_FF;
    setvar(v->initialstate, complement(ffoutput(running)));
    powerup_state = v->initialstate;
    v = findvariable(CURRENTSTATE, MUSTNOTEXIST, 1);
    v->flags |= SYM_STATE;
}

assertoutputs(struct variable *currentstate) {
    struct variable *v;
    struct varlist *scope;

    /* Make sure that the most recent variable assignments are added
     * to the flip flop inputs (or output pins), as the clock is about
     * to tick.
     */

    for(scope = scopestack; scope; scope = scope->next) {
	v = scope->variable;
	if(!strcmp(v->copyof->name, TICKMARK))
	    break;
	if(v->copyof->flags & (SYM_STATE | SYM_UPTODATE))
	    continue;
	makeff(v->copyof);
	if(v->flags & SYM_TEMP)
	    v->state = currentstate;
	else {
	    addtoff(v->copyof, currentstate, v);
	    modifiedvar(ffoutput(v->copyof));
	}
	v->copyof->flags |= SYM_UPTODATE;
    }
    clearvarflag(SYM_UPTODATE);
}

/* Shift v right by nbits.  If nbits is negative, shift left */

struct variable *shift(struct variable *v, int nbits) {
    struct variable *result;
    int n;
    struct bitlist *bl, *bl2;

    n = v->width - nbits;

    /* If you shift the entire value to the right, return the top bit */
    if(n <= 0)
	return (topbit(v));
    result = newtempvar("shft", n);
    for(bl = v->bits; bl; bl = bl->next) {
	if(nbits <= 0)
	    break;
	--nbits;
    }
    for(bl2 = result->bits; bl2; bl2 = bl2->next) {
	if(nbits >= 0)
	    break;
	bl2->bit->flags |= SYM_KNOWNVALUE;
	bl2->bit->truth[0] = 0;
	nbits++;
    }
    for(; bl2; bl2 = bl2->next) {
	setbit(bl2->bit, bl->bit);
	if(bl->next)
	    bl = bl->next;
    }
    modifiedvar(result);
    return (result);
}

struct variable *shiftbyvar(struct variable *v, struct variable *shiftby, int left) {
    struct variable *result, *x;
    struct variable *temp1, *temp2;
    struct bitlist *bl;
    int i;
    int shiftamount;
    int logwidth;
    int width;

    result = v;
    bl = shiftby->bits;

    /* We don't want to create barrel shifters that are wider than
     * necessary (2**32 is right out).  K&R say that shifting by more
     * than the width of the left hand argument is undefined, but this
     * would be really hard on someone who writes:
     *
     *      x = 1<<y;
     *
     * since fpgac keeps the 1 as a 2 bit wide variable.  So let's use
     * a minimum of 32 (2**5) bits, so that most naive C code will work.
     * We also want to allow:
     *
     *      verywidevar = 1<<y;
     *
     * so let's not do anything that is narrower than the eventual
     * target variable.  And don't go wider than the shiftby variable.
     *
     * Shifts by a constant amount are ok, on the other hand.
     */

    if(shiftby->flags & SYM_LITERAL)
	logwidth = shiftby->width;
    else {
	width = MAX(v->width, 32);
	width = MAX(width, gettargetwidth());
	logwidth = 0;
	while ((1 << logwidth) < width)
	    logwidth++;
	logwidth = MIN(logwidth, shiftby->width);
    }

    for(i = 0; i < logwidth; i++) {
	x = newtempvar("shiftindex", 1);
	x->bits->bit = bl->bit;
	bl = bl->next;
	shiftamount = 1 << i;
	if(left)
	    shiftamount = -shiftamount;
	if(x->bits->bit->flags & SYM_KNOWNVALUE) {
	    if(x->bits->bit->truth[0])
		result = shift(result, shiftamount);
	} else {
	    temp1 = twoop(x, shift(result, shiftamount), and);
	    temp2 = twoop(complement(x), result, and);
	    result = twoop(temp1, temp2, or);
	}
    }
    return (result);
}

struct variable *equals(struct variable *x, struct variable *y) {
    struct variable *temp, *oldy;
    struct bitlist *bx, *by, *bt;
    struct bit *signx, *signy;

    if(x->width < y->width) {
	temp = x;
	x = y;
	y = temp;
    }
    temp = newtempvar("eq", x->width);
    bx = x->bits;
    bt = temp->bits;
    oldy = y;
    y = complement(y);
    for(by = y->bits; by; by = by->next) {
	twoop1bit(bt->bit, bx->bit, by->bit, equal);
	signy = by->bit;
	bx = bx->next;
	bt = bt->next;
    }
    if((x->width > y->width) && (oldy->flags & SYM_LITERAL)) {
	if(signy->truth[0]) {	/* Original Y was a positive constant */
	    for(; bx; bx = bx->next) {
		setbit(bt->bit, bx->bit);
		bt = bt->next;
	    }
	} else {		/* Original Y was a negative constant */
	    for(; bx; bx = bx->next) {
		complementbit(bt->bit, bx->bit);
		bt = bt->next;
	    }
	}
    } else {
	if(bx) {
	    /* X is wider than Y, and Y is a variable.
	     * We'll say that X and Y are equal as long as
	     * they match in all of the bits that exist in Y,
	     * and all of the rest of the bits in X are the
	     * same (either all 0 or all 1).  But if X<0 and
	     * Y>0, then they aren't equal.
	     */
	    signx = bx->bit;
	    twoop1bit(bt->bit, signx, signy, and);
	    bt = bt->next;
	    bx = bx->next;
	}
	for(; bx; bx = bx->next) {
	    twoop1bit(bt->bit, bx->bit, signx, notequal);
	    bt = bt->next;
	}
    }
    return (complement(nonzero(temp)));
}

/* Called just before printing output.  Look at the list of values that
 * should be assigned to each flip flop, and create an input multiplexor
 * for it.
 *
 * input = ( !(s1|s2|s3...) ^ FFoutput ) | (s1^v1) | (s2^v2) | (s3^v3) ...
 */

makeffinputs() {
    struct variable *states, *new;
    struct variable *temp, *temp1, *temp2;
    struct bitlist *vbl, *sbl;
    struct bit *b;
    int i;

    for(i = 0; i < nbits; i++) {
	b = &bits[i];
	if(!(b->flags & SYM_FF))
	    continue;
	if(!b->modifying_values)
	    continue;
	if(b->primaries && !(b->flags & SYM_BUSPORT))
	    error2("This should not happen: makeffinputs found inputs in",
		   bitname(b));
	if((b->flags & SYM_OUTPUTPORT) && !(b->flags & BIT_HASFF)) {
	    /* This is an output from the circuit, and they
	     * don't want a flip-flop.  If it is only set in
	     * one state, just set it to that value.  If it
	     * is set in more than one state, guard each of the
	     * values by the state that set it, and or them
	     * together.
	     */
	    if(countlist(b->modifying_values) > 1) {
		new = wordop(b->modifying_states_and_values, or);
		setbit(b, new->bits->bit);
	    } else {
		setbit(b, b->modifying_values->bit);
	    }
	    continue;
	}
	if((countlist(b->modifying_values) == 1) && use_clock_enables) {
	    /* If the FF is set in only one state, then we
	     * can use the clock_enable input on the FF if
	     * it exists in this architecture.  The state is
	     * typically a reference to another flip-flop, if
	     * so, pull it up and avoid generating a buffer.
	     * Makes the output easier to understand.
	     */
	    b->clock_enable = b->modifying_states->bit;
	    if((countlist(b->clock_enable->primaries) == 1)
		&& (b->clock_enable->truth[1] == 1))
		b->clock_enable = b->clock_enable->primaries->bit;
	    setbit(b, b->modifying_values->bit);
	    continue;
	}

	/* If we aren't using the clock enable input, then the
	 * FF output has to be fed back to the FF input in all
	 * states where the value is not modified.
	 */

	b->flags &= ~SYM_KNOWNVALUE;
	addtolist(&b->primaries, b);
	b->truth[0] = 0;
	b->truth[1] = 1;

	if(countlist(b->modifying_values) > 2) {
	    temp = newtempvar("makeffinputs", 1);
	    temp->bits = (struct bitlist *) NULL;
	    addtolist(&temp->bits, b);
	    if(b->suppressing_states)
		states = wordop(b->suppressing_states, or);
	    else
		states = intconstant(0);
	    temp1 = complement(states);
	    temp2 = ffoutput(temp);
	    states = twoop(temp1, temp2, and);
	    addtolistwithduplicates(&b->modifying_states_and_values,
				    states->bits->bit);
	    new = wordop(b->modifying_states_and_values, or);
	    setbit(b, new->bits->bit);
	} else {
	    temp = newtempvar("makeffinputs", 1);
	    temp2 = newtempvar("makeffinputs2", 1);
	    new = newtempvar("ffin", 1);
	    new->bits->bit = b;
	    sbl = b->modifying_states;
	    vbl = b->modifying_states_and_values;
	    while (vbl) {
		temp->bits = (struct bitlist *) NULL;
		addtolist(&temp->bits, sbl->bit);
		temp2->bits = (struct bitlist *) NULL;
		addtolist(&temp2->bits, vbl->bit);
		if(!bitequal(vbl->bit, sbl->bit)) {
		    new = twoop(twoop(complement(temp),
				      new, and), temp2, or);
		} else
		    new = twoop(new, temp2, or);
		vbl = vbl->next;
		sbl = sbl->next;
	    }
	    setbit(b, new->bits->bit);
	}
    }
}

/* Look through all bits and eliminate duplicates */

struct bit *hash[NBITS];

struct bit *dupcheck(struct bit *b, int depth) {
    struct bitlist *bl;
    struct bit *newbit;
    int hashval;

    if(b->flags & (SYM_UPTODATE | SYM_KNOWNVALUE))
	return ((struct bit *) NULL);
    if((depth != 0) && (b->flags & SYM_FF))
	return ((struct bit *) NULL);

    /* In case of logic loops */
    if(depth > 1000)
	return ((struct bit *) NULL);

    hashval = 0;
    for(bl = b->primaries; bl; bl = bl->next) {
	hashval += NBITS / 4;
	newbit = dupcheck(bl->bit, depth + 1);
	if(newbit)
	    bl->bit = newbit;
	hashval ^= (bl->bit - bits);
    }
    if(hashval < 0)
	hashval = -hashval;
    hashval = hashval % NBITS;
    while (hash[hashval]) {
	if(bitequal(hash[hashval], b)) {
	    if(debug == 1) {
		printf("duplicate bit deleted\n");
		printbit(b);
		printbit(hash[hashval]);
	    }
	    return (hash[hashval]);
	}
	hashval++;
	if(hashval >= NBITS)
	    hashval = 0;
    }
    hash[hashval] = b;
    b->flags |= SYM_UPTODATE;
    return ((struct bit *) NULL);
}

checkforduplicates() {
    struct bit *b;
    int i;

    for(i = 0; i < nbits; i++) {
	b = &bits[i];
	if(!(b->flags & (SYM_OUTPUTPORT | SYM_FF)))
	    continue;
	dupcheck(b, 0);
    }
    clearflag(SYM_UPTODATE);
}

static int maxdepth;
static int deepest;

finddepth(struct bit *bit, int top) {
    int depth;
    struct bitlist *b;
    int nprimaries;

    if((bit->flags & (SYM_INPUTPORT | SYM_BUSPORT)) == SYM_INPUTPORT)
	return (0);
    if(!top && (bit->flags & SYM_FF))
	return (0);
    if(bit->flags & BIT_DEPTHVALID)
	return (bit->depth);

    /* Check for logic loop causing infinite recursion */

    if(bit->flags & SYM_UPTODATE)
	return (1000000);
    bit->flags |= SYM_UPTODATE;
    depth = 0;
    nprimaries = 0;
    for(b = bit->primaries; b; b = b->next) {
	b->bit->depth = finddepth(b->bit, 0);
	b->bit->flags |= BIT_DEPTHVALID;
	if(depth < b->bit->depth)
	    depth = b->bit->depth;
	nprimaries++;
    }

    /* If there is only one input to this 4-LUT, then it will just be
     * a buffer or an inverter, and will not add to the depth.
     * Otherwise, the depth is one more than the deepest input.
     */
    if(nprimaries > 1)
	depth = depth + 1;
    return (depth);
}

/* Look for functions that were called, but never defined */

checkundefined() {
    int i;

    for(i = 0; i < nvariables; i++) {
	if((variables[i].flags & (SYM_FUNCTION | SYM_FUNCTIONEXISTS))
	    == SYM_FUNCTION)
	    error2(variables[i].name, "is an undefined function");
    }
}

output() {
    prune();

    if(output_format == VHDL) {
	output_vhdl();
    } else if(output_format == STRATIX_VQM) {
	output_vqm("stratix");
    } else {
	output_XNF();
    }
}

/* Go through the circuit, and mark all of the elements that can affect
 * an output.  The rest can be thrown away.
 */

prune() {
    int changed, n, i, size;
    struct bitlist *bl;

    changed = 1;
    while (changed) {
	changed = 0;
	for(n = 0; n < nbits; n++) {
	    if(bits[n].flags &
		(SYM_AFFECTSOUTPUT | SYM_OUTPUTPORT | SYM_BUSPORT)) {
		bits[n].flags |= SYM_AFFECTSOUTPUT;
		optimizebit(&bits[n]);
		for(bl = bits[n].primaries; bl; bl = bl->next) {
		    if(!(bl->bit->flags & SYM_AFFECTSOUTPUT)) {
			bl->bit->flags |= SYM_AFFECTSOUTPUT;
			changed = 1;
		    }
		}
		if(bits[n].enable
		    && !(bits[n].enable->flags & SYM_AFFECTSOUTPUT)) {
		    bits[n].enable->flags |= SYM_AFFECTSOUTPUT;
		    changed = 1;
		}
		if(bits[n].clock_enable
		    && !(bits[n].clock_enable->
			 flags & SYM_AFFECTSOUTPUT)) {
		    bits[n].clock_enable->flags |= SYM_AFFECTSOUTPUT;
		    changed = 1;
		}
                if(bits[n].variable && bits[n].variable->arraysize) {
                    if(bits[n].variable->arraywrite && bits[n].variable->arraywrite->bits) {
                        for(i=0,bl = bits[n].variable->arraywrite->bits;i<bits[n].variable->arrayaddrbits;i++) {
                            if(!(bl->bit->flags & SYM_AFFECTSOUTPUT)) {
                                bl->bit->flags |= SYM_AFFECTSOUTPUT;
                                changed = 1;
                            }
                            if(bl->next) bl = bl->next;
                        }
                    }
                    if(bits[n].variable->arrayref && bits[n].variable->arrayref->bits) {
                        for(i=0,bl = bits[n].variable->arrayref->bits;i<bits[n].variable->arrayaddrbits;i++) {
                            if(!(bl->bit->flags & SYM_AFFECTSOUTPUT)) {
                                bl->bit->flags |= SYM_AFFECTSOUTPUT;
                                changed = 1;
                            }
                            if(bl->next) bl = bl->next;
                        }
                    }
                }
	    }
	}
    }
}

sizelog2(int size) {
        if(size <= 16) {
            return 4;
        } else if(size <= 32) {
            return 5;
        } else if(size <= 64) {
            return 6;
        } else if(size <= 128) {
            return 7;
        } else if(size <= 256) {
            return 8;
        } else if(size <= 512) {
            return 9;
        } else if(size <= (1*1024)) {
            return 10;
        } else if(size <= (2*1024)) {
            return 11;
        } else if(size <= (4*1024)) {
            return 12;
        } else if(size <= (8*1024)) {
            return 13;
        } else if(size <= (16*1024)) {
            return 14;
        } else if(size <= (32*1024)) {
            return 15;
        } else if(size <= (64*1024)) {
            return 16;
        } else {
            return 32;
        }
}

printbit(struct bit *b) {
    struct bitlist *bl;
    int nprimaries, j;

    printf("%-20s %2lx %2d ", bitname(b), b->flags, b->depth);
    printf("%-20s ", bitname(b->copyof));
    if(b->flags & SYM_FF)
	printf("FF ");
    else
	printf("   ");
    nprimaries = countlist(b->primaries);
    for(j = 0; j < (1 << nprimaries); j++)
	printf("%d ", b->truth[j]);
    for(bl = b->primaries; bl; bl = bl->next)
	printf("%s ", bitname(bl->bit));
    printf("\n");
}

printtree(struct bit *b, int offset) {
    int i;
    struct bitlist *bl;

    for(i = 0; i < offset; i++)
	putchar(' ');
    if(offset > 100 || ((offset > 30) && (b->flags & SYM_UPTODATE))) {
	printf("...\n");
	return;
    }
    b->flags |= SYM_UPTODATE;
    printf("%s\n", bitname(b));
    if((b->flags & (SYM_INPUTPORT | SYM_BUSPORT)) == SYM_INPUTPORT)
	return;
    if(offset && (b->flags & SYM_FF))
	return;
    for(bl = b->primaries; bl; bl = bl->next)
	printtree(bl->bit, offset + 4);
}

debugoutput() {
    int i, n;

    if(nerrors > 0)
	return;
    printf("Start of debug output\n\n");
    maxdepth = 0;
    for(n = 0; n < nbits; n++) {
	if(bits[n].flags & SYM_AFFECTSOUTPUT) {
	    bits[n].depth = finddepth(&bits[n], 1);
	    bits[n].flags |= BIT_DEPTHVALID;
	    if(maxdepth < bits[n].depth) {
		maxdepth = bits[n].depth;
		deepest = n;
	    }
	}
    }
    if(debug >= 0) {
	for(i = 0; i < nbits; i++)
	    printbit(&bits[i]);
    }
    clearflag(SYM_UPTODATE);
    for(i = 0; i < nbits; i++) {
	if(!(bits[i].flags & SYM_AFFECTSOUTPUT))
	    continue;
	if(bits[i].flags & (SYM_OUTPUTPORT | SYM_FF)) {
	    putchar('\n');
	    printtree(&bits[i], 0);
	}
    }
    printf("\n");
    printf("%d variables %d bits\n", nvariables, nbits);
    printf("maximum depth %d driving %s\n", maxdepth,
	   bitname(&bits[deepest]));
    printf("%d roms, %d flipflops,", nroms, nff);
    printf(" %d I/O signals (%d input, %d output, %d bidir)\n",
	   ninpins + noutpins + nbidirpins, ninpins, noutpins, nbidirpins);
    printf("Inputs  #roms\n");
    for(i = 0; i < 5; i++)
	printf("%4d %8d\n", i, inputcounts[i]);

    if(verbose) {
	if(output_format == XNFGATES)
	    fprintf(stderr, "Estimate of design size:\n");
	else
	    fprintf(stderr, "Design size:\n");
	fprintf(stderr, "%d lookup tables, %d flipflops,", nroms, nff);
	fprintf(stderr,
		" %d I/O signals (%d input, %d output, %d bidir)\n",
		ninpins + noutpins + nbidirpins, ninpins, noutpins,
		nbidirpins);
	fprintf(stderr, "Lookup table details: (Inputs, #LUTs)\n");
	for(i = 0; i < 5; i++)
	    fprintf(stderr, "%4d %8d\n", i, inputcounts[i]);
	fprintf(stderr, "Maximum depth: %d levels to produce %s\n",
		maxdepth, bitname(&bits[deepest]));
    }
}

/* Keep a stack of the variables that are the targets of any assignment
 * statements, as we parse them from left to right.  Some of the expression
 * operators (notably shift) need to know how large the result will be in
 * order to optimize the generated circuit.
 */

#define MAXASSIGNMENTDEPTH	100

struct variable *targetstack[MAXASSIGNMENTDEPTH + 1];
int targetptr = 0;

pushtargetwidth(struct variable *v) {
    if(targetptr >= MAXASSIGNMENTDEPTH) {
	error2("Too many nested assignment statements", "");
	return;
    }
    targetstack[++targetptr] = v;
}

poptargetwidth() {
    if(targetptr <= 0) {
	error2("This should not happen: target stack underflow", "");
	return;
    }
    --targetptr;
}

gettargetwidth() {
    if(targetstack[targetptr])
	return (targetstack[targetptr]->width);
    else
	return (0);
}

%}

%%

program:	sourcefile
		{
		    if(nerrors > 0)
		        return;
		    makeffinputs();
		    if(use_dupcheck)
		        checkforduplicates();
		    checkundefined();
		    output();
//                  debugoutput();
		}

sourcefile:	/* empty */
		| sourcefile function
		| sourcefile globaldeclaration

function:	functionhead LEFTCURLY funcbody RIGHTCURLY
		{
		    struct variable *currentstate;

		    $1.v->flags |= SYM_FUNCTIONEXISTS;
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    assertoutputs(currentstate);
		    setvar($1.v->finalstate, twoop($1.v->finalstate, currentstate, or));

		    scopestack = (struct varlist *) NULL;
		    currentscope = GLOBALSCOPE;
		}

functionhead:	optionaltype functionname LEFTPAREN parameterlist RIGHTPAREN parameterdeclarations
		{
		    struct variable *currentstate;
		    struct varlist *vl;

		    $2.v->flags |= SYM_FUNCTION_DECLARED;
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    if($2.v->initialstate->bits->bit->flags & SYM_FF)
		        assignment(currentstate, ffoutput($2.v->initialstate));
		    else
		        assignment(currentstate, $2.v->initialstate);
		    currentstate->flags |= SYM_STATE;

		    /* function may have been called before it was
		    * defined.  Replace the temporary arguments
		    * with the real ones, now that we know what they
		    * are.
		    */
		    for(vl = $2.v->arguments; vl; vl = vl->next) {
		        if(!$4.v->junk)
		            break;
		        setvar($4.v->junk->variable, vl->variable);
		        vl->variable = $4.v->junk->variable;
		        $4.v->junk = $4.v->junk->next;
		    }
		    if($4.v->junk) {
		        for(vl = $2.v->arguments; vl; vl = vl->next) {
		            if(!vl->next)
		                break;
		        }
		        if(vl)
		            vl->next = $4.v->junk;
		        else
		            $2.v->arguments = $4.v->junk;
		    }
		    $$ = $2;
		}

functionname:	identifier
		{
		    currentscope = $1.v;
		    declarefunction($1.v, currentwidth);
		    $$ = $1;
		}

parameterlist:	/* empty */
		{
		    $$.v = newtempvar("params", 1);
		    $$.v->junk = (struct varlist *) NULL;
		}

		| paramlist2

paramlist2:	optionaltype newidentifier
		{
		    $$ = $2;
		    $$.v->junk = (struct varlist *) NULL;
		    addtovlist(&$$.v->junk, $2.v);
		    makeff($2.v);
		}

		| paramlist2 COMMA optionaltype newidentifier
		{
		    $$ = $1;
		    addtovlistwithduplicates(&$$.v->junk, $4.v);
		    makeff($4.v);
		}

parameterdeclarations:	/* empty */

		| parameterdeclarations parameterdeclaration

parameterdeclaration:	 typename paramdeclist SEMICOLON

		| pragma

paramdeclist:	oldidentifier
		{
		    changewidth($1.v->copyof, currentwidth);
		    changewidth($1.v, currentwidth);
		}

		| paramdeclist COMMA oldidentifier
		{
		    changewidth($3.v->copyof, currentwidth);
		    changewidth($3.v, currentwidth);
		}

funcbody:	 declarations stmts

declarations:   /* empty */

		| declarations declaration

globaldeclaration:	optionaltype globalvarlist SEMICOLON
		{
		    if($$.type == 0)
		        error2("declaration has no type", "");
		}

		| pragma

declaration:	 typename varlist SEMICOLON

		| pragma

optionaltype:	/* empty */
		{
		    $$.type = currenttype = 0;
		    currentwidth = 0;
		}

		| typename

typename:	VOID
		{
		    $$.type = currenttype = 0;
		    currentwidth = 0;
		}

		| INT
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = defaultwidth;
		}

		| REGISTER
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = defaultwidth;
		}

		| REGISTER INT
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = defaultwidth;
		}

		| CHAR
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 8;
		}

		| SHORT
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 16;
		}

		| LONG
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 32;
		}

		| LONG LONG
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 64;
		}

		| SHORT INT
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 16;
		}

		| LONG INT
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 32;
		}

		| LONG LONG INT
		{
		    $$.type = currenttype = TYPE_INTEGER|TYPE_SIGNED;
		    currentwidth = 64;
		}

		| UNSIGNED CHAR
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 8;
		}

		| UNSIGNED INT
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = defaultwidth;
		}

		| UNSIGNED SHORT
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 16;
		}

		| UNSIGNED LONG
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 32;
		}

		| UNSIGNED LONG LONG
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 64;
		}

		| UNSIGNED SHORT INT
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 16;
		}

		| UNSIGNED LONG INT
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 32;
		}

		| UNSIGNED LONG LONG INT
		{
		    $$.type = currenttype = TYPE_INTEGER;
		    currentwidth = 64;
		}

varlist:	varlistmember

		| varlist COMMA varlistmember

varlistmember:	newidentifier

		| newidentifier EQUAL
		{ pushtargetwidth($1.v); }

		expn
		{ assignmentstmt($1.v, $4.v); }

		| identifier LEFTPAREN RIGHTPAREN
		{ declarefunction($1.v, currentwidth); }

globalvarlist:	 globalvarlistmember

		| globalvarlist COMMA globalvarlistmember

globalvarlistmember: newidentifier

		| newidentifier EQUAL expn
		{ error2("initialization of global variables not supported", ""); }

		| functionname LEFTPAREN parameterlist RIGHTPAREN
		{
		    declarefunction($1.v, currentwidth);
		    currentscope = GLOBALSCOPE;
		    if($3.v->junk)
		        error2("parameters not supported in function type specification of", $1.v->name);
		}

pragma:         INTBITS INTEGER
                        { defaultwidth = atoi($2.s); }

                | INPUTPORT LEFTPAREN oldidentifier pinlist RIGHTPAREN
                        { inputport($3.v, $4.v->junk); }

                | OUTPUTPORT LEFTPAREN oldidentifier pinlist RIGHTPAREN
                        { outputport($3.v, $4.v->junk); }

                | BUS_PORT LEFTPAREN oldidentifier pinlist RIGHTPAREN
                        { busport($3.v, $4.v->junk); }

                | BUS_IDLE LEFTPAREN oldidentifier RIGHTPAREN
                        { busidle($3.v); }

                | PORTFLAGS LEFTPAREN oldidentifier COMMA int_expr RIGHTPAREN
                        { portflags($3.v, $5.s); }

                | pragma SEMICOLON

stmts:		/* empty */

		| stmts stmt

stmt:	SEMICOLON

		| ifstmt

		| whileloop

		| breakstmt SEMICOLON

		| returnstmt SEMICOLON

		| expn SEMICOLON

		| LEFTCURLY stmts RIGHTCURLY

pinlist:	/* Empty */
		{
		    $$.v = newtempvar("pins", 1);
		    $$.v->bits->bit->pin = (char *) NULL;
		    $$.v->junk = (struct varlist *) NULL;
		}

		| pinlist COMMA INTEGER
		{
		    struct variable *temp;

		    $$.v = $1.v;
		    temp = newtempvar("pins", 1);
		    temp->bits->bit->pin = $3.s;
		    addtovlistwithduplicates(&$$.v->junk, temp);
		}

		| pinlist COMMA IDENTIFIER
		{
		    struct variable *temp;

		    $$.v = $1.v;
		    temp = newtempvar("pins", 1);
		    temp->bits->bit->pin = $3.s;
		    addtovlistwithduplicates(&$$.v->junk, temp);
		}

		| pinlist COMMA STRING
		{
		    struct variable *temp;

		    $$.v = $1.v;
		    temp = newtempvar("pins", 1);
		    temp->bits->bit->pin = $3.s;
		    addtovlistwithduplicates(&$$.v->junk, temp);
		}

ifstmt:	ifhead stmt
		{
		    struct variable *currentstate;
		    struct varlist *thenstack;

		    thenstack = scopestack;
		    scopestack = $1.v->junk;
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    assignment(currentstate, twoop(currentstate, complement($1.v), and));
		    if(countlist(currentstate->bits) > 1)
		        assignment(currentstate, ffoutput(currentstate));
		    ifstmt($1.v, thenstack, scopestack);
		}

		| ifhead stmt ELSE
		{
		    struct variable *currentstate;

		    $$.v = newtempvar("", 1);
		    $$.v->junk = scopestack;
		    scopestack = $1.v->junk;
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    assignment(currentstate, twoop(currentstate, complement($1.v), and));
		    if(countlist(currentstate->bits) > 1)
		        assignment(currentstate, ffoutput(currentstate));
		}

		stmt
		{
		    ifstmt($1.v, $4.v->junk, scopestack);
		}

ifhead:		IF LEFTPAREN expn RIGHTPAREN
		{
		    struct variable *currentstate;

		    $$.v = nonzero($3.v);
		    $$.v->junk = scopestack;
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    assignment(currentstate, twoop(currentstate, $$.v, and));
		    if(countlist(currentstate->bits) > 1)
		        assignment(currentstate, ffoutput(currentstate));
		}

whileloop:	WHILE
		{
		    $$.v = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    pushinputstream();
		    saveinput();
		}

		LEFTPAREN expn
		{
		    struct variable *loopstate, *currentstate, *endloop;
		    struct varlist *vl;

		    stopsavinginput();
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    tick(currentstate);

		    loopstate = newtempvar("looptop", 1);
		    loopstate->flags = SYM_STATE;
		    makeff(loopstate);
		    assignment(currentstate, ffoutput(loopstate));
		    $$.v = loopstate;
		    endloop = newtempvar("endloop", 1);
		    endloop->flags = SYM_STATE;
		    makeff(endloop);
		    vl = (struct varlist *) malloc(sizeof(struct varlist));
		    vl->next = breakstack;
		    breakstack = vl;
		    breakstack->variable = endloop;
		}

		RIGHTPAREN stmt
		{ replayinput(); }

		REPLAYSTART expn REPLAYEND
		{
		    struct variable *temp1, *temp2;

		    popinputstream();
		    temp1 = nonzero($4.v);
		    temp2 = nonzero($10.v);
		    whileloop(temp1, $2.v, $5.v, temp2);
		}

breakstmt:	BREAK
		{
		    struct variable *currentstate, *endloop, *neverstate;

		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    endloop = breakstack->variable;
		    setvar(endloop, twoop(endloop, currentstate, or));
		    tick(currentstate);
		    neverstate = newtempvar("never", 1);
		    assignment(currentstate, neverstate);
		}

returnstmt:	RETURN
		{
		    struct variable *currentstate, *neverstate;

		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    setvar(currentscope->finalstate, twoop(currentscope->finalstate, currentstate, or));
		    assertoutputs(currentstate);
		    neverstate = newtempvar("never", 1);
		    assignment(currentstate, neverstate);
		}

		| RETURN expn
		{
		    struct variable *currentstate, *neverstate, *retval;

		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);

		    /* If there is only one return statement in the
		    * function, then we don't have to build a complex
		    * expression for the return value.  If there is more
		    * than one return statement, then the return value
		    * is the or of all of the returned values anded with
		    * their states.
		    */

		    retval = currentscope->returnvalue;

		    if(!retval->state) {
		        setvar(retval, $2.v);
		        retval->state = currentstate;
		    } else {
		        if(!(retval->flags & SYM_MULTIPLE_RETURNS)) {
		            retval->flags |= SYM_MULTIPLE_RETURNS;
		            setvar(retval, twoop(retval, retval->state, and));
		        }
		        setvar(retval, twoop(retval, twoop($2.v, currentstate, and), or));
		    }
		    setvar(currentscope->finalstate, twoop(currentscope->finalstate, currentstate, or));
		    assertoutputs(currentstate);
		    neverstate = newtempvar("never", 1);
		    assignment(currentstate, neverstate);
		}

expn:		term

		| expn AND expn
		{ $$.v = twoopexpn($1.v, $3.v, and); }

		| expn OR expn
		{ $$.v = twoopexpn($1.v, $3.v, or); }

		| expn ANDAND expn
		{
		    struct variable *temp1, *temp2;

		    temp1 = nonzero($1.v);
		    temp2 = nonzero($3.v);
		    $$.v = twoopexpn(temp1, temp2, and);
		}

		| expn OROR expn
		{
		    struct variable *temp1, *temp2;

		    temp1 = nonzero($1.v);
		    temp2 = nonzero($3.v);
		    $$.v = twoopexpn(temp1, temp2, or);
		}

		| expn XOR expn
		{ $$.v = twoopexpn($1.v, $3.v, xor); }

		| expn ADD expn
		{ $$.v = add($1.v, $3.v); }

		| expn SUB expn
		{ $$.v = sub($1.v, $3.v); }

		| expn EQUALEQUAL expn
		{ $$.v = equals($1.v, $3.v); }

		| expn NOTEQUAL expn
		{ $$.v = complement(equals($1.v, $3.v)); }

		| expn GREATEROREQUAL expn
		{ $$.v = complement(topbit(sub($1.v, $3.v))); }

		| expn GREATER expn
		{
		    struct variable *temp1, *temp2;

		    $$.v = sub($1.v, $3.v);
		    temp1 = topbit($$.v);
		    temp2 = nonzero($$.v);
		    $$.v = twoop(complement(temp1), temp2, and);
		}

		| expn LESSTHANOREQUAL expn
		{
		    struct variable *temp1, *temp2;

		    $$.v = sub($1.v, $3.v);
		    temp1 = topbit($$.v);
		    temp2 = complement(nonzero($$.v));
		    $$.v = twoop(temp1, temp2, or);
		}

		| expn LESSTHAN expn
		{ $$.v = topbit(sub($1.v, $3.v)); }

		| expn SHIFTRIGHT expn
		{ $$.v = shiftbyvar($1.v, $3.v, 0); }

		| expn SHIFTLEFT expn
		{ $$.v = shiftbyvar($1.v, $3.v, 1); }

		| SUB expn %prec UNARYMINUS
		{ $$.v = sub(intconstant(0), $2.v); }

		| TILDE expn
		{ $$.v = complement($2.v); }

		| NOT expn
		{ $$.v = complement(nonzero($2.v)); }

		| PLUSPLUS lhsidentifier
		{
		    pushtargetwidth($2.v);
		    $$.v = assignmentstmt($2.v, add($2.v, intconstant(1)));
		}

		| MINUSMINUS lhsidentifier
		{
		    pushtargetwidth($2.v);
		    $$.v = assignmentstmt($2.v, sub($2.v, intconstant(1)));
		}

		| lhsidentifier PLUSPLUS
		{
		    $$.v = $1.v;
		    pushtargetwidth($1.v);
		    assignmentstmt($1.v, add($1.v, intconstant(1)));
		}

		| lhsidentifier MINUSMINUS
		{
		    $$.v = $1.v;
		    pushtargetwidth($1.v);
		    assignmentstmt($1.v, sub($1.v, intconstant(1)));
		}

		| lhsidentifier EQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, $4.v); }

		| lhsidentifier PLUSEQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, add($1.v, $4.v)); }

		| lhsidentifier MINUSEQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, sub($1.v, $4.v)); }

		| lhsidentifier SHIFTRIGHTEQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, shiftbyvar($1.v, $4.v, 0)); }

		| lhsidentifier SHIFTLEFTEQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, shiftbyvar($1.v, $4.v, 1)); }

		| lhsidentifier ANDEQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, twoopexpn($1.v, $4.v, and)); }

		| lhsidentifier XOREQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, twoopexpn($1.v, $4.v, xor)); }

		| lhsidentifier OREQUAL
		{ pushtargetwidth($1.v); }
		expn
		{ $$.v = assignmentstmt($1.v, twoopexpn($1.v, $4.v, or)); }

term:		INTEGER
		{ $$.v = intconstant(atoi($1.s)); }

		| oldidentifier

		| LEFTPAREN expn RIGHTPAREN
		{ $$ = $2; }

		| functioncall

functioncall:	identifier LEFTPAREN argumentlist RIGHTPAREN
		{
		    struct variable *currentstate, *callingstate;
		    struct variable *v, *tempstate;
		    struct variable *temp1, *temp2;
		    struct varlist **vlp;

		    makefunction($1.v, $1.v->width);

		    /* All functions are in global scope */

		    $1.v->scope = GLOBALSCOPE;
		    currentstate = findvariable(CURRENTSTATE, MUSTEXIST, 1);
		    setvar($1.v->initialstate, twoop($1.v->initialstate, currentstate, or));
		    vlp = &$1.v->arguments;
		    for(; $3.v->junk; $3.v->junk = $3.v->junk->next) {

		        /* If the function has already been declared,
		         * then we know how many parameters there are
		         * and how many bits each one has
		         */
		        if($1.v->flags & SYM_FUNCTION_DECLARED) {
		            if(!*vlp) {
		                warning2("too many arguments in call to", $1.v->name);
		                break;
		            }
		            addtoff((*vlp)->variable, currentstate, $3.v->junk->variable);
		        } else {
		            /* If the function has not yet been declared,
		            * then save all the arguments it is called
		            * with.  If the current argument is wider
		            * than the ones we have seen so far, create
		            * a wider temporary variable to store them
		            * in, and sign extend the narrower ones.
		            */
		            if(!*vlp) {
		                v = newtempvar("targ", $3.v->junk->variable->width);
		                v->flags &= ~SYM_TEMP;
		                makeff(v);
		                addtoff(v, currentstate, $3.v->junk->variable);
		                addtovlist(vlp, v);
		            } else if((*vlp)->variable->width < $3.v->junk->variable->width) {
		                v = newtempvar("targ", $3.v->junk->variable->width);
		                v->flags &= ~SYM_TEMP;
		                makeff(v);
		                setvar(v, (*vlp)->variable);
		                (*vlp)->variable = v;
		                addtoff((*vlp)->variable, currentstate, $3.v->junk->variable);
		            } else {
		                addtoff((*vlp)->variable, currentstate, $3.v->junk->variable);
		            }
		        }
		        vlp = &((*vlp)->next);
		    }
		    tick(currentstate);
		    callingstate = newtempvar("calling", 1);
		    callingstate->flags = SYM_STATE;
		    makeff(callingstate);
		    tempstate = ffoutput($1.v->finalstate);
		    temp1 = ffoutput(callingstate);
		    temp2 = complement(tempstate);
		    temp1 = twoop(temp1, temp2, and);
		    setvar(callingstate, twoop(currentstate, temp1, or));
		    currentstate = assignment(currentstate, twoop(ffoutput(callingstate), tempstate, and));
		    if(countlist(currentstate->bits) > 1)
		        assignment(currentstate, ffoutput(currentstate));
		    $$.v = ffoutput($1.v->returnvalue);
		    modifiedvar($$.v);
		}

argumentlist:				/* empty */
		{
		    $$.v = newtempvar("arglist", 1);
		    $$.v->junk = (struct varlist *) NULL;
		}

		| arglist2

arglist2:	expn
		{
		    $$ = $1;
		    $$.v->junk = (struct varlist *) NULL;
		    addtovlist(&$$.v->junk, $1.v);
		}

		| arglist2 COMMA expn
		{
		    $$ = $1;
		    addtovlistwithduplicates(&$$.v->junk, $3.v);
		}

lhsidentifier:  IDENTIFIER COLON INTEGER
                {
                    $$.v = findvariable($1.s, MUSTEXIST, atoi($3.s));
                    changewidth($$.v->copyof, atoi($3.s));
                    changewidth($$.v, atoi($3.s));
                }

                | IDENTIFIER LEFTBRACE expn RIGHTBRACE COLON INTEGER
                {
                    $$.v = findvariable($1.s, MUSTEXIST, atoi($6.s));
                    changewidth($$.v->copyof, atoi($6.s));
                    changewidth($$.v, atoi($6.s));
//printf("array %08x arraywrite %08x for %s is %s\n", $$.v, $$.v->arraywrite, $1, $3.v->name);
                    pushtargetwidth($$.v->arraywrite);
                    assignmentstmt($$.v->arraywrite, $3.v);
                }

                | INTEGER IDENTIFIER
                {
                    $$.v = findvariable($2.s, MUSTEXIST, atoi($1.s));
                    changewidth($$.v->copyof, atoi($1.s));
                    changewidth($$.v, atoi($1.s));
                }

                | INTEGER IDENTIFIER LEFTBRACE expn RIGHTBRACE
                {
                    $$.v = findvariable($2.s, MUSTEXIST, atoi($1.s));
                    changewidth($$.v->copyof, atoi($1.s));
                    changewidth($$.v, atoi($1.s));
//printf("array %08x arraywrite %08x for %s is %s\n", $$.v, $$.v->arraywrite, $1, $4.v->name);
                    pushtargetwidth($$.v->arraywrite);
                    assignmentstmt($$.v->arraywrite, $4.v);
                }

                | IDENTIFIER
                { $$.v = findvariable($1.s, MUSTEXIST, currentwidth); }

                | IDENTIFIER LEFTBRACE expn RIGHTBRACE
                {
                    $$.v = findvariable($1.s, MUSTEXIST, currentwidth);
//printf("array %08x arraywrite %08x for %s is %s\n", $$.v, $$.v->arraywrite, $1, $3.v->name);
                    pushtargetwidth($$.v->arraywrite);
                    assignmentstmt($$.v->arraywrite, $3.v);
                }

oldidentifier:	IDENTIFIER COLON INTEGER
		{
		    $$.v = findvariable($1.s, MUSTEXIST, atoi($3.s));
		    changewidth($$.v->copyof, atoi($3.s));
		    changewidth($$.v, atoi($3.s));
		}

		| IDENTIFIER LEFTBRACE expn RIGHTBRACE COLON INTEGER
		{
		    $$.v = findvariable($1.s, MUSTEXIST, atoi($6.s));
		    changewidth($$.v->copyof, atoi($6.s));
		    changewidth($$.v, atoi($6.s));
//printf("array %08x arrayref %08x for %s is %s\n", $$.v, $$.v->arrayref, $1, $3.v->name);
                    pushtargetwidth($$.v->arrayref);
                    assignmentstmt($$.v->arrayref, $3.v);
		}

		| INTEGER IDENTIFIER
		{
		    $$.v = findvariable($2.s, MUSTEXIST, atoi($1.s));
		    changewidth($$.v->copyof, atoi($1.s));
		    changewidth($$.v, atoi($1.s));
		}

		| INTEGER IDENTIFIER LEFTBRACE expn RIGHTBRACE
		{
		    $$.v = findvariable($2.s, MUSTEXIST, atoi($1.s));
		    changewidth($$.v->copyof, atoi($1.s));
		    changewidth($$.v, atoi($1.s));
//printf("array %08x arrayref %08x for %s is %s\n", $$.v, $$.v->arrayref, $1, $4.v->name);
                    pushtargetwidth($$.v->arrayref);
                    assignmentstmt($$.v->arrayref, $4.v);
		}

		| IDENTIFIER
		{ $$.v = findvariable($1.s, MUSTEXIST, currentwidth); }

		| IDENTIFIER LEFTBRACE expn RIGHTBRACE
		{
		    $$.v = findvariable($1.s, MUSTEXIST, currentwidth);
//printf("array %08x arrayref %08x for %s is %s\n", $$.v, $$.v->arrayref, $1, $3.v->name);
                    pushtargetwidth($$.v->arrayref);
                    assignmentstmt($$.v->arrayref, $3.v);
		}

newidentifier:	IDENTIFIER COLON INTEGER
		{
		    $$.v = findvariable($1.s, MUSTNOTEXIST, atoi($3.s));
		    $$.v->type = currenttype;
		}

		| IDENTIFIER LEFTBRACE INTEGER RIGHTBRACE COLON INTEGER
		{
		    $$.v = findvariable($1.s, MUSTNOTEXIST, atoi($6.s));
		    $$.v->type = currenttype;
		    $$.v->arraysize = atoi($3.s);
		    $$.v->arrayaddrbits = sizelog2(atoi($3.s));
		    $$.v->arraywrite = newtempvar("writeindex", $$.v->arrayaddrbits);
		    $$.v->arrayref = newtempvar("readindex", $$.v->arrayaddrbits);
//printf("Array %s %08x is size %d/%d with %08x/%08x\n", $1.s, $$.v, $$.v->arraysize, $$.v->arrayaddrbits, $$.v->arraywrite,$$.v->arrayref);
		}

		| INTEGER IDENTIFIER
		{
		    $$.v = findvariable($2.s, MUSTNOTEXIST, atoi($1.s));
		    $$.v->type = currenttype;
		}

		| INTEGER IDENTIFIER LEFTBRACE INTEGER RIGHTBRACE
		{
		    $$.v = findvariable($2.s, MUSTNOTEXIST, atoi($1.s));
		    $$.v->type = currenttype;
		    $$.v->arraysize = atoi($4.s);
		    $$.v->arrayaddrbits = sizelog2(atoi($4.s));
		    $$.v->arraywrite = newtempvar("writeindex", $$.v->arrayaddrbits);
		    $$.v->arrayref = newtempvar("readindex", $$.v->arrayaddrbits);
//printf("Array %s %08x is size %d/%d with %08x/%08x\n", $1.s, $$.v, $$.v->arraysize, $$.v->arrayaddrbits, $$.v->arraywrite,$$.v->arrayref);
		}

		| IDENTIFIER
		{
		    $$.v = findvariable($1.s, MUSTNOTEXIST, currentwidth);
		    $$.v->type = currenttype;
		}

		| IDENTIFIER LEFTBRACE INTEGER RIGHTBRACE
		{
		    $$.v = findvariable($1.s, MUSTNOTEXIST, currentwidth);
		    $$.v->type = currenttype;
		    $$.v->arraysize = atoi($3.s);
		    $$.v->arrayaddrbits = sizelog2(atoi($3.s));
		    $$.v->arraywrite = newtempvar("writeindex", $$.v->arrayaddrbits);
		    $$.v->arrayref = newtempvar("readindex", $$.v->arrayaddrbits);
//printf("Array %s %08x is size %d/%d with %08x/%08x\n", $1.s, $$.v, $$.v->arraysize, $$.v->arrayaddrbits, $$.v->arraywrite,$$.v->arrayref);
		}

identifier:	IDENTIFIER COLON INTEGER
		{
		    $$.v = findvariable($1.s, MAYEXIST, atoi($3.s));
		    changewidth($$.v->copyof, atoi($3.s));
		    changewidth($$.v, atoi($3.s));
		}

		| INTEGER IDENTIFIER
		{
		    $$.v = findvariable($1.s, MAYEXIST, atoi($1.s));
		    changewidth($$.v->copyof, atoi($1.s));
		    changewidth($$.v, atoi($1.s));
		}

		| IDENTIFIER
		{ $$.v = findvariable($1.s, MAYEXIST, currentwidth); }

int_expr:	INTEGER

		| int_expr OR int_expr
		{ $$.s = intop($1.s, $3.s, or); }

%%

yyerror(char *s) {
    extern char yytext[];

    fprintf(stderr, "\"%s\", line %d: %s at or near symbol %s\n", inputfilename, inputlineno, s, yytext);
}

error2(char *s1, char *s2) {

    fprintf(stderr, "\"%s\", line %d: %s %s\n", inputfilename, inputlineno, s1, s2);
    nerrors++;
}

warning2(char *s1, char *s2) {

    fprintf(stderr, "\"%s\", line %d: warning %s %s\n", inputfilename, inputlineno, s1, s2);
}

char *get_designname(void) {

    /* What is the basename of this C file ? */

    static char buf[BUFSIZ];
    char *cp;

    if(original_inputfilename[0] == '\0') {
	return ("no_designname");
    }

    cp = strrchr(buf, '/');
    strncpy(buf, basename(original_inputfilename), BUFSIZ);
    buf[BUFSIZ - 1] = '\0';
    cp = strchr(buf, '.');
    if(cp != NULL) {
	*cp = '\0';
    }

    return (buf);
}
