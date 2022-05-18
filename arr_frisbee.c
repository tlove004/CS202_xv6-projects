#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "threads.h"
#include "mmu.h"

int throws = 0;
int threads = 0;
volatile int start = 0;
volatile int next = 0;
arr_lock_t lock;
int *players;
int *pos;

typedef struct _args {
  int token;
  int throws;
} _args;

void *frisbee(void *args) {
  _args *a = (_args*)args;
  while (!start) // condition variable serializes the creation of all threads prior to frisbee game
    ;
  while (a->throws < throws) {
    lock_acquire_arr(&lock, pos[getpid()], threads);
    // next is the player who should have the token this round
    int mine;
    for (mine = 0; mine < threads; ++mine)
      if (getpid() == *(players + mine % threads)) break;
    if (a->throws < throws && *(players + mine % threads) == getpid()) {
      a->throws++;
      printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n", a->throws, getpid(), *(players + (mine+1) % threads));
    }
    if (a->throws == throws)
      arr_wakeup_reset(&lock, threads);
   lock_release_arr(&lock, mine, threads);
  }
  start--;
  return 0;
}

int
main(int argc, char *argv[])
{
  int pid;
  struct _args *args = (_args*)malloc(sizeof(_args));
  start = 0;
  lock_init_arr(&lock);

  if(argc != 3) {
    threads = 3;
    throws = 3;
  } else {
    threads = atoi(argv[1]);
    throws = atoi(argv[2]);
  }
  args->throws = 0;

  // to store thread ids
  int p[throws];
  int ps[getpid()+throws*2];
  pos = ps;
  players = p;

  int i;
  for(i=0; i<threads; ++i) {
    pid = thread_create(frisbee, (void*) &args);
    if(pid) {
      p[i] = pid; // store pid of children
      ps[pid]=i;
    }
  }

  int time = 0;
  next = 0;
  //next = p[0];
  start = threads;
  time = uptime();
  //wait for threads to finish
  while(start)
    ;
  for(i = 0; i < threads; i++) {
    wait();
  }

  printf(1, "Simulation of Frisbee game has finished, %d rounds were played in total!\n", throws);
  printf(1, "total time: %d\n", uptime()-time);
  exit();
  return 0;
}
