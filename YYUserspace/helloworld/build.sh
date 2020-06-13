#!/bin/bash

nasm -o helloworld.o helloworld.s -f elf64
~/opt/cross/bin/x86_64-elf-ld -o fsroot/Binaries/helloworld helloworld.o