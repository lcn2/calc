BEGIN {
    ebase = 10000;
    printf("#define E__BASE %d\t/* calc errors start above here */\n\n", ebase);
}
NF > 1 {
    if (length($1) > 7) {
	printf("#define %s\t", $1, NR);
    } else {
	printf("#define %s\t\t", $1, NR);
    }
    printf("%d\t/* ", ebase+NR);
    for (i=2; i < NF; ++i) {
	printf("%s ", $i);
    }
    printf("%s */\n", $NF);
}
END {
    printf("\n#define E__HIGHEST\t%d\t/* highest calc error */\n", NR+ebase);
    printf("#define E__COUNT\t\t%d\t/* number of calc errors */\n", NR);
    printf("#define E_USERDEF\t20000\t/* base of user defined errors */\n\n");
    printf("/* names of calc error values */\n");
}
