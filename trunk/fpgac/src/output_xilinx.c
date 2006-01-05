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
/* CHANGES:
 *
 *  MTP converted strcpy to strncpy  
 *
*/
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "names.h"
#include "outputvars.h"
#include "patchlevel.h"

static printROM(struct bit *b, int count);
static printGates(struct bit *b, int count);
static printEQN(struct bit *b, int count);
static printAND(int i, QMtab table[], int count, struct bit *b);

static printExt(char *extname, char *type, char *pin) {
    if (pin) {
	if ((pin[0] >= '0') && (pin[0] <= '9'))
	    fprintf(outputfile, "EXT, %s, %s,, LOC=P%s\n", extname, type, pin);
	else
	    fprintf(outputfile, "EXT, %s, %s,, LOC=%s\n", extname, type, pin);
    } else
	fprintf(outputfile, "EXT, %s, %s\n", extname, type);
}


extern char Revision[];

output_XNF() {
    int n,i;
    int count;
    time_t now;
    int printed;
    char *datestring;
    struct bit *b;
    struct bitlist *bl;

    if (nerrors > 0)
	return;
    fprintf(outputfile, "LCANET, 4\n");
    fprintf(outputfile, "PWR, 1, VCC\n");
    fprintf(outputfile, "PWR, 0, GND\n");
    now = time((time_t) NULL);
    datestring = ctime(&now);
    datestring[strlen(datestring) - 1] = '\0';
    Revision[strlen(Revision) - 2] = '\0';
    if (((int) strlen(Revision)) <= 11)
	strncpy(Revision, "Revision unknown", REVISIONLENGTH);
    fprintf(outputfile, "PROG, fpgac, %s, \"%s\"\n", &Revision[11], datestring);
    if (partname)
	fprintf(outputfile, "PART, %s\n", partname);
    if (genclock) {
	fprintf(outputfile, "SYM, OSC4, OSC4\n");
	fprintf(outputfile, "PIN, F15, O, CLKin\n");
	fprintf(outputfile, "END\n");
	fprintf(outputfile, "SYM, CLK-AA, BUFGS\n");
	fprintf(outputfile, "PIN, I, I, CLKin\n");
	fprintf(outputfile, "PIN, O, O, %s\n", clockname);
	fprintf(outputfile, "END\n");
    }
    printed = 0;
    for (n = 0; n < nbits; n++) {
	b = &bits[n];
	if (b->variable && !strcmp(b->variable->name, "VCC"))
	    continue;

	switch (b->flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF | SYM_BUSPORT)) {
	case SYM_INPUTPORT | BIT_HASPIN:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s, IBUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", externalname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    printExt(externalname(b), "I", b->pin);
	    break;

	case SYM_INPUTPORT | BIT_HASPIN | BIT_HASFF:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s-IBUF, IBUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", externalname(b));
	    fprintf(outputfile, "PIN, O, O, FFin-%s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    printExt(externalname(b), "I", b->pin);
	    fprintf(outputfile, "SYM, %s, DFF\n", bitname(b));
	    fprintf(outputfile, "PIN, D, I, FFin-%s\n", bitname(b));
	    fprintf(outputfile, "PIN, C, I, %s\n", clockname);
	    fprintf(outputfile, "PIN, CE, I, VCC\n");
	    fprintf(outputfile, "PIN, Q, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    break;


	case SYM_INPUTPORT | BIT_HASFF:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s-BUF, BUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", externalname(b));
	    fprintf(outputfile, "PIN, O, O, FFin-%s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    fprintf(outputfile, "SYM, %s, DFF\n", bitname(b));
	    fprintf(outputfile, "PIN, D, I, FFin-%s\n", bitname(b));
	    fprintf(outputfile, "PIN, C, I, %s\n", clockname);
	    fprintf(outputfile, "PIN, CE, I, VCC\n");
	    fprintf(outputfile, "PIN, Q, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    break;

	case SYM_INPUTPORT:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s, BUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", externalname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    break;

	case SYM_OUTPUTPORT | BIT_HASFF | BIT_HASPIN:
	case SYM_OUTPUTPORT | BIT_HASPIN:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s-OBUF, OBUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", externalname(b));
	    fprintf(outputfile, "END\n");
	    printExt(externalname(b), "O", b->pin);
	    break;

	case SYM_OUTPUTPORT | BIT_HASFF:
	case SYM_OUTPUTPORT:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s-OBUF, BUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", externalname(b));
	    fprintf(outputfile, "END\n");
	    break;

	case SYM_INPUTPORT | SYM_BUSPORT | BIT_HASPIN | BIT_HASFF:
	case SYM_INPUTPORT | SYM_BUSPORT | BIT_HASPIN:
	    printed = 1;
	    fprintf(outputfile, "SYM, %s, IBUF\n", bitname(b));
	    fprintf(outputfile, "PIN, I, I, %s\n", externalname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");
	    fprintf(outputfile, "SYM, %s-OBUFT, OBUFT\n", bitname(b));
	    fprintf(outputfile, "PIN, T, I, %s,, INV\n", bitname(b->enable));
	    fprintf(outputfile, "PIN, I, I, out%s\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", externalname(b));
	    fprintf(outputfile, "END\n");
	    printExt(externalname(b), "B", b->pin);
	    break;

	case 0:		/* normal variables */
	    break;

	default:
	    fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n", bitname(b),
		    b->flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF | SYM_BUSPORT));
	    break;
	}

	if ((b->flags & BIT_HASPIN) && (b->flags & BIT_HASPULLUP)) {
	    fprintf(outputfile, "SYM, %s-PULLUP, PULLUP\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", externalname(b));
	    fprintf(outputfile, "END\n");
	}

	if ((b->flags & BIT_HASPIN) && (b->flags & BIT_HASPULLDOWN)) {
	    fprintf(outputfile, "SYM, %s-PULLDOWN, PULLDOWN\n", bitname(b));
	    fprintf(outputfile, "PIN, O, O, %s\n", externalname(b));
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

	if (b->flags & SYM_AFFECTSOUTPUT) {
	    printed = 1;
	    count = countlist(b->primaries) - 1;
	    if (count <= 0) {
                if (b->flags & SYM_FF && !b->variable->arraysize) {
		    nff++;
		    fprintf(outputfile, "SYM, FFin-%s, ", bitname(b));
                } else if (b->flags & SYM_FF && b->variable->arraysize) {
                    fprintf(outputfile, "SYM, RAMin-%s, ", bitname(b));
		} else
		    fprintf(outputfile, "SYM, %s, ", bitname(b));
		if (b->truth[0])
		    fprintf(outputfile, "INV\n");
		else
		    fprintf(outputfile, "BUF\n");
		if (count == 0)
		    fprintf(outputfile, "PIN, I, I, %s\n", bitname(b->primaries->bit));
		else
		    fprintf(outputfile, "PIN, I, I, GND\n");
	    } else {
		nroms++;
		inputcounts[count + 1]++;
		if (b->flags & SYM_FF)
		    nff++;

		switch (output_format) {

		case XNFROMS:
		    printROM(b, count);
		    break;

		case XNFGATES:
		    printGates(b, count);
		    break;

		case XNFEQNS:
		    printEQN(b, count);
		    break;

		default:
		    error2("unknown output format", "this should not happen");
		    abort();
		}
	    }
            if (b->flags & SYM_FF && !b->variable->arraysize)
		fprintf(outputfile, "PIN, O, O, FFin-%s\n", bitname(b));
            else if (b->flags & SYM_FF && b->variable->arraysize)
                fprintf(outputfile, "PIN, O, O, RAMin-%s\n", bitname(b));
	    else if (b->flags & SYM_BUSPORT)
		fprintf(outputfile, "PIN, O, O, out%s\n", bitname(b));
	    else
		fprintf(outputfile, "PIN, O, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");

            if (b->flags & SYM_FF && b->variable->arraysize) {
                  if (b->variable->arraysize < 16) {
                    fprintf(outputfile, "SYM, %s, ram16x1d\n", bitname(b));
                  } else if (b->variable->arraysize <= 32) {
                    fprintf(outputfile, "SYM, %s, ram32x1d\n", bitname(b));
                  } else if (b->variable->arraysize <= 64) {
                    fprintf(outputfile, "SYM, %s, ram64x1d\n", bitname(b));
                  } else {
                    fprintf(outputfile, "SYM, %s, blockram\n", bitname(b));
                  }
                fprintf(outputfile, "PIN, D, I, RAMin-%s\n", bitname(b));
                fprintf(outputfile, "PIN, WCLK, I, CLK\n");
                  if(bits[n].variable->arraywrite && bits[n].variable->arraywrite->bits) {
                      for (i=0,bl = bits[n].variable->arraywrite->bits;i<bits[n].variable->arrayaddrbits;i++) {
                          if (bl && bl->bit) {
                              if (bl->bit->flags & SYM_AFFECTSOUTPUT) {
                                    fprintf(outputfile, "PIN, A%d, I, %s\n", i, bitname(bl->bit));
                              }
                          } else
                              fprintf(outputfile, "PIN, A%d, I, GND\n", i);
                          if(bl) bl = bl->next;
                      }
                  }
                  if(bits[n].variable->arrayref && bits[n].variable->arrayref->bits) {
                      for (i=0,bl = bits[n].variable->arrayref->bits;i<bits[n].variable->arrayaddrbits;i++) {
                          if (bl && bl->bit) {
                              if (bl->bit->flags & SYM_AFFECTSOUTPUT) {
                                    fprintf(outputfile, "PIN, DPRA%d, I, %s\n", i, bitname(bl->bit));
                              }
                          } else
                              fprintf(outputfile, "PIN, DPRA%d, I, GND\n", i);
                          if(bl) bl = bl->next;
                      }
                  }
                if (b->clock_enable)
                    fprintf(outputfile, "PIN, WE, I, %s\n", bitname(b->clock_enable));
                  else
                    fprintf(outputfile, "PIN, WE, I, VCC\n");
                fprintf(outputfile, "PIN, SPO, O, RAMout%s\n", bitname(b));
                fprintf(outputfile, "PIN, DPO, O, %s\n", bitname(b));
                fprintf(outputfile, "END\n");
            } else if (b->flags & SYM_FF) {
		if (b->flags & SYM_BUSPORT)
		    fprintf(outputfile, "SYM, out%s, DFF\n", bitname(b));
		else
		    fprintf(outputfile, "SYM, %s, DFF\n", bitname(b));
		fprintf(outputfile, "PIN, D, I, FFin-%s\n", bitname(b));
		fprintf(outputfile, "PIN, C, I, %s\n", clockname);
		if (b->clock_enable)
		    fprintf(outputfile, "PIN, CE, I, %s\n", bitname(b->clock_enable));
		else
		    fprintf(outputfile, "PIN, CE, I, VCC\n");
		if (b->flags & SYM_BUSPORT)
		    fprintf(outputfile, "PIN, Q, O, out%s\n", bitname(b));
		else
		    fprintf(outputfile, "PIN, Q, O, %s\n", bitname(b));
		fprintf(outputfile, "END\n");
	    }
	}
    }
    fprintf(outputfile, "EOF\n");
    if (!printed)
	warning2("compiler produced no output", "");
}

static printROM(struct bit *b, int count) {
    struct bitlist *bl;
    int i, hex;

    if (b->flags & SYM_FF && !b->variable->arraysize)
	fprintf(outputfile, "SYM, FFin-%s, ROM, ", bitname(b));
    else if (b->flags & SYM_FF && b->variable->arraysize)
        fprintf(outputfile, "SYM, RAMin-%s, ROM, ", bitname(b));
    else
	fprintf(outputfile, "SYM, %s, ROM, ", bitname(b));
    hex = 0;
    for (i = 0; i < (1 << (count + 1)); i++)
	hex |= (b->truth[i] << i);
    fprintf(outputfile, "INIT=%04X\n", hex);
    for (i = 3; i > count; --i)
	fprintf(outputfile, "PIN, A%d, I, GND\n", i);
    for (bl = b->primaries; bl; bl = bl->next) {
	fprintf(outputfile, "PIN, A%d, I, %s\n", count, bitname(bl->bit));
	--count;
    }
}

/* The following code originally by Dr. John Forrest of UMIST, Manchester, UK */

static printEQN(struct bit *b, int count) {
    int first = 1, i, j, first_in_term, top;
    struct bitlist *bl;
    QMtab table[128];

    QMtruthToTable(b->truth, table, &top, count + 1);
    if (simpleQM(table, &top, QMtabSize, count + 1) != 0) {
	error2("QM overflow in printEQN, should not happen", bitname(b));
	abort();
    }
    if (b->flags & SYM_FF && !b->variable->arraysize)
	fprintf(outputfile, "SYM, FFin-%s, EQN, EQN=(", bitname(b));
      else if (b->flags & SYM_FF && b->variable->arraysize)
        fprintf(outputfile, "SYM, RAMin-%s, EQN, EQN=(", bitname(b));
    else
	fprintf(outputfile, "SYM, SYM%s, EQN, EQN=(", bitname(b));

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
	    fprintf(outputfile, "I%d", j);
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
    fprintf(outputfile, ")\n");
    for (bl = b->primaries; bl; bl = bl->next) {
	fprintf(outputfile, "PIN, I%d, I, %s\n",
		count--, bitname(bl->bit));
    }
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
              fprintf(outputfile, "SYM, FFin-%s, BUF\n", bitname(b));
          else if (b->flags & SYM_FF && b->variable->arraysize)
              fprintf(outputfile, "SYM, RAMin-%s, BUF\n", bitname(b));
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
                        fprintf(outputfile, "SYM, FFin-%s, OR\n", bitname(b));
                    else if (b->flags & SYM_FF && b->variable->arraysize)
                        fprintf(outputfile, "SYM, RAMin-%s, OR\n", bitname(b));
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

    bitCnt = QMtermBits(table[i], count + 1);
    fprintf(outputfile, "SYM, %dT_SYM%s, %s\n", i, bitname(b), (bitCnt == 1) ? "BUF" : "AND");
    for (bl = b->primaries, j = count; bl; bl = bl->next, j--) {
	if (!(table[i].dc & (1 << j))) {
	    if (bitCnt == 1)
		fprintf(outputfile, "PIN, I, I, %s%s\n", bitname(bl->bit),
			(table[i].value & (1 << j)) ? "" : ",,INV");
	    else
		fprintf(outputfile, "PIN, I%d, I, %s%s\n", --bitCnt, bitname(bl->bit),
			(table[i].value & (1 << j)) ? "" : ",,INV");
	}
    }
    fprintf(outputfile, "PIN, O, O, %dT_%s\n", i, bitname(b));
    fprintf(outputfile, "END\n");
}
