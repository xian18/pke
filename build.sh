#! /bin/bash
#
rm -r build
mkdir build
cd build
../configure --prefix=$RISCV --host=riscv64-unknown-elf
make