Summary: C-style arbitrary precision calculator
Name: calc
Version: 2.11.5
Release: 4.5
Copyright: LGPL
Group: Applications/Engineering
Source: http://www.isthe.com/chongo/src/calc/calc-2.11.5t4.5.tar.gz
Patch0: rpm.mk.patch
BuildRoot: /var/tmp/%{name}-buildroot

%description
Calc is an interactive calculator which provides for easy large numeric
calculations, but which also can be easily programmed for difficult or
long calculations.

All numbers are represented as fractions with arbitrarily large numerators
and denominators which are always reduced to lowest terms.  Real or
exponential format numbers are converted to the equivalent fraction.
One use enter decinal, hex, octal, binary and complex values.

Commands are statements in a C-like language, where each input line is
treated as the body of a procedure.  You can define your own functions
by using the 'define' keyword, followed by a function declaration very
similar to C.  Calc also comes with a rich set of builtin functions
and calc shell commands.

In addition to numeric global, local and static variables, Calc as
lists, associated arrays, matrices, byte blocks, dymanic strings and
user defined objects.

%prep
%setup -q

%build
make -f Makefile.linux all RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
make -f Makefile.linux install RPM_OPT_FLAGS="$RPM_OPT_FLAGS" T="$RPM_BUILD_ROOT"

%clean
rm -rf $RPM_BUILD_ROOT

%files -f install.list
%dir /usr/share/calc
%dir /usr/share/calc/help
%dir /usr/include/calc
%dir /usr/share/calc/custom
%dir /usr/share/calc/custhelp
%dir /usr/include/calc/custom
%dir /usr/bin/cscript
%defattr(-,root,root)
%doc BUGS README COPYING COPYING-LGPL HOWTO.INSTALL
%doc README LIBRARY README.WINDOWS calc.1
%doc help/resource help/errorcodes help/custom_cal help/new_custom
%doc help/cscript help/full

%changelog
* Fri Jun 08 2001 Landon Curt Noll <http://www.isthe.com/chongo/index.html>
- calc version 2.11.5t4.5
