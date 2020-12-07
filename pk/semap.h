#include "wait_q.h"

typedef struct semaphore{
    intptr_t vaddr;
    int value;
    wait_queue_t wait_queue;
} semaphore_t;


#define NSEMA 10
semaphore_t sema_q[NSEMA];  

void sem_init(semaphore_t *sem, int value);
void up(semaphore_t *sem);
void down(semaphore_t *sem);
bool try_down(semaphore_t *sem);
void init_semaq();
