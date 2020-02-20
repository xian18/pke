#!/bin/sh
riscv64-unknown-elf-gcc -c print_hello.c -o print_hello.o
riscv64-unknown-elf-gcc-ar -rcs libhello.a print_hello.o
riscv64-unknown-elf-gcc -o app main.c libhello.a 
spike ../riscv-pk/build/pk app