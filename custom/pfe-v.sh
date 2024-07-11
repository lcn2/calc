#!/usr/bin/env bash

if [[ $(which vpath) ]];then
	source "$(vpath vbash base)" # vike's path to vbash base
else
	_vit(){ read -sp	"$*…"	;env "$@";} # underscore is half legacy, half trad. private; verbose interrupt test
	_vnq(){ echo		"$*"	;env "$@";} # underscore is half legacy, half trad. private; verbose not quiet
	fi

#cspell:ignore readline dyld CALCCUSTOMHELP

(($#))||set -- "$(dirname "$0")"/pfe.cal

if((make		));then
	(_vit ~/Library/Python/3.8/bin/compile\db	make clobber all READLINE_INCLUDE=' -I/opt/local/include/readline' READLINE_LIB=' -L/opt/local/lib -l''readline');fi
if((install		));then
	installed=1
	(_vit										make	 install READLINE_INCLUDE=' -I/opt/local/include/readline' READLINE_LIB=' -L/opt/local/lib -l''readline');fi
if((installed	));then
	(_vnq CALCCUSTOMHELP=/usr/local/share/calc/						/usr/local/bin/calc -Cp "read $1;" "${@:2}")
else(_vnq CALCCUSTOMHELP=./						DYLD_LIBRARY_PATH=.				 ./calc -Cp "read $1;" "${@:2}");fi

# uses personal root /∂/
# $ (export make=0 install=0 installed=install+0 && dev=0 && unset test&&test='base' && { ((dev))&&f=-dev||{ ((dev>1))&&f=-v||f=+conf;};}&&cd1 /∂/git/calc custom/pfe-v.sh custom/pfe$f.cal "$(((dev))||([[ ${test+?} ]]&&echo ";pfe${test:+_$test}_test()"))$(((dev>1))||echo ';custom("help","pfe")')")
