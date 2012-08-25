#include <limits.h>
#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)

#define DEBUG 0
//#define DEBUG 1




typedef struct __MFS_Stat_t {
  int type;   // MFS_DIRECTORY or MFS_REGULAR
  int size;   // bytes
  // note: no permissions, access times, etc.
  // need to create permissions etc?
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
  char name[28];  // up to 28 bytes of name in directory (including \0)
  int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

typedef enum msg_type{
  READ,
  LOOKUP,
  STAT,
  WRITE,
  CREATE,
  UNLINK,
  SHUTDOWN,
  INIT
} msg_type;

typedef enum file_type{
  REGULAR,
  DIRECTORY
} file_type;




typedef struct msg_t{
  msg_type msgType;
  char name[28];
  char buffer[4096];
  int pinum, inum, block,returnVal,type;
  MFS_Stat_t stat;
} msg_t; 


typedef struct MFS_Ckpt{
  // num entries in imap = num inodes/num entries per imap piece
  // 256 = 4096/16
  int imapPiecePtrs[256], endLog;
} MFS_Ckpt; 

typedef struct MFS_Inode{
  MFS_Stat_t stats;
  // 16 - 2 for type, size
  int blockPtrs[14];
}MFS_Inode;


typedef struct MFS_Imap_Piece{
  int imapPtrs[16]; //this should really be renamed to inodePtrs, confusing
}MFS_Imap_Piece;

//this needs to be 4096 bytes, check somewhere
typedef struct MFS_Dir_Block{
  MFS_DirEnt_t dirEntries[128];
}MFS_Dir_Block;

//this is what will be loaded in memory
typedef struct MFS_Inode_Map{
  int inodes[MFS_BLOCK_SIZE];
}MFS_Inode_Map;



/* may not need this
typedef struct mfs_directory{
  mfs_inode dir_inode;
}mfs_directory;
*/

//TODO
//declare a message struct with the enum as a param
//clear up Read
//implement all the not implemented ones

//Client side functions
int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();
char * getHost();
void printMessageParams(msg_t message);


#endif // __MFS_h__
