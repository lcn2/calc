BEGIN {
    printf("#include <stdio.h>\n");
    printf("#include \"calcerr.h\"\n\n");
    printf("#include \"have_const.h\"\n\n");
    printf("/*\n");
    printf(" * names of calc error values\n");
    printf(" */\n");
    printf("CONST char *error_table[E__COUNT+2] = {\n");
    printf("    \"No error\",\n");
}
{
    print $0;
}
END {
    printf("    NULL\n");
    printf("};\n");
}
