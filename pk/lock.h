// Mutual exclusion lock.
#include "defs.h"
struct  lock {
  uint_t locked;       // Is the lock held?
  char *name;        // Name of lock.
};

