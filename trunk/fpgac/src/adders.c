/* adders.c -- Create adders, subtractors and counters for fpgac programs.
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

#include "names.h"

extern use_carry_select_adders;

static struct variable *addsub(), *ripple_add(), *ripple_sub(), *bottombits();

struct variable *
add(left, right)
	struct variable *left, *right;
	{
	return(addsub(left, right, ripple_add));
	}


struct variable *
sub(left, right)
	struct variable *left, *right;
	{
	return(addsub(left, right, ripple_sub));
	}



static struct variable *
addsub(left, right, func)
	struct variable *left, *right;
	struct variable *(*func)();
	{
	int width, i;
	struct variable *bottom, *topleft, *topright, *top, *top_plus1;
	struct variable *result, *temp1, *temp2, *temp3;
	struct bitlist *rbl, *t1bl, *t2bl, *t3bl, *bbl, *tbl, *tp1bl;
	struct bit *bottom_carryout;

	left = thistick(left);
	right = thistick(right);
	width =  MAX(left->width, right->width);

	if((width < 4) || !use_carry_select_adders)
		return((*func)(left, right, 0, (struct bit *) 0));

	/* Otherwise, build a carry select adder.  This is an adder that
	 * works almost twice the speed of a simple ripple carry adder, but
	 * is about 60% larger.  It works by splitting a normal ripple
	 * carry adder in half.  The bottom bits of the result are calculated
	 * with a ripple carry.  At the same time, two versions of the top
	 * bits are calculated.  One version assumes that the bottom will
	 * not produce a carry out, the other assumes that it will.
	 * Once the carry out from the bottom is available, it is used to
	 * select which version of the top gets used.
	 */

	temp1 = bottombits(left, width/2);
	temp2 = bottombits(right, width/2);
	bottom = (*func)(temp1, temp2, 0, &bottom_carryout);
	topleft = shift(left, width/2);
	topright = shift(right, width/2);
	top = (*func)(topleft, topright, 0, (struct bit *) 0);
	top_plus1 = (*func)(topleft, topright, 1, (struct bit *) 0);

	result = newtempvar("csadd", width);
	rbl = result->bits;
	bbl = bottom->bits;

	for(i=0; i<width/2; i++) {
		setbit(rbl->bit, bbl->bit);
		rbl = rbl->next;
		bbl = bbl->next;
		}

	temp1 = newtempvar("csaddt1", width - width/2);
	temp2 = newtempvar("csaddt2", width - width/2);
	temp3 = newtempvar("csaddt3", width - width/2);
	t1bl = temp1->bits;
	t2bl = temp2->bits;
	t3bl = temp3->bits;
	tbl = top->bits;
	tp1bl = top_plus1->bits;

	for(; rbl; rbl = rbl->next) {
		muxbit(rbl->bit, bottom_carryout, tp1bl->bit, tbl->bit,
				t1bl->bit, t2bl->bit, t3bl->bit);
		tp1bl = tp1bl->next;
		tbl = tbl->next;
		t1bl = t1bl->next;
		t2bl = t2bl->next;
		t3bl = t3bl->next;
		}
	modifiedvar(result);
	return(result);
	}



static struct variable *
ripple_add(left, right, cin, coutp)
	struct variable *left, *right;
	int cin;
	struct bit **coutp;
	{
	struct variable *carry, *result, *propagate, *generate, *temp3;
	struct variable *carryin;
	struct bitlist *resultb, *carryb;
	struct bitlist *propagateb, *generateb, *temp3b;

	/* Ripple carry addition */

	result = newtempvar("add", MAX(left->width, right->width));
	carry = newtempvar("car", MAX(left->width, right->width) + 1);
	propagate = twoop(left, right, xor);
	generate = twoop(left, right, and);
	temp3 = newtempvar("addt3", MAX(left->width, right->width));
	carryin = intconstant(cin);

	resultb = result->bits;
	carryb = carry->bits;
	setbit(carryb->bit, carryin->bits->bit);
	propagateb = propagate->bits;
	generateb = generate->bits;
	temp3b = temp3->bits;
	for(;;) {
		twoop1bit(resultb->bit, propagateb->bit, carryb->bit, xor);
		twoop1bit(temp3b->bit, propagateb->bit, carryb->bit, and);
		carryb = carryb->next;
		twoop1bit(carryb->bit, temp3b->bit, generateb->bit, or);
		resultb = resultb->next;
		if(!resultb)
			break;
		propagateb = propagateb->next;
		generateb = generateb->next;
		temp3b = temp3b->next;
		}
	if(coutp)
		*coutp = carryb->bit;
	modifiedvar(result);
	return(result);
	}



static struct variable *
bottombits(v, nbits)
	struct variable *v;
	int nbits;
	{
	struct variable *result;
	struct bitlist *vbl, *rbl;

	vbl = v->bits;
	result = newtempvar("bot", nbits);
	
	for(rbl = result->bits; rbl; rbl = rbl->next) {
		setbit(rbl->bit, vbl->bit);
		if(vbl->next)
			vbl = vbl->next;
		}
	return(result);
	}



static struct variable *
ripple_sub(left, right, cin, coutp)
	struct variable *left, *right;
	int cin;
	struct bit **coutp;
	{
	struct variable *carry, *result, *propagate, *generate, *temp3,
			*propbar, *carryin;
	struct bitlist *resultb, *carryb;
	struct bitlist *propagateb, *generateb, *temp3b, *propbarb;

	/* Ripple carry subtraction */

	result = newtempvar("sub", MAX(left->width, right->width));
	carry = newtempvar("car", MAX(left->width, right->width) + 1);
	propagate = twoop(left, right, xor);
	propbar = complement(propagate);
	generate = twoop(complement(left), right, and);
	temp3 = newtempvar("addt3", MAX(left->width, right->width));
	carryin = intconstant(cin);

	resultb = result->bits;
	setbit(resultb->bit, propagate->bits->bit);
	carryb = carry->bits;
	setbit(carryb->bit, carryin->bits->bit);
	propagateb = propagate->bits;
	propbarb = propbar->bits;
	generateb = generate->bits;
	temp3b = temp3->bits;
	for(;;) {
		twoop1bit(resultb->bit, propagateb->bit, carryb->bit, xor);
		twoop1bit(temp3b->bit, propbarb->bit, carryb->bit, and);
		carryb = carryb->next;
		twoop1bit(carryb->bit, temp3b->bit, generateb->bit, or);
		resultb = resultb->next;
		if(!resultb)
			break;
		propagateb = propagateb->next;
		propbarb = propbarb->next;
		generateb = generateb->next;
		temp3b = temp3b->next;
		}
	if(coutp)
		*coutp = carryb->bit;
	modifiedvar(result);
	return(result);
	}
