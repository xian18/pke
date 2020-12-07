// See LICENSE for license details.

#include "pk.h"
#include "config.h"
#include "syscall.h"
#include "mmap.h"
#include "sched.h"
#include "vm.h"

static void handle_instruction_access_fault(trapframe_t *tf)
{
  if (handle_page_fault(tf->badvaddr, PROT_EXEC) != 0)
  {  vmprint((pte_t *)currentproc->upagetable);
    panic("Instruction access fault!");
  }

}

static void handle_load_access_fault(trapframe_t *tf)
{
  dump_tf(tf);
  panic("Load access fault!");
}

static void handle_store_access_fault(trapframe_t *tf)
{
  dump_tf(tf);
  panic("Store/AMO access fault!");
}

static void handle_illegal_instruction(trapframe_t* tf)
{
//  your code here:
//  读取tf->epc
//  根据tf->epc得到tf->insn
//  打印tf
//  修改panic
//	panic("you need add your code!");
tf->insn = *(uint16_t*)tf->epc;
  int len = insn_len(tf->insn);
  if (len == 4)
    tf->insn |= ((uint32_t)*(uint16_t*)(tf->epc + 2) << 16);
  else
    kassert(len == 2);

  dump_tf(tf);
 panic("An illegal instruction was executed!");
}

static void handle_breakpoint(trapframe_t* tf)
{
  dump_tf(tf);
  printk("Breakpoint!\n");
  tf->epc += 4;
}

static void handle_misaligned_fetch(trapframe_t* tf)
{
  dump_tf(tf);
  panic("Misaligned instruction access!");
}

static void handle_misaligned_load(trapframe_t* tf)
{
  dump_tf(tf);
  panic("Misaligned Load!");
}

static void handle_misaligned_store(trapframe_t* tf)
{
  dump_tf(tf);
  panic("Misaligned AMO!");
}

static void segfault(trapframe_t* tf, uintptr_t addr, const char* type)
{
//  your code here:
//  打印trapframe
//  判断是内核空间还是用户空间
//  修改panic信息
// 	panic("you need add your code!");
dump_tf(tf);
  const char* who = (tf->status & SSTATUS_SPP) ? "Kernel" : "User";
  panic("%s %s segfault @ %p", who, type, addr);
}

static void handle_fault_fetch(trapframe_t* tf)
{
  if (handle_page_fault(tf->badvaddr, PROT_EXEC) != 0)
    segfault(tf, tf->badvaddr, "fetch");
}

static void handle_fault_load(trapframe_t* tf)
{
  if (handle_page_fault(tf->badvaddr, PROT_READ) != 0)
    segfault(tf, tf->badvaddr, "load");
}

static void handle_fault_store(trapframe_t* tf)
{
  if (handle_page_fault(tf->badvaddr, PROT_WRITE) != 0)
    segfault(tf, tf->badvaddr, "store");
}
void
print_trapframe(trapframe_t *tf) {
    printk("  status   0x%08x\n", tf->status);
    printk("  epc      0x%08x\n", tf->epc);
    printk("  badvaddr 0x%08x\n", tf->badvaddr);
    printk("  cause    0x%08x\n", tf->cause);
}
static void handle_syscall(trapframe_t* tf)
{ 

  tf->epc += 4;

  tf->gpr[10] = do_syscall(tf->gpr[10], tf->gpr[11], tf->gpr[12], tf->gpr[13],
                           tf->gpr[14], tf->gpr[15], tf->gpr[17]);

}

static void handle_interrupt(trapframe_t* tf)
{
   
  extern volatile size_t ticks;
//  printk("tf->cause %x\n",(intptr_t)tf->cause);
   intptr_t cause = (tf->cause << 1) >> 1;
  switch (cause) {
          case IRQ_S_TIMER:
            // "All bits besides SSIP and USIP in the sip register are
            // read-only." -- privileged spec1.9.1, 4.1.4, p59
            // In fact, Call sbi_set_timer will clear STIP, or you can clear it
            // directly.
            // clear_csr(sip, SIP_STIP);
            clock_set_next_event();
            ++ticks;
         //     printk("printf next event fisished\n");
          //   printk("printf run timer time slice %d \n",currentproc->time_slice );
            if (currentproc == idleproc || currentproc->time_slice == 0) {
                currentproc->need_resched = 1;
            } else if (currentproc->time_slice > 0) {
                currentproc->time_slice--;
            }
            if (currentproc->need_resched) {
          //      printk("printf run timer schedule \n");
                schedule();
            }
            // printk("run timer finished fisished\n");
            break;
  }
  
}


static void handlr_user_timer(trapframe_t* tf)
{
  printk("handle user timer");
}

#define read_reg(reg) ({ unsigned long __tmp; \
  asm volatile ("sw " #reg ", %0": "=m"(__tmp)); \
  __tmp; })



void trapret(trapframe_t *tf,pte_t *pagetable);
void forkrets(trapframe_t *tf,pte_t * pagetable);
void handle_trap(trapframe_t* tf)
{

  if ((intptr_t)tf->cause < 0){
    handle_interrupt(tf);    
    return ;
  }
    

  typedef void (*trap_handler)(trapframe_t*);

  const static trap_handler trap_handlers[] = {
    [CAUSE_MISALIGNED_FETCH] = handle_misaligned_fetch,
    [CAUSE_FETCH_ACCESS] = handle_instruction_access_fault,
    [CAUSE_LOAD_ACCESS] = handle_load_access_fault,
    [CAUSE_STORE_ACCESS] = handle_store_access_fault,
    [CAUSE_FETCH_PAGE_FAULT] = handle_fault_fetch,
    [CAUSE_ILLEGAL_INSTRUCTION] = handle_illegal_instruction,
    [CAUSE_USER_ECALL] = handle_syscall,
    [CAUSE_BREAKPOINT] = handle_breakpoint,
    [CAUSE_MISALIGNED_LOAD] = handle_misaligned_load,
    [CAUSE_MISALIGNED_STORE] = handle_misaligned_store,
    [CAUSE_LOAD_PAGE_FAULT] = handle_fault_load,
    [CAUSE_STORE_PAGE_FAULT] = handle_fault_store,
  };

  kassert(tf->cause < ARRAY_SIZE(trap_handlers) && trap_handlers[tf->cause]);

  trap_handlers[tf->cause](tf);

  uintptr_t satp=MAKE_SATP(currentproc->upagetable);
  
  forkrets(tf,(pte_t *)satp);
}
