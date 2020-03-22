#! /bin/bash
#
rm -r build
mkdir build
cd build
../configure --host=riscv64-unknown-elf
make
echo "build riscv-pk success!"