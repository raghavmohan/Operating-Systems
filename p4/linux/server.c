#include "cs537.h"
#include "request.h"
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <float.h>
#include <limits.h>
#define WEBROOT "./webroot" // the web server's root directory

//
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
//void * producer(void * arg);
//void * consumer(void * arg);



pthread_mutex_t mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  empty   = PTHREAD_COND_INITIALIZER;
pthread_cond_t  full   = PTHREAD_COND_INITIALIZER;
//pthread_cond_t  has_space   = PTHREAD_COND_INITIALIZER;

int fill = 0;
int use = 0;
int count = 0;
int max;
int bufsize;
char policyStr[5];
int policy = -1;
server_request* structBuffer;
//void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr) ;
int compare(const void *p1, const void *p2)
{
  if(policy == FIFO){
    const server_request *elem1 = (server_request *)p1;    
    const server_request *elem2 = (server_request *)p2;

    if (elem1->time < elem2->time)
      return -1;
    else if (elem1->time >elem2->time)
      return 1;
    else
      return 0;
  }
  else{
    const server_request *elem1 = (server_request *)p1;    
    const server_request *elem2 = (server_request *)p2;

    if (elem1->size < elem2->size)
      return -1;
    else if (elem1->size >elem2->size)
      return 1;
    else
      return 0;
    
  }
}




void  producer(void * arg){
  //while(1){
  pthread_mutex_lock(&mutex);
  //while(count == max){
  while(count == bufsize){
    pthread_cond_wait(&empty, &mutex);
  }
  pthread_mutex_unlock(&mutex);
  put(* (int*) arg);
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&full);
  pthread_mutex_unlock(&mutex);
  //}
}

void * consumer(void * arg){
  //for testing values in buffer before they get taken out
  //double t1 = Time_GetSeconds();
  //while ((Time_GetSeconds() - t1) < 15.0)
  //sleep(1);
  
  while(1){
    pthread_mutex_lock(&mutex);
    while(count == 0){
      pthread_cond_wait(&full, &mutex);
    }
    server_request requestToHandle = get();
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    //REQUEST HANDLE HERE    
    requestHandle(requestToHandle);
    Close(requestToHandle.connValue);
  }
}

void put(int value){
  server_request temp = getFileStat(value);
  temp.time = Time_GetSeconds();
  temp.connValue = value;
  temp.size = temp.sbuf.st_size;
  

  structBuffer[fill] = temp;
  fill = (fill + 1) % bufsize;
  count++;
  
  qsort (structBuffer, bufsize, sizeof(server_request), compare);
  int i;
  if(DEBUG){
    printf("\nFILL %d",fill);  
    for(i = 0; i < bufsize; i++){
      printf("\nBuf[%d] Time : %f",i, structBuffer[i].time);
      printf("\nBuf[%d] Value : %d",i, structBuffer[i].connValue);
      printf("\nBuf[%d] Size : %d",i, structBuffer[i].size);
    }
  }

}

server_request get(){
  use =0;
  server_request temp = structBuffer[use];

  structBuffer[use].time = DBL_MAX;
  structBuffer[use].connValue = -1;
  structBuffer[use].size = INT_MAX;
  qsort (structBuffer, bufsize, sizeof(server_request), compare);

  count--;
  return temp;
}
//Still need to implement Policy
void getargs(int *port,
	     int *max,
	     int *loops,int argc, char *argv[])
{
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <port> <numThreads> <loops> <policy>\n", argv[0]);
    //if (argc != 4) {
    //  fprintf(stderr, "Usage: %s <port> <numThreads> <loops>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
  *max = atoi(argv[2]);
  *loops = atoi(argv[3]);
  

}

 
int main(int argc, char *argv[])
{

  int listenfd, connfd, port, clientlen, i;
  struct sockaddr_in clientaddr;
  getargs(&port,&max,&bufsize, argc, argv);
  strcpy(policyStr, argv[4]);
  
  if(strcmp(policyStr,"SFF\0")== 0 || strcmp(policyStr,"sff\0")== 0){
    policy = SFF;
  }
  else if(strcmp(policyStr,"FIFO\0")== 0 || strcmp(policyStr,"fifo\0")== 0){
    policy = FIFO;
  }
  else{
    fprintf(stderr, "Usage: %s <port> <numThreads> <loops> <policy>\n", argv[0]);
    fprintf(stderr, "Policy must be FIFO or SFF.\n");
    exit(1);
  }
  // 
  // CS537: Create some threads...
  //
  pthread_t cid[max];
  
  //be careful about GARBAGE VALS
  structBuffer = (server_request *) malloc(bufsize*sizeof(server_request));
  for(i = 0; i < bufsize; i++){
    structBuffer[i].time = DBL_MAX;
    structBuffer[i].connValue = -1;
    structBuffer[i].size = INT_MAX;
  }

  for(i = 0; i< max; i++){
    pthread_create(&cid[i], NULL, consumer, NULL);
  }
  
  listenfd = Open_listenfd(port); 
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    //handle_connection(listenfd, &clientaddr);
    // 
    // CS537: In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. However, for SFF, you may have to do a little work
    // here (e.g., a stat() on the filename) ...
    // 
   
    //CALL PRODUCER, ADD connfd to buffer
    
    pthread_mutex_lock(&mutex2);
    producer((void *)&connfd);
    pthread_mutex_unlock(&mutex2);
  }
  
  //deadcode
  for(i = 0; i < max; i++){
    pthread_join(cid[i], NULL);
  }
  printf("\nend");
  exit(0);
}

double Time_GetSeconds() {
  struct timeval t;
  int rc = gettimeofday(&t, NULL);
  assert(rc == 0);
  return (double) ((double)t.tv_sec + (double)t.tv_usec / 1e6);
}
