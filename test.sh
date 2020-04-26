#!/bin/bash

#use this when debugging triple faults
#qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -d int

#use this when running ubuntu
x-terminal-emulator -e "qemu-system-x86_64 -cdrom YayOS.iso -m 2G -no-reboot -nographic"