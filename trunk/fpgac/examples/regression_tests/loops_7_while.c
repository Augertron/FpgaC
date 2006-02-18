main()
{
        int a:2 ;
#pragma fpgac_inputport (a,a9)
        int sum_of_products:2 ;
        int sum_of_products1:2 ;
#pragma fpgac_outputport (sum_of_products,a11)
#pragma fpgac_outputport (sum_of_products1,a12)
        int loopvar:2 ;
        int loopvar1:2 ;
        
       loopvar = 0 ;
       loopvar1 = 0 ;
    while ( loopvar < 2 )
       {
                sum_of_products =  loopvar & a ;
                sum_of_products1 =  loopvar1 & a ;
                loopvar ++;
                loopvar1 ++;
       }
}


