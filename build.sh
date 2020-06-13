#! /bin/bash
#
if [ ! -d "build/" ];then
  mkdir build
else
  rm -r build
  mkdir build
fi
cd build
../configure --host=riscv64-unknown-elf
make
