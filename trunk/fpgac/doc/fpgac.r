.TL
FPGA C
.AU
John L. Bass
.AI
DMS Design
February 2005
.R
.sp
.SH Introduction
.sp
.LP
FPGA C (or fpgac)
is a compiler for a simple hardware description language [1].
It takes a program written in a subset of the C programming language, and produces
a circuit that will implement the program.
The circuit is intended for FPGAs, CPLDs or even ASICs.
This manual describes version 4.2 of fpgac
.LP
FPGA C sources and documentation are derived from TMCC by Dave Galloway, Department of Electrical and Computer Engineering,
University of Toronto, who did this great pioneering work in support of the Transmogrifier projects.

.SH
Example
.LP
Here is a simple fpgac program that makes the 8 LEDs on the Xilinx 4000
demo board count up:
.CW
.nf
.sp
	main() {
		char lights, count;

	#pragma	outputport(lights, 60, 59, 58, 57, 66, 65, 62, 61);

		count = 0;
		while(1) {
			count = count + 1;
			lights = ~count;
		}
	}
.sp
.if
.R
.sp
.LP
The 
.B
#pragma outputport
.R
statement telling the compiler
the
.B
lights
.R
variable is really a set of 8 output pins on the chip, and specifies the pin
numbers.
The program then goes into an infinite loop, incrementing the count.
The LEDs on the demo board are wired so that a 0 output turns them on, and
a 1 output turns them off.
To reverse this, we use C's
.B
~
.R
complement operator.
.SH
Running fpgac
.LP
To compile the sample program, call it counter.c, and
run:
.CW
.nf
.sp
	fpgac counter.c
.sp
.fi
.R
The compiler will produce a counter.xnf file, then pass it through
your FPGA vendors tool chain to produce a bit stream file suitable
for programming your FPGA, CPLD, or even as input to your ASIC design
stream.  [You will need to edit scripts for this to work at your site.]  All other intermediate files will be removed.
If you want to save them, use the -v flag on fpgac
.sp
.LP
If you just want to see the xnf output, and not run the vendor tool chain, use the -S flag.
The optional -p partname flag can be used to tell fpgac what part
you are targeting
The compiler will put that information into the xnf file that it generates,
for the use of other tools.
The compiler also supports these normal C compiler flags: -U, and -D.
.SH
Differences from Standard C
.LP
These C operators and keywords are implemented by the fpgac compiler:
.CW
.nf
.sp
	!         !=      &       &&         &=      ()      +
	++        +=      -       --         -=      <       <<
	<<=       <=      =       ==         >       >=      >>
	>>=       ^       ^=      break      else    if      int
	return    while   {}      |          |=      ||      ~
	register  long    short   unsigned   char    void
.sp
.fi
.R
.LP
These C operators and keywords are not implemented by the fpgac compiler:
.CW
.nf
.sp
	"strings"  %        %=        (casts)   *         *=
        ,          ->       .         /         /=        :
        ?          []       auto      case      continue  default
        do         double   extern    float     for       goto
        sizeof     static   union     struct    switch    typedef
        enum       unary &  unary *
.sp
.fi
.R
.sp
.LP
In other words, fpgac has 8, 16, and 32 bit integer variables,
constants, expressions and assignment statements.  It has if statements,
while loops and function calls.  You can also use any of the
cpp # directives and macros.
.LP
It does not have multiply or divide, pointers or structures.
You can not use recursion. Arrays (an early alpha test feature) are partially implemented, and require
hand editing of the XNF output to correct some generations errors, so
they are not recommended for general use at this time.
Some of the simple omitted stuff may get added to fpgac in the future.
.LP
All integer variables are created as their specified width, plus a
sign bit. For now, that includes unsigned variables too. In a future
release, signed integers will be one bit less as the width specification
will include the sign bit, and unsigned will not include a sign bit.
Char currently is 8 bits plus a sign bit, short is 16 bits plus a sign bit,
and long is 32 bits plus a sign bit.
.KS
.SH
Extensions to C
.CW
.nf
.sp
	#pragma intbits nnn
.sp
.fi
.R
.sp
.LP
The
.B
intbits
.R
pragma sets the number of bits stored in each integer.  Any integer
variables declared after the intbits directive will have nnn bits in
.KE
them.  You can have any number of intbits directives in your program.
They only affect the integer declarations up to the next intbits
directive.  The default number of bits in an integer is 16.
.LP
The compiler also accepts both Handle-C and C bit field style bit
width controls for
.B
int
.R
variables.
.CW
.nf
.sp
	int 12 myvar;
	int othervar:12;
.sp
.fi
.R
Both
.B
myvar
.R
and
.B
othervar
.R
will be created as 12 bits wide, plus a sign bit. 
.LP
The fpgac compiler accepts constants in decimal, octal, and hex as found
in most C compilers. It also accepts binary in the 0b0010 form found in
the Intel C/C++ compiler (and others) and other HDL's.
.SH
Input, Output and Bus Ports
.sp
.LP
A variable can be associated with a set of output pins with the call:
.sp
.CW
.nf
.sp
	#pragma outputport( variablename \fR[\f(CW , nnn \fR{\f(CW , nnn \fR} ]\f(CW );
.sp
.fi
.R
.LP
Any assignments to that variable from that point on will cause the
named pins to take on the new value.
They will continue to assert that value until they are explicitly
changed.
Trying to use the value of the variable in an expression is undefined.
.LP
The pin numbers are listed with the least significant bit first.
If you do not specify the pin numbers, fpgac will leave them undefined in
the output, and the Xilinx placement and routing software will pick them
for you.
Non-numeric pin numbers can be specified using double quotes, for
example:
.CW
.nf
.sp
	#pragma outputport( variablename, 23, "J7", "K8", 16 );
.sp
.fi
.R
.LP
A variable can be associated with a set of input pins with the call:
.CW
.nf
.sp
	#pragma inputport( variablename \fR[\f(CW , nnn \fR{\f(CW , nnn \fR} ]\f(CW );
.sp
.fi
.R
.LP
Any reference to the variable after the inputport statement will return
the value on the given input pins.
Assignments to the variable are not allowed.
As in the outputport case, the pin numbers may be left undefined.
.LP
A variable can be associated with a set of bi-directional input/output
pins with the call:
.CW
.nf
.sp
	#pragma bus_port( variablename \fR[\f(CW , nnn \fR{\f(CW , nnn \fR} ]\f(CW );
.sp
.fi
.R
.LP
Any reference to the variable after the bus_port statement will return
the value on the given pins.
Any assignment to the variable after the bus_port statement will drive
the pins to the new value.
As in the outputport case, the pin numbers may be left undefined.
.LP
A bus_port will start out in input (ie: tristate) mode, and will not be driven.
An assignment to a bus_port variable will cause the pins to go into
output mode.
.KS
They will stay in output mode until the program puts them back into
input mode with the call:
.CW
.nf
.sp
	#pragma bus_idle( variablename );
.sp
.fi
.R
.KE
.SH
Timing and Clock Ticks
.LP
Tmcc generates a simple synchronous design.
It has one clock, and all flipflops change on the same edge of the clock.
.LP
Tmcc will attempt to stuff as much of your program as it can into the
current clock period.
Multiple assignment statements and even if statements will all get
packed into the current clock period.
It will only stop doing this and wait for the next clock tick at any
of these points in the program:
.IP
the top of a while() loop
.IP
a function call
.LP
For example, suppose we want to raise an output for 2 clock ticks and
then lower it.
The following program:
.CW
.nf
.sp
	main() {
		int out;

		outputport(out);
		out = 1;
		out = 1;
		out = 0;
	}
.sp
.fi
.R
does not work.
All 3 assignments will be packed into the same clock tick, and only the
last one will have any affect.
The port will be set to 0.
.LP
To do it correctly, we need to do this:
.CW
.nf
.sp
	main() {
		int out:1;
		int count:2;

		outputport(out);
		count = 2;
		while(count) {
			out = 1;
			count = count - 1;
		}
		out = 0;
	}
.sp
.fi
.R
The while loop will be executed twice, taking one clock tick in each case.
The output port will be set to 1 for 2 clock ticks, and 0 thereafter.
.LP
Although fpgac attempts to pack as many statements as possible into
the same clock tick, it still works like C.
Statements are executed in the order that you write them in.
.KS
Variable assignments take affect immediately, although they may not
show up on an output pin until the following clock cycle.
This code:
.CW
.nf
.sp
	temp = a;
	a = b;
	b = temp;
.sp
.fi
.R
will exchange the values of a and b, just as it does in C.
.KE
The exchange will take place in one clock cycle, and no flipflops or
other circuitry will be generated for temp.
.SH
Clock Source
.LP
By default, fpgac gets the clock for the circuit from the XC4000 series
internal oscillator, running at 15 Hz [sic].
This is suitable for testing the compiler, but useless for anything else.
.LP
If you want to get the clock from a different source, define a signal called
CLK
in your own xnf file, and compile the program this way:
.CW
.nf
.sp
	fpgac program.c myclock.xnf
.sp
.fi
.R
.LP
For instance, this xnf file will use a clock signal from an external pin
named FPSCLK:
.CW
.nf
.sp
	SYM, CLK-AA, BUFGP
	PIN, I, I, FPSCLK
	PIN, O, O, CLK
	END
	EXT, FPSCLK, I
	SYM, STARTUP, STARTUP
	PIN, CLK, I, CLK
	END
.sp
.fi
.R
.LP
If you want to use a name other than CLK for the clock source, use
the -cYOURCLOCKNAME option.
A simple -c option will leave the clock name as CLK, but will not generate
the default 15 Hz clock circuitry.
.SH
Multiple Threads of Control
.LP
A simple fpgac program has one thread
of control, which starts at the beginning of the main() routine and
continues from there.
Many circuits have to do several tasks simultaneously, for instance:
handling the bus protocol for an input and output bus, talking to several
RAM chips, and doing some real-time computation.
Rather than try to cram several different parallel processes
into the same program by hand,
fpgac allows you to have multiple threads of control, one for each
independent task.
.LP
To do this, write a separate fpgac program for each thread, compile
them separately and then
merge the resulting xnf files.
A circuit with 3 parallel threads can be produced like this:
.CW
.nf
.sp
	fpgac -T1 -c -S inputbus.c
	fpgac -T2 -c -S sram.c
	fpgac compute.c inputbus.xnf sram.xnf myclock.xnf
.sp
.fi
.R
Each of the fpgac programs has its own main() routine, functions and variables.
To ensure that temporary variable names do not conflict between
the separate compilations, use the -Tnnn flag on fpgac, which will modify
the names in the output net list.
The -c flag prevents the compiler from generating multiple copies of the
default clock circuit, one for each thread.
.LP
The separate threads of the computation can communicate using input and
output port variables.
Such port variables should have one writer, and one or more readers.
The writer thread declares the variable to be an output port.
The reader threads declare the same variable name to be an input port.
A call to the portflags() routine can be used to produce
port variables that do not have external pins, and are suitable for
communicating between threads.
.KS
.SH
Modifying Input and Output Port Semantics \(em portflags()
.LP
An input or output port has a set of attributes, which can be set
with the call:
.CW
.nf
.sp
	#define PORT_WIRE               0x0
	#define PORT_PIN                0x1
	#define PORT_REGISTERED         0x2
	#define PORT_PULLUP             0x4
	#define PORT_PULLDOWN           0x8

	portflags( variablename, constant_expression );
.sp
.fi
.R
.LP
PORT_PIN means that the port needs a pin on the outside of the chip.
PORT_REGISTERED on an input port means that the input signal will be
captured in a flipflop and the output of the flipflop will be used as the
variable by the rest of the circuit.
.KE
PORT_REGISTERED on an output port means that the output signal will be
saved in a flipflop, and the output of the flipflop will drive the
external circuitry.
PORT_PULLUP will enable a pull up resistor on an external bi-directional pin.
PORT_PULLDOWN will enable
a pull down resistor on an external bi-directional pin.
PORT_WIRE means none of the above, and acts like a wire.
.LP
The default attributes of an input port are: PORT_PIN.
Output ports are: (PORT_REGISTERED|PORT_PIN).
.SH
Output Formats
.LP
The -target flag specifies the output netlist format.
The default -target xc4000gates
will generate a simple XNF (Xilinx Netlist
Format) file that uses AND, OR and INV gates.
This format can be read by several FPGA synthesis and optimization CAD tools.
.LP
The compiler performs a technology mapping step as it is compiling a
program, and converts the circuit to a network of 4 input lookup tables and
flipflops.
The -target xc4000roms flag will generate
a more compact output format with each
lookup table expressed as a 16x1 bit ROM.
The -target xc4000eqns flag will generate
a more readable compact format with each
lookup table expressed as a Boolean equation.
The -target flex8000 flag will generate an XNF file using AND, OR and INV gates
that is suitable for the Altera Flex 8000 parts, and
can be read by the Altera MaxPlus software.
.LP
The output circuit is a single clocked synchronous circuit, with
a "one hot" state encoding scheme.
A 1 bit port variable with the name xxx will be called xxx in the circuit.
An n-bit port variable called xxx will be called xxx_v0,
xxx_v1, and so on.
Temporary variables generated by the compiler will be
called TTT_NNN_BBB_LMMMstring, where TTT is the thread number (usually 0),
NNN is the line number in the program
where this variable was produced,
BBB is the bit number,
MMM is a unique id,
and
string is some indication of what the variable
is for.
.SH
Circuit Size and Speed
.LP
The compiler will generate carry select adders and subtractors by default.
These adders are usually almost twice the speed of a simple ripple carry adder,
but are roughly 50% larger in area.
If you are more concerned about size than speed,
use the -fno-carry-select flag to
force the compiler to use ripple carry adders instead.
.LP
The -dverbose flag will print an estimate of the circuit's size and speed on
the standard error output.
It will include the number of lookup tables and flipflops needed, and the
number of lookup tables encountered in the longest combinational path.
The estimate may be incorrect, as ppr may find a different way of implementing
the circuit.
.SH
Generating Good Circuits
.LP
Using < or >= comparisons will produce smaller, faster
circuits than using <= or >,
since the circuit just has to check the sign bit of a subtractor in the
first two cases.
.LP
Using the same variable for different things in your circuit may produce
a larger and slower circuit.
Each assignment to a variable adds another input to a multiplexor, and large
multiplexors may become the critical path in your circuit.
Use different variables for different things, and don't try to save space
by re-using a variable.
.SH
Common Design Errors
.LP
There are several things that can cause a fpgac-generated circuit to fail
mysteriously.
First, be sure to use xdelay to check that the circuit will run at the
desired clock frequency.
.LP
Secondly, fpgac assumes that all of the inputs to the circuit will remain
stable
during a clock period, and will change only when the clock changes.
If that is not true for one or more of your inputs, you must specify
an input register for those signals using a call to portflags().
For example:
.CW
.nf
.sp
	int changing_input;

	inputport(changing_input);
	portflags(changing_input, PORT_REGISTERED|PORT_PIN);
.sp
.fi
.R
If you do not do this, and an input changes late in a clock period,
part of your circuit's state
machine may see the change, and part of it may miss the change, resulting
in undefined behaviour.
The state machine may halt, or start executing code from two different parts
of your program at the same time.
.LP
Make sure that each thread has a different thread number, using the -T flag.
Never compile two different threads with the same -Tnnn flag, or their
internal nets may be connected together at random.
.LP
All fpgac integer variables are signed integers, and they will be sign-extended
when used in expressions.
For example:
.CW
.nf
.sp
	#pragma intbits	8
	int a, b;
	#pragma intbits 16
	int result;

	result = (a<<8) | b;
.sp
.fi
.R
does not work when b is 0xF0, because it will be sign-extended to 0xFFF0
before being or-ed with a.
Instead, use:
.CW
.nf
.sp
	#pragma intbits	8
	int a, b;
	#pragma intbits 16
	int result;

	result = (a<<8) | (b&0xFF);
.sp
.fi
.R
which works because literal constants are sign-extended only when they
are clearly negative.
.LP
However, the last fix fails when the variables are more than 31 bits
wide, because the compiler can only handle constants up to 32 bits wide.
In particular, 0xFFFFFFFF is identical to -1, and will be sign extended.
To build a larger mask, use a constant expression like ((1<<NBITS) - 1).
.SH
Known Bugs
.LP
FpgaC has not been used extensively, and probably has bugs.
If it is doing something funny, please contact us and we'll be glad to
look at it.
The bugs listed here are the ones I know about right now.
Most of them can be fixed.
.LP
Although there is no limit on the size of a variable, integer constants
are limited to 32 bits.
.LP
If the program exits from main(), the circuit will hang.
.LP
If a function changes a global variable, the rest of the circuit won't
see the change until the next clock tick.
There isn't an automatic clock tick when a function returns.
If the function changes a global variable and immediately returns, you
can't use the value of that global variable until after the next clock
tick.
.LP
There isn't strong typing for VOID, which is included for portability
and to visually comment functions without returns. Unsigned variables
are really signed, and included as an alias for signed ints to ease
portability of existing C code for the time being. Signed variables
are actually one bit wider that you might expect because of the added
sign bit.
.SH
For More Information
.LP
There are more details on the internals of the compiler in [1].
The most recent version of the compiler can be retrieved by anonymous ftp
from:
.sp
http://sourceforge.net/projects/fpgac
.sp
The fpgac World Wide Web page can be found at URL:
.sp
http://fpgac.sourceforge.net/
.SH
References
.LP
[1]
David Galloway, "The Transmogrifier C Hardware Description
Language and Compiler for FPGAs", IEEE Symposium on FPGAs for Custom Computing
Machines, Napa, California, April 1995, pp 136-144.
.LP
[2]
David Galloway, David Karchmer, Paul Chow,
David Lewis, Jonathan Rose, "The Transmogrifier: The University of Toronto
Field-Programmable System", CSRI Technical Report 306, June 1994,
available via anonymous ftp from
ftp://ftp.csri.toronto.edu/csri-technical-reports/306/.
.LP
[3]
David M. Lewis, David R. Galloway, Marcus van Ierssel, Jonathan Rose and Paul Chow,
"The Transmogrifier-2: A 1 Million Gate Rapid Prototyping System",
submitted to the 1997 ACM/SIGDA Fifth International Symposium on FPGAs,
February 1997.
.KS
.SH
A Larger Example
.LP
This program drives the 7 segment displays on the XC4000 demo board, and
makes them count from 0 to 99 repeatedly.
.CW
.nf
.sp
char seven_seg(int x:4) {
	char result;

	x = x & 0xf; result = 0;
	if(x == 0x0) result = 0xfc;
	if(x == 0x1) result = 0x60;
	if(x == 0x2) result = 0xda;
	if(x == 0x3) result = 0xf2;
	if(x == 0x4) result = 0x66;
	if(x == 0x5) result = 0xb6;
	if(x == 0x6) result = 0xbe;
	if(x == 0x7) result = 0xe0;
	if(x == 0x8) result = 0xfe;
	if(x == 0x9) result = 0xf6;
	if(x == 0xa) result = 0xee;
	if(x == 0xb) result = 0x3e;
	if(x == 0xc) result = 0x9c;
	if(x == 0xd) result = 0x7a;
	if(x == 0xe) result = 0x9e;
	if(x == 0xf) result = 0x8e;
	return(~result);
}
.KE

.KS
void delay(char n) {
	while(n != 0)
		n = n - 1;
}

void twodigit(char y) {
	char tens;
	char leftdigit, rightdigit;

#pragma	outputport(leftdigit, 37, 44, 40, 29, 35, 36, 38, 39);
#pragma	outputport(rightdigit, 41, 51, 50, 45, 46, 47, 48, 49);

	tens = 0;
	while(y >= 10) {
		tens++;
		y -= 10;
	}
	leftdigit = seven_seg(tens);
	rightdigit = seven_seg(y);
}
.KE

.KS
main() {
	char count;
	char switches;

#pragma	inputport(switches, 28, 27, 26, 25, 24, 23, 20, 19);

	count = 0;
	while(1) {
		twodigit(count);
		count = count + 1;
		if(count >= 100)
			count = 0;
		delay(switches);
	}
}
.KE
.fi
.R
