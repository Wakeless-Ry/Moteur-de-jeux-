#!/bin/sh
bindir=$(pwd)
cd /home/cleme/C++/Master_Imagine/Moteur_jeu/TP1_code/TP1/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "x" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		GDB_COMMAND-NOTFOUND -batch -command=$bindir/gdbscript  USERFILE_COMMAND-NOTFOUND 
	else
		"USERFILE_COMMAND-NOTFOUND"  
	fi
else
	"USERFILE_COMMAND-NOTFOUND"  
fi
