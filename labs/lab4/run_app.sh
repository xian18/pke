#!/bin/sh
riscv64-unknown-elf-gcc -o app/app app/get_mem_size.c
spike riscv-pk/build/pk app/app