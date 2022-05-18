#include "types.h"
#include "user.h"
#include "stat.h"
#include "threads.h"

typedef struct _args {
  lock_t * lk;
} _args;

void *foo(void *args)
{
   struct _args *arg = args;
   printf(1, "passed %d\n", arg->lk);
   return 0;
}

int main()
{
  struct _args *args;
  lock_t lock;
  lock_init(&lock);
  args->lk = &lock;
  int i = thread_create(foo, (void*) &args);
  printf(1, "%d\tdone\n", i);
  int j = 0;
  while (j++ < 15000) {
    int k = 0;
    while (k++ < 15000)
    	asm("nop");
  }
  printf(1, "%d exiting, %d\n", i, j);
 
  if (i != 0)
    wait();
  exit();
  return -1;
}
