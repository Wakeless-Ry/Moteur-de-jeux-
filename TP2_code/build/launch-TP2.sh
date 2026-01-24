#!/bin/sh

bindir=$(pwd)
cd /home/cleme/C++/Moteur_jeu/TP2_code/TP2

if test "x$1" = "x--debugger"; then
    shift
    echo "r"  >  $bindir/gdbscript
    echo "bt" >> $bindir/gdbscript
    gdb -batch -command=$bindir/gdbscript ./TP2
else
    ./TP2
fi
