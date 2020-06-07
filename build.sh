#!/bin/bash

export PREFIX="$HOME/opt/cross"
export CPP="$PREFIX/bin/x86_64-elf-g++"
export ASM="nasm"
export LD="$PREFIX/bin/x86_64-elf-g++"

function cleanup {
    local error_code="$?"

    test "$error_code" == 0 && echo -e -n "\033[1;32mDone. "; 
    test "$error_code" == 0 || echo -e -n "\033[1;31mError ($error_code). ";
    echo -e "Cleaning up\033[0m"
    echo -e "\033[1;32m[ RM ]\033[0m \033[1;37misotree\033[0m"
    rm -rf isotree 
    echo -e "\033[1;32m[ RM ]\033[0m \033[1;37mobj\033[0m"
    rm -rf YYSloth/obj 
    return;
}

trap cleanup EXIT

mkdir YYSloth/obj
mkdir isotree

cppPaths="eval find YYSloth/src -type f -name '*.cpp'"
for cppSource in $($cppPaths)
do
    echo -e "\033[1;31m[ CC ]\033[0m \033[1;37m$cppSource\033[0m"
    objectPath="YYSloth/obj/cpp_$(basename "$cppSource" .c).o"

    $CPP -c -o $objectPath $cppSource -IYYSloth/include\
    -ffreestanding -funroll-loops -Wall -Wextra -Werror -O2 -mno-sse -mno-sse2 -mno-sse3\
    -mcmodel=large -mno-red-zone -fno-exceptions -fno-rtti\
    -mno-red-zone -Wno-attributes -Wno-nonnull-compare -g || exit
done

asmPaths="eval find YYSloth/src -type f -name '*.s'"
for asmSource in $($asmPaths)
do
    echo  -e "\033[1;33m[ AS ]\033[0m \033[1;37m$asmSource\033[0m"
    objectPath="YYSloth/obj/asm_$(basename "$asmSource" .s).o"
    $ASM -f elf64 $asmSource "-IYYSloth/include" -o  $objectPath -w-number-overflow || exit
done

echo -e "\033[1;33m[ LD ]\033[0m \033[1;37mobj/kernel.bin\033[0m"
$LD -n -T YYSloth/ld/linker.ld YYSloth/obj/* -o YYSloth/kernel.bin -ffreestanding -O2 -lgcc -nostdlib || exit

mkdir isotree/boot || exit
mkdir isotree/boot/grub || exit
cp grub/grub.cfg isotree/boot/grub/grub.cfg || exit
cp YYSloth/kernel.bin isotree/boot/kernel.bin || exit

echo -e "\033[1;32m[ RD ]\033[0m \033[1;37minitrd\033[0m"
tar -cf isotree/initrd.img initrd/ --format=v7 || exit

echo -e "\033[1;32m[ BL ]\033[0m \033[1;37mgrub/grub.cfg\033[0m"
grub-mkrescue isotree -o YayOS.iso 2> /dev/null || exit

true