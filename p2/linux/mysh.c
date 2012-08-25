#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

int 
chomp(char *s) {
   int returnVal = 0;   
   while(*s && *s != '\n' && *s != '\r'){
      if(*s == '>')
         returnVal = 1;          
      s++;
   }
   *s = 0;
   return returnVal;
}


char *substr(const char *pstr, int start, int numchars)
{
   char *pnew = malloc(numchars+1);
   strncpy(pnew, pstr + start, numchars);
   pnew[numchars] = '\0';
   return pnew;
}


char *trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
int checkFun(char * s1){
   int i = strlen(s1);
  
   if(s1[i-1] == 'c' && s1[i-2] == '.'){
      return 1;
   }
   return 0;
}

int check(char *s1){

while(*s1){
   if(*s1 == ' ')
      return 1;
   s1++;
}

   return 0;
}


int
equal 
(char *s1, char *s2) {
   while(*s1 && *s2){
   if(!(*s1 == *s2))
      return -1;
       s1++;s2++;
    }
    return 0;   

}
void errorExit(){
   char error_message[30] = "An error has occurred\n";
   write(STDERR_FILENO, error_message, strlen(error_message));
   //exit(0);
}

int 
main (int argc, char **argv)
{
  FILE *ifp;
   
  if(argc == 2){
      ifp = fopen(argv[1], "r");
       if (ifp == NULL) {
         errorExit();
         exit(1);
      }
   }
   else if(argc == 1){
      ifp =stdin;    
   }
   else{
      errorExit();
      exit(1);
   }


   char * exitString = "exit";
   char * pwdString = "pwd";
   char *noFile = "/no/such/file";   
   char * cdString = "cd";
   char *outFile = "/no/such/file";
   int flag = 0;
   int flag2 = 0;    
   char input[1200];      
   while(1)
   {
      flag2 = 0;
      if(argc < 2){
         write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
      }
         
      if(fgets(input,sizeof(input),ifp) == NULL){
         exit(0);
      }
      
      flag = 0;      
      // strip trailing '\n' if it exists
      int len = strlen(input)-1;
      if(input[len] == '\n') 
         input[len] = 0;
      int childPid;
      int j, i=0; // used to iterate through array
      j = 1; 
      char  *token[1200],*token1[1200]; //ustrcat (str, foo);ser input and array to hold max possible tokens, aka 80.
      if(argc == 2){         
         write(STDOUT_FILENO, input, strlen(input));
         write(STDOUT_FILENO, "\n", strlen("\n"));
      }
      
      //errorExit();
      if(strlen(input) < 1){
         exit(0);
      }
      token1[0] = strdup(strtok(input, ">")); //get pointer to first token found and store in 0
      //place in array
      if(token1[0] == NULL){
         //errorExit();
         exit(1);
      }
      while(token1[i]!= NULL) {   //ensure a pointer was found
         token1[i] = strdup(token1[i]);
         trim(token1[i]);
         i++;
         token1[i] = strtok(NULL, ">"); //continue to tokenize the string
         
      }
      strcpy(input, *token1);         
      if(token1[1] != NULL){
/*      
   temp[0] = strtok(token1[1], " "); //get pointer to first token found and store in 0
            //place in array
            i =0;
            while(temp[i]!= NULL) {   //ensure a pointer was found
               i++;
               temp[i] = strtok(NULL, " "); //continue to tokenize the string
            }
         if(temp[1]!= NULL){
            errorExit();
            flag = 1;
         }
*/
         token1[1] = trim(token1[1]);
         
         if(check(token1[1]) == 1){
            
            errorExit();            
            flag2 = 1;
         }
         else{
            outFile =token1[1];
         }         
         //printf(token1[1]);exit(0);
            
      }
      if(strlen(input)> 512){
         errorExit();
         flag2 = 1;
      }
 //     write(1, outFile, strlen(outFile));exit(0);
      if(flag2 == 0){

         token[0] = strdup(strtok(input, " ")); //get pointer to first token found and store in 0
         //place in array
         i =0;
         while(token[i]!= NULL) {   //ensure a pointer was found
            token[i] = strdup(token[i]);            
            i++;
            token[i] = strtok(NULL, " "); //continue to tokenize the string

         }
        
        // write(1, token[0], strlen(token[0]));
        // write(1, "\n", strlen("\n"));

         char * command = token[0];
         if(equal(command,exitString) ==0){
            if(token[1] != NULL){
               errorExit();
               exit(0);
            }
            exit(0);      
         }
         if(equal(command,cdString) ==0){
            if(token[1] == NULL){
               token[1] = getenv("HOME");            
            }
            
            if (chdir(token[1]) != 0)
			   { 
               errorExit();
			   }
            char pathname[1200];
            getcwd(pathname, sizeof(pathname));
         }
         else if(equal(command,pwdString) ==0){
            if(token[1] != NULL){
               errorExit();
               exit(0);
            }            
            char pathname[1200];
            getcwd(pathname, sizeof(pathname));
            write(1, pathname, strlen(pathname));
            write(1, "\n", strlen("\n"));
         }
         
         else
         {
            char *myargv[i];
            char message[1024];  
            strcpy(message,"");
            strcat(message, token[0]);
            if(checkFun(message) == 1){
              myargv[0] = strdup("gcc");
              myargv[1] = strdup(message);
              myargv[2] = strdup("-o");
              myargv[3] = strdup(substr(message,0,strlen(message) -2));
              flag = 1;            
            }
            else{            
               myargv[0] = strdup(message);
               
               
               for(j = 1; j < i; j++) {
                  myargv[j] = strdup(token[j]);
                  //printf("\n%s", myargv[j]);           
               }
            }

            myargv[i] = (char *) 0;
            for(j = 0; j < i; j++) { 
              chomp(myargv[j]);
                       
            }
            childPid = fork();
            if (childPid == 0){
               int fd;         
                  //printf("TEST %d",equal(outFile, noFile));exit(0);              
                           
               if(equal(outFile, noFile) == 0){
                  fd = STDOUT_FILENO;
               }
               else{
                  
                  outFile = trim(outFile);
                                
                  fd = open(outFile,O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                  //fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
                  if (fd < 0) {
                     errorExit();
                     exit(0);
                  }
                  if (dup2(fd, 1) == -1) {
                      errorExit();
                     //exit(0);
                  }
                 // close unused file descriptors   
                 close(fd);
               }
               
               if( execvp(myargv[0], myargv)){
                  errorExit();
                  exit(0);  
               }
            }
            else {
               int x =	wait (NULL);
               x = x+0;
               if(flag == 1){
                  char * myargv2[i];
                  myargv2[0] = myargv[3];
                  for(j = 1; j<  i; j++){
                     myargv2[j] = strdup(token[j]);
                  }
                  myargv2[i] = '\0';
                  if( execvp(myargv2[0], myargv2)){
                  errorExit();
                  exit(0);  
                  }
               }
                        
            //printf("%d", x);
            }//end else
         }//end cd string else
      }
   }//end loop
   fclose(ifp);
return 0;
   
}
