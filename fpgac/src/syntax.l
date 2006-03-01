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
        printf("newtoken %d '%s' saving %d replaying %d ignore %d\n", t, yytext, saving, replaying ,ignore_tokens );
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

fpgac_input			return(newtoken(INPUT));

fpgac_output			return(newtoken(OUTPUT));

fpgac_tristate			return(newtoken(TRISTATE));

fpgac_mailbox			return(newtoken(MAILBOX));

fpgac_process			return(newtoken(PROCESS));

void				return(newtoken(VOID));

enum                            return(newtoken(ENUM));

struct                          return(newtoken(STRUCT));

union                           return(newtoken(UNION));

typedef                         return(newtoken(TYPEDEF));

if				return(newtoken(IF));

else				return(newtoken(ELSE));

do				return(newtoken(DO));

while				return(newtoken(WHILE));

for				return(newtoken(FOR));

break				return(newtoken(BREAK));

return				return(newtoken(RETURN));

^#[ \t]*pragma[ \t]*fpgac_clock		return(newtoken(CLOCK));

^#[ \t]*pragma[ \t]*fpgac_inputport	return(newtoken(INPUTPORT));

^#[ \t]*pragma[ \t]*fpgac_outputport	return(newtoken(OUTPUTPORT));

^#[ \t]*pragma[ \t]*fpgac_bus_port	return(newtoken(BUS_PORT));

^#[ \t]*pragma[ \t]*fpgac_bus_idle	return(newtoken(BUS_IDLE));

^#[ \t]*pragma[ \t]*fpgac_portflags	return(newtoken(PORTFLAGS));

^#[ \t]*pragma[ \t]*fpgac_intbits	return(newtoken(INTBITS));

^#[ \t]*pragma[ \t]*fpgac_shortbits	return(newtoken(SHORTBITS));

^#[ \t]*pragma[ \t]*fpgac_longbits	return(newtoken(LONGBITS));

^#[ \t]*pragma[ \t]*fpgac_longlongbits	return(newtoken(LONGLONGBITS));

^#[ \t]*pragma[ \t]*fpgac_floatbits	return(newtoken(FLOATBITS));

^#[ \t]*pragma[ \t]*fpgac_doublebits	return(newtoken(DOUBLEBITS));

^#[ \t]*pragma[ \t]*fpgac_longdoublebits	return(newtoken(LONGDOUBLEBITS));

^#[ \t]+[0-9]+[ \t]+"$pragma"[ \t]+fpgac_intbits	{
				    /* DEC and SGI cpps do this to #pragma */
				    sscanf(yytext, "# %d ", &inputlineno);
				    inputlineno -= 2;
				    return(newtoken(INTBITS));
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
				    sprintf(buf, "_%s", yytext);
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

0[0-7]+|0[xX][0-9a-fA-F]+	{
				    char *buf;
				    int temp;

				    if((buf = (char *) malloc(strlen(yytext)*2+1)) == (char *) NULL) {
					fprintf(stderr, "fpgac: malloc returned NULL\n");
					exit(1);
				    }
				    sscanf(yytext, "%i", &temp);
				    sprintf(buf, "%d", temp);
				    yylval.s = buf;
				    return(newtoken(INTEGER));
				}

0[bB][0-1]+                     {
				    char *buf,*ebuf;
				    int temp;

				    if((buf = (char *) malloc(strlen(yytext)*2+1)) == (char *) NULL) {
					fprintf(stderr, "fpgac: malloc returned NULL\n");
					exit(1);
				    }
				    temp = strtol(yytext+2,&ebuf,2);
				    sprintf(buf, "%d", temp);
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

\n				{ inputlineno++; }

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
