// See LICENSE for license details.

#include "encoding.h"
#include "bits.h"

  .macro save_tf
  # save gprs
  STORE  x1,1*REGBYTES(x2)
  STORE  x3,3*REGBYTES(x2)
  STORE  x4,4*REGBYTES(x2)
  STORE  x5,5*REGBYTES(x2)
  STORE  x6,6*REGBYTES(x2)
  STORE  x7,7*REGBYTES(x2)
  STORE  x8,8*REGBYTES(x2)
  STORE  x9,9*REGBYTES(x2)
  STORE  x10,10*REGBYTES(x2)
  STORE  x11,11*REGBYTES(x2)
  STORE  x12,12*REGBYTES(x2)
  STORE  x13,13*REGBYTES(x2)
  STORE  x14,14*REGBYTES(x2)
  STORE  x15,15*REGBYTES(x2)
  STORE  x16,16*REGBYTES(x2)
  STORE  x17,17*REGBYTES(x2)
  STORE  x18,18*REGBYTES(x2)
  STORE  x19,19*REGBYTES(x2)
  STORE  x20,20*REGBYTES(x2)
  STORE  x21,21*REGBYTES(x2)
  STORE  x22,22*REGBYTES(x2)
  STORE  x23,23*REGBYTES(x2)
  STORE  x24,24*REGBYTES(x2)
  STORE  x25,25*REGBYTES(x2)
  STORE  x26,26*REGBYTES(x2)
  STORE  x27,27*REGBYTES(x2)
  STORE  x28,28*REGBYTES(x2)
  STORE  x29,29*REGBYTES(x2)
  STORE  x30,30*REGBYTES(x2)
  STORE  x31,31*REGBYTES(x2)

  # get sr, epc, badvaddr, cause
  #将sscratch置0
  csrrw  t0,sscratch,x0
  #将sstatus的值保存在t0中
  csrr   s0,sstatus
  #将spec的值保存在t1中
  csrr   t1,sepc
  csrr   t2,sbadaddr
  csrr   t3,scause
  STORE  t0,2*REGBYTES(x2)
  STORE  s0,32*REGBYTES(x2)
  STORE  t1,33*REGBYTES(x2)
  STORE  t2,34*REGBYTES(x2)
  STORE  t3,35*REGBYTES(x2)

  # get faulting insn, if it wasn't a fetch-related trap
  li x5,-1
  STORE x5,36*REGBYTES(x2)
1:
  .endm
  .text
  .align 2
  .global  trap_entry
trap_entry:
  #将sp与sscratch中的值互换
  csrrw sp, sscratch, sp
  #如果sscratch中的值为0，表示其从内核态进入trap，跳转1f
  #如果不是0，则表示其从用户态进入trap，sscratch存的即为内核sp指针，则读取sscratch中的内核sp到sp寄存器
  #sp为0，则为内核态进入，将sp用sscratch还原
  #bnez sp, 1f
  bnez sp, write_stap
  csrr sp, sscratch
  addi sp,sp,-320
  save_tf
  jal 1f
  

#++
write_stap:
  addi sp,sp,-320
  save_tf
  move a0,sp
  ld t1, 37*REGBYTES(a0) # 
  csrw satp, t1 #
  sfence.vma zero, zero # 
  #++
 
  #保存栈帧
1:
  #addi sp,sp,-320
  # save_tf
  #将sp作为参数传递给中断处理函数
  move  a0,sp
  jal handle_trap


  #将sp读取到a0寄存器
  mv a0,sp
  # don't restore sscratch if trap came from kernel
  #
  #如果从是内核下的trap则直接跳转至start_user
  andi s0,s0,SSTATUS_SPP
  bnez s0,start_user
  #否则恢复内核栈，再次存储sscratch
  addi sp,sp,320
  csrw sscratch,sp
  
.globl start_user
start_user:
 # csrw satp, a1
  LOAD t0, 32*REGBYTES(a0)
  LOAD t1, 33*REGBYTES(a0)
  csrw sstatus, t0
  csrw sepc, t1


  # restore x registers
  LOAD  x1,1*REGBYTES(a0)
  LOAD  x2,2*REGBYTES(a0)
  LOAD  x3,3*REGBYTES(a0)
  LOAD  x4,4*REGBYTES(a0)
  LOAD  x5,5*REGBYTES(a0)
  LOAD  x6,6*REGBYTES(a0)
  LOAD  x7,7*REGBYTES(a0)
  LOAD  x8,8*REGBYTES(a0)
  LOAD  x9,9*REGBYTES(a0)
  LOAD  x11,11*REGBYTES(a0)
  LOAD  x12,12*REGBYTES(a0)
  LOAD  x13,13*REGBYTES(a0)
  LOAD  x14,14*REGBYTES(a0)
  LOAD  x15,15*REGBYTES(a0)
  LOAD  x16,16*REGBYTES(a0)
  LOAD  x17,17*REGBYTES(a0)
  LOAD  x18,18*REGBYTES(a0)
  LOAD  x19,19*REGBYTES(a0)
  LOAD  x20,20*REGBYTES(a0)
  LOAD  x21,21*REGBYTES(a0)
  LOAD  x22,22*REGBYTES(a0)
  LOAD  x23,23*REGBYTES(a0)
  LOAD  x24,24*REGBYTES(a0)
  LOAD  x25,25*REGBYTES(a0)
  LOAD  x26,26*REGBYTES(a0)
  LOAD  x27,27*REGBYTES(a0)
  LOAD  x28,28*REGBYTES(a0)
  LOAD  x29,29*REGBYTES(a0)
  LOAD  x30,30*REGBYTES(a0)
  LOAD  x31,31*REGBYTES(a0)
  # restore a0 last
  LOAD  x10,10*REGBYTES(a0)
  

  # gtfo
  sret

    .globl forkrets
forkrets:

     andi s0,s0,SSTATUS_SPP
     bnez s0,start_user
     move sp, a0
     csrw sptbr, a1
     sfence.vma zero, zero
     addi sp,sp,320
     csrw sscratch,sp
     j start_user

    # set stack to this new process's trapframe
    #  move sp, a0
    # addi sp,sp,320
    # csrw sscratch,sp
    # j start_user
