#!/bin/bash

nasm -o init.o forktest.s -f elf64
~/opt/cross/bin/x86_64-elf-ld -o fsroot/bin/init init.o