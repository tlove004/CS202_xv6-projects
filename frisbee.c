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
lock_t lock;
int *players;

typedef struct _args {
  int token;
  int throws;
} _args;

void *frisbee(void *args) {
  _args *a = (_args*)args;
  while (start < threads) // condition variable serializes the creation of all threads prior to frisbee game
    ;
  while (a->throws < throws) {
    lock_acquire(&lock);
    // next is the player who should have the token this round
    if (a->throws < throws && next == getpid()) {
      a->throws++;
      next = *(players + a->throws % threads); //get who's up next
      printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n", a->throws, getpid(), next);
    }
    lock_release(&lock);
    wait();
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
  lock_init(&lock);

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
  players = p;

  int i;
  for(i=0; i<threads; ++i) {
    pid = thread_create(frisbee, (void*) &args);
    if(pid) {
      p[i] = pid; // store pid of children
    }
  }

  int time = 0;
  next = p[0];
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
