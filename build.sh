export PREFIX="$HOME/opt/cross"
export CPP="$PREFIX/bin/x86_64-elf-g++"
export ASM="nasm"
export LD="$PREFIX/bin/x86_64-elf-g++"

mkdir obj
mkdir isotree

for cppSource in src/*.cpp
do
    echo "Compiling $cppSource..."
    objectPath="obj/cpp_$(basename "$cppSource" .c).o"

    $CPP -c -o $objectPath $cppSource -Iinclude\
    -ffreestanding -Wall -Wextra -O2 -mno-sse -mno-sse2 -mno-sse3\
    -mcmodel=large -mno-red-zone -fno-exceptions -fno-rtti\
    -fno-asynchronous-unwind-tables -mno-red-zone -Wno-attributes || exit
done

for asmSource in src/*.s
do
    echo "Assembling $asmSource..."
    objectPath="obj/asm_$(basename "$asmSource" .s).o"
    $ASM -f elf64 $asmSource -o $objectPath || exit
done

echo "Linking kernel object file..."
$LD -n -T ld/linker.ld obj/* -o obj/kernel.bin -ffreestanding -O2 -lgcc -nostdlib

echo "Creating iso tree..."
mkdir isotree/boot
mkdir isotree/boot/grub
cp grub/grub.cfg isotree/boot/grub/grub.cfg
cp obj/kernel.bin isotree/boot/kernel.bin

echo "Creating OS bootable image..."
grub-mkrescue isotree -o YaYaOS.iso 2>/dev/null

echo "Cleaning up..."
rm -rf obj
rm -rf isotree

echo -e "Done\a"