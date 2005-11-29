/*
 * Copyright notice taken from BSD source, and suitably modified:
 *
 * Copyright (c) 1994-2004 University of Toronto
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
 *   This product includes software developed by the University of
 *   Toronto
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
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "names.h"
#include "outputvars.h"

#include "output_altera_vqm.h"

extern char Revision[];

void output_vqm(char *familyname)
   {
   int i, n, hex, count;
   time_t now;
   int printed;
   char *datestring;
   struct bit *b;
   struct bitlist *bl;
   char size_string[64];

   if(nerrors > 0) {
      return;
   }

   fprintf(outputfile, "// PROGRAM \"fpgac\"\n");

   now = time((time_t) NULL);
   datestring = ctime(&now);
   datestring[strlen(datestring) - 1] = '\0';
   Revision[strlen(Revision) - 2] = '\0';
   if(((int) strlen(Revision)) <= 11)
      strcpy(Revision, "Revision unknown");
   fprintf(outputfile, "// VERSION  \"%s, %s\"\n", &Revision[11],
         datestring);
   fprintf(outputfile, "\n");

   fprintf(outputfile, "module \\%s (\n", get_designname());
   fprintf(outputfile, "\t\\%s ", clockname);

   printed = 0;

   for(n=0; n<nbits; n++) {
      b = &bits[n];

      if(b->variable && !strcmp(b->variable->name, "VCC")) {
         continue;
      }

      if(b->variable && b->bitnumber != 0) {
         continue;
      }

      switch(b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT)) {

      case SYM_OUTPUTPORT:
      case SYM_INPUTPORT:
      case SYM_BUSPORT|SYM_INPUTPORT:
         fprintf(outputfile, ",\n\t\\%s ", b->variable->name+1);
         break;

      case 0: /* normal variables */
         break;

      default:
         fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n",
            externalname(b),
            b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT));
         break;
      }
   }

   fprintf(outputfile, ");\n\n");
   fprintf(outputfile, "input \\%s ;\n", clockname);

   for(n=0; n<nbits; n++) {
      b = &bits[n];

      if(b->variable && !strcmp(b->variable->name, "VCC")) {
         continue;
      }

      if(b->variable && b->bitnumber != 0) {
         continue;
      }

      if(b->variable && b->variable->width > 1) {
         sprintf(size_string, "[%d:0] ", b->variable->width -1);
      }
      else {
         sprintf(size_string, "");
      }

      switch(b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT)) {

      case SYM_OUTPUTPORT:
         fprintf(outputfile, "output\t%s\\%s ;\n", size_string, b->variable->name+1);
         break;

      case SYM_INPUTPORT:
         fprintf(outputfile, "input\t%s\\%s ;\n", size_string, b->variable->name+1);
         break;

      case SYM_BUSPORT|SYM_INPUTPORT:
         fprintf(outputfile, "inout\t%s\\%s ;\n", size_string, b->variable->name+1);
         break;

      case 0: /* normal variables */
         break;

      default:
         fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n",
            externalname(b),
            b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT));
         break;
      }
   }

   for(n=0; n<nbits; n++) {
      b = &bits[n];

      if(b->variable && !strcmp(b->variable->name, "VCC")) {
         continue;
      }

      switch(b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT)) {

      case SYM_OUTPUTPORT:
      case SYM_INPUTPORT:
      case SYM_BUSPORT|SYM_INPUTPORT:
         break;

      case 0: /* normal variables */
         if(b->flags & SYM_AFFECTSOUTPUT) {
            fprintf(outputfile, "wire \\%s ;\n", bitname(b));
         }
         break;

      default:
         fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n",
            externalname(b),
            b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT));
         break;
      }
   }

   fprintf(outputfile, "\n");

   for(n=0; n<nbits; n++) {
      b = &bits[n];

      if(b->variable && !strcmp(b->variable->name, "VCC")) {
         continue;
      }

      switch(b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|BIT_HASPIN|BIT_HASFF|SYM_BUSPORT)) {

      case SYM_INPUTPORT:
      case SYM_INPUTPORT|BIT_HASPIN:
         printed = 1;
         fprintf(outputfile, "assign \\%s  = \\%s ;\n\n",
                        bitname(b), externalname(b));
         break;

      case SYM_INPUTPORT|BIT_HASFF:
      case SYM_INPUTPORT|BIT_HASPIN|BIT_HASFF:
         printed = 1;

         fprintf(outputfile, "%s_lcell \\%s~I (\n", familyname, bitname(b));
         for(i=3; i>0; --i) {
            fprintf(outputfile, "\t.data%c(gnd),\n", 'a' + i);
         }
         fprintf(outputfile, "\t.dataa(\\%s ),\n", externalname(b));
         fprintf(outputfile, "\t.clk(\\%s ),\n", clockname);
         fprintf(outputfile, "\t.regout(\\%s ));\n", bitname(b));
         fprintf(outputfile, "defparam \\%s~I .operation_mode = \"normal\";\n",
			bitname(b));

         fprintf(outputfile, "defparam \\%s~I .lut_mask = \"0002\";\n",
                        bitname(b));
         fprintf(outputfile, "\n");
         break;

      case SYM_OUTPUTPORT|BIT_HASPIN:
      case SYM_OUTPUTPORT|BIT_HASFF|BIT_HASPIN:
      case SYM_OUTPUTPORT|BIT_HASFF:
      case SYM_OUTPUTPORT:
         printed = 1;
         fprintf(outputfile, "assign \\%s  = \\%s ;\n\n",
                        externalname(b), bitname(b));
         break;

      case SYM_INPUTPORT|SYM_BUSPORT|BIT_HASPIN|BIT_HASFF:
      case SYM_INPUTPORT|SYM_BUSPORT|BIT_HASPIN:
         printed = 1;
         fprintf(outputfile, "%s_io \\%s~I (\n", familyname, externalname(b));
         fprintf(outputfile, "\t.datain(\\out%s ),\n", bitname(b));
         fprintf(outputfile, "\t.oe(\\%s ),\n", bitname(b->enable));
         fprintf(outputfile, "\t.combout(\\%s ),\n", bitname(b));
         fprintf(outputfile, "\t.padio(\\%s ));\n", externalname(b));
         fprintf(outputfile, "defparam \\%s~I .operation_mode = \"bidir\";\n",
			externalname(b));
         fprintf(outputfile, "\n");
         break;

      case 0: /* normal variables */
         break;

      default:
         fprintf(stderr, "Warning: %s has unknown combination of flags %lx\n",
            bitname(b),
            b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|BIT_HASPIN|BIT_HASFF|SYM_BUSPORT));
         break;
      }

      if((b->flags & (SYM_INPUTPORT|SYM_BUSPORT)) == SYM_INPUTPORT) {
         ninpins++;
         continue;
         }
      if(b->flags & SYM_BUSPORT)
         nbidirpins++;
      if(b->flags & SYM_OUTPUTPORT)
         noutpins++;

      if((b->flags & (SYM_OUTPUTPORT|SYM_BUSPORT))
            && !(b->flags & BIT_HASFF))
         b->flags &= ~SYM_FF;

      if(b->flags & SYM_AFFECTSOUTPUT) {
         count = countlist(b->primaries) - 1;

         if(count == 0 && !(b->flags & SYM_FF)) {
            if(b->flags & SYM_BUSPORT) {
               fprintf(outputfile, "assign \\out%s  = ", bitname(b));
            }
            else {
               fprintf(outputfile, "assign \\%s  = ", bitname(b));
            }

            if(b->truth[0]) {
               fprintf(outputfile, "~ \\%s ;\n", bitname(b->primaries->bit));
            }
            else {
               fprintf(outputfile, "\\%s ;\n", bitname(b->primaries->bit));
            }
            fprintf(outputfile, "\n");

            continue;
         }
            
         if(count != 0) {
            nroms++;
            inputcounts[count+1]++;
         }

         if(b->flags & SYM_FF) {
            nff++;
         }

         fprintf(outputfile, "%s_lcell \\%s~I (\n", familyname, bitname(b));
         for(i=3; i>count; --i) {
            fprintf(outputfile, "\t.data%c(gnd),\n", 'a' + i);
         }
         for(bl = b->primaries; bl; bl=bl->next) {
            fprintf(outputfile, "\t.data%c(\\%s ),\n", 'a' + count, bitname(bl->bit));
            --count;
         }
         if(b->flags & SYM_FF) {
            fprintf(outputfile, "\t.clk(\\%s ),\n", clockname);
            if(b->clock_enable) {
               fprintf(outputfile, "\t.ena(\\%s ),\n",
                  bitname(b->clock_enable));
            }
            if(b->flags & SYM_BUSPORT) {
               fprintf(outputfile, "\t.regout(\\out%s ));\n", bitname(b));
            }
            else {
               fprintf(outputfile, "\t.regout(\\%s ));\n", bitname(b));
            }
         }
         else {
            if(b->flags & SYM_BUSPORT) {
               fprintf(outputfile, "\t.combout(\\out%s ));\n", bitname(b));
            }
            else {
               fprintf(outputfile, "\t.combout(\\%s ));\n", bitname(b));
            }
         }
         fprintf(outputfile, "defparam \\%s~I .operation_mode = \"normal\";\n",
			bitname(b));

         count = countlist(b->primaries);
         hex = 0;
         for(i=0; i < (1<<count); i++) {
            hex |= (b->truth[i]<<i);
         }
         fprintf(outputfile, "defparam \\%s~I .lut_mask = \"%04X\";\n",
                        bitname(b), hex);
         fprintf(outputfile, "\n");
         }
      }

   fprintf(outputfile, "endmodule\n");

   if(!printed) {
      warning2("compiler produced no output", "");
   }
}
