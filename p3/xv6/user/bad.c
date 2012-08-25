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

   assert(mprotect(start, 1) == -1);
   assert(munprotect(start, 1) == -1);

   sbrk(PGSIZE * 1);
   assert(mprotect(start, 2) == -1);
   assert(munprotect(start, 2) == -1);

   assert(mprotect(start + 1, 1) == -1);
   assert(munprotect(start + 1, 1) == -1);

   assert(mprotect(start, 0) == -1);
   assert(munprotect(start, 0) == -1);

   assert(mprotect(start, -2) == -1);
   assert(munprotect(start, -2) == -1);

   assert(mprotect(start, 1) == 0);
   assert(munprotect(start, 1) == 0);

   printf(1, "TEST PASSED\n");
   exit();
}
