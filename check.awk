# This awk script will print 3 lines before and after any non-blank line that
# does not begin with a number.  This allows the 'make debug' rule to remove
# all non-interest lines the the 'make check' regression output while providing
# 3 lines of context around unexpected output.
#
BEGIN {
    havebuf0=0;
    buf0=0;
    havebuf1=0;
    buf1=0;
    havebuf2=0;
    buf2=0;
    error = 0;
}

NF == 0 {
    if (error > 0) {
	if (havebuf2) {
	    print buf2;
	}
	--error;
    }
    buf2 = buf1;
    havebuf2 = havebuf1;
    buf1 = buf0;
    havebuf1 = havebuf0;
    buf0 = $0;
    havebuf0 = 1;
    next;
}

$1 ~ /^[0-9]+:/ {
    if (error > 0) {
	if (havebuf2) {
	    print buf2;
	}
	--error;
    }
    buf2 = buf1;
    havebuf2 = havebuf1;
    buf1 = buf0;
    havebuf1 = havebuf0;
    buf0 = $0;
    havebuf0 = 1;
    next;
}

{
    error = 6;
    if (havebuf2) {
	print buf2;
    }
    buf2 = buf1;
    havebuf2 = havebuf1;
    buf1 = buf0;
    havebuf1 = havebuf0;
    buf0 = $0;
    havebuf0 = 1;
    next;
}

END {
    if (error > 0 && havebuf2) {
	print buf2;
	--error;
    }
    if (error > 0 && havebuf1) {
	print buf1;
	--error;
    }
    if (error > 0 && havebuf0) {
	print buf0;
    }
    exit (error > 0);
}
