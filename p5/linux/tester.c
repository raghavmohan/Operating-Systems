#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
  //when the server starts up, it has to know what machine to bind to 
  //int rc = MFS_Init("mumble-25.cs.wisc.edu", 10000);
  /*****************************************************************/
  //int crc = system("server 100000 disk");
  /*****************************************************************/
 
  int i;
  int rc = MFS_Init(getHost(), 10000);
  //rc = MFS_Shutdown();
  assert(rc == 0);
  
  char *stringToWrite = malloc(4096);
  
  for(i = 0; i < 420; i++){
    sprintf(stringToWrite + strlen(stringToWrite), "Ii is:%d\n", i);
  }
  
  char *retBuffer = malloc(4096);
  rc = MFS_Read(1, retBuffer,2);
  printf("rc: |%d|\n", rc);
  printf("retBuffer: |%s|\n", retBuffer);
  assert(strcmp(retBuffer, stringToWrite) ==0);
 
  rc = MFS_Shutdown();
  assert(rc == 0);
  printf("end");
  return 0;
}
