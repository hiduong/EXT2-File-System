/****************************************************************************
*                   Hien testing ext2 file system                            *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>
#include <unistd.h>

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
MNTABLE table[NTAB];
PROC   proc[NPROC], *running;

char   gpath[256]; // global for tokenized components
char   *name[64];  // assume at most 64 components in pathname
int    n;          // number of component strings

int    fd, dev;
int    nblocks, ninodes, bmap, imap, inode_start;
char   line[256], cmd[32], pathname[256], pathname2[256];

#include "util.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink_symlink.c"
#include "misc_functions.c"
#include "open_close_lseek.c"
#include "read_cat.c"
#include "write_cp_mv.c"
#include "mount_umount.c"

char disk[64];

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}


int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    if(i == 1){
      p->uid = 1;
    }else{
    p->uid = 0;
    }
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{
  int ino;
  char buf[BLKSIZE];

  printf("ENTER DISK: ");
  scanf("%s",disk);
  
  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;
  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  printf("ninodes = %d nblocks = %d\n",ninodes,nblocks);

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  
  MNTABLE *mnptr = &table[0];
  printf("mount_root()\n");
  root = iget(dev, 2);
  strcpy(mnptr->devname,disk);
  strcpy(mnptr->path,"/");
  mnptr->dev = dev;
  mnptr->ninodes = ninodes;
  mnptr->nblocks = nblocks;
  mnptr->bmap = bmap;
  mnptr->imap = imap;
  mnptr->inode_start = inode_start;
  mnptr->minodeptr = iget(dev,2);
  root->mounted = 1;
  root->mptr = &table[0];
  
  printf("%s mounted on %s Opened at FD[%d]\n",root->mptr->devname,root->mptr->path,root->mptr->dev);

  printf("root refCount = %d\n", root->refCount);
  
  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  
  printf("root refCount = %d\n", root->refCount);
  
}

int main(int argc, char *argv[ ])
{
  mount_root();
  
  //printf("hit a key to continue : "); getchar();
  while(1){
    memset(pathname,0,sizeof(pathname));
    memset(pathname,0,sizeof(pathname2));
    printf("Commands: [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink|stat|chmod|utime|touch|pfd|cat|cp|mv|mount|umount|quit]\n");
    printf("Input Commands >> ");
    
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;
    if (line[0]==0)
      continue;
    pathname[0] = 0;
    cmd[0] = 0;
    pathname2[0] = 0;
    
    sscanf(line, "%s %s %s", cmd, pathname,pathname2);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       list_file(pathname);
    if (strcmp(cmd, "cd")==0)
      change_dir(pathname);
    if (strcmp(cmd, "pwd")==0)
      pwd(running->cwd);
    if (strcmp(cmd, "mkdir")==0)
      make_dir(pathname);
     if (strcmp(cmd, "creat")==0)
       creat_file(pathname);
     if (strcmp(cmd, "rmdir")==0)
       myrmdir(pathname);
     if (strcmp(cmd, "link")==0)
       mylink(pathname,pathname2);
     if(strcmp(cmd,"unlink")==0)
       myunlink(pathname);
     if(strcmp(cmd,"symlink")==0)
       mysymlink(pathname,pathname2);
     if(strcmp(cmd,"readlink")==0){
       char buffer[256];
        myreadlink(pathname,buffer);
     printf("symlink name = %s\n",buffer);
     buffer[0] = 0;
     }
     if(strcmp(cmd,"stat")==0)
       mystat(pathname);
      if(strcmp(cmd,"chmod")==0)
	mychmod(pathname,pathname2);
      if(strcmp(cmd,"utime")==0)
	myutime(pathname);
      if(strcmp(cmd,"touch")==0)
	creat_file(pathname);
      if(strcmp(cmd,"pfd")==0){
	pfd();
      }
      if(strcmp(cmd,"cat")==0){
	mycat(pathname);
      }
      if(strcmp(cmd,"cp")==0){
	mycp(pathname,pathname2);
      }
      if(strcmp(cmd,"mv")==0){
	mymv(pathname,pathname2);
      }
      if(strcmp(cmd,"mount")==0){
	mymount(pathname,pathname2);
      }
      if(strcmp(cmd,"umount")==0){
	myumount(pathname);
      }
    if (strcmp(cmd, "quit")==0)
      quit();
  }
}
 
