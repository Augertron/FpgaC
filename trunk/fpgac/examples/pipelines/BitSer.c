/*
 * BitSer.c - FpgaC Pipelined Bit Serial Sort Example 1
 * copyright 2006 by John Bass, DMS Design under FpgaC BSD License
 *
 * This example builds a pipelined parallel sort for bit serial unsigned
 * integers, using LUT based multiplexors. The sort happens in log2(N)
 * stages, with a log2(N) latency thru the pipeline. The muxes each
 * check the data relationship, and latch the sort mswap mux selector
 * at the first inequality until endword is seen. This approach of using
 * a large number of small pipelined sorting engines is called "systolic".
 *
 * One variation of this design is to buffer the bit serial words into
 * LUT rams and return the sorted data on the same I/O pins. In FpgaC
 * that would be a stage of small arrays for retiming. This would allow
 * a sorting engine with X pins to sort unsigned bit serial integers of 
 * length N in time 2*N clocks, and a latency of N clocks.
 *
 * Another variation of this design is to fill the FPGA with additional
 * stages and make the internal sort wider than the available I/O pins
 * in support of a much larger streaming sort with multiple passes. The
 * additional array memory and sorting muxes would form a bubble sort
 * that would carry that many words down thru the stream. This approach
 * of using internal holding queues, forms "systolic priority queues".
 * See "Systolic Priority Queues" by C. Leiserson, 1979
 * Technical Report CMU-CS-79-115, Carnegie-Mellon University
 *
 * Other variations are changing the word flag to the first bit, to clear
 * the mux selectors.
 */

/*
 * Pipeline 8 streams of bit serial data words
 */
struct stage {
    int    s0:1;      // stream 0
    int    s1:1;      // stream 1
    int    s2:1;      // stream 2
    int    s3:1;      // stream 3
    int    s4:1;      // stream 4
    int    s5:1;      // stream 5
    int    s6:1;      // stream 6
    int    s7:1;      // stream 7
    int    endword:1;  // sentinal to reset mux selector, active high
} in, p1, p2, out;   // pipeline stages

/*
 * Setup I/O port mapping
 */
#pragma fpgac_inputport (in.s0)
#pragma fpgac_inputport (in.s1)
#pragma fpgac_inputport (in.s2)
#pragma fpgac_inputport (in.s3)
#pragma fpgac_inputport (in.s4)
#pragma fpgac_inputport (in.s5)
#pragma fpgac_inputport (in.s6)
#pragma fpgac_inputport (in.s7)
#pragma fpgac_inputport (in.endword)

#pragma fpgac_outputport (out.s0)
#pragma fpgac_outputport (out.s1)
#pragma fpgac_outputport (out.s2)
#pragma fpgac_outputport (out.s3)
#pragma fpgac_outputport (out.s4)
#pragma fpgac_outputport (out.s5)
#pragma fpgac_outputport (out.s6)
#pragma fpgac_outputport (out.s7)
#pragma fpgac_outputport (out.endword)

/*
 * Define C Preprocessor macro for basic sorting mux engine
 */
#define mux(stagein, stageout, s1, s0) { \
    int mlatch:1, mswap:1; \
    if(~mlatch & (stagein.s1 ^ stagein.s0 )) { \
        mlatch = 1; mswap = stagein.s0; \
    } \
    if(mlatch && mswap) { \
        stageout.s1 = stagein.s0; stageout.s0 = stagein.s1; \
    } else { \
        stageout.s1 = stagein.s1; stageout.s0 = stagein.s0; \
    } \
    if(stagein.endword) \
        mlatch = mswap = 0; \
}


/*
 * An FpgaC process is started up when the bitstream is configured
 * and processes implicitly loop forever
 */
fpgac_process Sort() {

    /*
     * To build a pipeline, we describe the last stage first,
     * and the first stage last. Thus as execution proceeds
     * logically down this procedure, stage n gets it's data
     * form stage n-1 below till we get to the device input
     */

    // Pipeline Stage 3: Sort stage 2 to output pins
    mux(p2,out,s7,s3);mux(p2,out,s6,s2);mux(p2,out,s5,s1);mux(p2,out,s4,s0);
    out.endword = p2.endword;

    // Pipeline Stage 2: Sort stage 1 to stage 2
    mux(p1,p2,s7,s5);mux(p1,p2,s6,s4);mux(p1,p2,s3,s1);mux(p1,p2,s2,s0);
    p2.endword = p1.endword;

    // Pipeline Stage 1: Sort input pins to stage 1
    mux(in,p1,s7,s6);mux(in,p1,s5,s4);mux(in,p1,s3,s2);mux(in,p1,s1,s0);
    p1.endword = in.endword;
}
