/*
 * This is the Sbox table expressed as conditional assignment tree to create a ROM
 * While admittedly a hack, it has the side effect of provoking LUT term sharing to
 * reduce total LUT count compared to a real 256 byte rom in LUTs.
 */
#define Sbox(in) (Xbox(in))

#define Xbox(in) \
{ unsigned char out; \
    if(!(in&0x80)){ \
        if(!(in&0x40)){ \
            if(!(in&0x20)){ \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=99; else out=124; } else { if(!(in&0x01)) out=119;else out=123; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=242;else out=107; } else { if(!(in&0x01)) out=111;else out=197; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=48; else out=1;   } else { if(!(in&0x01)) out=103;else out=43;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=254;else out=215; } else { if(!(in&0x01)) out=171;else out=118; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=202;else out=130; } else { if(!(in&0x01)) out=201;else out=125; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=250;else out=89;  } else { if(!(in&0x01)) out=71; else out=240; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=173;else out=212; } else { if(!(in&0x01)) out=162;else out=175; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=156;else out=164; } else { if(!(in&0x01)) out=114;else out=192; } \
                        } \
                    } \
                } \
            } else { \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=183;else out=253; } else { if(!(in&0x01)) out=147;else out=38;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=54; else out=63;  } else { if(!(in&0x01)) out=247;else out=204; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=52; else out=165; } else { if(!(in&0x01)) out=229;else out=241; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=113;else out=216; } else { if(!(in&0x01)) out=49; else out=21;  } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=4;  else out=199; } else { if(!(in&0x01)) out=35; else out=195; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=24; else out=150; } else { if(!(in&0x01)) out=5;  else out=154; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=7;  else out=18;  } else { if(!(in&0x01)) out=128;else out=226; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=235;else out=39;  } else { if(!(in&0x01)) out=178;else out=117; } \
                        } \
                    } \
                } \
            } \
        } else { \
            if(!(in&0x20)){ \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=9;  else out=131; } else { if(!(in&0x01)) out=44; else out=26;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=27; else out=110; } else { if(!(in&0x01)) out=90; else out=160; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=82; else out=59;  } else { if(!(in&0x01)) out=214;else out=179; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=41; else out=227; } else { if(!(in&0x01)) out=47; else out=132; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=83; else out=209; } else { if(!(in&0x01)) out=0;  else out=237; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=32; else out=252; } else { if(!(in&0x01)) out=177;else out=91;  } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=106;else out=203; } else { if(!(in&0x01)) out=190;else out=57;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=74; else out=76;  } else { if(!(in&0x01)) out=88; else out=207; } \
                        } \
                    } \
                } \
            } else { \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=208;else out=239; } else { if(!(in&0x01)) out=170;else out=251; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=67; else out=77;  } else { if(!(in&0x01)) out=51; else out=133; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=69; else out=249; } else { if(!(in&0x01)) out=2;  else out=127; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=80; else out=60;  } else { if(!(in&0x01)) out=159;else out=168; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=81; else out=163; } else { if(!(in&0x01)) out=64; else out=143; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=146;else out=157; } else { if(!(in&0x01)) out=56; else out=245; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=188;else out=182; } else { if(!(in&0x01)) out=218;else out=33;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=16; else out=255; } else { if(!(in&0x01)) out=243;else out=210; } \
                        } \
                    } \
                } \
            } \
        } \
    } else { \
        if(!(in&0x40)){ \
            if(!(in&0x20)){ \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=205;else out=12;  } else { if(!(in&0x01)) out=19; else out=236; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=95; else out=151; } else { if(!(in&0x01)) out=68; else out=23;  } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=196;else out=167; } else { if(!(in&0x01)) out=126;else out=61;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=100;else out=93;  } else { if(!(in&0x01)) out=25; else out=115; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=96; else out=129; } else { if(!(in&0x01)) out=79; else out=220; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=34; else out=42;  } else { if(!(in&0x01)) out=144;else out=136; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=70; else out=238; } else { if(!(in&0x01)) out=184;else out=20;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=222;else out=94;  } else { if(!(in&0x01)) out=11; else out=219; } \
                        } \
                    } \
                } \
            } else { \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=224;else out=50;  } else { if(!(in&0x01)) out=58; else out=10;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=73; else out=6;   } else { if(!(in&0x01)) out=36; else out=92;  } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=194;else out=211; } else { if(!(in&0x01)) out=172;else out=98;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=145;else out=149; } else { if(!(in&0x01)) out=228;else out=121; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=231;else out=200; } else { if(!(in&0x01)) out=55; else out=109; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=141;else out=213; } else { if(!(in&0x01)) out=78; else out=169; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=108;else out=86;  } else { if(!(in&0x01)) out=244;else out=234; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=101;else out=122; } else { if(!(in&0x01)) out=174;else out=8;   } \
                        } \
                    } \
                } \
            } \
        } else { \
            if(!(in&0x20)){ \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=186;else out=120; } else { if(!(in&0x01)) out=37; else out=46;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=28; else out=166; } else { if(!(in&0x01)) out=180;else out=198; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=232;else out=221; } else { if(!(in&0x01)) out=116;else out=31;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=75; else out=189; } else { if(!(in&0x01)) out=139;else out=138; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=112;else out=62;  } else { if(!(in&0x01)) out=181;else out=102; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=72; else out=3;   } else { if(!(in&0x01)) out=246;else out=14;  } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=97; else out=53;  } else { if(!(in&0x01)) out=87; else out=185; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=134;else out=193; } else { if(!(in&0x01)) out=29; else out=158; } \
                        } \
                    } \
                } \
            } else { \
                if(!(in&0x10)){ \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=225;else out=248; } else { if(!(in&0x01)) out=152;else out=17;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=105;else out=217; } else { if(!(in&0x01)) out=142;else out=148; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=155;else out=30;  } else { if(!(in&0x01)) out=135;else out=233; } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=206;else out=85;  } else { if(!(in&0x01)) out=40; else out=223; } \
                        } \
                    } \
                } else { \
                    if(!(in&0x08)){ \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=140;else out=161; } else { if(!(in&0x01)) out=137;else out=13;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=191;else out=230; } else { if(!(in&0x01)) out=66; else out=104; } \
                        } \
                    } else { \
                        if(!(in&0x04)){ \
                            if(!(in&0x02)){ if(!(in&0x01)) out=65; else out=153; } else { if(!(in&0x01)) out=45; else out=15;  } \
                        } else { \
                            if(!(in&0x02)){ if(!(in&0x01)) out=176;else out=84;  } else { if(!(in&0x01)) out=187;else out=22;  } \
                        } \
                    } \
                } \
            } \
        } \
    }; out &= 0xff; \
}

