main()
{

        int a:2 ;
        int b:2 ;
#pragma fpgac_inputport (a,a9)
#pragma fpgac_inputport (b,a10)
        int sum_of_products:2 ;
#pragma fpgac_outputport (sum_of_products,a11)
        int loopvar:2 ;
        
        for (loopvar = 0  ;    ; loopvar ++  )
       {
                sum_of_products =  loopvar & a ;
       }
}


