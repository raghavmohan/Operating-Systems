#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mem.h"

void handleMerges();


static void *base;			
struct free_header *head;		
int successfulInit = 0;


struct free_header {

  int size;			
  struct free_header *next;	

};


struct object_header {

  int size;			
  int test;		

};

int m_error;

int Mem_Init(int sizeOfRegion, int debug) {

  if (sizeOfRegion <= 0) {
    m_error = E_BAD_ARGS;
    return -1;
  } else if (successfulInit == 1) {
    m_error = E_BAD_ARGS;
    return -1;
  }

  // open the /dev/zero device
  int fd = open("/dev/zero", O_RDWR);
	
  // need to see if its divisible and returns a whole number
  int pageSize = getpagesize();
  int addTo = pageSize - (sizeOfRegion % pageSize);
  int newSize = sizeOfRegion;
  if((sizeOfRegion%pageSize) != 0)
    newSize += addTo;
        
        
  //int newSize = sizeOfRegion+addTo;

  // size (in bytes) needs to be evenly divisble by the page size
  base = mmap(NULL, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (base == MAP_FAILED || base == NULL) {
    m_error = E_BAD_ARGS;
    return -1;	
  }

  head = base;
  head->size = newSize;
  head->next = NULL;

  // close the device
  close(fd);

  // set flag saying the call was successful
  successfulInit = 1;
  return 0;
}



void *Mem_Alloc(int size) {
  
  //printf("HEAD %p\n", head);
  if (size <= 0) {
    m_error = E_NO_SPACE;
    return 0;
  }

  // figure out how to round up to multiple of 4
  size = size + sizeof(struct object_header);
  int newSize = size;
  int modAdjust = size % 8;
  if(modAdjust != 0){
    newSize  = newSize +(8- modAdjust);
  }
  if (successfulInit == 0) {
    m_error = E_NO_SPACE;
    return 0;
  }

  struct free_header *p, *prevp;
  int numItrs = 0;
  for (p = head; ; prevp = p, p = p->next) {
    /*OUTLINE CASES:
    // IF P has Space
    create newNode
    //   IF P HAVE EXACTLY NEEDED
    //   
    //   ELSE P HAS MORE THAN NEEDED
    CHOP UP
    Store p->next in temp
                  
              
    */
    int compareSize = (p->size);
    if (compareSize >= newSize) {

      if (compareSize == newSize) {
        struct object_header *obj = (struct object_header*)p;
				
        if (p == head) {

          struct free_header *moveHeadTo = head->next;
          head = moveHeadTo;
        } else {

          prevp->next = p->next;
        }

        obj->size = newSize;
       
        void *returnedPointer = (void*)obj + sizeof(struct object_header);
        return returnedPointer;
				

      } else {
        int leftover = p->size - newSize;
        leftover = leftover - sizeof(struct object_header);
        if(leftover <= 0){
           struct object_header *obj = (struct object_header*)p;
				
        if (p == head) {

          struct free_header *moveHeadTo = head->next;
          head = moveHeadTo;
        } else {

          prevp->next = p->next;
        }

        obj->size = newSize;
       
        void *returnedPointer = (void*)obj + sizeof(struct object_header);
        return returnedPointer;
        }
        struct object_header *obj = (struct object_header*)p;

        struct free_header *new_free_header = (void*)p + newSize;


        new_free_header->size = p->size - newSize;
        new_free_header->next = p->next;
				
        if (p == head) {

          head = new_free_header;
        } else {

          prevp->next = new_free_header;
        }
				
        obj->size = newSize;
       
                               
       
        void *returnedPointer = (void*)obj + sizeof(struct object_header);
        return returnedPointer;
      }
    }

    if ((p->next) == NULL) {


      m_error = E_NO_SPACE; 
      return 0;
    }
          

    numItrs++;
  }
}



Mem_Free(void *ptr, int coalesce) {
  
     /*   printf("SIZE OF%d: \n", sizeof(struct object_header) );
     printPointer(ptr);
     if(head< ptr)
     printf("DIFF PTR: %d\n", ptr - (void*)head);
     else
     printf("DIFF PTR: %d\n",(void*)head -ptr);
  */  
  int local_var_size;

  if (ptr == NULL) {
    m_error = E_BAD_ARGS;
    return 0;
  }
	
  if (ptr < (void*)head) {
    //    printf("PTR LESS THAN HEAD\n");
	
    struct object_header *obj = (void*)ptr - sizeof(struct object_header);
    // printf("SIZE of PTR: %d\n", obj->size);

   
    local_var_size = obj->size;
    struct free_header *new_free = (struct free_header*)obj;
    //new_free->size = local_var_size + sizeof(struct object_header);
    new_free->size = local_var_size;
    struct free_header *temp = head;
    head = new_free;
    head->next = temp;
    //new_free->next = head;
    //head = new_free;
    if(coalesce> 0)
      handleMerges();
    return 0;

  } else {


    struct object_header *obj = (void*)ptr - sizeof(struct object_header);

	

   


    struct free_header *p, *prevp, *before, *after;
	
    for (p = head; ; prevp = p, p = p->next) {
      if(p > obj){
        local_var_size = obj->size;
        struct free_header *new_free2 = (struct free_header*)obj;

        new_free2->size = local_var_size;
        struct free_header *temp2 = p;
        p = new_free2;
        p->next = temp2;
        prevp->next = p;
        if(coalesce> 0)
          handleMerges(); 
        return 0;
                            
      }
      else{
        if(p->next == NULL){
          local_var_size = obj->size;
          struct free_header *new_free3 = (struct free_header*)obj;
          new_free3->size = local_var_size;
          struct free_header *temp3 = p;
          p = new_free3;
          p->next = temp3;
          prevp->next = p;
          if(coalesce> 0)
            handleMerges();
          return 0;
        }
                            
      }
    }
  }
  return -1;
}


void handleMerges() {
   struct free_header *p, *prevp, *before, *after, *temp;	
  for (p = head; ; prevp = p) {
    if(p->next == NULL){
   
      return;
    }
    temp = (void *)p + p->size;
   
    if(temp == p->next){      
      int nextSize = (int)p->next->size;
      p->size += nextSize;
      
      p->next = p->next->next;
      
      
    }
    else
      p = p->next;
  }
  
}  



void Mem_Dump() {

  struct free_header *curr;

  printf("MEM_DUMP-------------------start\n");
  for (curr = head; ; curr = curr->next) {
    printf("Addr: %d (%08x)\n", (int)((void *)curr - base), (unsigned) curr);
    printf("Size: %d\n", curr->size);

    if (curr->next == NULL) {
      printf("Next Addr: %x\n", curr->next);
      break;
    } else {
      printf("Next Addr: %x\n", curr->next);
    }

  }
  printf("MEM_DUMP-------------------end\n");

}
void printPointer(void * printPointer){
  printf("\nAddr: %d (%08x)\n", (int)((void *)printPointer - base), (unsigned) printPointer);

}
