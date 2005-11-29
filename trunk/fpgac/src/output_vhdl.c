/*
 * Copyright notice taken from BSD source, and suitably modified:
 *
 * Copyright (c) 1994, 1995, 1996, 2003 University of Toronto
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

extern char *get_designname(void);

static void printEQN(struct bit *b, int count);
static char *get_nth_name(struct bitlist *bl, int n);

extern char Revision[];

extern int thread;

void output_vhdl(void)
   {
   int n;
   int count;
   time_t now;
   int printed;
   char *datestring, type[MAXNAMELEN];
   struct bit *b;

   if(nerrors > 0) {
      return;
   }

   fprintf(outputfile, "library ieee;\n");
   fprintf(outputfile, "use ieee.std_logic_1164.all;\n");
   fprintf(outputfile, "use ieee.std_logic_arith.all;\n");
   fprintf(outputfile, "use ieee.std_logic_unsigned.all;\n");

   now = time((time_t) NULL);
   datestring = ctime(&now);
   datestring[strlen(datestring) - 1] = '\0';
   Revision[strlen(Revision) - 2] = '\0';
   if(((int) strlen(Revision)) <= 11) {
      strcpy(Revision, "Revision unknown");
   }
   fprintf(outputfile, "\n-- fpgac %s %s\n\n", &Revision[11], datestring);

   fprintf(outputfile, "entity %s is port(\n", get_designname());
   fprintf(outputfile, "	%s : in std_logic;\n", clockname);
   fprintf(outputfile, "	%s : in std_logic", resetname);

   /* Print all of the input, output and bidir variables */

   for(n=0; n<nbits; n++) {
      b = &bits[n];

      if(b->variable && !strcmp(b->variable->name, "VCC")) {
         continue;
      }

      if(b->variable) {
         if(b->bitnumber != 0) {
            continue;
         }
         if(b->variable->width == 1) {
            sprintf(type, "std_logic");
         }
         else {
            struct bitlist *l;
            for(l = b->variable->bits;l;l = l->next) l->bit->flags |= BIT_WORD;
            sprintf(type, "std_logic_vector(%d downto 0)",
                                  b->variable->width - 1);
         }
      }

      switch(b->flags & (SYM_INPUTPORT|SYM_OUTPUTPORT|SYM_BUSPORT)) {

      case SYM_OUTPUTPORT:
         fprintf(outputfile, ";\n	%s : out %s", b->variable->name+1, type);
         noutpins++;
         break;

      case SYM_INPUTPORT:
         fprintf(outputfile, ";\n	%s : in %s", b->variable->name+1, type);
         ninpins++;
         break;

      case SYM_BUSPORT|SYM_INPUTPORT:
         fprintf(outputfile, ";\n	%s : inout %s", b->variable->name+1, type);
         nbidirpins++;
         noutpins++;
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

   fprintf(outputfile, "\n);\nend;\n\n");
   fprintf(outputfile, "architecture arch_%s of %s is\n\n", get_designname(), get_designname());

   /* Declare all of the variables we will use */

   for(n=0; n<nbits; n++) {
      b = &bits[n];
      if(b->variable && !strcmp(b->variable->name, "VCC"))
         continue;

      if(b->flags & (SYM_AFFECTSOUTPUT|SYM_INPUTPORT|SYM_OUTPUTPORT)) {
         if(b->variable && b->variable->width > 1 && b->bitnumber == 0) {
             char buf[MAXNAMELEN],*c;
             struct bitlist *l;

             // see if declaring all bits for this variable
             for(l = b->variable->bits;l;l = l->next)
                 if(!(l->bit->flags & (SYM_AFFECTSOUTPUT|SYM_INPUTPORT|SYM_OUTPUTPORT)))
                     break;
             if(!l) {
                 // yes, then use in word form
                 for(l = b->variable->bits;l;l = l->next) l->bit->flags |= BIT_WORD;
                 sprintf(buf, "%s", bitname(b));
                 for(c=buf;*c;c++);*--c=0;*--c=0;*--c=0;
                 fprintf(outputfile, "	signal %s : std_logic_vector(%d downto 0);\n", buf,b->variable->width-1);
                 continue;
             }
             // otherwise, fall thru and output as single bit.
         }
         if(!(b->flags&BIT_WORD))fprintf(outputfile, "	signal %s : std_logic;\n", bitname(b));
      }
   }

   for(n=0; n<nbits; n++) {
      b = &bits[n];
      if(b->variable && !strcmp(b->variable->name, "VCC"))
         continue;

      if(b->flags & (SYM_AFFECTSOUTPUT|SYM_INPUTPORT|SYM_OUTPUTPORT)) {
         if((b->flags & (SYM_INPUTPORT|SYM_BUSPORT|BIT_HASFF)) == SYM_INPUTPORT) {
            continue;
         }
         if(b->flags & SYM_BUSPORT) {
            if(b->variable && b->variable->width > 1 && b->bitnumber == 0) {
                char buf[MAXNAMELEN],*c;
                struct bitlist *l;

                // see if declaring all bits for this variable
                for(l = b->variable->bits;l;l = l->next)
                    if(!(l->bit->flags & SYM_BUSPORT))
                        break;
                if(!l) {
                    // yes, then use in word form
                    for(l = b->variable->bits;l;l = l->next) l->bit->flags |= BIT_WORD;
                    sprintf(buf, "%s", bitname(b));
                    for(c=buf;*c;c++);*--c=0;*--c=0;*--c=0;
                    fprintf(outputfile, "	signal %s : std_logic_vector(%d downto 0);\n", buf,b->variable->width-1);
                    continue;
                }
                // otherwise, fall thru and output as single bit.
            }
            if(!(b->flags&BIT_WORD))fprintf(outputfile, "	signal out%s : std_logic;\n", bitname(b));
         }
      }
   }

   for(n=0; n<nbits; n++) {
      b = &bits[n];
      if(b->variable && !strcmp(b->variable->name, "VCC"))
         continue;

      if(b->flags & (SYM_AFFECTSOUTPUT|SYM_INPUTPORT|SYM_OUTPUTPORT)) {
         if((b->flags & (SYM_INPUTPORT|SYM_BUSPORT|BIT_HASFF)) == SYM_INPUTPORT) {
            continue;
         }
         if((b->flags & (SYM_OUTPUTPORT|SYM_BUSPORT))
               && !(b->flags & BIT_HASFF)) {
            continue;
         }

         if(b->flags & SYM_FF) {
            if(b->variable && b->variable->width > 1 && b->bitnumber == 0) {
                char buf[MAXNAMELEN],*c;
                struct bitlist *l;

                // see if declaring all bits for this variable
                for(l = b->variable->bits;l;l = l->next)
                    if(!(l->bit->flags & SYM_FF))
                        break;
                if(!l) {
                    // yes, then use in word form
                    for(l = b->variable->bits;l;l = l->next) l->bit->flags |= BIT_WORD;
                    sprintf(buf, "%s", bitname(b));
                    for(c=buf;*c;c++);*--c=0;*--c=0;*--c=0;
                    fprintf(outputfile, "	signal FFin_%s : std_logic_vector(%d downto 0);\n", buf,b->variable->width-1);
                    continue;
                }
                // otherwise, fall thru and output as single bit.
            }
            if(!(b->flags&BIT_WORD))fprintf(outputfile, "	signal FFin_%s : std_logic;\n", bitname(b));
         }
      }
   }

   fprintf(outputfile, "\n\nbegin\n\n");

   /* Copy the internal names of the inputs and outputs to their real names */

   for(n=0; n<nbits; n++) {
      b = &bits[n];
      if(b->variable && !strcmp(b->variable->name, "VCC"))
         continue;

      switch(b->flags & (SYM_INPUTPORT|SYM_BUSPORT|SYM_OUTPUTPORT|BIT_HASFF)) {

      case SYM_INPUTPORT :
         if(b->flags & BIT_WORD) {
             if(b->bitnumber == 0)
                 fprintf(outputfile, "	T%d_%d%s <= %s;\n",
                         thread, b->variable->lineno, b->variable->name,
                         b->variable->name + 1);
         } else {
             fprintf(outputfile, "	%s <= %s;\n", bitname(b), externalname(b));
         }
         break;

      case SYM_INPUTPORT|SYM_BUSPORT :
      case SYM_INPUTPORT|SYM_BUSPORT|BIT_HASFF :
         if(b->flags & BIT_WORD) {
             if(b->bitnumber == 0) {
                 fprintf(outputfile, "	T%d_%d%s <= %s;\n",
                         thread, b->variable->lineno, b->variable->name,
                         b->variable->name + 1);
                 fprintf(outputfile, "	%s <= outT%d_%d%s",
                         b->variable->name + 1,
                         thread, b->variable->lineno, b->variable->name);
                 fprintf(outputfile, " when %s = '1'", bitname(b->enable));
                 fprintf(outputfile, " else 'Z';\n");
             }
         } else {
             fprintf(outputfile, "	%s <= %s;\n", bitname(b), externalname(b));
             fprintf(outputfile, "	%s <= out%s", externalname(b), bitname(b));
             fprintf(outputfile, " when %s = '1'", bitname(b->enable));
             fprintf(outputfile, " else 'Z';\n");
         }
         break;

      case SYM_INPUTPORT|BIT_HASFF :
         if(b->flags & BIT_WORD) {
             if(b->bitnumber == 0)
                 fprintf(outputfile, "	FFin_T%d_%d%s <= %s;\n",
                         thread, b->variable->lineno, b->variable->name,
                         b->variable->name + 1);
         } else {
             fprintf(outputfile, "	FFin_%s <= %s;\n", bitname(b), externalname(b));
         }
         break;

      case SYM_OUTPUTPORT :
      case SYM_OUTPUTPORT|BIT_HASFF :
         if(b->flags & BIT_WORD) {
             if(b->bitnumber == 0)
                 fprintf(outputfile, "	%s <= T%d_%d%s;\n", b->variable->name + 1,
                         thread, b->variable->lineno, b->variable->name);
         } else {
             fprintf(outputfile, "	%s <= %s;\n", externalname(b), bitname(b));
         }
         break;
      }
   }

   fprintf(outputfile, "\n");

   /* Print the combinational logic out */

   for(n=0; n<nbits; n++) {
      b = &bits[n];
      if(b->variable && !strcmp(b->variable->name, "VCC"))
         continue;

      if((b->flags & (SYM_INPUTPORT|SYM_BUSPORT)) == SYM_INPUTPORT) {
         continue;
      }

      if((b->flags & (SYM_OUTPUTPORT|SYM_BUSPORT))
            && !(b->flags & BIT_HASFF)) {
         b->flags &= ~SYM_FF;
      }

      if(b->flags & SYM_AFFECTSOUTPUT) {
         printed = 1;
         count = countlist(b->primaries)-1;
         if(count <= 0) {
            if(b->flags & SYM_FF) {
               nff++;
               fprintf(outputfile, "	FFin_%s <= ", bitname(b));
            }
            else if(b->flags & SYM_BUSPORT) {
               fprintf(outputfile, "	out%s <= ", bitname(b));
            }
            else {
               fprintf(outputfile, "	%s <= ", bitname(b));
            }
            if(count == -1) {
               if(b->truth[0])
                  fprintf(outputfile, "'1';\n");
               else
                  fprintf(outputfile, "'0';\n");
            }
            if(count == 0) {
               if(b->truth[0])
                  fprintf(outputfile, "not %s;\n", bitname(b->primaries->bit));
               else
                  fprintf(outputfile, "%s;\n", bitname(b->primaries->bit));
            }
         }
         else {
            nroms++;
            inputcounts[count+1]++;

            if(b->flags & SYM_FF) {
               nff++;
            }

            printEQN(b, count);
         }
      }
   }

   /* Now do the flip-flops */

   fprintf(outputfile, "\n\nprocess(%s, %s) begin\n\n", resetname, clockname);
   fprintf(outputfile, "\n	if (%s = '1') then\n\n", resetname);

   /* Do all of the reset values */

   for(n=0; n<nbits; n++) {
      b = &bits[n];

      if(b->variable && !strcmp(b->variable->name, "VCC")) {
         continue;
      }

      if((b->flags & (SYM_INPUTPORT|SYM_BUSPORT|BIT_HASFF)) == SYM_INPUTPORT) {
         continue;
      }

      if(b->flags & SYM_AFFECTSOUTPUT) {
         if(b->flags & SYM_FF) {
            if(b->flags & BIT_WORD) {
               if(b->bitnumber == 0)
                  fprintf(outputfile, "		%sT%d_%d%s <= \"%.*s\";\n",
                          b->flags & SYM_BUSPORT?"out":"",
                          thread, b->variable->lineno, b->variable->name,
                          b->variable->width,
                          "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
            } else {
               fprintf(outputfile, "		%s%s <= '0';\n",
                       b->flags & SYM_BUSPORT?"out":"",
                       bitname(b));
            }
         }
      }
   }

   fprintf(outputfile, "\n	elsif (%s'event and %s = '1') then\n\n",
			clockname, clockname);

   for(n=0; n<nbits; n++) {
      b = &bits[n];
      if(b->variable && !strcmp(b->variable->name, "VCC"))
         continue;

      if((b->flags & (SYM_INPUTPORT|SYM_BUSPORT|BIT_HASFF)) == SYM_INPUTPORT) {
         continue;
      }

      if(b->flags & SYM_AFFECTSOUTPUT) {
         if((b->flags & SYM_FF) && (!(b->flags & BIT_WORD) || ((b->flags & BIT_WORD) && b->bitnumber == 0))) {
            if(b->clock_enable) {
               fprintf(outputfile, "\t\tif (%s = '1') then\n\t",
                                 bitname(b->clock_enable));
            }
            if(b->flags & SYM_BUSPORT) {
               if((b->flags & BIT_WORD) && b->bitnumber == 0) {
                  fprintf(outputfile, "\t\toutT%d_%d%s <= FFin_T%d_%d%s;\n",
                          thread, b->variable->lineno, b->variable->name,
                          thread, b->variable->lineno, b->variable->name);
               } else
                  fprintf(outputfile, "\t\tout%s <= FFin_%s;\n",
                                   bitname(b), bitname(b));
            }
            else {
               if((b->flags & BIT_WORD) && b->bitnumber == 0) {
                  fprintf(outputfile, "\t\tT%d_%d%s <= FFin_T%d_%d%s;\n",
                          thread, b->variable->lineno, b->variable->name,
                          thread, b->variable->lineno, b->variable->name);
               } else
                  fprintf(outputfile, "\t\t%s <= FFin_%s;\n",
                                   bitname(b), bitname(b));
            }
            if(b->clock_enable) {
               fprintf(outputfile, "\t\tend if;\n");
            }
         }
      }
   }

   fprintf(outputfile, "	end if;\n\n"); 
   fprintf(outputfile, "end process;\n\n");
   fprintf(outputfile, "end arch_%s;\n", get_designname());
}


/* The following code originally by Dr. John Forrest of UMIST, Manchester, UK */

static void printEQN(struct bit *b, int count) {

   /* Print a one line VHDL expression for the boolean function for this bit */

   int first = 1, i, j, first_in_term, top;
   struct bitlist *bl;
   QMtab table[128];

   QMtruthToTable(b->truth, table, &top, count+1);
   if(simpleQM(table, &top, QMtabSize, count+1) != 0) {
      error2("QM overflow in printEQN, should not happen", bitname(b));
      abort();
   }

   if(b->flags & SYM_FF) {
      fprintf (outputfile, "	FFin_%s <= ", bitname(b));
   }
   else if(b->flags & SYM_BUSPORT) {
      fprintf(outputfile, "	out%s <= ", bitname(b));
   }
   else {
      fprintf (outputfile, "	%s <= ", bitname(b));
   }

   for(i=0; i <= top; i++) {
      if(table[i].covered)
         continue;
      first_in_term = 1;
      if(!first)
         fprintf (outputfile, " or ");
      first = 0;
      fprintf (outputfile, "(");
      for (j=0;j<count+1;j++) {
         if(table[i].dc & (1<<j))
            continue;
         if(!first_in_term)
            fprintf (outputfile, " and ");
         first_in_term = 0;
         if(!(table[i].value & (1<<j)))
            fprintf (outputfile, "not ");
         fprintf (outputfile, "%s", get_nth_name(b->primaries, count - j));
      }
      if(first_in_term) {
         fprintf (stderr, "%s is Vcc!\n", bitname(b));
         /* printTab (table, &top, count+1); */
         fprintf (outputfile, "'1'");
      }
      fprintf (outputfile, ")");
   }
   if(first) /* no terms were true */
      fprintf (outputfile, "'0'");
   fprintf (outputfile, ";\n");
}



static char *get_nth_name(struct bitlist *bl, int n) {

   /* Return the bitname of the nth element on the list */

   int j;

   for(j=0; j<n; j++) {
      bl = bl->next;
      if(bl == NULL) {
         break;
      }
   }

   if(bl != NULL) {
      return(bitname(bl->bit));
   }
   else {
      return("UNKNOWN_BIT");
   }
}
