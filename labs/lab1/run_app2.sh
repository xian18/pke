#!/bin/sh
riscv64-unknown-elf-gcc -o app/app2 app/app2.c
spike riscv-pk/build/pk app/app2