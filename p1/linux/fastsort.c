#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/stat.h>
#include <sys/types.h>
#define RECORDSIZE (4096)

void
usage(char *prog) 
{
   fprintf(stderr, "Usage: fastsort -i inputfile -o outputfile\n");
    //fprintf(stderr, "usage: %s -i input file -o output file\n", prog);
    exit(1);
}


int compare(const void *p1, const void *p2)
{
    const rec_t *elem1 = p1;    
    const rec_t *elem2 = p2;

   if ( (int)elem1->key < (int)elem2->key)
      return -1;
   else if ((int)elem1->key > (int)elem2->key)
      return 1;
   else
      return 0;
}

int
main(int argc, char *argv[])
{
   int i = 0;

   if(argc != 5){
      usage(argv[0]);
   }
   char *inFile = "/no/such/file";
   char *outFile = "/no/such/file";

   // input params
   int c;
   opterr = 0;
   while ((c = getopt(argc, argv, "i:o:")) != -1) {      
      switch (c) {
         case 'i':
            inFile = strdup(optarg);

            break;
         case 'o':
            outFile = strdup(optarg);
            break;
         default:
            usage(argv[0]);
      }
   }
   if(inFile == "/no/such/file"){
      fprintf(stderr, "Usage: fastsort -i inputfile -o outputfile\n");
      exit(1);
   }
   // open and create output file
   int fd = open(inFile, O_RDONLY);
   if (fd < 0) {
     fprintf(stderr, "Error: Cannot open file %s\n", inFile);   
   //fprintf(stderr, "Usage: fastsort -i inputfile -o outputfile\n");
      exit(1);
   }

   rec_t r;
   

   struct stat file_status;
   if(stat(inFile, &file_status) != 0){
      fprintf(stderr, "Error: Cannot stat file %s\n", inFile);
            //perror("Error: Cannot open file foo");
      exit(1);
   }
   int fileSize = (int) file_status.st_size;
   int numRecords = fileSize/RECORDSIZE;   

   printf("File Size:%d", fileSize);
   printf("Number of Records:%d", numRecords);
   rec_t * records = malloc(numRecords*sizeof(rec_t));
   //rec_t records[numRecords];      
   int counter = 0;
   while (1) {	
      int rc;
      rc = read(fd, &r, sizeof(rec_t));
      if (rc == 0) // 0 indicates EOF
          break;
      if (rc < 0) {
          perror("read");
          exit(1);
      }
      records[counter] = r;
      counter++;
    }
   
   (void) close(fd);
   qsort (records, numRecords, sizeof(rec_t), compare);

   fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
   if (fd < 0) {
	   fprintf(stderr, "Error: Cannot open file %s\n", outFile);
      exit(1);
   }
   
   rec_t rTemp;
   int n;
   for (n=0; n<numRecords; n++){
      rTemp = records[n];      
      int rc;
      rc = write(fd, &rTemp, sizeof(rec_t));
    }

   (void) close(fd);

   return 0;
}


