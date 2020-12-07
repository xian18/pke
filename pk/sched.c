#include <list.h>
//s#include <sync.h>
#include "proc.h"
#include "sched.h"
#include "atomic.h"
#include "intr.h"
#include "mmap.h"
#include "frontend.h"

static spinlock_t vm_lock = SPINLOCK_INIT;








static void prvSetNextTimerInterrupt(void)
{
    volatile uint64_t* mtime =   (uint64_t*)(CLINT_BASE + 0xbff8);
    volatile uint64_t* timecmp=  (uint64_t*)(CLINT_BASE + 0x4000);

    if (mtime && timecmp) 
        *timecmp = *mtime + time_interval;
}
/*-----------------------------------------------------------*/

/* Sets and enable the timer interrupt */
void vPortSetupTimer(void)
{
    printm("set up timmer\n");
    /* reuse existing routine */
    prvSetNextTimerInterrupt();

	/* Enable timer interupt */
	__asm volatile("csrs sie,%0"::"r"(MIP_STIP));
}

/*---------------------------------------------------------------------------*/
// the list of timer
static list_entry_t timer_list;

#define SBI_SET_TIMER 0
static struct sched_class *sched_class;
static struct run_queue *rq;


/*  time irq  set */
volatile size_t ticks;

#define SBI_CALL_0(which) SBI_CALL(which, 0, 0, 0)
#define SBI_CALL_1(which, arg0) SBI_CALL(which, arg0, 0, 0)
#define SBI_CALL_2(which, arg0, arg1) SBI_CALL(which, arg0, arg1, 0)

static inline uint64_t get_cycles(void) {
  
    uint64_t n;
    __asm__ __volatile__("rdtime %0" : "=r"(n));
  //  printm("++ get circle %d\n",n);
    return n;
}


//which 0
//arg0 stime_value
//arg1 0
//arg2 0
#define SBI_CALL(which, arg0, arg1, arg2) ({                    \
        register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0);   \
        register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1);   \
        register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2);   \
        register uintptr_t a7 asm ("a7") = (uintptr_t)(which);  \
        asm volatile ("ecall"                                   \
                      : "+r" (a0)                               \
                      : "r" (a1), "r" (a2), "r" (a7)            \
                      : "memory");                              \
        a0;                                                     \
})

static inline void sbi_set_timer(uint64_t stime_value)
{
#if __riscv_xlen == 32
        SBI_CALL_2(SBI_SET_TIMER, stime_value, stime_value >> 32);
#else
        SBI_CALL_1(SBI_SET_TIMER, stime_value);
#endif
}


void clock_set_next_event(void) { sbi_set_timer(get_cycles() + timebase); }
void clock_init(void) {
    // divided by 500 when using Spike(2MHz)
    // divided by 100 when using QEMU(10MHz)
    clock_set_next_event();
    set_csr(sie, MIP_STIP);

    // initialize time counter 'ticks' to zero
    ticks = 0;

    printm("++ setup timer interrupts\n");
}



/*-----------------default------------------------------*/
static void
RR_init(struct run_queue *rq) {
    list_init(&(rq->run_list));
    rq->proc_num = 0;
}

static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    assert(list_empty(&(proc->run_link)));
    list_add_before(&(rq->run_list), &(proc->run_link));
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;
    }
    proc->rq = rq;
    rq->proc_num ++;
}

static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
//    assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
    list_del_init(&(proc->run_link));
    rq->proc_num --;
}

static struct proc_struct *
RR_pick_next(struct run_queue *rq) {
    list_entry_t *le = list_next(&(rq->run_list));
    if (le != &(rq->run_list)) {
        return le2proc(le, run_link);
    }
    return NULL;
}

static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;
    }
}

struct sched_class default_sched_class = {
    .name = "RR_scheduler",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .pick_next = RR_pick_next,
    .proc_tick = RR_proc_tick,
};


/*   timer */
static struct run_queue __rq;


// del timer from timer_list
void
del_timer(pke_timer_t *timer) {
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        if (!list_empty(&(timer->timer_link))) {
            if (timer->expires != 0) {
                list_entry_t *le = list_next(&(timer->timer_link));
                if (le != &timer_list) {
                    pke_timer_t *next = le2timer(le, timer_link);
                    next->expires += timer->expires;
                }
            }
            list_del_init(&(timer->timer_link));
        }
    }
    local_intr_restore(intr_flag);
}




/* ----sehcdule class */

void
sched_init(void) {
    list_init(&timer_list);

    sched_class = &default_sched_class;

    rq = &__rq;
    rq->max_time_slice = 5;
    sched_class->init(rq);

    printk("sched class: %s\n", sched_class->name);

}

static inline void
sched_class_enqueue(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->enqueue(rq, proc);
    }
}

void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE);
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        if (proc->state != PROC_RUNNABLE) {
            proc->state = PROC_RUNNABLE;
            proc->wait_state = 0;
            if (proc != currentproc) {
                sched_class_enqueue(proc);
            }
        }
        else {
            printk("wakeup runnable process.\n");
        }
    }
    local_intr_restore(intr_flag);
}

static void
sched_class_proc_tick(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->proc_tick(rq, proc);
    }
    else {
        proc->need_resched = 1;
    }
}

// call scheduler to update tick related info, and check the timer is expired? If expired, then wakup proc
void
run_timer_list(void) {
    printk("5_2 run_timer_list 1\n");
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        list_entry_t *le = list_next(&timer_list);
        if (le != &timer_list) {
              printk("5_2 run_timer_list 1\n");
            printk("in timer scheduler");
            pke_timer_t *timer = le2timer(le, timer_link);
            assert(timer->expires != 0);
            timer->expires --;
            while (timer->expires == 0) {
                le = list_next(le);
                struct proc_struct *proc = timer->proc;
                if (proc->wait_state != 0) {
                    assert(proc->wait_state & WT_INTERRUPTED);
                }
                else {
                    printk("process %d's wait_state == 0.\n", proc->pid);
                }
                wakeup_proc(proc);
                del_timer(timer);
                if (le == &timer_list) {
                    break;
                }
                timer = le2timer(le, timer_link);
            }
        }
        sched_class_proc_tick(currentproc);
    }
    local_intr_restore(intr_flag);
}

static inline struct proc_struct *
sched_class_pick_next(void) {
    return sched_class->pick_next(rq);
}

static inline void
sched_class_dequeue(struct proc_struct *proc) {
    sched_class->dequeue(rq, proc);
}

static inline void printRQ(struct run_queue *rq){
    list_entry_t *le = list_next(&(rq->run_list));
    while(le != &(rq->run_list)) {
        int pid= le2proc(le, run_link)->pid;
        printk(" %d ",pid);
        le=list_next(le);
    }
}
void
schedule(void) {
   // printk("in schedule\n");
    bool intr_flag;
    struct proc_struct *next;
    local_intr_save(intr_flag);
    {    
      //  printk("RQ 1:");
      // printRQ(rq);
      // printk("\n");
        currentproc->need_resched = 0;
        if (currentproc->state == PROC_RUNNABLE) {
            sched_class_enqueue(currentproc);
        }
        // printk("RQ 2:");
        // printRQ(rq);
        // printk("\n");
        if ((next = sched_class_pick_next()) != NULL) {
            sched_class_dequeue(next);
        }

        // printk("RQ 3:");
        // printRQ(rq);
        // printk("\n");
        if (next == NULL) {
            next = idleproc;
            shutdown(0);
        }
        //   printk("Run proc: %d\n",next->pid);
        next->runs ++;
        if (next != currentproc) {
            proc_run(next);
        }
    }
    local_intr_restore(intr_flag);
}

