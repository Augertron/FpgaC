/*
 * output_edif.c -- EDIF netlist output for Fpgac
 * SVN $xRevision: 46 $  hosted on http://sourceforge.net/projects/fpgac
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
//#include "patchlevel.h"

/*
 * Notes about EDIF output format:
 *
 */

static printROM(struct bit *b, int count);
static printGates(struct bit *b, int count);
static printEQN(struct bit *b, int count);
static printAND(int i, QMtab table[], int count, struct bit *b);
static void printRam (struct bit *, struct varlist * , int ) ;

static printExt(char *extname, char *type, char *pin) {
    if (pin) {
	if ((pin[0] >= '0') && (pin[0] <= '9'))
	    fprintf(outputfile, "EXT, %s_pad, %s,, LOC=P%s\n", extname, type, pin);
	else
	    fprintf(outputfile, "EXT, %s_pad, %s,, LOC=%s\n", extname, type, pin);
    } else
	fprintf(outputfile, "EXT, %s_pad, %s\n", extname, type);
}

extern char Revision[];


char *bitname_edif(struct bit *b) {
    char *n;

    if(b->name) n=b->name;
    else if(b->variable) n = b->variable->name;

    while(*n == '_') n++;

    if(*n) return(n);
 
    if(b->variable->width == 1) {
        if(b->flags & SYM_VCC) {
            asprintf(&b->name, "%s", n);
        } else {
            if(b->variable->copyof->arraysize) {
                asprintf(&b->name, "%s_p%d", n, b->variable->port);
            } else {
                asprintf(&b->name, "%s", n);
            }
        }
    } else {
        if(b->variable->copyof->arraysize) {
            asprintf(&b->name, "%s_p%d_%d", n , b->variable->port , b->bitnumber);
        } else {
            asprintf(&b->name, "%s_%d", n, b->bitnumber);
        }
    }
    return (b->name);
}

edif_header() {
    time_t t = time(0);
    struct tm *tm = localtime(&t);

    fprintf(outputfile, "(edif %s\n", get_designname());
    fprintf(outputfile, "  (edifVersion 2 0 0)\n");
    fprintf(outputfile, "  (edifLevel 0)\n");
    fprintf(outputfile, "  (keywordMap\n");
    fprintf(outputfile, "    (keywordLevel 0)\n");
    fprintf(outputfile, "  )\n");
    fprintf(outputfile, "  (status\n");
    fprintf(outputfile, "    (written\n");
    fprintf(outputfile, "      (timeStamp %d %d %d %d %d %d)\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    fprintf(outputfile, "      (author \"FpgaC\")\n");
    fprintf(outputfile, "      (program \"FpgaC Compiler\"\n");
    fprintf(outputfile, "        (version \"%s\")\n", Revision);
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "  )\n");
    fprintf(outputfile, "  (library Active_lib\n");
    fprintf(outputfile, "    (edifLevel 0)\n");
    fprintf(outputfile, "    (technology\n");
    fprintf(outputfile, "      (numberDefinition\n");
    fprintf(outputfile, "        (scale 1 (e 1 -11)(unit TIME))\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell BUF\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell BUFG\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell EQN\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I0\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port I1\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port I2\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port I3\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell FDCP\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port C\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port CE\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port D\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port Q\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell GND\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port GROUND\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell IBUF\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell INV\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell OBUF\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell OBUFT\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port I\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port O\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "          (port T\n");
    fprintf(outputfile, "            (direction INPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (cell VCC\n");
    fprintf(outputfile, "      (cellType generic)\n");
    fprintf(outputfile, "      (view net (viewType netlist)\n");
    fprintf(outputfile, "        (interface\n");
    fprintf(outputfile, "          (port VCC\n");
    fprintf(outputfile, "            (direction OUTPUT)\n");
    fprintf(outputfile, "          )\n");
    fprintf(outputfile, "        )\n");
    fprintf(outputfile, "      )\n");
    fprintf(outputfile, "    )\n");
}
edif_part() {
    fprintf(outputfile, "  (design %s\n", get_designname());
    fprintf(outputfile, "    (cellRef %s\n", get_designname());
    fprintf(outputfile, "      (libraryRef Active_lib)\n");
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "    (property PART\n");
    fprintf(outputfile, "      (string \"%s\")\n",partname);
    fprintf(outputfile, "    )\n");
    fprintf(outputfile, "  )\n");
    fprintf(outputfile, ")\n");
}

output_EDIF() {

    if (nerrors > 0)
	return;
    edif_header();
    edif_inst();
//  edif_net();
    edif_part();
}

edif_inst() {
    int n,i;
    int count;
    int printed;
    struct bit *b;
    struct bitlist *bl;

    if (genclock) {
	fprintf(outputfile, "SYM, OSC4, OSC4\n");
	fprintf(outputfile, "PIN, F15, O, CLKin\n");
	fprintf(outputfile, "END\n");
	fprintf(outputfile, "SYM, CLK-AA, BUFGS\n");
	fprintf(outputfile, "PIN, I, I, CLKin\n");
	fprintf(outputfile, "PIN, O, O, %s\n", clockname);
	fprintf(outputfile, "END\n");
    }
    if(!clockname[0]) {
        clockname = "CLK";
        fprintf(outputfile, "          (instance INST_%s_clk_pad\n", get_designname());
        fprintf(outputfile, "            (viewRef net\n");
        fprintf(outputfile, "              (cellRef IBUF)\n");
        fprintf(outputfile, "            )\n");
        fprintf(outputfile, "          )\n");
        fprintf(outputfile, "          (instance INST_%s_clk\n", get_designname());
        fprintf(outputfile, "            (viewRef net\n");
        fprintf(outputfile, "              (cellRef BUFG)\n");
        fprintf(outputfile, "            )\n");
        fprintf(outputfile, "          )\n");
    }
    printed = 0;
    for(b=bits; b; b=b->next) {
	if (b->variable && !strcmp(b->variable->name, "VCC"))
	    continue;

	switch (b->flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF | SYM_BUSPORT | SYM_CLOCK)) {
	case SYM_INPUTPORT | BIT_HASPIN | SYM_CLOCK:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_pad\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef IBUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
            fprintf(outputfile, "          (instance INST_%s\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef BUFG)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case SYM_INPUTPORT | BIT_HASPIN:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_pad\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef IBUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case SYM_INPUTPORT | BIT_HASPIN | BIT_HASFF:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_pad\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef IBUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
            fprintf(outputfile, "          (instance INST_%s\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef FDCP)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;


	case SYM_INPUTPORT | BIT_HASFF:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_buf\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef BUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
            fprintf(outputfile, "          (instance INST_%s\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef FDCP)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case SYM_INPUTPORT:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_buf\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef BUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case SYM_OUTPUTPORT | BIT_HASFF | BIT_HASPIN:
	case SYM_OUTPUTPORT | BIT_HASPIN:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_pad\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef OBUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case SYM_OUTPUTPORT | BIT_HASFF:
	case SYM_OUTPUTPORT:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_buf\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef BUF)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case SYM_INPUTPORT | SYM_BUSPORT | BIT_HASPIN | BIT_HASFF:
	case SYM_INPUTPORT | SYM_BUSPORT | BIT_HASPIN:
	    printed = 1;
            fprintf(outputfile, "          (instance INST_%s_obuft\n",bitname(b));
            fprintf(outputfile, "            (viewRef net\n");
            fprintf(outputfile, "              (cellRef OBUFT)\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "            (portInstance T\n");
            fprintf(outputfile, "              (property INV\n");
            fprintf(outputfile, "                (string \"\")\n");
            fprintf(outputfile, "              )\n");
            fprintf(outputfile, "            )\n");
            fprintf(outputfile, "          )\n");
	    break;

	case 0:		/* normal variables */
	    break;

	default:
	    fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n", bitname(b),
		    b->flags & (SYM_INPUTPORT | SYM_OUTPUTPORT | BIT_HASPIN | BIT_HASFF | SYM_BUSPORT));
	    break;
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

	if (b->flags & SYM_AFFECTSOUTPUT) {
	    printed = 1;
	    count = countlist(b->primaries) - 1;
	    if (count <= 0) {
                if (b->flags & SYM_FF && !b->variable->arraysize) {
		    nff++;
		    fprintf(outputfile, "SYM, %s_FFin, ", bitname(b));
                } else if (b->flags & SYM_FF && b->variable->arraysize) {
                    fprintf(outputfile, "SYM, %s_RAMin, ", bitname(b));
		} else
		    fprintf(outputfile, "SYM, %s, ", bitname(b));
		if (Get_Bit(b->truth,0))
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
		fprintf(outputfile, "PIN, O, O, %s_FFin\n", bitname(b));
            else if (b->flags & SYM_FF && b->variable->arraysize)
                fprintf(outputfile, "PIN, O, O, %s_RAMin\n", bitname(b));
	    else if (b->flags & SYM_BUSPORT)
		fprintf(outputfile, "PIN, O, O, %s_FF\n", bitname(b));
	    else
		fprintf(outputfile, "PIN, O, O, %s\n", bitname(b));
	    fprintf(outputfile, "END\n");

            if (b->flags & SYM_FF && b->variable->arraysize) {
                    if ( b->variable->arrayref->variable  == b->variable->arraywrite ) 
                    {
                            if (b->variable->arraysize <= 16) {
                                    fprintf(outputfile, "SYM, %s, ram16x1s\n", bitname(b));
                            } else if (b->variable->arraysize <= 32) {
                                    fprintf(outputfile, "SYM, %s, ram32x1s\n", bitname(b));
                            } else if (b->variable->arraysize <= 64) {
                                    fprintf(outputfile, "SYM, %s, ram64x1s\n", bitname(b));
                            } else if (b->variable->arraysize <= 128) {
                                    fprintf(outputfile, "SYM, %s, ram128x1s\n", bitname(b));
                            } else if (b->variable->arraysize <= 4096) {
                                    fprintf(outputfile, "SYM, %s, ramb4_s1\n", bitname(b));
                            } else {
                                    fprintf(outputfile, "SYM, %s, rammacro\n", bitname(b));
                            }

// For older ISE use this instead of above
//			    fprintf(outputfile, "SYM, %s, RAMS\n", bitname(b));
                            fprintf(outputfile, "PIN, D, I, %s_RAMin\n", bitname(b));
                            fprintf(outputfile, "PIN, WCLK, I, CLK\n");
                            if(b->variable->arraywrite && b->variable->arraywrite->index->bits) {
                                    for (i=0,bl = b->variable->arraywrite->index->bits;i<b->variable->arrayaddrbits;i++) {
                                            if (bl && bl->bit) {
                                                    if (bl->bit->flags & SYM_AFFECTSOUTPUT) {
                                                            fprintf(outputfile, "PIN, A%d, I, %s\n", i, bitname(bl->bit));
                                                    }
                                            } else
                                                    fprintf(outputfile, "PIN, A%d, I, GND\n", i);
                                            if(bl) bl = bl->next;
                                    }
                            }
                            if (b->clock_enable)
                                    fprintf(outputfile, "PIN, WE, I, %s\n", bitname(b->clock_enable));
                            else
                                    fprintf(outputfile, "PIN, WE, I, VCC\n");
                            fprintf(outputfile, "PIN, SPO, O, %s\n", bitname(b));
                            for (bl = b->variable->arrayref->variable->bits; bl; bl = bl->next) {
                                    if(b->bitnumber == bl->bit->bitnumber) {
                                            fprintf(outputfile, "PIN, O, O, %s\n", bitname(bl->bit));
                                            break;
                                    }
                            }
                            fprintf(outputfile, "END\n");
                    }
                    else 
                    { 
                            // else instance a dual port RAM 
                            struct varlist  *array_read_reference_list = b->variable->arrayref;
                            int ram_count = 0 ;
                            // arrayref = list index to the dual port ram 
                            // replicate dual port RAM for as many read indexes 
                            // all of them will have a common/replicated write port and seperate read port
                            for ( array_read_reference_list = b->variable->arrayref ;array_read_reference_list!=NULL ;array_read_reference_list = array_read_reference_list->next,ram_count++) { 
                                    // a default read port is added  , ignore it 
                                    if ( array_read_reference_list->variable  == b->variable->arraywrite ) break;

                                    // Not all of these sizes are available ... edit for your device
                                    if (b->variable->arraysize <= 16) {
                                            fprintf(outputfile, "SYM, %s, ram16x1d\n", bitname(b));
                                    } else if (b->variable->arraysize <= 32) {
                                            fprintf(outputfile, "SYM, %s, ram32x1d\n", bitname(b));
                                    } else if (b->variable->arraysize <= 64) {
                                            fprintf(outputfile, "SYM, %s, ram64x1d\n", bitname(b));
                                    } else if (b->variable->arraysize <= 128) {
                                            fprintf(outputfile, "SYM, %s, ram128x1d\n", bitname(b));
                                    } else if (b->variable->arraysize <= 4096) {
                                            fprintf(outputfile, "SYM, %s, ramb4_s1_s1\n", bitname(b));
                                    } else {
                                            fprintf(outputfile, "SYM, %s, rammacro\n", bitname(b));
                                    }

// uncomment for older ISE, and comment out above
//				    fprintf(outputfile, "SYM, %s_%d, RAMD\n", bitname(b),ram_count);

                                    fprintf(outputfile, "PIN, D, I, %s_RAMin\n", bitname(b));
                                    fprintf(outputfile, "PIN, WCLK, I, CLK\n");
                                    if(b->variable->arraywrite && b->variable->arraywrite->index->bits) {
                                            for (i=0,bl = b->variable->arraywrite->index->bits;i<b->variable->arrayaddrbits;i++) {
                                                    if (bl && bl->bit) {
                                                            if (bl->bit->flags & SYM_AFFECTSOUTPUT) {
                                                                    fprintf(outputfile, "PIN, A%d, I, %s\n", i, bitname(bl->bit));
                                                            }
                                                    } else
                                                            fprintf(outputfile, "PIN, A%d, I, GND\n", i);
                                                    if(bl) bl = bl->next;
                                            }
                                    }
                                    if(b->variable->arrayref && array_read_reference_list->variable->bits)
                                    {
                                            for (i=0,bl =  array_read_reference_list->variable->index->bits ;i<b->variable->arrayaddrbits;i++) {
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
                                    fprintf(outputfile, "PIN, SPO, O, %s\n", bitname(b));
                                    for (bl = array_read_reference_list->variable->bits; bl; bl = bl->next) {
                                            if(b->bitnumber == bl->bit->bitnumber) {
                                                    fprintf(outputfile, "PIN, DPO, O, %s\n", bitname(bl->bit));
                                                    break;
                                            }
                                    }
                                    fprintf(outputfile, "END\n");
                            }
                    }
            } else if (b->flags & SYM_FF) {
		if (b->flags & SYM_BUSPORT)
		    fprintf(outputfile, "SYM, %s_DFF, DFF\n", bitname(b));
		else
		    fprintf(outputfile, "SYM, %s, DFF\n", bitname(b));
		fprintf(outputfile, "PIN, D, I, %s_FFin\n", bitname(b));
		fprintf(outputfile, "PIN, C, I, %s\n", clockname);
		if (b->clock_enable)
		    fprintf(outputfile, "PIN, CE, I, %s\n", bitname(b->clock_enable));
		else
		    fprintf(outputfile, "PIN, CE, I, VCC\n");
		if (b->flags & SYM_BUSPORT)
		    fprintf(outputfile, "PIN, Q, O, %s_FF\n", bitname(b));
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


void printRam (struct bit *b, struct varlist * array_read_reference_list , int count) 
{

        struct bitlist *bl;
        int i;
        if ( array_read_reference_list == NULL ) return ;
        else printRam ( b , array_read_reference_list->next,count+1 );
}



static printROM(struct bit *b, int count) {
    struct bitlist *bl;
    int i, hex;

    if (b->flags & SYM_FF && !b->variable->arraysize)
	fprintf(outputfile, "SYM, %s_FFin, ROM, ", bitname(b));
    else if (b->flags & SYM_FF && b->variable->arraysize)
        fprintf(outputfile, "SYM, %s_RAMin, ROM, ", bitname(b));
    else
	fprintf(outputfile, "SYM, %s, ROM, ", bitname(b));
    hex = 0;
    for (i = 0; i < (1 << (count + 1)); i++)
	hex |= (Get_Bit(b->truth,i) << i);
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
	fprintf(outputfile, "SYM, %s_FFin, EQN, EQN=(", bitname(b));
      else if (b->flags & SYM_FF && b->variable->arraysize)
        fprintf(outputfile, "SYM, %s_RAMin, EQN, EQN=(", bitname(b));
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
              fprintf(outputfile, "SYM, %s_FFin, BUF\n", bitname(b));
          else if (b->flags & SYM_FF && b->variable->arraysize)
              fprintf(outputfile, "SYM, %s_RAMin, BUF\n", bitname(b));
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
                        fprintf(outputfile, "SYM, %s_FFin, OR\n", bitname(b));
                    else if (b->flags & SYM_FF && b->variable->arraysize)
                        fprintf(outputfile, "SYM, %s_RAMin, OR\n", bitname(b));
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
