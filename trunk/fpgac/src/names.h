/*
 * names.h -- defines for the fpgac compiler
 * SVN $Revision$  hosted on http://sourceforge.net/projects/fpgac
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

/* The EXTFIX dance is used to avoid complaints from compilers that insist on
 * seeing only one definition of each global variable
 */

#ifndef EXTFIX
#define EXTFIX extern
#endif

typedef enum {
    add_op, andand_op, and_op, assign_op, complement_op,
    const_op, equalequal_op, if_op, greaterequal_op, greater_op,
    lessthanequal_op, lessthan_op, minusminus_op, notequal_op,
    not_op, or_op, oror_op, plusplus_op, shiftleft_op,
    shiftright_op, sub_op, while_op, xor_op
} op_types;

#define TYPE_INTEGER    0x0001
#define TYPE_FLOAT      0x0002

#define TYPE_INPUT      0x0010
#define TYPE_OUTPUT     0x0020
#define TYPE_BUS        0x0040
#define TYPE_MAILBOX    0x0080

#define TYPE_PROCESS    0x0100
#define TYPE_UNSIGNED   0x1000
#define TYPE_BITSERIAL  0x2000


#define MAXNAMELEN	128

#define SYM_KNOWNVALUE		0x1
#define SYM_INPUTPORT		0x2
#define SYM_OUTPUTPORT		0x4
#define SYM_AFFECTSOUTPUT	0x8
#define SYM_TEMP		0x10
#define SYM_UPTODATE		0x20
#define SYM_FF			0x40
#define SYM_STATE		0x80
#define SYM_FUNCTION		0x100
#define SYM_LITERAL		0x200
#define SYM_DONTPULLUP		0x800
#define SYM_BUSPORT		0x1000
#define SYM_VCC			0x2000
#define BIT_HASPIN		0x4000
#define BIT_HASFF		0x8000
#define SYM_CLOCK		0x10000
#define BIT_HASPULLUP		0x20000
#define BIT_HASPULLDOWN		0x40000
#define BIT_DEPTHVALID		0x80000
#define SYM_FUNCTION_DECLARED	0x100000
#define SYM_MULTIPLE_RETURNS	0x200000
#define SYM_ARRAY               0x400000
#define SYM_ARRAY_INDEX         0x800000
#define SYM_FUNCTIONEXISTS	0x1000000
#define SYM_STRUCT_MEMBER       0x10000000
#define SYM_ENUM                0x20000000
#define SYM_STRUCT              0x40000000
#define SYM_UNION               0x80000000
#define SYM_TAG                 0xE0000000

EXTFIX struct variable {
	char name[MAXNAMELEN];
	int lineno;
	int temp;
	int dscnt;
	struct variable *next;
	long int flags;
	int width;
	int type;
	long value;
	struct bitlist *bits;
	struct variable *scope;
	struct variable *dscope;
	struct variable *copyof;
	struct variable *state;
	struct varlist *junk;

/* For functions */

	struct varlist *arguments;
	struct variable *initialstate;
	struct variable *finalstate;
	struct variable *returnvalue;

/* For Structures */
        struct varlist *members;             // list of members in structure
        int offset;                          // members bit offset in structure instance
        struct variable *parent;             // parent for members

/* For Arrays */
        int arraysize;
        int arrayaddrbits;
        int port;
        struct varlist *arrayref;
        struct variable *index;
        struct variable *arraywrite;
        struct variable *arrayparent;

/* For busports */
	struct variable *enable;
} *variables;

/* A start on separating the bit->flags from the variable->flags */

#define BIT_TEMP		0x10
#define BIT_WORD		0x400000

EXTFIX struct bit {
	char *name;
	struct bit *next;
	long int flags;
	int temp;
	char *pin;
	long *truth;
	int pcnt;
	struct bitlist *primaries;
	struct bit *copyof;
	struct variable *variable;
	int bitnumber;

	/* For timing calculations */
	int depth;

/* For flipflops */

	struct bitlist *modifying_states;
	struct bitlist *suppressing_states;
	struct bitlist *modifying_values;
	struct bitlist *modifying_states_and_values;
	struct bit *clock_enable;

/* For busports */
	struct bit *enable;
} *bits, *bitst;

EXTFIX int nvariables, nbits;

struct bitlist {
	struct bit *bit;
	struct bitlist *next;
};

struct varlist {
	struct variable *variable;
	struct varlist *next;
};

struct scopelist {
        struct varlist **scope;
        struct scopelist *next;
};

#define YYSTYPE	yystype

typedef union {
	struct variable *v;
	struct varlist *vl;
	char *s;
	int type;
} YYSTYPE;

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))

struct variable *findvariable(), *complement(), *shift(), *intconstant();
struct variable *add(), *sub(), *thistick(), *newtempvar(), *twoop();
int xor(), and(), or();
char *bitname();

/* Structure for terms that is used in minimize_lut.c and output_xilinx.c */
   
typedef struct {
    unsigned char value;	/* Bits that make up this term */
    unsigned char dc;		/* Don't care mask for above */
    unsigned char covered;	/* this term is covered by a simpler one */
} QMtab;

#define QMtabSize (1024*8)
#define MAXPRI     4

#define IGNORE_FORLOOP 1

#define Get_Bit(longvector, bit)  ((longvector[(bit)>>5]>>((bit)&0x1f))&1)
#define Set_Bit(longvector, bit)  (longvector[(bit)>>5] |= 1<<((bit)&0x1f))
#define Clr_Bit(longvector, bit)  (longvector[(bit)>>5] &= ~(1<<((bit)&0x1f)))

EXTFIX int debug;
