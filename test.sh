#!/bin/bash

echo -e "\033[1;34m[ EM ]\033[0m\033[1;37m Testing kernel...\033[0m"

#use this when debugging triple faults
#qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -d int -nographic

#use this when debugging mem contents
qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -monitor stdio -d int

#use for running os with no specific goal
#xterm -e "qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -nographic" > /dev/null 2> /dev/null

#use this for debugging
#qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -s -nographic