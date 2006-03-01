/*
 * output_netlist.c -- Compact Netlist output format for FpgaC
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "names.h"
#include "outputvars.h"

extern char Revision[];

static printROM(struct bit *b, int count);
static printGates(struct bit *b, int count);
static printEQN(struct bit *b, int count);
static printAND(int i, QMtab table[], int count, struct bit *b);

char *bitname_cnf(struct bit *b) {
    char *n;

    if(b->name) n=b->name;
    else if(b->variable) n = b->variable->name;

    while(*n == '_') n++;

    if(*n) return(n);


    if(b->variable->width == 1) {
        if(b->flags & SYM_VCC) {
            strncpy(b->name, b->variable->name, MAXNAMELEN);
        } else {
            if(b->variable->copyof->arraysize) {
                sprintf(b->name, "%s_p%d", n, b->variable->port);
            } else {
                sprintf(b->name, "%s", n);
            }
        }
    } else {
        if(b->variable->copyof->arraysize) {
            sprintf(b->name, "%s_p%d_%d", n , b->variable->port , b->bitnumber);
        } else {
            sprintf(b->name, "%s_%d", n, b->bitnumber);
        }
    }
    return (b->name);
}

output_CNF() {
    int n,i;
    int count;
    time_t now;
    int printed;
    char *datestring;
    struct bit *b;
    struct bitlist *bl;

    if (nerrors > 0)
	return;

    now = time((time_t) NULL);
    datestring = ctime(&now);
    datestring[strlen(datestring) - 1] = '\0';
    fprintf(outputfile, "// fpgac, %s, \"%s\"\n", Revision, datestring);
    if (partname) {
        fprintf(outputfile, "// part=%s\n", partname);
    }

    printed = 0;
    for(b=bits; b; b=b->next) {
        char *ram;

	if (b->variable && !strcmp(b->variable->name, "VCC"))
	    continue;
	if (!(b->flags & (SYM_AFFECTSOUTPUT | SYM_OUTPUTPORT | SYM_INPUTPORT |SYM_BUSPORT)))
	    continue;

	printed = 1;

	if (b->enable)
            fprintf(outputfile, "if(~%s) ", bitname(b->enable));

	if (b->flags & SYM_OUTPUTPORT) {
            if(b->pin)
                fprintf(outputfile, "port(%s,\"%s\")", b->name, b->pin+1);
            else
                fprintf(outputfile, "port(%s)", b->name);
	} else if (b->flags & SYM_BUSPORT) {
	    fprintf(outputfile, "out%s", bitname(b));
        } else if ((b->flags & SYM_FF) && b->variable->arraysize) {
            struct varlist *vl;

            if (b->variable->arraysize <= 16) {
            ram = "RAM16_";
            } else if (b->variable->arraysize <= 32) {
	    ram = "RAM32_";
            } else if (b->variable->arraysize <= 64) {
	    ram = "RAM64_";
            } else {
	    ram = "RAM_";
            }
            fprintf(outputfile, "%s%s_%d[", ram, b->variable->copyof->name+1,b->bitnumber);

            if(b->variable->arraywrite && b->variable->arraywrite->index->bits) {
                for (i=0,bl = b->variable->arraywrite->index->bits;i<b->variable->arrayaddrbits;i++) {
                    if (bl && bl->bit) {
                        fprintf(outputfile, "%s", bitname(bl->bit));
                    } else
                        fprintf(outputfile, "GND");
                    if(bl->next) fprintf(outputfile, ",");
                    if(bl) bl = bl->next;
                }
            }
            fprintf(outputfile, "]");
	} else if(!(b->flags & SYM_ARRAY))
	    fprintf(outputfile, "%s", bitname(b));

	if (b->flags & (SYM_FF|BIT_HASFF)) {
            if (b->clock_enable)
                fprintf(outputfile, "^(%s*%s)", clockname, bitname(b->clock_enable));
            else
                fprintf(outputfile, "^%s", clockname);
        }

	if(!(b->flags & SYM_ARRAY))
	    fprintf(outputfile, " = ");

	switch (b->flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF | SYM_BUSPORT)) {
	case SYM_INPUTPORT:
	case SYM_INPUTPORT | BIT_HASFF:
	case SYM_INPUTPORT | BIT_HASPIN:
	case SYM_INPUTPORT | BIT_HASPIN | BIT_HASFF:
	case SYM_INPUTPORT | SYM_BUSPORT | BIT_HASPIN:
	case SYM_INPUTPORT | SYM_BUSPORT | BIT_HASPIN | BIT_HASFF:
            if(b->pin)
                fprintf(outputfile, "port(%s,\"%s\");\n", b->name, b->pin+1);
            else
                fprintf(outputfile, "port(%s);\n", b->name);
	    continue;
	    break;

	case SYM_OUTPUTPORT:
	case SYM_OUTPUTPORT | BIT_HASFF:
	case SYM_OUTPUTPORT | BIT_HASPIN:
	case SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF:
	case 0:		/* normal variables */
	    break;

	default:
	    fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n", bitname(b),
		    b->flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF | SYM_BUSPORT));
	    break;
	}

	if ((b->flags & BIT_HASPIN) && (b->flags & BIT_HASPULLUP)) {
	    fprintf(outputfile, "SYM, %s-PULLUP, PULLUP\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s_pad\n", bitname(b));
	    fprintf(outputfile, "END\n");
	}

	if ((b->flags & BIT_HASPIN) && (b->flags & BIT_HASPULLDOWN)) {
	    fprintf(outputfile, "SYM, %s-PULLDOWN, PULLDOWN\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s_pad\n", bitname(b));
	    fprintf(outputfile, "END\n");
	}

	if ((b->flags & (SYM_INPUTPORT | SYM_BUSPORT)) == SYM_INPUTPORT) {
	    ninpins++;
	    continue;
	}
	if (b->flags & SYM_BUSPORT)
	    nbidirpins++;
	if (b->flags & SYM_OUTPUTPORT)
	    noutpins++;

	if ((b->flags & (SYM_OUTPUTPORT | SYM_BUSPORT)) && !(b->flags & BIT_HASFF))
	    b->flags &= ~SYM_FF;

	if ((b->flags & SYM_ARRAY))
	    b->flags &= ~(SYM_FF | SYM_AFFECTSOUTPUT);

        count = countlist(b->primaries) - 1;

        if (count <= 0) {
            if (!(b->flags & SYM_ARRAY)) {
                if (count == 0) {
                    if (b->truth[0])
                        fprintf(outputfile, "~%s;\n", bitname(b->primaries->bit));
                    else
                        fprintf(outputfile, "%s;\n", bitname(b->primaries->bit));
                } else {
                    if (b->truth[0])
                        fprintf(outputfile, "VCC;\n");
                    else
                        fprintf(outputfile, "GND;\n");
                }
            }
        } else {
            nroms++;
            inputcounts[count + 1]++;
            if (b->flags & SYM_FF)
            nff++;

            switch (output_format) {

            case CNFROMS:
                printROM(b, count);
                break;

            case CNFGATES:
                printGates(b, count);
                break;

            case CNFEQNS:
                printEQN(b, count);
                break;

            default:
                error2("unknown output format", "this should not happen");
                abort();
            }
        }

        if (b->flags & SYM_ARRAY) {
            fprintf(outputfile, "%s = %s%s[", bitname(b), ram, bitname(b));
            for (i=0,bl = b->variable->copyof->index->bits;i<b->variable->arrayparent->arrayaddrbits;i++) {
                if (bl && bl->bit) {
                    fprintf(outputfile, "%s", bitname(bl->bit));
                } else
                    fprintf(outputfile, "GND");
                if(bl->next) fprintf(outputfile, ",");
                if(bl) bl = bl->next;
            }
            fprintf(outputfile, "];\n");
	}
    }
    if (!printed)
	warning2("compiler produced no output", "");
}

static printROM(struct bit *b, int count) {
    struct bitlist *bl;
    int i;

    fprintf(outputfile, " \"");
    for (i = 0; i < (1 << (count + 1)); i++)
        fprintf(outputfile, "%d", b->truth[i] & 1);
    fprintf(outputfile, "\"[");
    for (i = 3; i > count; --i)
	fprintf(outputfile, "GND,", i);
    for (bl = b->primaries; bl; bl = bl->next) {
	fprintf(outputfile, "%s,", bitname(bl->bit));
	--count;
    }
    fprintf(outputfile, "];");
}

/* The following code originally by Dr. John Forrest of UMIST, Manchester, UK */

static printEQN(struct bit *b, int count) {
    int first = 1, i, j, first_in_term, top;
    struct bitlist *bl;
    QMtab table[128];
    char *names[4], **p = names;

    QMtruthToTable(b->truth, table, &top, count + 1);
    if (simpleQM(table, &top, QMtabSize, count + 1) != 0) {
	error2("QM overflow in printEQN, should not happen", bitname(b));
	abort();
    }

    for (bl = b->primaries; bl; bl = bl->next) {
	*p++ = bitname(bl->bit);
    }

    for (i = 0; i <= top; i++) {
	if (table[i].covered)
	    continue;
	first_in_term = 1;
	if (!first)
	    fprintf(outputfile, "+");
	first = 0;
	fprintf(outputfile, "(");
	for (j = 0; j < count + 1; j++) {
	    if (table[i].dc & (1 << j))
		continue;
	    if (!first_in_term)
		fprintf(outputfile, "*");
	    first_in_term = 0;
	    if (!(table[i].value & (1 << j)))
		fprintf(outputfile, "~");
	    fprintf(outputfile, "%s", names[count-j]);
	}
	if (first_in_term) {
	    fprintf(stderr, "%s is Vcc!\n", bitname(b));
	    /* printTab (table, &top, count+1); */
	    fprintf(outputfile, "Vcc");
	}
	fprintf(outputfile, ")");
    }
    if (first)			/* no terms were true */
	fprintf(outputfile, "GND");
    fprintf(outputfile, ";\n");
}

#define ORlimit 4		/* max size of OR gate */

static printGates(struct bit *b, int count) {
    int i, top;
    int used_terms = 0;
    int posCnt = 0;
    int subOr = 0, oldSubOr;
    QMtab table[128];

    /* The idea is to produce an AND or a BUF for each term
     * and then OR them together. If only one
     * used term we will produce a BUF instead.
     */

    QMtruthToTable(b->truth, table, &top, count + 1);
    if (simpleQM(table, &top, QMtabSize, count + 1) != 0) {
	error2("QM overflow in printGates, should not happen", bitname(b));
	abort();
    }

    for (i = 0; i <= top; i++) {
	if (table[i].covered)
	    continue;
	used_terms += 1;
	printAND(i, table, count, b);
    }

    if (used_terms <= 1) {
          if (b->flags & SYM_FF && !b->variable->arraysize)
              fprintf(outputfile, "SYM, FFin_%s, BUF\n", bitname(b));
          else if (b->flags & SYM_FF && b->variable->arraysize)
              fprintf(outputfile, "SYM, RAMin_%s, BUF\n", bitname(b));
          else
              fprintf(outputfile, "SYM, SYM%s, BUF\n", bitname(b));
	if (used_terms == 0)
	    fprintf(outputfile, "PIN, I, I, GND\n");
	else
	    for (i = 0; i <= top; i++) {
		if (!table[i].covered)
		    fprintf(outputfile, "PIN, I, I, %dT_%s\n", i, bitname(b));
	    }
    } else {
	posCnt = 0;

	for (i = 0; i <= top; i++) {
	    if (table[i].covered)
		continue;
	    if (posCnt == 0) {	/* start of new symbol */
		oldSubOr = subOr;
		if (used_terms > ORlimit) {
		    subOr += 1;
		    fprintf(outputfile, "SYM, %dOR_%s, OR\n", subOr, bitname(b));
		} else {
		    subOr = 0;
                    if (b->flags & SYM_FF && !b->variable->arraysize)
                        fprintf(outputfile, "SYM, FFin_%s, OR\n", bitname(b));
                    else if (b->flags & SYM_FF && b->variable->arraysize)
                        fprintf(outputfile, "SYM, RAMin_%s, OR\n", bitname(b));
                    else
                        fprintf(outputfile, "SYM, SYM%s, OR\n", bitname(b));
		}
		if (oldSubOr)
		    fprintf(outputfile, "PIN, I%d, I, %dOR_%s\n", posCnt++, oldSubOr, bitname(b));
	    }
	    fprintf(outputfile, "PIN, I%d, I, %dT_%s\n", posCnt++, i, bitname(b));
	    if (subOr && posCnt >= ORlimit) {
		fprintf(outputfile, "PIN, O, O, %dOR_%s\n", subOr, bitname(b));
		fprintf(outputfile, "END\n");
		posCnt = 0;
	    }
	    used_terms -= 1;
	}
    }
}

static printAND(int i, QMtab table[], int count, struct bit *b) {
    int bitCnt, j;
    struct bitlist *bl;

    fprintf(outputfile, "%dT_%s = ", i, bitname(b));
    bitCnt = QMtermBits(table[i], count + 1);
    for (bl = b->primaries, j = count; bl; bl = bl->next, j--) {
	if (!(table[i].dc & (1 << j))) {
	    fprintf(outputfile, "%s%s", (table[i].value & (1 << j)) ? "~" : "", bitname(bl->bit));
	    if (bitCnt > 1)
		fprintf(outputfile, " * ");
	    --bitCnt;
	}
    }
    fprintf(outputfile, ";\n");
}
