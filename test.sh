#!/bin/bash
#qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -nographic
x-terminal-emulator -e "qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -nographic" 2> /dev/null