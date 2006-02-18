main()
{

        int a:3 ;
        int b:3 ;
#pragma fpgac_inputport (a,a9)
#pragma fpgac_inputport (b,a10)
        int sum_of_products:3 ;
#pragma fpgac_outputport (sum_of_products,a11)
        int loopvar:3 ;
        loopvar = 0 ;
        for (loopvar++;0;) ;
        sum_of_products =  loopvar & a ;
        for (loopvar++;0;) ;
        sum_of_products =  loopvar & b ;
}


