typedef unsigned long ulong;

typedef struct lock_t { // (\\cs202) lab2
  volatile uint locked;
} lock_t;

#define CACHELINE 32
#define THREADS 50
typedef struct arr_lock_t {
  volatile uint locked[THREADS];
} arr_lock_t;

typedef struct seq_lock_t {
  volatile uint locked;
  volatile ulong seq;
} seq_lock_t;

typedef struct qnode {
  volatile void *next;
  volatile char locked;
  char __pad[0] __attribute__((aligned(CACHELINE)));
} qnode;

typedef struct {
  struct qnode *v __attribute__((aligned(64)));
  int lock_idx __attribute__((aligned(64)));
} mcslock_t;


int thread_create(void*(*)(void*), void*);
void lock_init(lock_t*);
void lock_acquire(lock_t*);
void lock_release(lock_t*);
void lock_init_arr(arr_lock_t*);
void lock_acquire_arr(arr_lock_t*, int, int);
void lock_release_arr(arr_lock_t*, int, int);
void arr_wakeup_reset(arr_lock_t*, int);
void lock_init_seq(seq_lock_t*);
void lock_acquire_seq(seq_lock_t*);
void lock_release_seq(seq_lock_t*, int);
ulong lock_read_seq(seq_lock_t*);

/*
 * MCS locks, adapted from https://pdos.csail.mit.edu/6.828/2011/lec/scalable-lock-code.c
 */

static inline long 
fetch_and_store(mcslock_t *L, long val)
{
  __asm__ volatile(
   		"lock; xchgl %0, %1\n\t"
                : "+m" (L->v), "+r" (val)
                :
                : "memory", "cc");
  return val;
}

static inline long
cmp_and_swap(mcslock_t *L, long cmpval, long newval)
{
  long out;
  __asm__ volatile(
                "lock; cmpxchgl %2, %1"
                : "=a" (out), "+m" (L->v)
                : "q" (newval), "0"(cmpval)
                : "cc");
  return out == cmpval;
}

static inline void
mcs_init(mcslock_t *l)
{
  l->v = NULL;
}

static inline void
mcs_lock(mcslock_t *L, volatile struct qnode *mynode)
{
  struct qnode *predecessor;
  mynode->next = NULL;
  predecessor = (struct qnode *)fetch_and_store(L, (long)mynode);
  if (predecessor) {
    mynode->locked = 1;
    asm volatile("":::"memory");
    predecessor->next = mynode;
    while (mynode->locked)
      asm volatile("pause");
    }
}

static inline void
mcs_unlock(mcslock_t *L, volatile struct qnode *mynode)
{
  if (!mynode->next) {
    if (cmp_and_swap(L, (long)mynode, 0))
      return;
    while (!mynode->next)
      asm volatile("pause");
    }
  ((struct qnode *)mynode->next)->locked = 0;
}
