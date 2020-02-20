#!/bin/sh
riscv64-unknown-elf-gcc -c app/print_hello.c -o app/print_hello.o
riscv64-unknown-elf-gcc-ar -rcs app/libhello.a app/print_hello.o
riscv64-unknown-elf-gcc -o app/app app/main.c app/libhello.a 
spike riscv-pk/build/pk app/app