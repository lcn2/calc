s/VALUE/int/
s/NUMBER[	 ]*\*/int /
s/NUMBER/int/
s/STRINGHEAD/int/
s/\(".*",.*,.*\),.*,.*,.*,.*,/\1, 0, 0, 0, 0,/
s/[	 ][	 ]*$//
p
