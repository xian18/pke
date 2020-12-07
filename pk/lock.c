#include "lock.h"

//初始化互斥锁
void
initlock(struct lock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
}



