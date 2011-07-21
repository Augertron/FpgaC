%option stack
%x SAVETOKENS
%{
/*
 * syntax.l -- the scanner for fpgac
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
#include <string.h>
#include "names.h"
#include "y.tab.h"

extern YYSTYPE yylval;

int inputlineno = 1;

extern char inputfilename[];

static int saving, replaying;
static int replaystarting;

int number_br = 0;           // to get the last closing brace in the for loop
int ignore_tokens = 0;       // To check if the token has to eb ignored
int ignore_token_type =0;    // only FOR loop for now
int pragma = 0;
int fpgac = 0;
int omp = 0;

YY_BUFFER_STATE include_stack[2];

struct token {
    int t;
    YYSTYPE yylval;
    struct token *next;
};

struct tokenlist {
    struct token *first;
    struct token *last;
    struct tokenlist *next;
};

struct tokenlist *tokenstack;

newtoken(int t) {
    struct token *token;
    extern int debug;

    if(debug & 1) {
        printf("newtoken %d '%s' %s %s %s\n", t, yytext, saving?"saving":"", replaying?"replaying":"" ,ignore_tokens?"ignore":"" );
    }

    if(saving) {
        if((token = (struct token *) malloc(sizeof(struct token))) == (struct token *) NULL) {
            fprintf(stderr, "fpgac: newtoken malloc failed\n");
            exit(1);
        }
        token->t = t;
        token->yylval = yylval;
        token->next = (struct token *) NULL;
        if(tokenstack->last)
            tokenstack->last->next = token;
        if(!tokenstack->first)
            tokenstack->first = token;
        tokenstack->last = token;
    }

    /* ignore token */
    if(ignore_tokens == 1 && saving == 1) {
        switch (ignore_token_type) {
        case IGNORE_FORLOOP:
                             if(t == LEFTPAREN) number_br++ ;
                             if(t == RIGHTPAREN) number_br--; 
                             if(number_br == 0) {
                                 ignore_tokens = 0;
                                 saving = 0;
                             }
                             break;
        }
    }

    if (ignore_tokens == 1)
        return (IGNORETOKEN ) ;
    else
        return(t);
}

mk_identifier()
{
    char *buf;

    if((buf = (char *) malloc(strlen(yytext)+2)) == (char *) NULL) {
	fprintf(stderr, "fpgac: malloc returned NULL\n");
	exit(1);
    }
    sprintf(buf, "%s", yytext);
    yylval.s = buf;
    return(newtoken(IDENTIFIER));
}


saveinput() {

    if(!tokenstack) {
        if((tokenstack = (struct tokenlist *) malloc(sizeof(struct tokenlist))) == (struct tokenlist *) NULL) {
            fprintf(stderr, "fpgac: saveinput malloc failed\n");
            exit(1);
        }
    }
    saving = 1;
}

stopsavinginput() {
    saving = 0;
}

startsavinginput() {
    saving = 1;
}

replayinput() {
    replaystarting = 1;
    replaying = 1;
}

pushinputstream() {
    struct tokenlist *tl;

    if((tl = (struct tokenlist *) malloc(sizeof(struct tokenlist))) == (struct tokenlist *) NULL) {
        fprintf(stderr, "fpgac: saveinput malloc failed\n");
        exit(1);
    }
    tl->next = tokenstack;
    tl->first = tl->last = (struct token *) NULL;
    tokenstack = tl;
}

popinputstream() {
    if(tokenstack)
        tokenstack = tokenstack->next;
}

/*
 * function to pop off the nth element from the end of the list
 * the list is a stack of tokens stored during lexing ( if the start bit is set)
 * n >= 1 ( if n==1 ) the last element of the list is deleted
 */
pop_input_token(int n ) {
    struct token *iterator_temp_current , * iterator_temp_prev=NULL;

    // first current move n steps 
    iterator_temp_current=tokenstack->first;
    for(; ((iterator_temp_current != NULL) && (n>0)); n--) {
        iterator_temp_current = iterator_temp_current->next;
    }

    // now move current and previous pointers ( now previous will be n steps behind current)
    iterator_temp_prev = tokenstack->first;
    while(iterator_temp_current != NULL) {
        iterator_temp_prev  = iterator_temp_prev->next;
        iterator_temp_current = iterator_temp_current->next;
    }

    // delete the previous->next element
    if((iterator_temp_prev != NULL) && (iterator_temp_prev->next !=NULL)) {

        iterator_temp_current = iterator_temp_prev->next;           // use current as temp pointer
        iterator_temp_prev->next = iterator_temp_prev->next->next;

        free(iterator_temp_current);                                // delete the nth element
    }

}

%}


%%
%{
    if(replaying && tokenstack && tokenstack->first) {
        struct token *token;

        if(replaystarting) {
            replaystarting = 0;
            return(newtoken(REPLAYSTART));
        }
        yylval = tokenstack->first->yylval;
        token = tokenstack->first;
        tokenstack->first = tokenstack->first->next;
        return(newtoken(token->t));

    } else if(replaying) {

        replaying = 0;
        return(newtoken(REPLAYEND));
    }

%}

"("				return(newtoken(LEFTPAREN));

")"				return(newtoken(RIGHTPAREN));

"["				return(newtoken(LEFTBRACE));

"]"				return(newtoken(RIGHTBRACE));

"."				return(newtoken(PERIOD));

","				return(newtoken(COMMA));

"{"				return(newtoken(LEFTCURLY));

"}"				return(newtoken(RIGHTCURLY));

"&"				return(newtoken(AND));

"|"				return(newtoken(OR));

"&&"				return(newtoken(ANDAND));

"||"				return(newtoken(OROR));

"^"				return(newtoken(XOR));

"+"				return(newtoken(ADD));

"-"				return(newtoken(SUB));

"*"				return(newtoken(MULTIPLY));

"/"				return(newtoken(DIVIDE));

"%"				return(newtoken(REMAINDER));

"~"				return(newtoken(TILDE));

"--"				return(newtoken(MINUSMINUS));

"++"				return(newtoken(PLUSPLUS));

":"				return(newtoken(COLON));

"?"				return(newtoken(QUESTION));

";"				return(newtoken(SEMICOLON));

"="				return(newtoken(EQUAL));

"+="				return(newtoken(PLUSEQUAL));

"-="				return(newtoken(MINUSEQUAL));

"*="				return(newtoken(MULTIPLYEQUAL));

"/="				return(newtoken(DIVIDEEQUAL));

"%="				return(newtoken(REMAINDEREQUAL));

">>="				return(newtoken(SHIFTRIGHTEQUAL));

"<<="				return(newtoken(SHIFTLEFTEQUAL));

"&="				return(newtoken(ANDEQUAL));

"^="				return(newtoken(XOREQUAL));

"|="				return(newtoken(OREQUAL));

"=="				return(newtoken(EQUALEQUAL));

"!="				return(newtoken(NOTEQUAL));

">="				return(newtoken(GREATEROREQUAL));

">"				return(newtoken(GREATER));

"<="				return(newtoken(LESSTHANOREQUAL));

"<"				return(newtoken(LESSTHAN));

">>"				return(newtoken(SHIFTRIGHT));

"<<"				return(newtoken(SHIFTLEFT));

"!"				return(newtoken(NOT));

_Bool				return(newtoken(BOOL));

int				return(newtoken(INT));

short				return(newtoken(SHORT));

long				return(newtoken(LONG));

char				return(newtoken(CHAR));

signed				return(newtoken(SIGNED));

unsigned			return(newtoken(UNSIGNED));

float				return(newtoken(FLOAT));

double				return(newtoken(DOUBLE));

register			return(newtoken(REGISTER));

extern				return(newtoken(EXTERN));

const				return(newtoken(CONST));

volatile			return(newtoken(VOLATILE));

fpgac_process			return(newtoken(PROCESS));

void				return(newtoken(VOID));

enum                            return(newtoken(ENUM));

struct                          return(newtoken(STRUCT));

union                           return(newtoken(UNION));

typedef                         return(newtoken(TYPEDEF));

else				return(newtoken(ELSE));

switch                          return(newtoken(SWITCH));

case                            return(newtoken(CASE));

do				return(newtoken(DO));

while				return(newtoken(WHILE));

break				return(newtoken(BREAK));

return				return(newtoken(RETURN));



auto				{ if (omp) return(newtoken(OMP_AUTO));                else return(newtoken(AUTO)); }

default                         { if (omp) return(newtoken(OMP_DEFAULT));             else return(newtoken(DEFAULT)); }

for				{ if (omp) return(newtoken(OMP_FOR));                 else return(newtoken(FOR)); }

if				{ if (omp) return(newtoken(OMP_IF));                  else return(newtoken(IF)); }

static				{ if (omp) return(newtoken(OMP_STATIC));              else return(newtoken(STATIC)); }



atomic				{ if (omp) return(newtoken(OMP_ATOMIC));              else return(mk_identifier()); }

barrier				{ if (omp) return(newtoken(OMP_BARRIER));             else return(mk_identifier()); }

capture				{ if (omp) return(newtoken(OMP_CAPTURE));             else return(mk_identifier()); }

collapse			{ if (omp) return(newtoken(OMP_COLLAPSE));            else return(mk_identifier()); }

copyin				{ if (omp) return(newtoken(OMP_COPYIN));              else return(mk_identifier()); }

copyprivate			{ if (omp) return(newtoken(OMP_COPYPRIVATE));         else return(mk_identifier()); }

critical			{ if (omp) return(newtoken(OMP_CRITICAL));            else return(mk_identifier()); }

dynamic				{ if (omp) return(newtoken(OMP_DYNAMIC));             else return(mk_identifier()); }

final				{ if (omp) return(newtoken(OMP_FINAL));               else return(mk_identifier()); }

firstprivate			{ if (omp) return(newtoken(OMP_FIRSTPRIVATE));        else return(mk_identifier()); }

flush				{ if (omp) return(newtoken(OMP_FLUSH));               else return(mk_identifier()); }

guided				{ if (omp) return(newtoken(OMP_GUIDED));              else return(mk_identifier()); }

lastprivate			{ if (omp) return(newtoken(OMP_LASTPRIVATE));         else return(mk_identifier()); }

master				{ if (omp) return(newtoken(OMP_MASTER));              else return(mk_identifier()); }

max				{ if (omp) return(newtoken(OMP_MAX));                 else return(mk_identifier()); }

mergable			{ if (omp) return(newtoken(OMP_MERGABLE));            else return(mk_identifier()); }

min				{ if (omp) return(newtoken(OMP_MIN));                 else return(mk_identifier()); }

none				{ if (omp) return(newtoken(OMP_NONE));                else return(mk_identifier()); }

nowait				{ if (omp) return(newtoken(OMP_NOWAIT));              else return(mk_identifier()); }

num_threads			{ if (omp) return(newtoken(OMP_NUM_THREADS));         else return(mk_identifier()); }

ordered				{ if (omp) return(newtoken(OMP_ORDERED));             else return(mk_identifier()); }

parallel			{ if (omp) return(newtoken(OMP_PARALLEL));            else return(mk_identifier()); }

parallel+[ \t]+for		{ if (omp) return(newtoken(OMP_PARALLEL_FOR));        else return(mk_identifier()); }

parallel+[ \t]+sections		{ if (omp) return(newtoken(OMP_PARALLEL_SECTIONS));   else return(mk_identifier()); }

private				{ if (omp) return(newtoken(OMP_PRIVATE));             else return(mk_identifier()); }

read				{ if (omp) return(newtoken(OMP_READ));                else return(mk_identifier()); }

reduction			{ if (omp) return(newtoken(OMP_REDUCTION));           else return(mk_identifier()); }

runtime				{ if (omp) return(newtoken(OMP_RUNTIME));             else return(mk_identifier()); }

schedule			{ if (omp) return(newtoken(OMP_SCHEDULE));            else return(mk_identifier()); }

section				{ if (omp) return(newtoken(OMP_SECTION));             else return(mk_identifier()); }

sections			{ if (omp) return(newtoken(OMP_SECTIONS));            else return(mk_identifier()); }

shared				{ if (omp) return(newtoken(OMP_SHARED));              else return(mk_identifier()); }

single				{ if (omp) return(newtoken(OMP_SINGLE));              else return(mk_identifier()); }

task				{ if (omp) return(newtoken(OMP_TASK));                else return(mk_identifier()); }

taskwait			{ if (omp) return(newtoken(OMP_TASKWAIT));            else return(mk_identifier()); }

taskyield			{ if (omp) return(newtoken(OMP_TASKYIELD));           else return(mk_identifier()); }

threadprivate			{ if (omp) return(newtoken(OMP_THREADPRIVATE));       else return(mk_identifier()); }

untied				{ if (omp) return(newtoken(OMP_UNTIED));              else return(mk_identifier()); }

update				{ if (omp) return(newtoken(OMP_UPDATE));              else return(mk_identifier()); }

write				{ if (omp) return(newtoken(OMP_WRITE));               else return(mk_identifier()); }

omp				{ if (pragma) { omp = 1; return(newtoken(OMP)); }     else return(mk_identifier()); }

clock				{ if (fpgac) return(newtoken(FPGAC_CLOCK));           else return(mk_identifier()); }

charbits			{ if (fpgac) return(newtoken(FPGAC_CHARBITS));        else return(mk_identifier()); }

intbits				{ if (fpgac) return(newtoken(FPGAC_INTBITS));         else return(mk_identifier()); }

shortbits			{ if (fpgac) return(newtoken(FPGAC_SHORTBITS));       else return(mk_identifier()); }

longbits			{ if (fpgac) return(newtoken(FPGAC_LONGBITS));        else return(mk_identifier()); }

longlongbits			{ if (fpgac) return(newtoken(FPGAC_LONGLONGBITS));    else return(mk_identifier()); }

floatbits			{ if (fpgac) return(newtoken(FPGAC_FLOATBITS));       else return(mk_identifier()); }

doublebits			{ if (fpgac) return(newtoken(FPGAC_DOUBLEBITS));      else return(mk_identifier()); }

longdoublebits			{ if (fpgac) return(newtoken(FPGAC_LONGDOUBLEBITS));  else return(mk_identifier()); }

fpgac				{ if (pragma) { fpgac = 1; return(newtoken(FPGAC)); } else return(mk_identifier()); }

^[ \t]+#+[ \t]+"pragma"		{ pragma=1; return(newtoken(PRAGMA)); }

^#[ \t]+[0-9]+[ \t]+"$pragma"	{
				    /* DEC and SGI cpps do this to #pragma */
				    sscanf(yytext, "# %d ", &inputlineno);
				    inputlineno -= 2;
				    pragma=1;
				    return(newtoken(PRAGMA));
				}

^#[ \t]*[0-9]+[ \t]+\"[^\"]*\".*\n	{
				    sscanf(yytext, "# %d \"%s\"", &inputlineno, inputfilename);
				    inputfilename[strlen(inputfilename)-1] = '\0';
				}

^#[ \t]*line[ \t]*[0-9]+[ \t]+\"[^\"]*\".*\n	{
				    /* IBM cpp does this */
				    sscanf(yytext, "# line %d \"%s\"", &inputlineno, inputfilename);
				    inputfilename[strlen(inputfilename)-1] = '\0';
				}

^#[ \t]*line[ \t]*[0-9]+\n	{
				    /* IBM cpp does this */
				    sscanf(yytext, "# line %d", &inputlineno);
				}

^#[ \t]+[^0-9].*$		/* Ignore all other cpp directives */;

^#[^p \t].*$			/* Ignore all other cpp directives */;

[A-Za-z_][A-Za-z0-9_]*		{
				    char *buf;

				    if((buf = (char *) malloc(strlen(yytext)+2)) == (char *) NULL) {
				        fprintf(stderr, "fpgac: malloc returned NULL\n");
				        exit(1);
				    }
				    sprintf(buf, "%s", yytext);
				    yylval.s = buf;
				    return(newtoken(IDENTIFIER));
				}

0|[1-9][0-9]*			{
				    char *buf;

				    if((buf = (char *) malloc(strlen(yytext)+1)) == (char *) NULL) {
				        fprintf(stderr, "fpgac: malloc returned NULL\n");
				        exit(1);
				    }
				    sprintf(buf, "%s", yytext);
				    yylval.s = buf;
				    return(newtoken(INTEGER));
				}

0[0-7]+				{
				    char *buf;
				    long long temp;

				    if((buf = (char *) malloc(strlen(yytext)*2+1)) == (char *) NULL) {
					fprintf(stderr, "fpgac: malloc returned NULL\n");
					exit(1);
				    }
				    sscanf(yytext, "%Li", &temp);
				    sprintf(buf, "%lld", temp);
				    yylval.s = buf;
				    return(newtoken(INTEGER));
				}

0[xX][0-9a-fA-F]+		{   // %Li is broken, does not convert long longs
				    char *buf;
				    long long temp;

				    if((buf = (char *) malloc(strlen(yytext)*2+1)) == (char *) NULL) {
					fprintf(stderr, "fpgac: malloc returned NULL\n");
					exit(1);
				    }
				    sscanf(yytext, "%Lx", &temp);
				    sprintf(buf, "%lld", temp);
				    yylval.s = buf;
				    return(newtoken(INTEGER));
				}

0[bB][0-1]+                     {
				    char *buf,*ebuf;
				    long long temp;

				    if((buf = (char *) malloc(strlen(yytext)*2+1)) == (char *) NULL) {
					fprintf(stderr, "fpgac: malloc returned NULL\n");
					exit(1);
				    }
				    temp = strtoll(yytext+2,&ebuf,2);
				    sprintf(buf, "%lld", temp);
				    yylval.s = buf;
				    return(newtoken(INTEGER));
				}

\"[^"]*\"			{
				    char *buf;

				    if((buf = (char *) malloc(strlen(yytext)+1)) == (char *) NULL) {
					fprintf(stderr, "fpgac: malloc returned NULL\n");
					exit(1);
				    }
				    /* Return the contents of the string, without
				     * the quotes
				     */
				    sprintf(buf, "%s", &yytext[1]);
				    buf[strlen(buf)-1] = '\0';
				    yylval.s = buf;
				    return(newtoken(STRING));
				}

[ \t]				; /* Skip white space */

\n				{if(omp){pragma=fpgac=omp=0; inputlineno++; return(newtoken(NEW_LINE));} else { pragma=fpgac=omp=0; inputlineno++;}}

.				return(newtoken(ILLEGAL));
%%

/*
 * Set lex to ignore tokens
 */
ignore_token(int ignore_type) {
        ignore_token_type = ignore_type ;
        ignore_tokens = 1;

        switch (ignore_type) {
        case IGNORE_FORLOOP: 
              number_br = 1;
              break;
        }
}
