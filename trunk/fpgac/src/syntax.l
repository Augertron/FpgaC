%{
/* syntax.l -- the scanner for fpgac
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

/* CHANGES:
 *
 * MTP Changed all malloc calls to calloc as per patch 'patch.malloc' fix
 *     submitted by John Bass
 *
 * MTP converted sprintf to snprintf
 *
 */

#include <stdio.h>
#include "names.h"
#include "y.tab.h"

extern YYSTYPE yylval;

int inputlineno = 1;

extern char inputfilename[];

static int saving, replaying;
static int replaystarting;

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

newtoken(t)
	int t;
	{
	struct token *token;
	extern int debug;

	if(debug == 1) {
		printf("newtoken %d '%s' saving %d replaying %d\n", t,
			yytext, saving, replaying);
		}
	if(saving) {
		if((token = (struct token *) calloc(1, sizeof(struct token)))
			== (struct token *) NULL) {
			fprintf(stderr, "fpgac: newtoken calloc failed\n");
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
	return(t);
	}



saveinput() {
	if(!tokenstack) {
		if((tokenstack = (struct tokenlist *) calloc(1, sizeof(struct tokenlist)))
				== (struct tokenlist *) NULL) {
			fprintf(stderr, "fpgac: saveinput calloc failed\n");
			exit(1);
			}
		}
	saving = 1;
	}



stopsavinginput() {
	saving = 0;
	}



replayinput() {
	replaystarting = 1;
	replaying = 1;
	}


pushinputstream() {
	struct tokenlist *tl;

	if((tl = (struct tokenlist *) calloc(1, sizeof(struct tokenlist)))
			== (struct tokenlist *) NULL) {
		fprintf(stderr, "fpgac: saveinput calloc failed\n");
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
		}
	else if(replaying) {
		replaying = 0;
		return(newtoken(REPLAYEND));
		}
%}

"("				return(newtoken(LEFTPAREN));

")"				return(newtoken(RIGHTPAREN));

"["				return(newtoken(LEFTBRACE));

"]"				return(newtoken(RIGHTBRACE));

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

"~"				return(newtoken(TILDE));

"--"				return(newtoken(MINUSMINUS));

"++"				return(newtoken(PLUSPLUS));

":"				return(newtoken(COLON));

";"				return(newtoken(SEMICOLON));

"="				return(newtoken(EQUAL));

"+="				return(newtoken(PLUSEQUAL));

"-="				return(newtoken(MINUSEQUAL));

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

unsigned			return(newtoken(UNSIGNED));

register			return(newtoken(REGISTER));

void				return(newtoken(VOID));

if				return(newtoken(IF));

else				return(newtoken(ELSE));

while				return(newtoken(WHILE));

break				return(newtoken(BREAK));

return				return(newtoken(RETURN));

^#[ \t]*pragma[ \t]*inputport	return(newtoken(INPUTPORT));

^#[ \t]*pragma[ \t]*outputport	return(newtoken(OUTPUTPORT));

^#[ \t]*pragma[ \t]*bus_port	return(newtoken(BUS_PORT));

^#[ \t]*pragma[ \t]*bus_idle	return(newtoken(BUS_IDLE));

^#[ \t]*pragma[ \t]*portflags	return(newtoken(PORTFLAGS));

^#[ \t]*pragma[ \t]*intbits	return(newtoken(INTBITS));

^#[ \t]+[0-9]+[ \t]+"$pragma"[ \t]+intbits	{
				/* DEC and SGI cpps do this to #pragma */
				sscanf(yytext, "# %d ", &inputlineno);
				inputlineno -= 2;
				return(newtoken(INTBITS));
				}

^#[ \t]*[0-9]+[ \t]+\"[^\"]*\".*\n	{
				sscanf(yytext, "# %d \"%s\"", &inputlineno,
						inputfilename);
				inputfilename[strlen(inputfilename)-1] = '\0';
				}

^#[ \t]*line[ \t]*[0-9]+[ \t]+\"[^\"]*\".*\n	{
				/* IBM cpp does this */
				sscanf(yytext, "# line %d \"%s\"", &inputlineno,
						inputfilename);
				inputfilename[strlen(inputfilename)-1] = '\0';
				}

^#[ \t]*line[ \t]*[0-9]+\n	{
				/* IBM cpp does this */
				sscanf(yytext, "# line %d", &inputlineno);
				}

^#[ \t]+[^0-9].*$		/* Ignore all other cpp directives */;

^#[^p \t].*$			/* Ignore all other cpp directives */;

[A-Za-z_][A-Za-z0-9_]*	{
				char *buf;

				if((buf = (char *) calloc(1, strlen(yytext)+2))
					== (char *) NULL) {
					fprintf(stderr, "fpgac: calloc returned NULL\n");
					exit(1);
					}
				snprintf(buf, strlen(yytext)+2, "_%s", yytext);
				yylval.s = buf;
				return(newtoken(IDENTIFIER));
				}

0|[1-9][0-9]*				{
				char *buf;

				if((buf = (char *) calloc(1, strlen(yytext)+1))
					== (char *) NULL) {
					fprintf(stderr, "fpgac: calloc returned NULL\n");
					exit(1);
					}
				snprintf(buf, strlen(yytext)+1, "%s", yytext);
				yylval.s = buf;
				return(newtoken(INTEGER));
				}

0[0-7]+|0[xX][0-9a-fA-F]+	{
				char *buf;
				int temp;

				if((buf = (char *) calloc(1, strlen(yytext)*2+1))
					== (char *) NULL) {
					fprintf(stderr, "fpgac: calloc returned NULL\n");
					exit(1);
					}
				sscanf(yytext, "%i", &temp);
				snprintf(buf, strlen(yytext)*2+1, "%d", temp);
				yylval.s = buf;
				return(newtoken(INTEGER));
				}

0[bB][0-1]+                     {
				char *buf,*ebuf;
				int temp;

				if((buf = (char *) calloc(1, strlen(yytext)*2+1))
					== (char *) NULL) {
					fprintf(stderr, "fpgac: calloc returned NULL\n");
					exit(1);
					}
				temp = strtol(yytext+2,&ebuf,2);
				snprintf(buf, strlen(yytext)*2+1,"%d", temp);
				yylval.s = buf;
				return(newtoken(INTEGER));
				}

\"[^"]*\"			{
				char *buf;

				if((buf = (char *) calloc(1, strlen(yytext)+1))
					== (char *) NULL) {
					fprintf(stderr, "fpgac: calloc returned NULL\n");
					exit(1);
					}
				/* Return the contents of the string, without
				 * the quotes
				 */
				snprintf(buf, strlen(yytext)+1, "%s", &yytext[1]);
				buf[strlen(buf)-1] = '\0';
				yylval.s = buf;
				return(newtoken(STRING));
				}

[ \t]				; /* Skip white space */

\n				{ inputlineno++; }

.				return(newtoken(ILLEGAL));
%%
