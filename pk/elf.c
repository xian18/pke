// See LICENSE for license details.

#include "mmap.h"
#include "pk.h"
#include "mtrap.h"
#include "boot.h"
#include "bits.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <string.h>

/**
 * The protection flags are in the p_flags section of the program header.
 * But rather annoyingly, they are the reverse of what mmap expects.
 */
static inline int get_prot(uint32_t p_flags)
{
  int prot_x = (p_flags & PF_X) ? PROT_EXEC  : PROT_NONE;
  int prot_w = (p_flags & PF_W) ? PROT_WRITE : PROT_NONE;
  int prot_r = (p_flags & PF_R) ? PROT_READ  : PROT_NONE;

  return (prot_x | prot_w | prot_r);
}



void load_elf(const char* fn, elf_info* info)
{
	 file_t* file = file_open(fn, O_RDONLY, 0);
  if (IS_ERR_VALUE(file))
    goto fail;

  Elf_Ehdr eh;
  ssize_t ehdr_size = file_pread(file, &eh, sizeof(eh), 0);
  if (ehdr_size < (ssize_t)sizeof(eh) ||
      !(eh.e_ident[0] == '\177' && eh.e_ident[1] == 'E' &&
        eh.e_ident[2] == 'L'    && eh.e_ident[3] == 'F'))
    goto fail;

#if __riscv_xlen == 64
  assert(IS_ELF64(eh));
#else
  assert(IS_ELF32(eh));
#endif

#ifndef __riscv_compressed
  assert(!(eh.e_flags & EF_RISCV_RVC));
#endif

  size_t phdr_size = eh.e_phnum * sizeof(Elf_Phdr);
  if (phdr_size > info->phdr_size)
    goto fail;
  ssize_t ret = file_pread(file, (void*)info->phdr, phdr_size, eh.e_phoff);
  if (ret < (ssize_t)phdr_size)
    goto fail;
  info->phnum = eh.e_phnum;
  info->phent = sizeof(Elf_Phdr);
  Elf_Phdr* ph = (typeof(ph))info->phdr;

  // compute highest VA in ELF
  uintptr_t max_vaddr = 0;
  for (int i = 0; i < eh.e_phnum; i++)
    if (ph[i].p_type == PT_LOAD && ph[i].p_memsz)
      max_vaddr = MAX(max_vaddr, ph[i].p_vaddr + ph[i].p_memsz);
  max_vaddr = ROUNDUP(max_vaddr, RISCV_PGSIZE);

   // don't load dynamic linker at 0, else we can't catch NULL pointer derefs
  uintptr_t bias = 0;
  if (eh.e_type == ET_DYN)
    bias = RISCV_PGSIZE;

  info->entry = eh.e_entry + bias;
  int flags = MAP_FIXED | MAP_PRIVATE;


   extern uintptr_t first_free_paddr;

   extern elf_info current;
  uintptr_t stack_top=current.stack_top - current.phdr_size;

   pte_t* pte_stack = __walk_create(stack_top-stack_top%RISCV_PGSIZE);
   kassert(pte_stack);
   uintptr_t ppn_stack = (stack_top>>RISCV_PGSHIFT) + (first_free_paddr / RISCV_PGSIZE);
   *pte_stack = pte_create(ppn_stack, prot_to_type(PROT_READ|PROT_WRITE, 1));


   for (int i = eh.e_phnum - 1; i >= 0; i--) {
    if(ph[i].p_type == PT_LOAD && ph[i].p_memsz) {
      uintptr_t prepad = ph[i].p_vaddr % RISCV_PGSIZE;
      uintptr_t vaddr = ph[i].p_vaddr + bias;

      if (vaddr + ph[i].p_memsz > info->brk_min)
        info->brk_min = vaddr + ph[i].p_memsz;
      int flags2 = flags | (prepad ? MAP_POPULATE : 0);
      int prot = get_prot(ph[i].p_flags);

      for (uintptr_t a = vaddr-prepad; a < vaddr+ph[i].p_filesz; a += RISCV_PGSIZE)
      {

          uintptr_t vpn = a >> RISCV_PGSHIFT;

          pte_t* pte = __walk_create(a);
          kassert(pte);
          uintptr_t ppn = vpn + (first_free_paddr / RISCV_PGSIZE);

          *pte = pte_create(ppn, prot_to_type(prot|PROT_WRITE, 0));

          flush_tlb();

          size_t flen = MIN(RISCV_PGSIZE, ph[i].p_filesz -a+vaddr);
          size_t ret = file_pread(file, (void*)a, flen, a- vaddr + ph[i].p_offset);

          kassert(ret);
       *pte = pte_create(ppn, prot_to_type(prot|PROT_READ|PROT_WRITE, 1));
         }

    }
  }

  file_decref(file);
  return;

fail:
  panic("couldn't open ELF program: %s!", fn);

}

