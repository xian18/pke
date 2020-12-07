// See LICENSE for license details.
#include "pmm.h"
#include "proc.h"
#include "sched.h"
#include "pk.h"
#include "mmap.h"
#include "boot.h"
#include "elf.h"
#include "mtrap.h"
#include "frontend.h"
#include <stdbool.h>
#include "semap.h"
elf_info current;
long disabled_hart_mask;

static void handle_option(const char* s)
{
  switch (s[1])
  {
    case 's': // print cycle count upon termination
      current.cycle0 = 1;
      break;

    case 'p': // disable demand paging
      demand_paging = 0;
      break;

    default:
      panic("unrecognized option: `%c'", s[1]);
      break;
  }
}

#define MAX_ARGS 64
typedef union {
  uint64_t buf[MAX_ARGS];
  char* argv[MAX_ARGS];
} arg_buf;

static size_t parse_args(arg_buf* args)
{
  long r = frontend_syscall(SYS_getmainvars, va2pa(args), sizeof(*args), 0, 0, 0, 0, 0);
  kassert(r == 0);
  uint64_t* pk_argv = &args->buf[1];
  // pk_argv[0] is the proxy kernel itself.  skip it and any flags.
  size_t pk_argc = args->buf[0], arg = 1;
  for ( ; arg < pk_argc && *(char*)(uintptr_t)pk_argv[arg] == '-'; arg++)
    handle_option((const char*)(uintptr_t)pk_argv[arg]);

  for (size_t i = 0; arg + i < pk_argc; i++)
    args->argv[i] = (char*)(uintptr_t)pk_argv[arg + i];
  return pk_argc - arg;
}

static void init_tf(trapframe_t* tf, long pc, long sp)
{
  memset(tf, 0, sizeof(*tf));
  tf->status = (read_csr(sstatus) &~ SSTATUS_SPP &~ SSTATUS_SIE) | SSTATUS_SPIE;
  tf->gpr[2] = sp;
  tf->epc = pc;
}

static void run_loaded_program(size_t argc, char** argv, uintptr_t kstack_top)
{
  // copy phdrs to user stack
  size_t stack_top = current.stack_top - current.phdr_size;
  #define PUSH_ARG(type, value) do { \
    *((type*)sp) = (type)value; \
    sp += sizeof(type); \
  } while (0)

  #define STACK_INIT(type) do { \
    unsigned naux = sizeof(aux)/sizeof(aux[0]); \
    stack_top -= (1 + argc + 1 + envc + 1 + 2*naux) * sizeof(type); \
    stack_top &= -16; \
    long sp = stack_top; \
    PUSH_ARG(type, argc); \
    for (unsigned i = 0; i < argc; i++) \
      PUSH_ARG(type, argv[i]); \
    PUSH_ARG(type, 0); /* argv[argc] = NULL */ \
    for (unsigned i = 0; i < envc; i++) \
      PUSH_ARG(type, envp[i]); \
    PUSH_ARG(type, 0); /* envp[envc] = NULL */ \
    for (unsigned i = 0; i < naux; i++) { \
      PUSH_ARG(type, aux[i].key); \
      PUSH_ARG(type, aux[i].value); \
    } \
  } while (0)



  trapframe_t tf;
  init_tf(&tf, current.entry, stack_top);
  __clear_cache(0, 0);
  do_fork(0,stack_top,&tf);
  write_csr(sscratch, kstack_top);
}

static void rest_of_boot_loader(uintptr_t kstack_top)
{ 	
  arg_buf args;
  size_t argc = parse_args(&args);
  if (!argc)
    panic("tell me what ELF to load!");

  // load program named by argv[0]
  long phdrs[128];
  current.phdr = (uintptr_t)phdrs;
  current.phdr_size = sizeof(phdrs);
  printk("elf name %s \n",args.argv[0]);
  memset(current.file_name,0,sizeof(current.file_name));
  memcpy(current.file_name,args.argv[0],sizeof(current.file_name));
  sched_init();
  clock_init();
  proc_init();
  load_elf_user(args.argv[0], &current,(pte_t *)currentproc->upagetable);
  init_semaq();
  run_loaded_program(argc, args.argv, kstack_top);
  cpu_idle();
}

void boot_loader(uintptr_t dtb)
{
  extern char trap_entry;
  write_csr(stvec, &trap_entry);
  write_csr(sscratch, 0);
  write_csr(sie, 0);
  set_csr(sstatus, SSTATUS_SUM | SSTATUS_FS);

  file_init();
  enter_supervisor_mode(rest_of_boot_loader, pk_vm_init(), 0);
}

void boot_other_hart(uintptr_t dtb)
{
  // stall all harts besides hart 0
  while (1)
    wfi();
}
