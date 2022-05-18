#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "mmu.h"
#include "x86.h"
#include "user.h"
#include "threads.h"


/* when using threads, must call wait() before exit() on all threads */

int
thread_create(void *(*start_routine)(void*), void *arg)
{
  void* stack = malloc(PGSIZE*2);
  int thread = clone(stack, PGSIZE*2);
  if (thread == 0) // child
  {
    start_routine(arg); //execute in child
    exit();
  }
  return thread;
}

// basic spinlock
void
lock_acquire(lock_t *lk)
{
  while(xchg(&lk->locked, 1) != 0)
    ;
}
void
lock_release(lock_t *lk)
{
  xchg(&lk->locked, 0);
}
void
lock_init(lock_t *lk)
{
  lk->locked = 0;
}

// array lock
void
lock_acquire_arr(arr_lock_t *lk, int mine, int numThreads)
{
  while(lk->locked[mine] != 1);
}

void
lock_release_arr(arr_lock_t *lk, int mine, int numThreads)
{
  xchg(&lk->locked[(mine+1)%numThreads],1);
  xchg(&lk->locked[mine], 0);
}

void
lock_init_arr(arr_lock_t *lk)
{
  lk->locked[0] = 1;
  int i;
  for(i=1;i<THREADS;++i)
    lk->locked[i] = 0;
}

void
arr_wakeup_reset(arr_lock_t *lk, int threads)
{
  int i;
  for(i=0;i<threads;i++)
    lk->locked[i]=1;
  wait();
  lock_init_arr(lk);
}

// seq lock
ulong
lock_read_seq(seq_lock_t* lk)
{
  return lk->seq;
}

void
lock_init_seq(seq_lock_t* lk)
{
  lk->seq = 0;
  lk->locked = 0;
}

void
lock_acquire_seq(seq_lock_t* lk)
{
  while (xchg(&lk->locked, 1) != 0);
  //lk->seq++;
}

void
lock_release_seq(seq_lock_t* lk, int next)
{
  xchg(&lk->locked, 0);
  //lk->seq++;
  lk->seq = next;
}

