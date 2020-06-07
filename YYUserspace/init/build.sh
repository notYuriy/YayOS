#!/bin/bash

nasm -o init.o init.s -f elf64
~/opt/cross/bin/x86_64-elf-ld -o fsroot/bin/init init.o