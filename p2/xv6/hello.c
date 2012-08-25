#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "param.h"
#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}

int
spin() {
   while (1) {
      int i, j;
      for (i = 0; i < 10000000; i++) {
         j = i % 11;
      }
   }
}

int
main(int argc, char *argv[])
{
   int pids[2];
   int ppid = getpid();
   int r, i, j;

   pids[0] = fork();
   if (pids[0] == 0) {
      r = settickets(2);
      if (r != 0) {
         printf(1, "settickets failed");
         kill(ppid);
      }
      spin();
   }

   pids[1] = fork();
   if (pids[1] == 0) {
      r = settickets(4);
      if (r != 0) {
         printf(1, "settickets failed");
         kill(ppid);
      }
      spin();
   }

   sleep(1000);

   int ticks[] = {-1, -1};
   struct pstat st;
   check(getpinfo(&st) == 0, "getpinfo");

   for(i = 0; i < NPROC; i++) {
      for(j = 0; j < 2; j++) {
         if (st.inuse[i] && st.pid[i] == pids[j]) {
            ticks[j] = st.ticks[i];
         }
      }
   }

   for (i = 0; i < 2; i++) {
      kill(pids[i]);
      wait();
   }

   for (i = 0; i < 2; i++) {
      check(ticks[i] > 0, "each process should have some ticks");
   }

   float ratio = (float)ticks[0] / (float)ticks[1];
   printf(1, "ticks 0: %d\n", ticks[0]);
   printf(1, "ticks 1: %d\n", ticks[1]);
   check(ratio > .25 && ratio < .75, "ratio should be about 1/2");

   printf(1, "TEST PASSED\n");
   exit();
}
