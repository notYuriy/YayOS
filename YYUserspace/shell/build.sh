#!/bin/bash

export PREFIX="$HOME/opt/cross"
export CPP="$PREFIX/bin/x86_64-elf-g++"
export ASM="nasm"
export LD="$PREFIX/bin/x86_64-elf-g++"
export CPPFLAGS="-ffreestanding -funroll-loops -O2 -mno-red-zone -fno-exceptions -fno-rtti"

$ASM -o init.o init.s -f elf64
$CPP -o shell.o shell.cpp -c $CPPFLAGS
$LD -o fsroot/Binaries/init init.o shell.o -T link.ld  -ffreestanding -O2 -lgcc -nostdlib