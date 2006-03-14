/*
 * semi-hack of Quine-MacCluskey by jf to allow minimal EQN expressions
 * in XNF. It is not important to be minimal, just more reduced than
 * naive printing out every term. It produces all implicants. 11/94
 *
 * Originally written by Dr. John Forrest of UMIST, Manchester, UK
 *
 * SVN $Revision$  hosted on http://sourceforge.net/projects/fpgac
 */

#include <stdio.h>
#include "names.h"

printTab (tab, top, bits)
	QMtab *tab;
	int *top;
	int bits;
{
    int i,j;
    fprintf (stderr, "Table:\n");
    for (i=0; i <= *top; i++) {
    	for (j=bits-1; j >= 0; j--) 
    	   fprintf (stderr, "%c", tab[i].dc&(1<<j) ? '-' :
    	   	                  tab[i].value&(1<<j) ? '1' : '0');
    	fprintf (stderr, " %d\n", tab[i].covered);
    }
}


/* Check 2 terms to see if they differ in only one bit position.  If so,
 * we can replace them with a simpler term that just ignores that bit.
 */

int QMbit_diff (value1, value2, bits)
	unsigned int value1, value2;
	int bits;
{
   int result = -1;
   int bit;
   
   for (bit = 0; bit < bits; bit++) {
      if ((value1 & (1<<bit)) != (value2 & (1<<bit))) {/* particular bit pos different */
          if (result < 0)
             result = bit;
          else /* 2nd bit difference */
             return -1; 
      }
   }
   return result;
}

int QMtermBits (entry, bits)
	QMtab entry;
	int bits;
{
    int i, count = 0;
    
    for (i=0; i< bits; i++)
       if (! (entry.dc&(1<<i)))
           count ++;
    return count;
}

int QMsame (tab1, tab2, bits)
	QMtab tab1, tab2;
	int bits;
{
   int mask = (1<<bits)-1;
   return ((tab1.value&mask) == (tab2.value&mask)) &&
          ((tab1.dc&mask) == (tab2.dc&mask));
}

int simpleQM (table, top, max_tab_size, bits)
	QMtab *table;
	int *top;
	int max_tab_size, bits;
{
	int curr_bottom = 0;
	int curr_top = *top;
	int changed;
	int i, j, k, bit;
	
	changed = 1;
	while (changed) {
	    changed = 0;
	    
	    if (debug & 2)
	        fprintf (stderr, "Curr=(%d,%d)\n", curr_bottom, curr_top);
	    for (i=curr_bottom; i <= curr_top; i++) {
	    	for (j=i+1; j <= curr_top; j++) {
	    	    if (table[i].dc == table[j].dc &&
	    	       (bit = QMbit_diff (table[i].value, table[j].value, bits))>=0) {
	    	        table[i].covered = table[j].covered =1;
 	    	        if (++(*top) >= max_tab_size)
	    	            return -1; /* failure */
	    	        table[*top].value = table[i].value & ~(1<<bit);
	    	        table[*top].dc = table[i].dc | (1<<bit);
	    	        table[*top].covered = 0;
	    	        changed = 1;
	    	        for (k=curr_top+1; k < *top; k++)
	    	            if (QMsame(table[k], table[*top], bits)) {
	    	                *top -=1; /* ignore duplicate */
	    	                break;
	    	            }
	    	    }
	    	}
	    }
	    curr_bottom = curr_top+1;
	    curr_top = *top;
	    if (debug & 2)
	        printTab(table, top, bits);
	}
	return 0; 
}
	    	    
QMtruthToTable (truth, table, top, bits)
	long *truth;
	QMtab *table;
	int *top;
	int bits;
{
    int i;
    
    *top = -1;
    
    for (i=0; i<(1<<bits); i++) {
        if(Get_Bit(truth,i)) {
            (*top) += 1;
            table[*top].value = (unsigned char) i;
            table[*top].dc = table[*top].covered = 0;
        }
    }
    if (debug & 2)
        printTab (table, top, bits);   
}
