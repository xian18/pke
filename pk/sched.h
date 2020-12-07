#ifndef __KERN_SCHEDULE_SCHED_H__
#define __KERN_SCHEDULE_SCHED_H__

#include "proc.h"
#include "defs.h"


void schedule(void);
void wakeup_proc(struct proc_struct *proc);

#endif /* !__KERN_SCHEDULE_SCHED_H__ */




void vPortSetupTimer( void );
void clock_init(void);
void clock_set_next_event(void) ;
static void prvSetNextTimerInterrupt( void );
void sched_init(void);
void run_timer_list(void);

static uint64_t time_interval = 1e7 / 100;
static uint64_t timebase = 1e7 / 100 ;


struct sched_class {
    // the name of sched_class
    const char *name;
    // Init the run queue
    void (*init)(struct run_queue *rq);
    // put the proc into runqueue, and this function must be called with rq_lock
    void (*enqueue)(struct run_queue *rq, struct proc_struct *proc);
    // get the proc out runqueue, and this function must be called with rq_lock
    void (*dequeue)(struct run_queue *rq, struct proc_struct *proc);
    // choose the next runnable task
    struct proc_struct *(*pick_next)(struct run_queue *rq);
    // dealer of the time-tick
    void (*proc_tick)(struct run_queue *rq, struct proc_struct *proc);
    /* for SMP support in the future
     *  load_balance
     *     void (*load_balance)(struct rq* rq);
     *  get some proc from this rq, used in load_balance,
     *  return value is the num of gotten proc
     *  int (*get_proc)(struct rq* rq, struct proc* procs_moved[]);
     */
};

struct run_queue {
    list_entry_t run_list;
    unsigned int proc_num;
    int max_time_slice;
    // For LAB6 ONLY
 //   skew_heap_entry_t *lab6_run_pool;
};

typedef struct {
    unsigned int expires;       //the expire time
    struct proc_struct *proc;   //the proc wait in this timer. If the expire time is end, then this proc will be scheduled
    list_entry_t timer_link;    //the timer list
} pke_timer_t;

#define le2timer(le, member)   to_struct((le), pke_timer_t, member)

