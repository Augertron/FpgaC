/*
 * BasicMath.c - FpgaC intrinsic math functions
 * copyright 2006 by John Bass, DMS Design under FpgaC BSD License
 *
 * Basic long hand math functions.
 * These are large, and depending on the target and compiler options,
 * may produce a long combinatorial chain. No attempt is made to
 * check for exceptions. Depending on your application, you may want
 * to modify these examples to check and flag obvious problems.
 *
 * re-writing these functions to loop will save considerable space,
 * with the tradeoff of one additional clock per loop execution.
 *
 * include these directly in your application, and adjust types as needed.
 */

long fpgac_multiply(long a, long b) {
    long c;

    if(b & 1)
        c  = a;
    else
        c = 0;

    if(b & (1 <<  1)) c += a <<  1;
    if(b & (1 <<  2)) c += a <<  2;
    if(b & (1 <<  3)) c += a <<  3;
    if(b & (1 <<  4)) c += a <<  4;
    if(b & (1 <<  5)) c += a <<  5;
    if(b & (1 <<  6)) c += a <<  6;
    if(b & (1 <<  7)) c += a <<  7;
    if(b & (1 <<  8)) c += a <<  8;
    if(b & (1 <<  9)) c += a <<  9;
    if(b & (1 << 10)) c += a << 10;
    if(b & (1 << 11)) c += a << 11;
    if(b & (1 << 12)) c += a << 12;
    if(b & (1 << 13)) c += a << 13;
    if(b & (1 << 14)) c += a << 14;
    if(b & (1 << 15)) c += a << 15;
    if(b & (1 << 16)) c += a << 16;
    if(b & (1 << 17)) c += a << 17;
    if(b & (1 << 18)) c += a << 18;
    if(b & (1 << 19)) c += a << 19;
    if(b & (1 << 20)) c += a << 20;
    if(b & (1 << 21)) c += a << 21;
    if(b & (1 << 22)) c += a << 22;
    if(b & (1 << 23)) c += a << 23;
    if(b & (1 << 24)) c += a << 24;
    if(b & (1 << 25)) c += a << 25;
    if(b & (1 << 26)) c += a << 26;
    if(b & (1 << 27)) c += a << 27;
    if(b & (1 << 28)) c += a << 28;
    if(b & (1 << 29)) c += a << 29;
    if(b & (1 << 30)) c += a << 30;
    if(b & (1 << 31)) c += a << 31;

    return(c);
}

// cache last operation, and return if a hit
long fpgac_div_a, fpgac_div_b, fpgac_div_rem, fpgac_div_quot;

long fpgac_divide(long a, long b) {
    long c, temp;

    if(a != fpgac_div_a || b != fpgac_div_b) {
        fpgac_div_a = a;
        fpgac_div_b = b;
        fpgac_div_quot = 0;
        if((temp=(a-(b<<31))) >= 0) {a=temp; fpgac_div_quot |= (1<<31);}
        if((temp=(a-(b<<30))) >= 0) {a=temp; fpgac_div_quot |= (1<<30);}
        if((temp=(a-(b<<29))) >= 0) {a=temp; fpgac_div_quot |= (1<<29);}
        if((temp=(a-(b<<28))) >= 0) {a=temp; fpgac_div_quot |= (1<<28);}
        if((temp=(a-(b<<27))) >= 0) {a=temp; fpgac_div_quot |= (1<<27);}
        if((temp=(a-(b<<26))) >= 0) {a=temp; fpgac_div_quot |= (1<<26);}
        if((temp=(a-(b<<25))) >= 0) {a=temp; fpgac_div_quot |= (1<<25);}
        if((temp=(a-(b<<24))) >= 0) {a=temp; fpgac_div_quot |= (1<<24);}
        if((temp=(a-(b<<23))) >= 0) {a=temp; fpgac_div_quot |= (1<<23);}
        if((temp=(a-(b<<22))) >= 0) {a=temp; fpgac_div_quot |= (1<<22);}
        if((temp=(a-(b<<21))) >= 0) {a=temp; fpgac_div_quot |= (1<<21);}
        if((temp=(a-(b<<20))) >= 0) {a=temp; fpgac_div_quot |= (1<<20);}
        if((temp=(a-(b<<19))) >= 0) {a=temp; fpgac_div_quot |= (1<<19);}
        if((temp=(a-(b<<18))) >= 0) {a=temp; fpgac_div_quot |= (1<<18);}
        if((temp=(a-(b<<17))) >= 0) {a=temp; fpgac_div_quot |= (1<<17);}
        if((temp=(a-(b<<16))) >= 0) {a=temp; fpgac_div_quot |= (1<<16);}
        if((temp=(a-(b<<15))) >= 0) {a=temp; fpgac_div_quot |= (1<<15);}
        if((temp=(a-(b<<14))) >= 0) {a=temp; fpgac_div_quot |= (1<<14);}
        if((temp=(a-(b<<13))) >= 0) {a=temp; fpgac_div_quot |= (1<<13);}
        if((temp=(a-(b<<12))) >= 0) {a=temp; fpgac_div_quot |= (1<<12);}
        if((temp=(a-(b<<11))) >= 0) {a=temp; fpgac_div_quot |= (1<<11);}
        if((temp=(a-(b<<10))) >= 0) {a=temp; fpgac_div_quot |= (1<<10);}
        if((temp=(a-(b<< 9))) >= 0) {a=temp; fpgac_div_quot |= (1<< 9);}
        if((temp=(a-(b<< 8))) >= 0) {a=temp; fpgac_div_quot |= (1<< 8);}
        if((temp=(a-(b<< 7))) >= 0) {a=temp; fpgac_div_quot |= (1<< 7);}
        if((temp=(a-(b<< 6))) >= 0) {a=temp; fpgac_div_quot |= (1<< 6);}
        if((temp=(a-(b<< 5))) >= 0) {a=temp; fpgac_div_quot |= (1<< 5);}
        if((temp=(a-(b<< 4))) >= 0) {a=temp; fpgac_div_quot |= (1<< 4);}
        if((temp=(a-(b<< 3))) >= 0) {a=temp; fpgac_div_quot |= (1<< 3);}
        if((temp=(a-(b<< 2))) >= 0) {a=temp; fpgac_div_quot |= (1<< 2);}
        if((temp=(a-(b<< 1))) >= 0) {a=temp; fpgac_div_quot |= (1<< 1);}
        if((temp=(a-(b<< 0))) >= 0) {a=temp; fpgac_div_quot |= (1<< 0);}
        fpgac_div_rem = a;
    } else {
        fpgac_div_a = a;
        fpgac_div_b = b;
    }
    return(fpgac_div_quot);
}

long fpgac_remainder(long a, long b) {
    long c;

    if(a != fpgac_div_a || b != fpgac_div_b) {
        fpgac_divide(a, b);
    }
    return(fpgac_div_rem);
    
}
