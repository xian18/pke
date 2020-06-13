#!/bin/sh
riscv64-unknown-elf-gcc -o app/elf/app2 app/app2.c
spike build/elf/pk app/app2
