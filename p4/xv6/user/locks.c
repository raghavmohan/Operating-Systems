#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
lock_t lock;
int num_threads = 30;
int loops = 50000;


#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   int arg = 101;
   void *arg_ptr = &arg;

   lock_init(&lock);

   int i;
   for (i = 0; i < num_threads; i++) {
      int thread_pid = thread_create(worker, arg_ptr);
      assert(thread_pid > 0);
   }

   for (i = 0; i < num_threads; i++) {
      int join_pid = thread_join();
      assert(join_pid > 0);
   }
   printf(1, "Global: %d\n", global);
   printf(1, "num_threads * loops: %d\n", (num_threads * loops));
   assert(global == num_threads * loops);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
  printf(1, "In worker\n");
   int arg = *(int*)arg_ptr;
   assert(arg == 101);
   int i;
   for (i = 0; i < loops; i++) {
     lock_acquire(&lock);
      //printf(1, "in lock\n");
      global++;
      lock_release(&lock);
   }
   exit();
}

