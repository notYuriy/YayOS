#!/bin/bash

nasm -o uname.o uname.s -f elf64
x86_64-elf-ld -o fsroot/Binaries/uname uname.o
