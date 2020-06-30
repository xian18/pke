// See LICENSE for license details.
#include "vm.h"
#ifndef _MMAP_H
#define _MMAP_H

#include "vm.h"
#include "syscall.h"
#include "encoding.h"
#include "file.h"
#include "bits.h"
#include "mtrap.h"
#include <stddef.h>

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_PRIVATE 0x2
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_POPULATE 0x8000
#define MREMAP_FIXED 0x2

#define KSTACKPAGE	1                           // # of pages in kernel stack 内核每次alloc一个页面，stacksize暂定一个页面吧
#define KSTACKSIZE	(KSTACKPAGE * RISCV_PGSIZE)       // sizeof kernel stack

extern int demand_paging;
uintptr_t pk_vm_init();
int handle_page_fault(uintptr_t vaddr, int prot);
void populate_mapping(const void* start, size_t size, int prot);
void __map_kernel_range(uintptr_t va, uintptr_t pa, size_t len, int prot);
int __valid_user_range(uintptr_t vaddr, size_t len);
uintptr_t __do_mmap(uintptr_t addr, size_t length, int prot, int flags, file_t* file, off_t offset);
uintptr_t do_mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, off_t offset);
int do_munmap(uintptr_t addr, size_t length);
uintptr_t do_mremap(uintptr_t addr, size_t old_size, size_t new_size, int flags);
uintptr_t do_mprotect(uintptr_t addr, size_t length, int prot);
uintptr_t do_brk(uintptr_t addr);
uintptr_t __page_alloc();

pte_t* __walk_create(uintptr_t addr);
pte_t prot_to_type(int prot, int user);

 #define va2pa(va) ({ uintptr_t __va = (uintptr_t)(va); \
   extern uintptr_t first_free_paddr; \
     __va >= DRAM_BASE ? __va : __va + first_free_paddr; })


pte_t* __walk(uintptr_t addr);
#define va2pa_unfixed(va) ({    pte_t* pte = __walk((uintptr_t)va);  \
  uintptr_t pnn= ((uintptr_t)(*pte)) >> PTE_PPN_SHIFT; \
 uintptr_t pa = (pnn << RISCV_PGSHIFT) | ((uintptr_t)va&0xfff) ; \
	 uintptr_t pa1 = va2pa(va); \
	 (uintptr_t) va>=DRAM_BASE ? pa1 :pa ;  })

#endif
