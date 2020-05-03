#!/bin/bash

#use this when debugging triple faults
#qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -d int

#use this when debugging ram contents
qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -monitor stdio

#use for running os with no specific goal
#xterm -e "qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -nographic"