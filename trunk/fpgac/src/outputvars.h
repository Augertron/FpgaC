/*
 * outputvars.h -- Variables used in the output section of the code
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

EXTFIX FILE *outputfile;

EXTFIX int nerrors;

EXTFIX int genclock;

EXTFIX char *partname;

EXTFIX char *clockname;
EXTFIX char *resetname;

EXTFIX int nroms, nff, ninpins, noutpins, nbidirpins;
EXTFIX int inputcounts[MAXPRI+1];

/* Values for output_format */
#define XNFROMS		0
#define XNFGATES	1
#define XNFEQNS		2
#define VHDL		3
#define STRATIX_VQM	4
#define CNFROMS         5
#define CNFGATES        6
#define CNFEQNS         7
#define EDFROMS		8
#define EDFGATES	9
#define EDFEQNS		10


EXTFIX int output_format;
