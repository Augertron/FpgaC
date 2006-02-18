main()
{
int a:1,outs:1,outs1:1,b:1,c:1,t:1;
#pragma fpgac_inputport (a,a9)
#pragma fpgac_inputport (b,a10)
#pragma fpgac_inputport (c,a11)
#pragma fpgac_outputport (outs,a12)
#pragma fpgac_outputport (outs1,a12)


        while(b|c) {
                outs =  (b & c) ;
                while(a) {
                        outs1 =  (a & c) ;
                }
        }
        while(a|b) {
                outs =  (a & b) ;
        }
}



