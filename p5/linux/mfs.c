#include "mfs.h"
#include "udp.h"


#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

struct sockaddr_in saddr;
int sd;
int port;
struct timeval tv;
//fd_set rfds;
//struct timeval tv;
int retval;
fd_set rfds;

int MFS_Init(char *hostname, int portIn){
  //-1 lets the OS choose a free port  
  port = portIn;
  sd = UDP_Open(-1);
  assert(sd > -1);

  int rc = UDP_FillSockAddr(&saddr, hostname, port);
  assert(rc == 0);
  return rc;
}

int MFS_Lookup(int pinum, char *name){
  int rc;
  msg_t message;
  message.inum =-1;
  message.block =-1;
  message.pinum = pinum;
  message.msgType = LOOKUP;
  message.type = -1;
  memcpy(message.name, name, sizeof(message.name));
  // memcpy(message.buffer, "\0", sizeof(message.buffwe));
  //sprintf(message.name, name); 
  sprintf(message.buffer, "nothing");
  message.returnVal = -1; 
  

  retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);

    if(retval){
      if (rc > 0) {
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }
  return message.returnVal;
}

int MFS_Stat(int inum, MFS_Stat_t *m){
  //TODO
   int rc;
  msg_t message;
  message.inum =inum;
  message.block =-1;
  message.pinum = -1;
  message.msgType = STAT;
  message.type = -1;
  sprintf(message.name, "nothing"); 
  sprintf(message.buffer, "nothing");
  message.returnVal = -1; 

  retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);
    //}

    //  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    if(retval){
      if (rc > 0) {
    
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }
  m->size = message.stat.size;
  m->type = message.stat.type;
 
  return message.returnVal;
}

int MFS_Write(int inum, char *bufferIn, int block){
  int rc;
  msg_t message;
  message.inum =inum;
  message.block =block;
  message.pinum = -1;
  message.msgType = WRITE;
  message.type = -1;
  sprintf(message.name, "nothing"); 
  memcpy(message.buffer, bufferIn, 4096);
  message.returnVal = -1; 

  /*
  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
  if (rc > 0) {
    struct sockaddr_in raddr;
     rc= UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
  }
  */
  retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);
    //}

    //  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    if(retval){
      if (rc > 0) {
    
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }

  return message.returnVal;
}

int MFS_Read(int inum, char *bufferIn, int block){
  int rc;
  msg_t message;
  message.inum =inum;
  message.block =block;
  message.pinum = -1;
  message.msgType = READ;
  message.type = -1;
  sprintf(message.name, "nothing"); 
  sprintf(message.buffer, bufferIn);
  message.returnVal = -1; 
  /*
  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
  if (rc > 0) {
    struct sockaddr_in raddr;
    rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
 
  }
  */
  retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);
    //}

    //  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    if(retval){
      if (rc > 0) {
    
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }

  //memcpy(bufferIn, message.buffer, sizeof(message.buffer));
  memcpy(bufferIn, message.buffer, 4096);
  memcpy(bufferIn, message.buffer, MFS_BLOCK_SIZE);
  return message.returnVal;
}

int MFS_Creat(int pinum, int type, char *name){
  int rc;
  msg_t message;
  message.inum =-1;
  message.block =-1;
  message.pinum = pinum;
  message.msgType = CREATE;
  message.type = type;
  sprintf(message.name, name); 
  sprintf(message.buffer, "nothing");
  message.returnVal = -1; 
  /*
  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
  if (rc > 0) {
    struct sockaddr_in raddr;
    rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
  }
  */
  retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);
    //}

    //  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    if(retval){
      if (rc > 0) {
    
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }
  return message.returnVal;
}

int MFS_Unlink(int pinum, char *name){
  int rc;
  msg_t message;
  message.inum =-1;
  message.block =-1;
  message.pinum = pinum;
  message.msgType = UNLINK;
  message.type = -1;
  sprintf(message.name, name); 
  sprintf(message.buffer, "nothing");
  message.returnVal = -1; 

  /*
  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
  if (rc > 0) {
    struct sockaddr_in raddr;
    rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t));  
  }
    return message.returnVal;
  */
 retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);
    //}

    //  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    if(retval){
      if (rc > 0) {
    
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }
  return message.returnVal;

  /*
  retval = 0;
  while(retval == 0){
    rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(sd + 1, &rfds, NULL, NULL, &tv);
    //}

    //  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
    if(retval){
      if (rc > 0) {
    
	struct sockaddr_in raddr;
	rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t)); 
      }
    }
  }
    return message.returnVal;
  */



}

int MFS_Shutdown(){
  int rc;
  msg_t message;
  message.inum =-1;
  message.block =-1;
  message.pinum = -1;
  message.msgType = SHUTDOWN;
  message.type = -1;
  sprintf(message.name, "nothing"); 
  sprintf(message.buffer, "nothing");
  message.returnVal = 0; 


  rc = UDP_Write(sd, &saddr,(char *) &message, sizeof(msg_t));
  if (rc > 0) {
    struct sockaddr_in raddr;
    rc = UDP_Read(sd, &raddr,(char *) &message, sizeof(msg_t));  
  }
  return 0;
}

char * getHost(){
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  return strdup(hostname);
}

void printMessageParams(msg_t message){
  char msgTypes[8][128] = {
  "READ\0",
  "LOOKUP\0",
  "STAT\0",
  "WRITE\0",
  "CREATE\0",
  "UNLINK\0",
 "SHUTDOWN\0",
  "INIT\0"
  };
  printf("START PRINT MESSAGE\n") ;
  printf("message.inum =%d\n",message.inum) ;
  printf("message.block =%d\n",message.block) ;
  printf("message.pinum =%d\n",message.pinum) ;
  printf("message.msgType =|%s|\n",msgTypes[message.msgType]) ;
  printf("message.name =|%s|\n",message.name) ;
  printf("message.buffer =|%s|\n",message.buffer) ;
  printf("message.returnVal=%d\n",message.returnVal) ; 
}

