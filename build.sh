#!/bin/bash

export PREFIX="$HOME/opt/cross"
export CPP="$PREFIX/bin/x86_64-elf-g++"
export ASM="nasm"
export LD="$PREFIX/bin/x86_64-elf-g++"

mkdir obj
mkdir isotree

cppPaths="eval find . -type f -name '*.cpp'"
for cppSource in $($cppPaths)
do
    echo "[ CC ] $cppSource"
    objectPath="obj/cpp_$(basename "$cppSource" .c).o"

    $CPP -c -o $objectPath $cppSource -Iinclude\
    -ffreestanding -funroll-loops -Wall -Wextra -O2 -mno-sse -mno-sse2 -mno-sse3\
    -mcmodel=large -mno-red-zone -fno-exceptions -fno-rtti\
    -mno-red-zone -Wno-attributes || exit
done

asmPaths="eval find . -type f -name '*.s'"
for asmSource in $($asmPaths)
do
    echo "[ AS ] $asmSource"
    objectPath="obj/asm_$(basename "$asmSource" .s).o"
    $ASM -f elf64 $asmSource -o $objectPath -w-number-overflow || exit
done

echo "[ LD ] obj/kernel.bin"
$LD -n -T ld/linker.ld obj/* -o obj/kernel.bin -ffreestanding -O2 -lgcc -nostdlib

mkdir isotree/boot
mkdir isotree/boot/grub
cp grub/grub.cfg isotree/boot/grub/grub.cfg
cp obj/kernel.bin isotree/boot/kernel.bin

echo "[ BL ] grub/grub.cfg"
grub-mkrescue isotree -o YayOS.iso 2>/dev/null

echo "[ RM ] obj"
rm -rf obj
echo "[ RM ] isotree"
rm -rf isotree