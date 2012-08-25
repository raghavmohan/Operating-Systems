#include <stdio.h>
#include "udp.h"
#include "mfs.h"


#define BUFFER_SIZE (4096)

//functions to put into a different file
void handleMessage(char * message);

//server functions that need to made into a library
int serverInit(char * MFS_image);

int serverLookup(int pinum, char * name);

int serverCreate(int pinum, int type, char * name);

int serverUnlink(int pinum, char * name);

int serverWrite(int inum, char *buffer, int block);

int serverRead(const int inum, char *buffer, int block);

int serverStat(const int inum, MFS_Stat_t * m);

int serverShutdown();
//print functions for debugging purposes
void printDisk();

void printInode(const MFS_Inode * inodeToPrint );

void printDirBlock(const MFS_Dir_Block * dirBlockToPrint);

void printInodeMap(int numItemsToPrint);

//helper functions for various reasons
int stringSize(const char * stringIn);

int createInode(int pinum, int type);

int createImapPiece();

int deleteInode(int inum);

int deleteImapPiece(int imapPieceIndex);

int memLoad();

void gen_random(char *s, const int len,const int seed);

//Define Globals that are always in memory

//global socket descriptor
int sd;

struct sockaddr_in s;
char file[256];

//disk file descriptor
size_t diskFD;

//checkpoint region
MFS_Ckpt ckptregion;

//need to keep in memory
//all of the inode chunky chunks
MFS_Inode_Map inodeMap;


int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(stderr,"usage: server <portnum> <filename>\n");
    exit(0);
  }
  
  int port = atoi(argv[1]);

  
  sprintf(file, argv[2]);

  sd = UDP_Open(port);
  assert(sd > -1);
  
  serverInit(file);
  memLoad();
  printf("waiting in loop\n");
  while (1) {
    //use this to get the socket address
    struct sockaddr_in y;
    s = y;
    char buffer[sizeof(msg_t)];
    int rc = UDP_Read(sd, &s, buffer, sizeof(msg_t));
 
    //figure out what kind of message this is - read, write, lookup etc
    //when done go ahead and reply to it
	
    if (rc > 0) {
      // char reply[BUFFER_SIZE];
      //sprintf(reply, "reply");
      handleMessage (buffer);
      //need a message struct casted as a char []
      rc =  UDP_Write(sd, &s, buffer, sizeof(msg_t));
    }
  }

  return 0;
}

int serverInit(char * MFS_image){
  //this binds the server to port 100000
  //sd = UDP_Open(10000);
  // assert(sd > -1);

  int sentinel, i = 0;

  //first open disk
  diskFD = open(MFS_image,O_RDWR);
    
  //if disk doesn't exist, create it
  if((int)diskFD < 0){
    diskFD = open(MFS_image, O_RDWR| O_CREAT| O_TRUNC, S_IRWXU);
 
    
    //create checkpoint region and write it
    //MFS_Ckpt ckptregion;
      
    //this ideally needs to be done with lseek at the end, but this is the easiest way to do it
    ckptregion.endLog = sizeof(ckptregion)+ sizeof(MFS_Imap_Piece) +sizeof(MFS_Inode)+sizeof(MFS_Dir_Block);
    for(i = 0; i < 256; i++ ){
      //may need this calculation later
      ckptregion.imapPiecePtrs[i] = -1;
    }
      
    /* Disk Image implemented as:                                                      
       | CR | Imap p1 | Inode1| Data1 | Inode2 | Data2 | ...| Imap p2 | Inode1 | Data1 |... |<-endLog
    */
      
    //point the first piece of the imap to the end of the log
    ckptregion.imapPiecePtrs[0] = sizeof(ckptregion);  
    
    lseek(diskFD, 0, 0);
    write(diskFD, &ckptregion, sizeof(ckptregion));
       
    //create and write root data, inode, imap
    MFS_Imap_Piece imapP1;
    for(i = 0; i < 16; i++)
      imapP1.imapPtrs[i] = -1; 
       
       
    imapP1.imapPtrs[0] = sizeof(ckptregion)+sizeof(imapP1); 
        
    //check all good before write
    if(DEBUG){
      printf("imap1.imapPtrs[0]: %d\n", imapP1.imapPtrs[0]);
      printf("imap1.imapPtrs[1]: %d\n", imapP1.imapPtrs[1]);
    }
       
    write(diskFD, &imapP1, sizeof(imapP1));
       
    MFS_Inode root_inode;
    root_inode.stats.type = 0;
    for(i = 0; i < 14; i++)
      root_inode.blockPtrs[i] = -1;
       
    //point this to the first datablock, right next to the inode
    root_inode.blockPtrs[0] = sizeof(ckptregion)+sizeof(imapP1)+sizeof(root_inode) ; 
    //root_inode.stats.size = MFS_BLOCK_SIZE; 
    root_inode.stats.size = 2*MFS_BLOCK_SIZE; 
       
       
    write(diskFD, &root_inode, sizeof(root_inode));
       
    // This is the first Directory Block
    MFS_Dir_Block rootDirBlock;      
   
    //prepare rootDirBlock
    sentinel = sizeof(rootDirBlock)/sizeof(rootDirBlock.dirEntries[0]);
    for(i = 0; i < sentinel; i ++){
      rootDirBlock.dirEntries[i].inum= -1;
      memcpy(rootDirBlock.dirEntries[i].name, "\0", sizeof("\0"));
    }
    
    //these are the first two directory entries into the rootDirBlock
   
    memcpy(rootDirBlock.dirEntries[0].name, ".\0", sizeof(".\0"));
    rootDirBlock.dirEntries[0].inum= 0;
    
    memcpy(rootDirBlock.dirEntries[1].name, "..\0", sizeof("..\0"));
    rootDirBlock.dirEntries[1].inum=0;

    
    //write rootDirBlock
    write(diskFD, &rootDirBlock, sizeof(rootDirBlock));   
  }
  else{
    //to check all set up okay
    if(DEBUG)
      printDisk();

    //disk image exists
    //load checkpoint region into memory
    (void) read(diskFD,&ckptregion, sizeof(ckptregion));
   
    //check if this is right
    if(DEBUG){
      printf("printing ckptregion\n");
      printf("printing  ckptregion.endLog: %d \n", ckptregion.endLog);
      for(i = 0; i < 5; i++){
	printf("printing ckptregion.imapPiecePtrs[%d]: %d \n",i, ckptregion.imapPiecePtrs[i]);
      }
    }
  
  }
  //load inodeMap into memory
  for(i = 0; i < 4096; i++){
    inodeMap.inodes[i] = -1;
  }
  i =0;
  sentinel = 0;
  int j = 0; 
  MFS_Imap_Piece imapPiece;
  while(ckptregion.imapPiecePtrs[i] >= 0){
    lseek(diskFD,ckptregion.imapPiecePtrs[i], 0);
    read(diskFD,&imapPiece, sizeof(imapPiece));
    while(imapPiece.imapPtrs[sentinel] >= 0){
      inodeMap.inodes[j] = imapPiece.imapPtrs[sentinel];
      j++;sentinel++;
    }
    i++;
  }

  //check inodeMap, etc  
  if(DEBUG){
    printf("diskFD: |%d|", (int)diskFD);     
    for(i = 0; i < 10; i++){
      printf("inodeMap.inodes[%d]: %d\n",i, inodeMap.inodes[i]);
    }  
  }
  memLoad();
  //make sure you flush
  /************************************************************************/
  //fsync(diskFD); 
  /************************************************************************/
  return 0;
}


int serverLookup(int pinum, char * name){
  //check if pinum is valid
  if(pinum >4095  || pinum < 0)
    return -1;
  if(inodeMap.inodes[pinum] == -1)
    return -1;
  
 
  //check if name is valid
  if(strlen(name) < 1 || strlen(name) > 28)
    return -1;
  
  int bytesRead;
  int i,dirBlockLocation, j;
 
 int pinumLocation = inodeMap.inodes[pinum];
  //lseek to this location
  lseek(diskFD, pinumLocation, 0);
   
  //we have just lseeked to the inode, we need to read the inode
  MFS_Inode pInode;
  bytesRead = read(diskFD,&pInode, sizeof(pInode));

  
  //check if this is a directory or not
  if(pInode.stats.type != MFS_DIRECTORY){
    // close(diskFD);
    return -1;
  }

  //NEED TO LOOP THROUGH THE ENTIRE 14 blocks
  for(j = 0; j < 14; j++){
    //set up so that the directory block is right next to the inode
    dirBlockLocation = pInode.blockPtrs[j];
    //lseek too the directory block
    lseek(diskFD, dirBlockLocation, 0);
    //read the directory block
    MFS_Dir_Block dirBlock;
    bytesRead += read(diskFD,&dirBlock, sizeof(dirBlock));
    //loop through the directory block to find name
    for(i = 0; i < 128; i++){
      //string compare the names
      if(strncmp(dirBlock.dirEntries[i].name, name, 28) == 0){
	return dirBlock.dirEntries[i].inum;
      }
    }
  }
  return -1;
}

int serverCreate(int pinum, int type, char* name){
  //take care of error cases
  if((type != MFS_DIRECTORY && type != MFS_REGULAR_FILE) || pinum > 4095 || pinum < 0)
    return -1;
 
  //if name is too long return -1
  if(strlen(name) < 1 || strlen(name) > 28)
    return -1;
  
  //if pinum dne return -1
  if(inodeMap.inodes[pinum] == -1)
    return -1;

  //if file already exists, return success  
  if(serverLookup(pinum, name) >= 0)
    return 0;

  //get pinum address
  int pinumLocation = inodeMap.inodes[pinum];

  //lseek to this location
  lseek(diskFD, pinumLocation, 0);
  
  //we have just lseeked to the inode, we need to read the inode
  MFS_Inode pInode;
  int bytesRead = read(diskFD,&pInode, sizeof(pInode));
  
  //check if this is a directory or not
  if(pInode.stats.type != MFS_DIRECTORY){
    return -1;
  }
  
  if(pInode.stats.size >= (MFS_BLOCK_SIZE * 14*128)){
     return -1;
  }
  
  int newInum = createInode(pinum,type);
  
  //get the directoryBlockIndex in inode
  int inodeDirBlockIndex = (int) pInode.stats.size/(MFS_BLOCK_SIZE*128);
  
 
  //potential error case
  if(inodeDirBlockIndex < 0 ||inodeDirBlockIndex > 14){
    return -1;
  }
 
  //this is the last full index
  int dirBlockIndex = (int) (pInode.stats.size/(MFS_BLOCK_SIZE) %128);
  pInode.stats.size += MFS_BLOCK_SIZE;
 
 
  if(dirBlockIndex < 0){
    return -1;
  }

  //take care of case where directory index is full
  //need new directory block
  if(dirBlockIndex == 0){
    pInode.blockPtrs[inodeDirBlockIndex] = ckptregion.endLog;
    MFS_Dir_Block freshDirBlock;
    int k;
    for(k = 0; k < 128; k++){
      memcpy(freshDirBlock.dirEntries[k].name, "\0", sizeof("\0"));
      freshDirBlock.dirEntries[k].inum = -1;
    }
    lseek(diskFD, ckptregion.endLog, 0);
    write(diskFD, &freshDirBlock, sizeof(freshDirBlock));
    
    //update endlog and write out the checkpoint
    ckptregion.endLog += MFS_BLOCK_SIZE;
    lseek(diskFD, 0, 0);
    write(diskFD, &ckptregion, sizeof(ckptregion));   
  }
  
  //IMPORTANT
  //write out the inode here. Either way, inode needs to be written out since size is updated
  //do it here because if we need to update the block ptrs, that happens just before this
  lseek(diskFD, pinumLocation, 0);
  write(diskFD, &pInode, sizeof(pInode));
 
  memLoad();

  lseek(diskFD, pInode.blockPtrs[inodeDirBlockIndex], 0);
  MFS_Dir_Block dirBlock;
  bytesRead = read(diskFD,&dirBlock, sizeof(dirBlock));
  
  //set the name etc
  int newIndex = dirBlockIndex;
  memcpy(dirBlock.dirEntries[newIndex].name, "\0", sizeof("\0"));
  memcpy(dirBlock.dirEntries[newIndex].name, name, 28);
  dirBlock.dirEntries[newIndex].inum = newInum;
 
  //write out updated directory block to disk
  lseek(diskFD, pInode.blockPtrs[inodeDirBlockIndex], 0);
  write(diskFD, &dirBlock, sizeof(dirBlock));
  
  /************************************************************************/
  //fsync(diskFD); 
  /************************************************************************/
  memLoad();
  return 0;
}




void handleMessage(char * messageR){
  msg_t* message = (msg_t*)messageR;
  message->returnVal = -1;
  int returnVal;
  switch(message->msgType){
  case INIT:
    returnVal = serverInit(file);
    message->returnVal = returnVal;
    break;
  case READ:
    returnVal = serverRead(message->inum, message->buffer, message->block);
    message->returnVal = returnVal;
    break;
  case LOOKUP:
    returnVal = serverLookup(message->pinum, message->name);
    message->returnVal = returnVal;
    break;
  case WRITE:
    returnVal = serverWrite(message->inum, message->buffer, message->block);
    message->returnVal = returnVal;
    break;
  case STAT:
    returnVal = serverStat(message->inum, &(message->stat));
    message->returnVal = returnVal;
    break;
  case CREATE:
    returnVal = serverCreate(message->pinum, message->type, (message->name));
    message->returnVal = returnVal;
    break;
  case UNLINK:
    returnVal = serverUnlink(message->pinum, message->name);
    message->returnVal = returnVal;
    break;
  case SHUTDOWN:
    message->returnVal = 0;
    memcpy(messageR, message, sizeof(*message));
    UDP_Write(sd, &s, messageR, sizeof(msg_t));
    //fsync(diskFD);
    close(diskFD);
    // printDisk();
    exit(0);
    break;
  default:
    message->returnVal = -1;
    break;
  }
  memcpy(messageR, message, sizeof(*message));

}

//return the index of the new imap Piece
int createImapPiece(){
  
  memLoad();
  int i, newImapPieceIndex;
  for(i = 0; i < 256; i++){
    if(ckptregion.imapPiecePtrs[i] == -1){
      newImapPieceIndex = i;
      i = 5000;
    }
  }

  ckptregion.imapPiecePtrs[newImapPieceIndex] = ckptregion.endLog;
  //write to disk
  MFS_Imap_Piece newPiece;
  for(i = 0; i < 16; i++){
    newPiece.imapPtrs[i] = -1;
  } 
  ckptregion.endLog += sizeof(newPiece);
  lseek(diskFD,0, 0);
  write(diskFD, &ckptregion, sizeof(ckptregion));
  
  lseek(diskFD,ckptregion.imapPiecePtrs[newImapPieceIndex], 0);
  write(diskFD, &newPiece, sizeof(newPiece));
  memLoad();
  return newImapPieceIndex;
}


//updates imapPiece, deletes imapPiece if need be
int deleteInode(int inum){
  printf("inum is: %d\n", inum);
  int imapPieceIndex =inum/16;
  int imapInodeIndex = inum%16;
  int i;
  if(imapInodeIndex < 0){
    return -1;
  }

  MFS_Imap_Piece imapPiece;
  lseek(diskFD, ckptregion.imapPiecePtrs[imapPieceIndex], 0);
  read(diskFD, &imapPiece, sizeof(imapPiece));

  imapPiece.imapPtrs[imapInodeIndex] = -1;
  i = 0;
  while(imapPiece.imapPtrs[i] > 0 && i < 16)
    i++;
  printf("i is: %d\n", i);

  if(i == 0){
    int  test =deleteImapPiece(imapPieceIndex);
  }
  else{
    //write out to disk
     lseek(diskFD, ckptregion.imapPiecePtrs[imapPieceIndex], 0);
     write (diskFD, &imapPiece, sizeof(imapPiece));
  }
  memLoad();
  return 0;

}


int deleteImapPiece(int imapPieceIndex){
  memLoad();
  ckptregion.imapPiecePtrs[imapPieceIndex] = -1;
  //write to disk
  lseek(diskFD,0, 0);
  write(diskFD, &ckptregion, sizeof(ckptregion));
  memLoad();
  return 0;
}

//int createInode(int pinum, int type, int imapPieceIndex){
int createInode(int pinum, int type){
  memLoad();
  int i, newInodeNum, newImapInodeIndex, test;
  for(i = 0; i < 4096; i++ ){
    if(inodeMap.inodes[i] == -1){
      newInodeNum = i;
      i = 5000;
    }
  }
 
  int imapPieceIndex = newInodeNum/16;
  newImapInodeIndex = newInodeNum%16;
 
  if(newImapInodeIndex < 0){
    return -1;
  }
  if(newImapInodeIndex == 0){
    test =createImapPiece();
    newImapInodeIndex = 0;
  }
  
  MFS_Imap_Piece imapPiece;
  lseek(diskFD, ckptregion.imapPiecePtrs[imapPieceIndex], 0);
  read(diskFD, &imapPiece, sizeof(imapPiece));
  
  //update the inode address in the imapPiece
  imapPiece.imapPtrs[newImapInodeIndex] = ckptregion.endLog;
  lseek(diskFD, ckptregion.imapPiecePtrs[imapPieceIndex], 0);
  write(diskFD, &imapPiece, sizeof(imapPiece));
  
  MFS_Inode newInode;
  newInode.stats.type = type;
  for(i = 0; i < 14; i++)
    newInode.blockPtrs[i] = -1;
       
  //point this to the first datablock, right next to the inode
  newInode.blockPtrs[0] = ckptregion.endLog+sizeof(newInode); 
  if(type == MFS_DIRECTORY)
    newInode.stats.size = 2*MFS_BLOCK_SIZE; 
  else
    newInode.stats.size = 0; 
    
  lseek(diskFD, ckptregion.endLog, 0);
  write(diskFD, &newInode, sizeof(newInode));
  
  //update ckptregion.endLog
  ckptregion.endLog += sizeof(newInode);
  
  if(type == MFS_DIRECTORY){
   // This is the first Directory Block
    MFS_Dir_Block dirBlock;      
   
    //prepare rootDirBlock
    int sentinel = sizeof(dirBlock)/sizeof(dirBlock.dirEntries[0]);
    for(i = 0; i < sentinel; i ++){
      dirBlock.dirEntries[i].inum= -1;
      memcpy(dirBlock.dirEntries[i].name, "\0", sizeof("\0"));
    }

    //these are the first two directory entries into the rootDirBlock
    memcpy(dirBlock.dirEntries[0].name, ".\0", sizeof(".\0"));
    dirBlock.dirEntries[0].inum= newInodeNum;
    
    memcpy(dirBlock.dirEntries[1].name, "..\0", sizeof("..\0"));
    dirBlock.dirEntries[1].inum=pinum;
  
    write(diskFD, &dirBlock, sizeof(dirBlock));
    //update ckptregion.endLog
    ckptregion.endLog += sizeof(dirBlock);
  }
  else{
    char * newblock = malloc(MFS_BLOCK_SIZE);
    write(diskFD, newblock, MFS_BLOCK_SIZE);
    free(newblock);
    //update ckptregion.endLog
    ckptregion.endLog += MFS_BLOCK_SIZE;
  }

  //need to update endLog, write out to disk and then reload stuff back into memory
  lseek(diskFD, 0, 0);
  write(diskFD, &ckptregion, sizeof(ckptregion));

  //load inodeMap, ckptregion into mem
  memLoad();
  
  //return the new inode num
  return newInodeNum;
}

int memLoad(){
  //need to first load ckptregion into memory
  lseek(diskFD, 0, 0);
  read(diskFD, &ckptregion, sizeof(ckptregion));
  
  //load inodeMap into memory
  int i =0,sentinel = 0,  j = 0;
    
  for(i = 0; i < 4096; i++){
    inodeMap.inodes[i] = -1;
  }
  
  i=0;sentinel = 0;
  MFS_Imap_Piece imapPiece; 
  for(i = 0; i < 256; i++){
    if(ckptregion.imapPiecePtrs[i] >= 0){
      lseek(diskFD, ckptregion.imapPiecePtrs[i], 0);
      read(diskFD,&imapPiece, sizeof(imapPiece));
      for(j = 0 ; j < 16 ; j++){
	if(imapPiece.imapPtrs[j] >= 0){
	  inodeMap.inodes[sentinel] = imapPiece.imapPtrs[j];
	  sentinel++;
	}
	else{
	  //j = 5000;
	}
      }
    }
    else{
      // i = 5000;
    }
  }

  return 0;
}

int serverRead(const int inum, char *buffer, int block){
  if(inum > 4095 || inum < 0)
    return -1;

  if(block > 13 || block < 0 )
    return -1;

  memLoad();
  
  if(inodeMap.inodes[inum] == -1){
    return -1;
  }

  //else read inode from disk
  MFS_Inode inode;
  lseek(diskFD, inodeMap.inodes[inum], 0);
  read(diskFD, &inode, sizeof(inode));

  //check if block is allocated or not
  if(inode.blockPtrs[block] == -1){
    return -1;
  }
  
  //else lseek to this location and write out 4096 bytes
  lseek(diskFD,inode.blockPtrs[block], 0);
  read(diskFD, buffer, MFS_BLOCK_SIZE);
  return 0;
}

int serverWrite(int inum, char *buffer, int block){
  if(inum > 4095 || inum < 0)
    return -1;
  
  if(block > 13 || block < 0 )
    return -1;

  memLoad();

  if(inodeMap.inodes[inum] == -1){
    return -1;  
  }
  
  //else read inode from disk
  MFS_Inode inode;
  lseek(diskFD, inodeMap.inodes[inum], 0);
  read(diskFD, &inode, sizeof(inode));
  
  //check inode type, if not regular file, return -1
  if(inode.stats.type != MFS_REGULAR_FILE){
    return -1;
  }
  
  //check if block is allocated or not
  if(inode.blockPtrs[block] == -1){
    int oldEndLog = ckptregion.endLog;
    
    MFS_Dir_Block freshDirBlock;
    int k;
    for(k = 0; k < 128; k++){
      memcpy(freshDirBlock.dirEntries[k].name, "\0", sizeof("\0"));
      freshDirBlock.dirEntries[k].inum = -1;
    }
    lseek(diskFD, ckptregion.endLog, 0);
    write(diskFD, &freshDirBlock, sizeof(freshDirBlock));
    
    ckptregion.endLog += MFS_BLOCK_SIZE;
    lseek(diskFD, 0, 0);
    write(diskFD, &ckptregion, sizeof(ckptregion));
    
    inode.blockPtrs[block] = oldEndLog;
    inode.stats.size = (block+1)*(MFS_BLOCK_SIZE);
    lseek(diskFD, inodeMap.inodes[inum], 0);
    write(diskFD, &inode, sizeof(inode));
    
    lseek(diskFD, oldEndLog, 0);
  }
  else{
  
    //else lseek to this location and write out 4096 bytes
    lseek(diskFD,inode.blockPtrs[block], 0);
    inode.stats.size = (block + 1)*(MFS_BLOCK_SIZE);
  }

  write(diskFD, buffer, MFS_BLOCK_SIZE);
 
  lseek(diskFD, inodeMap.inodes[inum], 0);
  write(diskFD, &inode, sizeof(inode));
  /********************************************************************/
  //fsync(diskFD);
  /********************************************************************/
  memLoad();
  return 0;
}

//You are overwriting the value of the imap Piece.

int serverUnlink(int pinum, char * name){
  int i, k;
  memLoad();
  //take care of error cases
  if(pinum < 0 || pinum > 4095)
    return -1;
  
  if(strlen(name) > 28 || strlen(name) < 0)
    return -1;


  if(inodeMap.inodes[pinum] == -1 )
    return -1;  
  
  if(strcmp(name, "..\0") == 0 || strcmp(name, ".\0") == 0 )
    return -1;

  
  //get pinum address
  int pinumLocation = inodeMap.inodes[pinum];

  //lseek to this location
  lseek(diskFD, pinumLocation, 0);
  
  //we have just lseeked to the inode, we need to read the inode
  MFS_Inode pInode;
  int bytesRead =  read(diskFD,&pInode, sizeof(pInode));
  if(DEBUG){
    printf("bytesRead: |%d|\n", bytesRead);
    
  }

  /*************************************************************************/
  /*
  close(diskFD);printDisk();exit(0);
  if(pInode.stats.type == MFS_REGULAR_FILE){
    printInode(&pInode);
    exit(0);
  }
  */
  /*************************************************************************/
  //check if this is a directory or not
   if(pInode.stats.type != MFS_DIRECTORY)
     return -1;
  
  int found = -1, deleteDirBlockLoc, deleteIndex, deleteInodeLocation;
  MFS_Dir_Block dirBlock;
  for(i = 0; i < 14; i++){
    if(pInode.blockPtrs[i] >= 0 ){
      lseek(diskFD,pInode.blockPtrs[i],0);
      read(diskFD, &dirBlock, sizeof(dirBlock));
      for(k = 0; k < 128; k++){
	if(strcmp(dirBlock.dirEntries[k].name, name) ==0){
	  //printf("dirBlock.dirEntries[k].name, name: %s, %s\n",dirBlock.dirEntries[k].name, name );
	  //printf(" dirBlock.dirEntries[k].inum:%d\n", dirBlock.dirEntries[k].inum);
	  //exit(0);
	  found = 1;
	  deleteInodeLocation = inodeMap.inodes[dirBlock.dirEntries[k].inum];
	  deleteIndex = k;
	  deleteDirBlockLoc = pInode.blockPtrs[i];
	  i = 5000;
	 
	}
      }
    }
  } 
 
  if(found < 0)
    return 0;
  
  /**************************************ERROR***********************************/
  //there is something wrong here with the inodeLocation that you are passing to delete
  /**************************************ERROR***********************************/
  MFS_Inode inodeToDelete;
  lseek(diskFD, deleteInodeLocation,0);
  read(diskFD, &inodeToDelete, sizeof(inodeToDelete));
  //printf("SIZE: %d\n", inodeToDelete.stats.size);exit(0);  

 

  if(inodeToDelete.stats.type == MFS_DIRECTORY){  
    if(inodeToDelete.stats.size > 2*(MFS_BLOCK_SIZE)){
      return -1;
    }


   
    /* else{
      dirBlock.dirEntries[deleteIndex].inum = -1;
      memcpy(dirBlock.dirEntries[deleteIndex].name, "\0", sizeof("\0"));
      //write out the directory block
      lseek(diskFD, pInode.blockPtrs[i],0);
      write(diskFD, &dirBlock, sizeof(dirBlock));
      
       }
      */
   }
  
  deleteInode(deleteInodeLocation);

  
  dirBlock.dirEntries[deleteIndex].inum = -1;
  memcpy(dirBlock.dirEntries[deleteIndex].name, "\0", sizeof("\0"));
  //write out the directory block
  lseek(diskFD, deleteDirBlockLoc,0);
  write(diskFD, &dirBlock, sizeof(dirBlock));
      
  //update pInode
  int sizeIndex = 0;
  for(i =0; i < 14; i++)
    if(pInode.blockPtrs[i] != -1)
      sizeIndex =i;
	
  //if(sizeIndex > 3)
  pInode.stats.size = (sizeIndex+1)*MFS_BLOCK_SIZE;
  //pInode.stats.size -=MFS_BLOCK_SIZE;
  //write out the pInode
  lseek(diskFD, pinumLocation,0);
  write(diskFD, &pInode, sizeof(pInode));
  memLoad();
  //delete the directory file, update pInode
  //close(diskFD);
  //printDisk();
  //diskFD = open(file, O_RDWR);
  return 0;


  //TODO
  //get to the directory block, search for name
  //if the index is 3 AND no other files exist, delete(update Imap, ckpt, etc) pInode from the whole disk
  //if last piece in Imappiece, then remove the Imap, update ckpt
 
  // return 0;
}

int serverStat(const int inum, MFS_Stat_t * m){
  if (inum < 0|| inum > 4095)
    return -1;
 
  if(diskFD < 0)
    return -1;

  //load into memory (maybe unnecessary, but always assume the worst 
  //eg - someone forgot to call it somewhere else)
  memLoad();  

  if(inodeMap.inodes[inum] == -1){
    return -1;  
  }
  
  //else read inode from disk
  MFS_Inode inode;
  lseek(diskFD, inodeMap.inodes[inum], 0);
  read(diskFD, &inode, sizeof(inode));
  
  //deep copy struct and return
  m->type = inode.stats.type;
  m->size = inode.stats.size;
  //close(diskFD);
  return 0;
}

int serverShutdown(){
  //may need to check disk here
  close(diskFD);
  exit(0);
  return 0;
}

void printInodeMap(int numItemsToPrint){
  //load stuff into memory
  memLoad();
  int i;
  printf("\nBegin print inode Map\n");
   
  for(i = 0; i <numItemsToPrint; i++ ){
    printf("inodeMap.inodes[%d]: %d\n",i, inodeMap.inodes[i]);
  }
  printf("\nEnd print inode Map\n"); 
}

void printDirBlock(const MFS_Dir_Block * dirBlockToPrint){
  int i;
  printf("\nBegin print DirBlock\n");
  for(i = 0; i < 16; i++ ){
    printf(" dirBlockToPrint->[%d].name: %s\n",i,    dirBlockToPrint->dirEntries[i].name);
    printf(" dirBlockToPrint->dirEntries[%d].inum: %d\n",i,    dirBlockToPrint->dirEntries[i].inum);
  }
  printf("\nEnd print DirBlock");
}

void printInode(const MFS_Inode *inodeToPrint){
  int i;
  printf("\nBegin print  inode");
  printf("\ninode type:|%d|" , inodeToPrint->stats.type);
  printf("\ninode size:|%d|" , inodeToPrint->stats.size);
  for(i = 0; i < 14; i++ ){
    printf("inodeToPrint->blockPtrs[%d]: %d\n",i, inodeToPrint->blockPtrs[i]);
  }
  printf("\nEnd print root inode");
}

void printDisk(){
  int i, test;
  diskFD = open(file,O_RDONLY);
  //if(diskFD<0)
   
  MFS_Ckpt ckptregion;
  MFS_Imap_Piece imapP1;
  MFS_Inode root_inode;
  MFS_DirEnt_t  rootParentDir,  rootDir;
  MFS_Dir_Block rootDirBlock;
  
  test = read(diskFD,&ckptregion, sizeof(ckptregion));
  test += read(diskFD,&imapP1, sizeof(imapP1));
  test += read(diskFD,&root_inode, sizeof(root_inode));
  printf("test is |%d|\n", test);
  test += read(diskFD,&rootDirBlock, sizeof(rootDirBlock));
  //  MFS_Dir_Block DirBlock;
  //memLoad();
  //lseek(diskFD, inodeMap.inodes[1],0);
  //read(diskFD, &DirBlock, sizeof(DirBlock));
  printf(" sizeof(rootDirBlock): |%d|\n", (int)sizeof(rootDirBlock) );
  rootParentDir = rootDirBlock.dirEntries[0];
  rootDir = rootDirBlock.dirEntries[1];
 
  printf("num Bytes read: |%d|\n", test);
  printf("endlog: |%d|\n", ckptregion.endLog);
  printf("\nBegin ckpt print Imap");
  for(i = 0; i < 10; i++ ){
    printf("ckptregion.imapPiecePtrs[%d]: %d\n",i, ckptregion.imapPiecePtrs[i]);
  }
  printf("\nEnd ckpt print Imap");
  
  
  printf("\nBegin print ImapP1");
  for(i = 0; i < 16; i++ ){
    printf("imapP1.imapPtrs[%d]: %d\n",i, imapP1.imapPtrs[i]);
  }
  printf("\nEnd ckpt print ImapP1");

  printf("\nBegin print root inode");
  printf("\nroot inode type:|%d|" , root_inode.stats.type);
  printf("\nroot inode size:|%d|" , root_inode.stats.size);
  for(i = 0; i < 14; i++ ){
    printf("root_inode.blockPtrs[%d]: %d\n",i, root_inode.blockPtrs[i]);
  }
  printf("\nEnd print root inode");

  printf("\nBegin print rootDirBlock\n");
  for(i = 0; i < 128; i++ ){
    printf(" rootDirBlock.dirEntries[%d].name: %s\n",i,    rootDirBlock.dirEntries[i].name);
    printf(" rootDirBlock.dirEntries[%d].inum: %d\n",i,    rootDirBlock.dirEntries[i].inum);
  }
  printf("\nEnd print rootDirBlock");
  
  printInodeMap(15);
  /*
  MFS_Inode newInode;
  printf("\ninodeMap.inodes[1]: %d",inodeMap.inodes[1] );
  lseek(diskFD, inodeMap.inodes[1],0);
  read(diskFD, &newInode, sizeof(newInode));
  printInode(&newInode);
  MFS_Dir_Block dirBlock;
  // printf("\n[1]: %d",inodeMap.inodes[1] );
  lseek(diskFD,newInode.blockPtrs[0],0);
  read(diskFD, &dirBlock, sizeof(dirBlock));
  printDirBlock(&dirBlock);
  */
  //printDirBlock(&DirBlock);
  close(diskFD);
}

int stringSize(const char * stringIn){
  int size =0;
  char* testing = strdup(stringIn);
  while(*testing != '\0'){
    testing++;
    size+=sizeof(testing);
  }
  return size;

}
void gen_random(char *s, const int len,const  int seed) {
  int i;
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  for (i = 0; i < len; ++i) {
    s[i] = alphanum[(i*17234+seed) % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
}
