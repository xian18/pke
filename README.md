# pke
Proxy-Kernel for Education LAB1 Branch

bulid step:
$ cd riscv-pk
$ mkdir build
$ cd build
$ ../configure --prefix=$RISCV --host=riscv64-unknown-elf
$ make
$ make install

usage:
$ cd app
$ spike ../riscv-pk/build/pk  elf_file


