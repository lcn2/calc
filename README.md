# TL;DR Try calc

## TL;DR Install calc

```
misc linux: sudo yum install calc
on Debian:  sudo apt install calc
on RHEL:    sudo dnf install calc
on Ubuntu:  sudo apt install calc
via Termux: apt install calc
via src:    sudo make clobber all chk instsll
```

## TL;DR Run calc

```
misc shell:   calc
via bash:     calc
via misc app: launch calc via Termux
via zsh:      calc
```

# What is calc?

Calc is an interactive calculator which provides for easy large
numeric calculations, but which also can be easily programmed
for difficult or long calculations.	 It can accept a command line
argument, in which case it executes that single command and exits.
Otherwise, it enters interactive mode.  In this mode, it accepts
commands one at a time, processes them, and displays the answers.
In the simplest case, commands are simply expressions which are
evaluated.  For example, the following line can be input:

```sh
3 * (4 + 1)
```

and the calculator will print:

```sh
15
```

Calc has the usual collection of arithmetic operators +, -, /, * as
well as ^ (exponentiation), % (modulus) and // (integer divide).
For example:

```sh
3 * 19^43 - 1
```

will produce:

```sh
29075426613099201338473141505176993450849249622191102976
```

Notice that calc values can be very large.  For example:

```sh
2^23209-1
```

will print:

```sh
402874115778988778181873329071 ... many digits ... 3779264511
```

The special '.' symbol (called dot), represents the result of the
last command expression, if any.  This is of great use when a series
of partial results are calculated, or when the output mode is changed
and the last result needs to be redisplayed.  For example, the above
result can be modified by typing:

```sh
. % (2^127-1)
```

and the calculator will print:

```sh
47385033654019111249345128555354223304
```

For more complex calculations, variables can be used to save the
intermediate results.  For example, the result of adding 7 to the
previous result can be saved by typing:

```sh
curds = 15
whey = 7 + 2*curds
```

Functions can be used in expressions.  There are a great number of
pre-defined functions.  For example, the following will calculate
the factorial of the value of 'whey':

```sh
fact(whey)
```

and the calculator prints:

```sh
13763753091226345046315979581580902400000000
```

The calculator also knows about complex numbers, so that typing:

```sh
(2+3i) * (4-3i)
cos(.)
```

will print:

```sh
17+6i
-55.50474777265624667147+193.9265235748927986537i
```

The calculator can calculate transcendental functions, and accept and
display numbers in real or exponential format. For example, typing:

```sh
config("display", 70),
epsilon(1e-70),
sin(1)
```

prints:

```sh
0.8414709848078965066525023216302989996225630607983710656727517099919104
```

Calc can output values in terms of fractions, octal or hexadecimal.
For example:

```sh
config("mode", "fraction"),
(17/19)^23
print
base(16),
(19/17)^29
print
log(79.3i)
```

 will print:

```sh
19967568900859523802559065713/257829627945307727248226067259

0x9201e65bdbb801eaf403f657efcf863/0x5cd2e2a01291ffd73bee6aa7dcf7d1

0x17b5164ac24ee836bf/0xc7b7a8e3ef5fcf752+0x883eaf5adadd26be3/0xc7b7a8e3ef5fcf752i
```

All numbers are represented as fractions with arbitrarily large
numerators and denominators which are always reduced to lowest terms.
Real or exponential format numbers can be input and are converted
to the equivalent fraction.  Hex, binary, or octal numbers can be
input by using numbers with leading '0x', '0b' or '0' characters.
Complex numbers can be input using a trailing 'i', as in '2+3i'.
Strings and characters are input by using single or double quotes.

Commands are statements in a C-like language, where each input
line is treated as the body of a procedure.  Thus the command
line can contain variable declarations, expressions, labels,
conditional tests, and loops.  Assignments to any variable name
will automatically define that name as a global variable.  The
other important thing to know is that all non-assignment expressions
which are evaluated are automatically printed.  Thus, you can evaluate
an expression's value by simply typing it in.

Many useful built-in mathematical functions are available.  Use the:

```sh
help builtin
```

command to list them.

You can also define your own functions by using the 'define' keyword,
followed by a function declaration very similar to C.

```sh
define f2(n)
{
	local ans;

	ans = 1;
	while (n > 1)
		ans *= (n -= 2);
	return ans;
}
```

Thus the input:

```sh
f2(79)
```

will produce:

```sh
1009847364737869270905302433221592504062302663202724609375
```

Functions which only need to return a simple expression can be defined
using an equals sign, as in the example:

```sh
define sc(a,b) = a^3 + b^3
```

Thus the input:

```sh
sc(31, 61)
```

will produce:

```sh
256772
```

Variables in functions can be defined as either 'global', 'local',
or 'static'.  Global variables are common to all functions and the
command line, whereas local variables are unique to each function
level, and are destroyed when the function returns.  Static variables
are scoped within single input files, or within functions, and are
never destroyed.  Variables are not typed at definition time, but
dynamically change as they are used.

For more information about the calc language and features, try:

```sh
help overview
```

Calc has a help command that will produce information about
every builtin function, command as well as a number of other
aspects of calc usage.  Try the command:

```sh
help help
```

for and overview of the help system.  The command:

```sh
help builtin
```

provides information on built-in mathematical functions, whereas:

```sh
help asinh
```

will provides information a specific function.  The following
help files:

```sh
help command
help define
help operator
help statement
help variable
```

provide a good overview of the calc language.  If you are familiar
with C, you should also try:

```sh
help unexpected
```

It contains information about differences between C and calc
that may surprise C programmers.
