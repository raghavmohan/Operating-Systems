// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


void test1();

void test1(){
    printf(1, "hello from test");
    //exit();
}

void test2();

void test2(void* y){
    printf(1, "hello from test2\n");
    printf(1, "hello from test22\n");
    printf(1, "hello from test23\n");
    printf(1, "y: %d\n",*(int*) y);
    /*
    uint z;
    z = 5-6;
    printf(1, "z: %d\n",(uint) z);
    */
    exit();
}


int
main(int argc, char *argv[])
{
  /* int x = clone();
  x = x+1;
  printf(1, "x is %d", x);
  */
  //printf(1, "&test2: %p\n", &test2);
  int y = 5;
  thread_create(&test2, (void *)&y);
  thread_join();
  //wait();
  printf(1, "hello from main\n");
  exit();
}
