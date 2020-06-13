#!/bin/sh
riscv64-unknown-elf-gcc -o app/elf/app1 app/app1.c
spike build/pk app/elf/app1
