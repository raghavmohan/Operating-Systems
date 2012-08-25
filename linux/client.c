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
 
  
  int rc = MFS_Init(getHost(), 10000);
  //rc = MFS_Shutdown();
  assert(rc == 0);
  
  rc = MFS_Creat(0, 1, "test\0");
  assert(rc == 0);
  
  
  
  rc = MFS_Creat(0, 0, "test2\0");
  assert(rc == 0);
  
  char *sendBuffer = malloc(4096);
  sprintf(sendBuffer, "START BLOCK 1");
  //  rc = MFS_Write(1, "hello\0",0);
  rc = MFS_Write(1, sendBuffer,0);
  assert(rc == 0);
  /*
  char *sendBuffer = malloc(4096);
  sprintf(sendBuffer, "START BLOCK 1");

  rc = MFS_Lookup(0, "test\0");
  printf("rc: |%d|\n", rc);
  

  rc = MFS_Lookup(0, "test2\0");
  printf("rc: |%d|\n", rc);
  */
  
  char *retBuffer = malloc(4096);
  rc = MFS_Read(1, retBuffer,0);
  printf("rc: |%d|\n", rc);
  printf("retBuffer: |%s|\n", retBuffer);
  
  assert(strcmp(retBuffer, sendBuffer) ==0);
 int i; 
 for(i =0; i < MFS_BLOCK_SIZE; i++){
   assert(retBuffer[i] == sendBuffer[i]);
 }
  
  char stringToWrite[8192];
 
  for(i = 0; i < 420; i++){
    sprintf(stringToWrite + strlen(stringToWrite), "Ii is:%d\n", i);
  }
  printf("stringToWrite: |%s|\n", stringToWrite);
  rc = MFS_Write(1, stringToWrite,2);
  assert(rc == 0);
   
  
  //char *retBuffer = malloc(4096);
  
  rc = MFS_Read(1, retBuffer,2);
  
  printf("rc: |%d|\n", rc);
  printf("retBuffer: |%s|\n", retBuffer);
  
  
  assert(strcmp(retBuffer, stringToWrite) ==0);
 
  //assert(rc == 0);
  //rc = MFS_Unlink(0, "test\0");
 // rc = MFS_Unlink(0, "test2\0");
 //rc = MFS_Unlink(0, "test\0");
 //printf("RC:%d\n", rc);
  rc = MFS_Shutdown();
  printf("end");

  //int crc = system("server 100000 disk");
  /*
  rc = MFS_Read(1, retBuffer,2);
  assert(strcmp(retBuffer, stringToWrite) ==0);
  
  printf("rc: |%d|\n", rc);
  printf("retBuffer: |%s|\n", retBuffer);
  
  */
  
  return 0;
}


