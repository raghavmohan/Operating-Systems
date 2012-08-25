#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;

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

   void *stack = malloc(PGSIZE);
   assert(stack != NULL);

   int arg = 42;
   void *arg_ptr = &arg;
   int clone_pid = clone(worker, arg_ptr, stack);
   assert(clone_pid > 0);

   void *join_stack;
   int join_pid = join(&join_stack);
   assert(join_pid == clone_pid);
   printf(1, "&join_stack: %p\n", &join_stack);
   assert(stack == join_stack);
   assert(global == 2);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   int arg = *(int*)arg_ptr;
   assert(arg == 42);
   assert(global == 1);
   global++;
   exit();
}

