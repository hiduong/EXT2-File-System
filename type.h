/*************** type.h file ************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

#define FREE        0
#define READY       1

#define BLKSIZE  1024
#define NMINODE    64
#define NFD         8
#define NPROC       2
#define NTAB       10

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  // for level-3
  int mounted;
  int rdev;
  struct mntable *mptr;
}MINODE;


typedef struct oft{ // for level-2
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid;
  int          status;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

typedef struct mntable{
  int dev;
  char devname[64];
  char path[64];
  int bmap;
  int imap;
  int nblocks;
  int ninodes;
  int inode_start;
  MINODE *minodeptr;
  
}MNTABLE;
