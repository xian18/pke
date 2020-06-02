#!/bin/sh
riscv64-unknown-elf-gcc -o app/app1 app/app1.c
spike build/pk app/app1
