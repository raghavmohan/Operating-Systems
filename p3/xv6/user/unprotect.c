/* null pointer dereference via exec syscall */
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)
#define PGROUNDUP(sz) ((((uint)sz)+PGSIZE-1) & ~(PGSIZE-1))

int ppid;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

int
main(int argc, char *argv[])
{
   ppid = getpid();

   char *brk = sbrk(0);
   sbrk(PGROUNDUP(brk) - (uint)brk);
   char *start = sbrk(0);

   sbrk(PGSIZE * 1);

   int i;
   // run test multiple times to catch if pg table it updated
   for (i = 0; i < 20; i++) {
      *start = i;
      assert(*start == i);
      assert(mprotect(start, 1) == 0);
      assert(*start == i);
      assert(munprotect(start, 1) == 0);
      *start = i + 100;
      assert(*start == i + 100);
   }

   printf(1, "TEST PASSED\n");
   exit();
}
