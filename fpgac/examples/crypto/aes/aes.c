/*
 * Proof of Concept FpgaC AES implimentation
 * copyright 2006 by John Bass, DMS Design under FpgaC BSD License
 *
 * Note:
 *        1) Coded with lines as long as 140 chars.
 *        2) Sbox is stubbed out for FpgaC
 *          See http://groups.google.com/group/comp.arch.fpga/browse_frm/thread/b81fe0af9185e61f/3c0ccb22560f35cc?hl=en#3c0ccb22560f35cc
 *
 * The AES implmentation reference is:
 *       Federal Information Processing Standards Publication 197
 *
 */

#define PIPELINED

#ifdef FPGAC
#pragma fpgac_clock(aes_clk)
#pragma fpgac_intbits 8

/*
 * Sbox 
 * derived from work done by David Canright's
 * compact implementation of AES S-box via subfield operations
 *   case # 4 : [d^16, d], [alpha^8, alpha^2], [Omega^2, Omega]
 *   nu = beta^8 = N^2*alpha^2, N = w^2

 */
#include "srom.c"
//#define Sbox(a) (~a)

#else

#include <stdio.h>

typedef unsigned char fpgac_input;
typedef unsigned char fpgac_output;
typedef void fpgac_process;

/*
 * Sbox table for test harness
 */
#include "srom.c"
//#define Sbox(a) (S[a])

char S[256] = {
 99, 124, 119, 123, 242, 107, 111, 197,  48,   1, 103,  43, 254, 215, 171, 118,
202, 130, 201, 125, 250,  89,  71, 240, 173, 212, 162, 175, 156, 164, 114, 192,
183, 253, 147,  38,  54,  63, 247, 204,  52, 165, 229, 241, 113, 216,  49,  21,
  4, 199,  35, 195,  24, 150,   5, 154,   7,  18, 128, 226, 235,  39, 178, 117,
  9, 131,  44,  26,  27, 110,  90, 160,  82,  59, 214, 179,  41, 227,  47, 132,
 83, 209,   0, 237,  32, 252, 177,  91, 106, 203, 190,  57,  74,  76,  88, 207,
208, 239, 170, 251,  67,  77,  51, 133,  69, 249,   2, 127,  80,  60, 159, 168,
 81, 163,  64, 143, 146, 157,  56, 245, 188, 182, 218,  33,  16, 255, 243, 210,
205,  12,  19, 236,  95, 151,  68,  23, 196, 167, 126,  61, 100,  93,  25, 115,
 96, 129,  79, 220,  34,  42, 144, 136,  70, 238, 184,  20, 222,  94,  11, 219,
224,  50,  58,  10,  73,   6,  36,  92, 194, 211, 172,  98, 145, 149, 228, 121,
231, 200,  55, 109, 141, 213,  78, 169, 108,  86, 244, 234, 101, 122, 174,   8,
186, 120,  37,  46,  28, 166, 180, 198, 232, 221, 116,  31,  75, 189, 139, 138,
112,  62, 181, 102,  72,   3, 246,  14,  97,  53,  87, 185, 134, 193,  29, 158,
225, 248, 152,  17, 105, 217, 142, 148, 155,  30, 135, 233, 206,  85,  40, 223,
140, 161, 137,  13, 191, 230,  66, 104,  65, 153,  45,  15, 176,  84, 187,  22,
};
#endif


/*
 * MixColumn function. KeyAddition and ShiftRow implict in paramenters
 * and Substitution buried inside with the Sbox macro reference
 */
#define Mix(r0, i0, r1, i1, r2, i2, r3, i3) { \
    char a0, a1, a2, a3; \
    char b0, b1, b2, b3; \
    a0 = Sbox(i0); a1 = Sbox(i1); a2 = Sbox(i2); a3 = Sbox(i3); \
    b0 = (a0 << 1) & 0xff; if(a0 & 0x80) b0 ^= 0x1b; \
    b1 = (a1 << 1) & 0xff; if(a1 & 0x80) b1 ^= 0x1b; \
    b2 = (a2 << 1) & 0xff; if(a2 & 0x80) b2 ^= 0x1b; \
    b3 = (a3 << 1) & 0xff; if(a3 & 0x80) b3 ^= 0x1b; \
    r0 = (b0 ^ a3 ^ a2 ^ b1 ^ a1) & 0xff; \
    r1 = (b1 ^ a0 ^ a3 ^ b2 ^ a2) & 0xff; \
    r2 = (b2 ^ a1 ^ a0 ^ b3 ^ a3) & 0xff; \
    r3 = (b3 ^ a2 ^ a1 ^ b0 ^ a0) & 0xff; \
}

/*
 * Set function. KeyAddition and ShiftRow implict in paramenters
 * and Substitution buried inside with the Sbox macro reference
 */
#define Set(r0, i0, r1, i1, r2, i2, r3, i3) { \
    r0 = Sbox(i0); r1 = Sbox(i1); r2 = Sbox(i2); r3 = Sbox(i3); \
}

/*
 * Add function. KeyAddition and ShiftRow implict in paramenters
 * so it reduces to a simple assignment statement.
 */
#define Add(r0, i0, r1, i1, r2, i2, r3, i3) { \
    r0 = i0; r1 = i1; r2 = i2; r3 = i3; \
}

/*
 * declarations for round data
 */

fpgac_input   a000, a001, a002, a003, a010, a011, a012, a013, a020, a021, a022, a023, a030, a031, a032, a033;
unsigned char a100, a101, a102, a103, a110, a111, a112, a113, a120, a121, a122, a123, a130, a131, a132, a133;
unsigned char a200, a201, a202, a203, a210, a211, a212, a213, a220, a221, a222, a223, a230, a231, a232, a233;
unsigned char a300, a301, a302, a303, a310, a311, a312, a313, a320, a321, a322, a323, a330, a331, a332, a333;
unsigned char a400, a401, a402, a403, a410, a411, a412, a413, a420, a421, a422, a423, a430, a431, a432, a433;
unsigned char a500, a501, a502, a503, a510, a511, a512, a513, a520, a521, a522, a523, a530, a531, a532, a533;
unsigned char a600, a601, a602, a603, a610, a611, a612, a613, a620, a621, a622, a623, a630, a631, a632, a633;
unsigned char a700, a701, a702, a703, a710, a711, a712, a713, a720, a721, a722, a723, a730, a731, a732, a733;
unsigned char a800, a801, a802, a803, a810, a811, a812, a813, a820, a821, a822, a823, a830, a831, a832, a833;
unsigned char a900, a901, a902, a903, a910, a911, a912, a913, a920, a921, a922, a923, a930, a931, a932, a933;
unsigned char a1000, a1001, a1002, a1003, a1010, a1011, a1012, a1013, a1020, a1021, a1022, a1023, a1030, a1031, a1032, a1033;
fpgac_output  a1100, a1101, a1102, a1103, a1110, a1111, a1112, a1113, a1120, a1121, a1122, a1123, a1130, a1131, a1132, a1133;

/*
 * declarations for round keys
 */

fpgac_input rk000, rk001, rk002, rk003, rk010, rk011, rk012, rk013, rk020, rk021, rk022, rk023, rk030, rk031, rk032, rk033;
unsigned char rk100, rk101, rk102, rk103, rk110, rk111, rk112, rk113, rk120, rk121, rk122, rk123, rk130, rk131, rk132, rk133;
unsigned char rk200, rk201, rk202, rk203, rk210, rk211, rk212, rk213, rk220, rk221, rk222, rk223, rk230, rk231, rk232, rk233;
unsigned char rk300, rk301, rk302, rk303, rk310, rk311, rk312, rk313, rk320, rk321, rk322, rk323, rk330, rk331, rk332, rk333;
unsigned char rk400, rk401, rk402, rk403, rk410, rk411, rk412, rk413, rk420, rk421, rk422, rk423, rk430, rk431, rk432, rk433;
unsigned char rk500, rk501, rk502, rk503, rk510, rk511, rk512, rk513, rk520, rk521, rk522, rk523, rk530, rk531, rk532, rk533;
unsigned char rk600, rk601, rk602, rk603, rk610, rk611, rk612, rk613, rk620, rk621, rk622, rk623, rk630, rk631, rk632, rk633;
unsigned char rk700, rk701, rk702, rk703, rk710, rk711, rk712, rk713, rk720, rk721, rk722, rk723, rk730, rk731, rk732, rk733;
unsigned char rk800, rk801, rk802, rk803, rk810, rk811, rk812, rk813, rk820, rk821, rk822, rk823, rk830, rk831, rk832, rk833;
unsigned char rk900, rk901, rk902, rk903, rk910, rk911, rk912, rk913, rk920, rk921, rk922, rk923, rk930, rk931, rk932, rk933;
unsigned char rk1000, rk1001, rk1002, rk1003, rk1010, rk1011, rk1012, rk1013, rk1020, rk1021, rk1022, rk1023, rk1030, rk1031, rk1032, rk1033;

/*
 * process for AES algorithm - fully unrolled loops
 *
 * Using an array in FpgaC is a sequential memory resource,
 * So this implementation enumerates all the array elements
 * for parallelism as individual char variables.
 *
 * The macros Mix, Set, and Add are arranged as dest, src pairs
 * in 4 line sets by column to represent the 4x4 arrays.
 *
 * The KeyAddition function is expanded directly into the src term for each 
 * of the macros in the form a ^ rk.
 * 
 * The Substitution a = Sbox[a] lookup is a macro reference by Mix and Set
 * 
 * ShiftRow is implemented with parameter ordering in Mix and Set and does
 * not * require any explict operations. 
 * 
 * Pipelining by round simply requires inverting the four line statement
 * groups pairs as blocks, as no retiming is necessary. 
 *
 * The round key is expanded concurrently here. Production use probably
 * would require it separate, or pipelined in previous stage.
 */

fpgac_process aes_encrypt() {

#ifdef PIPELINED

   // Round 11

   rk1003 = rk903 ^ (rk1002 = rk902 ^ (rk1001 = rk901 ^ (rk1000 = rk900 ^ Sbox(rk913) ^ 0x36)));
   rk1013 = rk913 ^ (rk1012 = rk912 ^ (rk1011 = rk911 ^ (rk1010 = rk910 ^ Sbox(rk923))));
   rk1023 = rk923 ^ (rk1022 = rk922 ^ (rk1021 = rk921 ^ (rk1020 = rk920 ^ Sbox(rk933))));
   rk1033 = rk933 ^ (rk1032 = rk932 ^ (rk1031 = rk931 ^ (rk1030 = rk930 ^ Sbox(rk903))));

   Add(a1100, (a1000 ^ rk1000), a1110, (a1010 ^ rk1010), a1120, (a1020 ^ rk1020), a1130, (a1030 ^ rk1030));
   Add(a1101, (a1001 ^ rk1001), a1111, (a1011 ^ rk1011), a1121, (a1021 ^ rk1021), a1131, (a1031 ^ rk1031));
   Add(a1102, (a1002 ^ rk1002), a1112, (a1012 ^ rk1012), a1122, (a1022 ^ rk1022), a1132, (a1032 ^ rk1032));
   Add(a1103, (a1003 ^ rk1003), a1113, (a1013 ^ rk1013), a1123, (a1023 ^ rk1023), a1133, (a1033 ^ rk1033));

   // Round 10

   rk903 = rk803 ^ (rk902 = rk802 ^ (rk901 = rk801 ^ (rk900 = rk800 ^ Sbox(rk813) ^ 0x1b)));
   rk913 = rk813 ^ (rk912 = rk812 ^ (rk911 = rk811 ^ (rk910 = rk810 ^ Sbox(rk823))));
   rk923 = rk823 ^ (rk922 = rk822 ^ (rk921 = rk821 ^ (rk920 = rk820 ^ Sbox(rk833))));
   rk933 = rk833 ^ (rk932 = rk832 ^ (rk931 = rk831 ^ (rk930 = rk830 ^ Sbox(rk803))));

   Set(a1000, (a900 ^ rk900), a1010, (a911 ^ rk911), a1020, (a922 ^ rk922), a1030, (a933 ^ rk933));
   Set(a1001, (a901 ^ rk901), a1011, (a912 ^ rk912), a1021, (a923 ^ rk923), a1031, (a930 ^ rk930));
   Set(a1002, (a902 ^ rk902), a1012, (a913 ^ rk913), a1022, (a920 ^ rk920), a1032, (a931 ^ rk931));
   Set(a1003, (a903 ^ rk903), a1013, (a910 ^ rk910), a1023, (a921 ^ rk921), a1033, (a932 ^ rk932));

   // Round 9

   rk803 = rk703 ^ (rk802 = rk702 ^ (rk801 = rk701 ^ (rk800 = rk700 ^ Sbox(rk713) ^ 0x80)));
   rk813 = rk713 ^ (rk812 = rk712 ^ (rk811 = rk711 ^ (rk810 = rk710 ^ Sbox(rk723))));
   rk823 = rk723 ^ (rk822 = rk722 ^ (rk821 = rk721 ^ (rk820 = rk720 ^ Sbox(rk733))));
   rk833 = rk733 ^ (rk832 = rk732 ^ (rk831 = rk731 ^ (rk830 = rk730 ^ Sbox(rk703))));

   Mix(a900, (a800 ^ rk800), a910, (a811 ^ rk811), a920, (a822 ^ rk822), a930, (a833 ^ rk833));
   Mix(a901, (a801 ^ rk801), a911, (a812 ^ rk812), a921, (a823 ^ rk823), a931, (a830 ^ rk830));
   Mix(a902, (a802 ^ rk802), a912, (a813 ^ rk813), a922, (a820 ^ rk820), a932, (a831 ^ rk831));
   Mix(a903, (a803 ^ rk803), a913, (a810 ^ rk810), a923, (a821 ^ rk821), a933, (a832 ^ rk832));

   // Round 8

   rk703 = rk603 ^ (rk702 = rk602 ^ (rk701 = rk601 ^ (rk700 = rk600 ^ Sbox(rk613) ^ 0x40)));
   rk713 = rk613 ^ (rk712 = rk612 ^ (rk711 = rk611 ^ (rk710 = rk610 ^ Sbox(rk623))));
   rk723 = rk623 ^ (rk722 = rk622 ^ (rk721 = rk621 ^ (rk720 = rk620 ^ Sbox(rk633))));
   rk733 = rk633 ^ (rk732 = rk632 ^ (rk731 = rk631 ^ (rk730 = rk630 ^ Sbox(rk603))));

   Mix(a800, (a700 ^ rk700), a810, (a711 ^ rk711), a820, (a722 ^ rk722), a830, (a733 ^ rk733));
   Mix(a801, (a701 ^ rk701), a811, (a712 ^ rk712), a821, (a723 ^ rk723), a831, (a730 ^ rk730));
   Mix(a802, (a702 ^ rk702), a812, (a713 ^ rk713), a822, (a720 ^ rk720), a832, (a731 ^ rk731));
   Mix(a803, (a703 ^ rk703), a813, (a710 ^ rk710), a823, (a721 ^ rk721), a833, (a732 ^ rk732));

   // Round 7

   rk603 = rk503 ^ (rk602 = rk502 ^ (rk601 = rk501 ^ (rk600 = rk500 ^ Sbox(rk513) ^ 0x20)));
   rk613 = rk513 ^ (rk612 = rk512 ^ (rk611 = rk511 ^ (rk610 = rk510 ^ Sbox(rk523))));
   rk623 = rk523 ^ (rk622 = rk522 ^ (rk621 = rk521 ^ (rk620 = rk520 ^ Sbox(rk533))));
   rk633 = rk533 ^ (rk632 = rk532 ^ (rk631 = rk531 ^ (rk630 = rk530 ^ Sbox(rk503))));

   Mix(a700, (a600 ^ rk600), a710, (a611 ^ rk611), a720, (a622 ^ rk622), a730, (a633 ^ rk633));
   Mix(a701, (a601 ^ rk601), a711, (a612 ^ rk612), a721, (a623 ^ rk623), a731, (a630 ^ rk630));
   Mix(a702, (a602 ^ rk602), a712, (a613 ^ rk613), a722, (a620 ^ rk620), a732, (a631 ^ rk631));
   Mix(a703, (a603 ^ rk603), a713, (a610 ^ rk610), a723, (a621 ^ rk621), a733, (a632 ^ rk632));

   // Round 6

   rk503 = rk403 ^ (rk502 = rk402 ^ (rk501 = rk401 ^ (rk500 = rk400 ^ Sbox(rk413) ^ 0x10)));
   rk513 = rk413 ^ (rk512 = rk412 ^ (rk511 = rk411 ^ (rk510 = rk410 ^ Sbox(rk423))));
   rk523 = rk423 ^ (rk522 = rk422 ^ (rk521 = rk421 ^ (rk520 = rk420 ^ Sbox(rk433))));
   rk533 = rk433 ^ (rk532 = rk432 ^ (rk531 = rk431 ^ (rk530 = rk430 ^ Sbox(rk403))));

   Mix(a600, (a500 ^ rk500), a610, (a511 ^ rk511), a620, (a522 ^ rk522), a630, (a533 ^ rk533));
   Mix(a601, (a501 ^ rk501), a611, (a512 ^ rk512), a621, (a523 ^ rk523), a631, (a530 ^ rk530));
   Mix(a602, (a502 ^ rk502), a612, (a513 ^ rk513), a622, (a520 ^ rk520), a632, (a531 ^ rk531));
   Mix(a603, (a503 ^ rk503), a613, (a510 ^ rk510), a623, (a521 ^ rk521), a633, (a532 ^ rk532));

   // Round 5

   rk403 = rk303 ^ (rk402 = rk302 ^ (rk401 = rk301 ^ (rk400 = rk300 ^ Sbox(rk313) ^ 0x08)));
   rk413 = rk313 ^ (rk412 = rk312 ^ (rk411 = rk311 ^ (rk410 = rk310 ^ Sbox(rk323))));
   rk423 = rk323 ^ (rk422 = rk322 ^ (rk421 = rk321 ^ (rk420 = rk320 ^ Sbox(rk333))));
   rk433 = rk333 ^ (rk432 = rk332 ^ (rk431 = rk331 ^ (rk430 = rk330 ^ Sbox(rk303))));

   Mix(a500, (a400 ^ rk400), a510, (a411 ^ rk411), a520, (a422 ^ rk422), a530, (a433 ^ rk433));
   Mix(a501, (a401 ^ rk401), a511, (a412 ^ rk412), a521, (a423 ^ rk423), a531, (a430 ^ rk430));
   Mix(a502, (a402 ^ rk402), a512, (a413 ^ rk413), a522, (a420 ^ rk420), a532, (a431 ^ rk431));
   Mix(a503, (a403 ^ rk403), a513, (a410 ^ rk410), a523, (a421 ^ rk421), a533, (a432 ^ rk432));

   // Round 4

   rk303 = rk203 ^ (rk302 = rk202 ^ (rk301 = rk201 ^ (rk300 = rk200 ^ Sbox(rk213) ^ 0x04)));
   rk313 = rk213 ^ (rk312 = rk212 ^ (rk311 = rk211 ^ (rk310 = rk210 ^ Sbox(rk223))));
   rk323 = rk223 ^ (rk322 = rk222 ^ (rk321 = rk221 ^ (rk320 = rk220 ^ Sbox(rk233))));
   rk333 = rk233 ^ (rk332 = rk232 ^ (rk331 = rk231 ^ (rk330 = rk230 ^ Sbox(rk203))));

   Mix(a400, (a300 ^ rk300), a410, (a311 ^ rk311), a420, (a322 ^ rk322), a430, (a333 ^ rk333));
   Mix(a401, (a301 ^ rk301), a411, (a312 ^ rk312), a421, (a323 ^ rk323), a431, (a330 ^ rk330));
   Mix(a402, (a302 ^ rk302), a412, (a313 ^ rk313), a422, (a320 ^ rk320), a432, (a331 ^ rk331));
   Mix(a403, (a303 ^ rk303), a413, (a310 ^ rk310), a423, (a321 ^ rk321), a433, (a332 ^ rk332));

   // Round 3

   rk203 = rk103 ^ (rk202 = rk102 ^ (rk201 = rk101 ^ (rk200 = rk100 ^ Sbox(rk113) ^ 0x02)));
   rk213 = rk113 ^ (rk212 = rk112 ^ (rk211 = rk111 ^ (rk210 = rk110 ^ Sbox(rk123))));
   rk223 = rk123 ^ (rk222 = rk122 ^ (rk221 = rk121 ^ (rk220 = rk120 ^ Sbox(rk133))));
   rk233 = rk133 ^ (rk232 = rk132 ^ (rk231 = rk131 ^ (rk230 = rk130 ^ Sbox(rk103))));

   Mix(a300, (a200 ^ rk200), a310, (a211 ^ rk211), a320, (a222 ^ rk222), a330, (a233 ^ rk233));
   Mix(a301, (a201 ^ rk201), a311, (a212 ^ rk212), a321, (a223 ^ rk223), a331, (a230 ^ rk230));
   Mix(a302, (a202 ^ rk202), a312, (a213 ^ rk213), a322, (a220 ^ rk220), a332, (a231 ^ rk231));
   Mix(a303, (a203 ^ rk203), a313, (a210 ^ rk210), a323, (a221 ^ rk221), a333, (a232 ^ rk232));

   // Round 2

   rk103 = rk003 ^ (rk102 = rk002 ^ (rk101 = rk001 ^ (rk100 = rk000 ^ Sbox(rk013) ^ 0x01)));
   rk113 = rk013 ^ (rk112 = rk012 ^ (rk111 = rk011 ^ (rk110 = rk010 ^ Sbox(rk023))));
   rk123 = rk023 ^ (rk122 = rk022 ^ (rk121 = rk021 ^ (rk120 = rk020 ^ Sbox(rk033))));
   rk133 = rk033 ^ (rk132 = rk032 ^ (rk131 = rk031 ^ (rk130 = rk030 ^ Sbox(rk003))));

   Mix(a200, (a100 ^ rk100), a210, (a111 ^ rk111), a220, (a122 ^ rk122), a230, (a133 ^ rk133));
   Mix(a201, (a101 ^ rk101), a211, (a112 ^ rk112), a221, (a123 ^ rk123), a231, (a130 ^ rk130));
   Mix(a202, (a102 ^ rk102), a212, (a113 ^ rk113), a222, (a120 ^ rk120), a232, (a131 ^ rk131));
   Mix(a203, (a103 ^ rk103), a213, (a110 ^ rk110), a223, (a121 ^ rk121), a233, (a132 ^ rk132));

   // Round 1

   Mix(a100, (a000 ^ rk000), a110, (a011 ^ rk011), a120, (a022 ^ rk022), a130, (a033 ^ rk033));
   Mix(a101, (a001 ^ rk001), a111, (a012 ^ rk012), a121, (a023 ^ rk023), a131, (a030 ^ rk030));
   Mix(a102, (a002 ^ rk002), a112, (a013 ^ rk013), a122, (a020 ^ rk020), a132, (a031 ^ rk031));
   Mix(a103, (a003 ^ rk003), a113, (a010 ^ rk010), a123, (a021 ^ rk021), a133, (a032 ^ rk032));

#else

   // Round 1

   Mix(a100, (a000 ^ rk000), a110, (a011 ^ rk011), a120, (a022 ^ rk022), a130, (a033 ^ rk033));
   Mix(a101, (a001 ^ rk001), a111, (a012 ^ rk012), a121, (a023 ^ rk023), a131, (a030 ^ rk030));
   Mix(a102, (a002 ^ rk002), a112, (a013 ^ rk013), a122, (a020 ^ rk020), a132, (a031 ^ rk031));
   Mix(a103, (a003 ^ rk003), a113, (a010 ^ rk010), a123, (a021 ^ rk021), a133, (a032 ^ rk032));

   // Round 2

   rk103 = rk003 ^ (rk102 = rk002 ^ (rk101 = rk001 ^ (rk100 = rk000 ^ Sbox(rk013) ^ 0x01)));
   rk113 = rk013 ^ (rk112 = rk012 ^ (rk111 = rk011 ^ (rk110 = rk010 ^ Sbox(rk023))));
   rk123 = rk023 ^ (rk122 = rk022 ^ (rk121 = rk021 ^ (rk120 = rk020 ^ Sbox(rk033))));
   rk133 = rk033 ^ (rk132 = rk032 ^ (rk131 = rk031 ^ (rk130 = rk030 ^ Sbox(rk003))));

   Mix(a200, (a100 ^ rk100), a210, (a111 ^ rk111), a220, (a122 ^ rk122), a230, (a133 ^ rk133));
   Mix(a201, (a101 ^ rk101), a211, (a112 ^ rk112), a221, (a123 ^ rk123), a231, (a130 ^ rk130));
   Mix(a202, (a102 ^ rk102), a212, (a113 ^ rk113), a222, (a120 ^ rk120), a232, (a131 ^ rk131));
   Mix(a203, (a103 ^ rk103), a213, (a110 ^ rk110), a223, (a121 ^ rk121), a233, (a132 ^ rk132));

   // Round 3

   rk203 = rk103 ^ (rk202 = rk102 ^ (rk201 = rk101 ^ (rk200 = rk100 ^ Sbox(rk113) ^ 0x02)));
   rk213 = rk113 ^ (rk212 = rk112 ^ (rk211 = rk111 ^ (rk210 = rk110 ^ Sbox(rk123))));
   rk223 = rk123 ^ (rk222 = rk122 ^ (rk221 = rk121 ^ (rk220 = rk120 ^ Sbox(rk133))));
   rk233 = rk133 ^ (rk232 = rk132 ^ (rk231 = rk131 ^ (rk230 = rk130 ^ Sbox(rk103))));

   Mix(a300, (a200 ^ rk200), a310, (a211 ^ rk211), a320, (a222 ^ rk222), a330, (a233 ^ rk233));
   Mix(a301, (a201 ^ rk201), a311, (a212 ^ rk212), a321, (a223 ^ rk223), a331, (a230 ^ rk230));
   Mix(a302, (a202 ^ rk202), a312, (a213 ^ rk213), a322, (a220 ^ rk220), a332, (a231 ^ rk231));
   Mix(a303, (a203 ^ rk203), a313, (a210 ^ rk210), a323, (a221 ^ rk221), a333, (a232 ^ rk232));

   // Round 4

   rk303 = rk203 ^ (rk302 = rk202 ^ (rk301 = rk201 ^ (rk300 = rk200 ^ Sbox(rk213) ^ 0x04)));
   rk313 = rk213 ^ (rk312 = rk212 ^ (rk311 = rk211 ^ (rk310 = rk210 ^ Sbox(rk223))));
   rk323 = rk223 ^ (rk322 = rk222 ^ (rk321 = rk221 ^ (rk320 = rk220 ^ Sbox(rk233))));
   rk333 = rk233 ^ (rk332 = rk232 ^ (rk331 = rk231 ^ (rk330 = rk230 ^ Sbox(rk203))));

   Mix(a400, (a300 ^ rk300), a410, (a311 ^ rk311), a420, (a322 ^ rk322), a430, (a333 ^ rk333));
   Mix(a401, (a301 ^ rk301), a411, (a312 ^ rk312), a421, (a323 ^ rk323), a431, (a330 ^ rk330));
   Mix(a402, (a302 ^ rk302), a412, (a313 ^ rk313), a422, (a320 ^ rk320), a432, (a331 ^ rk331));
   Mix(a403, (a303 ^ rk303), a413, (a310 ^ rk310), a423, (a321 ^ rk321), a433, (a332 ^ rk332));

   // Round 5

   rk403 = rk303 ^ (rk402 = rk302 ^ (rk401 = rk301 ^ (rk400 = rk300 ^ Sbox(rk313) ^ 0x08)));
   rk413 = rk313 ^ (rk412 = rk312 ^ (rk411 = rk311 ^ (rk410 = rk310 ^ Sbox(rk323))));
   rk423 = rk323 ^ (rk422 = rk322 ^ (rk421 = rk321 ^ (rk420 = rk320 ^ Sbox(rk333))));
   rk433 = rk333 ^ (rk432 = rk332 ^ (rk431 = rk331 ^ (rk430 = rk330 ^ Sbox(rk303))));

   Mix(a500, (a400 ^ rk400), a510, (a411 ^ rk411), a520, (a422 ^ rk422), a530, (a433 ^ rk433));
   Mix(a501, (a401 ^ rk401), a511, (a412 ^ rk412), a521, (a423 ^ rk423), a531, (a430 ^ rk430));
   Mix(a502, (a402 ^ rk402), a512, (a413 ^ rk413), a522, (a420 ^ rk420), a532, (a431 ^ rk431));
   Mix(a503, (a403 ^ rk403), a513, (a410 ^ rk410), a523, (a421 ^ rk421), a533, (a432 ^ rk432));

   // Round 6

   rk503 = rk403 ^ (rk502 = rk402 ^ (rk501 = rk401 ^ (rk500 = rk400 ^ Sbox(rk413) ^ 0x10)));
   rk513 = rk413 ^ (rk512 = rk412 ^ (rk511 = rk411 ^ (rk510 = rk410 ^ Sbox(rk423))));
   rk523 = rk423 ^ (rk522 = rk422 ^ (rk521 = rk421 ^ (rk520 = rk420 ^ Sbox(rk433))));
   rk533 = rk433 ^ (rk532 = rk432 ^ (rk531 = rk431 ^ (rk530 = rk430 ^ Sbox(rk403))));

   Mix(a600, (a500 ^ rk500), a610, (a511 ^ rk511), a620, (a522 ^ rk522), a630, (a533 ^ rk533));
   Mix(a601, (a501 ^ rk501), a611, (a512 ^ rk512), a621, (a523 ^ rk523), a631, (a530 ^ rk530));
   Mix(a602, (a502 ^ rk502), a612, (a513 ^ rk513), a622, (a520 ^ rk520), a632, (a531 ^ rk531));
   Mix(a603, (a503 ^ rk503), a613, (a510 ^ rk510), a623, (a521 ^ rk521), a633, (a532 ^ rk532));

   // Round 7

   rk603 = rk503 ^ (rk602 = rk502 ^ (rk601 = rk501 ^ (rk600 = rk500 ^ Sbox(rk513) ^ 0x20)));
   rk613 = rk513 ^ (rk612 = rk512 ^ (rk611 = rk511 ^ (rk610 = rk510 ^ Sbox(rk523))));
   rk623 = rk523 ^ (rk622 = rk522 ^ (rk621 = rk521 ^ (rk620 = rk520 ^ Sbox(rk533))));
   rk633 = rk533 ^ (rk632 = rk532 ^ (rk631 = rk531 ^ (rk630 = rk530 ^ Sbox(rk503))));

   Mix(a700, (a600 ^ rk600), a710, (a611 ^ rk611), a720, (a622 ^ rk622), a730, (a633 ^ rk633));
   Mix(a701, (a601 ^ rk601), a711, (a612 ^ rk612), a721, (a623 ^ rk623), a731, (a630 ^ rk630));
   Mix(a702, (a602 ^ rk602), a712, (a613 ^ rk613), a722, (a620 ^ rk620), a732, (a631 ^ rk631));
   Mix(a703, (a603 ^ rk603), a713, (a610 ^ rk610), a723, (a621 ^ rk621), a733, (a632 ^ rk632));

   // Round 8

   rk703 = rk603 ^ (rk702 = rk602 ^ (rk701 = rk601 ^ (rk700 = rk600 ^ Sbox(rk613) ^ 0x40)));
   rk713 = rk613 ^ (rk712 = rk612 ^ (rk711 = rk611 ^ (rk710 = rk610 ^ Sbox(rk623))));
   rk723 = rk623 ^ (rk722 = rk622 ^ (rk721 = rk621 ^ (rk720 = rk620 ^ Sbox(rk633))));
   rk733 = rk633 ^ (rk732 = rk632 ^ (rk731 = rk631 ^ (rk730 = rk630 ^ Sbox(rk603))));

   Mix(a800, (a700 ^ rk700), a810, (a711 ^ rk711), a820, (a722 ^ rk722), a830, (a733 ^ rk733));
   Mix(a801, (a701 ^ rk701), a811, (a712 ^ rk712), a821, (a723 ^ rk723), a831, (a730 ^ rk730));
   Mix(a802, (a702 ^ rk702), a812, (a713 ^ rk713), a822, (a720 ^ rk720), a832, (a731 ^ rk731));
   Mix(a803, (a703 ^ rk703), a813, (a710 ^ rk710), a823, (a721 ^ rk721), a833, (a732 ^ rk732));

   // Round 9

   rk803 = rk703 ^ (rk802 = rk702 ^ (rk801 = rk701 ^ (rk800 = rk700 ^ Sbox(rk713) ^ 0x80)));
   rk813 = rk713 ^ (rk812 = rk712 ^ (rk811 = rk711 ^ (rk810 = rk710 ^ Sbox(rk723))));
   rk823 = rk723 ^ (rk822 = rk722 ^ (rk821 = rk721 ^ (rk820 = rk720 ^ Sbox(rk733))));
   rk833 = rk733 ^ (rk832 = rk732 ^ (rk831 = rk731 ^ (rk830 = rk730 ^ Sbox(rk703))));

   Mix(a900, (a800 ^ rk800), a910, (a811 ^ rk811), a920, (a822 ^ rk822), a930, (a833 ^ rk833));
   Mix(a901, (a801 ^ rk801), a911, (a812 ^ rk812), a921, (a823 ^ rk823), a931, (a830 ^ rk830));
   Mix(a902, (a802 ^ rk802), a912, (a813 ^ rk813), a922, (a820 ^ rk820), a932, (a831 ^ rk831));
   Mix(a903, (a803 ^ rk803), a913, (a810 ^ rk810), a923, (a821 ^ rk821), a933, (a832 ^ rk832));

   // Round 10

   rk903 = rk803 ^ (rk902 = rk802 ^ (rk901 = rk801 ^ (rk900 = rk800 ^ Sbox(rk813) ^ 0x1b)));
   rk913 = rk813 ^ (rk912 = rk812 ^ (rk911 = rk811 ^ (rk910 = rk810 ^ Sbox(rk823))));
   rk923 = rk823 ^ (rk922 = rk822 ^ (rk921 = rk821 ^ (rk920 = rk820 ^ Sbox(rk833))));
   rk933 = rk833 ^ (rk932 = rk832 ^ (rk931 = rk831 ^ (rk930 = rk830 ^ Sbox(rk803))));

   Set(a1000, (a900 ^ rk900), a1010, (a911 ^ rk911), a1020, (a922 ^ rk922), a1030, (a933 ^ rk933));
   Set(a1001, (a901 ^ rk901), a1011, (a912 ^ rk912), a1021, (a923 ^ rk923), a1031, (a930 ^ rk930));
   Set(a1002, (a902 ^ rk902), a1012, (a913 ^ rk913), a1022, (a920 ^ rk920), a1032, (a931 ^ rk931));
   Set(a1003, (a903 ^ rk903), a1013, (a910 ^ rk910), a1023, (a921 ^ rk921), a1033, (a932 ^ rk932));

   // Round 11

   rk1003 = rk903 ^ (rk1002 = rk902 ^ (rk1001 = rk901 ^ (rk1000 = rk900 ^ Sbox(rk913) ^ 0x36)));
   rk1013 = rk913 ^ (rk1012 = rk912 ^ (rk1011 = rk911 ^ (rk1010 = rk910 ^ Sbox(rk923))));
   rk1023 = rk923 ^ (rk1022 = rk922 ^ (rk1021 = rk921 ^ (rk1020 = rk920 ^ Sbox(rk933))));
   rk1033 = rk933 ^ (rk1032 = rk932 ^ (rk1031 = rk931 ^ (rk1030 = rk930 ^ Sbox(rk903))));

   Add(a1100, (a1000 ^ rk1000), a1110, (a1010 ^ rk1010), a1120, (a1020 ^ rk1020), a1130, (a1030 ^ rk1030));
   Add(a1101, (a1001 ^ rk1001), a1111, (a1011 ^ rk1011), a1121, (a1021 ^ rk1021), a1131, (a1031 ^ rk1031));
   Add(a1102, (a1002 ^ rk1002), a1112, (a1012 ^ rk1012), a1122, (a1022 ^ rk1022), a1132, (a1032 ^ rk1032));
   Add(a1103, (a1003 ^ rk1003), a1113, (a1013 ^ rk1013), a1123, (a1023 ^ rk1023), a1133, (a1033 ^ rk1033));

#endif

}   

#ifndef FPGAC

/*
 * Test bench for aes encrypt function using FIPS 197 test data
 */

main() {
    /*
     * test data from Federal Information Processing Standards Publication 197
     */

    // col 0      col 1       col 2       col 3
    a000=0x32;  a001=0x88;  a002=0x31;  a003=0xe0; // row 0
    a010=0x43;  a011=0x5a;  a012=0x31;  a013=0x37; // row 1
    a020=0xf6;  a021=0x30;  a022=0x98;  a023=0x07; // row 2
    a030=0xa8;  a031=0x8d;  a032=0xa2;  a033=0x34; // row 3

    rk000=0x2b; rk001=0x28; rk002=0xab; rk003=0x09; // row 0
    rk010=0x7e; rk011=0xae; rk012=0xf7; rk013=0xcf; // row 1
    rk020=0x15; rk021=0xd2; rk022=0x15; rk023=0x4f; // row 2
    rk030=0x16; rk031=0xa6; rk032=0x88; rk033=0x3c; // row 3

#ifdef PIPELINED
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
    aes_encrypt();
#endif
    aes_encrypt();


    printf("Round 1 data after Mix Columns\n");
    printf("    %02x %02x %02x %02x\n",   a100, a101, a102, a103);
    printf("    %02x %02x %02x %02x\n",   a110, a111, a112, a113);
    printf("    %02x %02x %02x %02x\n",   a120, a121, a122, a123);
    printf("    %02x %02x %02x %02x  ",   a130, a131, a132, a133);

    if(a100==0x04 &&  a101==0xe0 &&  a102==0x48 &&  a103==0x28 &&
       a110==0x66 &&  a111==0xcb &&  a112==0xf8 &&  a113==0x06 &&
       a120==0x81 &&  a121==0x19 &&  a122==0xd3 &&  a123==0x26 &&
       a130==0xe5 &&  a131==0x9a &&  a132==0x7a &&  a133==0x4c)
       printf("Round 1 data is correct\n\n");
    else
       printf("Round 1 data is NOT correct\n\n");

    printf("Round 2 data after Mix Columns\n");
    printf("    %02x %02x %02x %02x\n",   a200, a201, a202, a203);
    printf("    %02x %02x %02x %02x\n",   a210, a211, a212, a213);
    printf("    %02x %02x %02x %02x\n",   a220, a221, a222, a223);
    printf("    %02x %02x %02x %02x  ",   a230, a231, a232, a233);

    if(a200==0x58 &&  a201==0x1b &&  a202==0xdb &&  a203==0x1b &&
       a210==0x4d &&  a211==0x4b &&  a212==0xe7 &&  a213==0x6b &&
       a220==0xca &&  a221==0x5a &&  a222==0xca &&  a223==0xb0 &&
       a230==0xf1 &&  a231==0xac &&  a232==0xa8 &&  a233==0xe5)
       printf("Round 2 data is correct\n\n");
    else
       printf("Round 2 data is NOT correct\n\n");

    printf("Round 11 final output data\n");
    printf("    %02x %02x %02x %02x\n",   a1100, a1101, a1102, a1103);
    printf("    %02x %02x %02x %02x\n",   a1110, a1111, a1112, a1113);
    printf("    %02x %02x %02x %02x\n",   a1120, a1121, a1122, a1123);
    printf("    %02x %02x %02x %02x  ",   a1130, a1131, a1132, a1133);

    if(a1100==0x39 &&  a1101==0x02 &&  a1102==0xdc &&  a1103==0x19 &&
       a1110==0x25 &&  a1111==0xdc &&  a1112==0x11 &&  a1113==0x6a &&
       a1120==0x84 &&  a1121==0x09 &&  a1122==0x85 &&  a1123==0x0b &&
       a1130==0x1d &&  a1131==0xfb &&  a1132==0x97 &&  a1133==0x32)
       printf("Round 11 final output data is correct\n\n");
    else
       printf("Round 11 final output data is NOT correct\n\n");
}
#endif
