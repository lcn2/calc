s/VALUE/int/
s/NUMBER[	 ]*\*/int /
s/NUMBER/int/
s/STRINGHEAD/int/
s/\(".*",.*,.*\),.*,.*,.*,.*,/\1, 0, 0, 0, 0,/
/sed me out/d
s/showbuiltins/main/
s/[	 ][	 ]*$//
p
