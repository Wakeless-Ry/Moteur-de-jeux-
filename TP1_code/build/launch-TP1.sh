#!/bin/sh

bindir=$(pwd)
cd /home/cleme/C++/Master_Imagine/Moteur_jeu/TP1_code/TP1/

if test "x$1" = "x--debugger"; then
    shift
    echo "r"  >  $bindir/gdbscript
    echo "bt" >> $bindir/gdbscript
    gdb -batch -command=$bindir/gdbscript ./TP1
else
    ./TP1
fi
