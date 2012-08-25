// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  void * addr;
  int y = 1;
  addr = &y;
  int len = 5;
  int x = mprotect(addr, len);
  x = x+1;
  return 0;
}
