/************* misc_functions.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

int mystat(char *pathname)
{
  char ftime[64];

  struct stat myst;
  if (strcmp(pathname, "")==0)
  {
    printf("---NO PATHNAME---\n");
    return 0;
  }
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);

  myst.st_dev = dev;
  myst.st_ino = ino;

  myst.st_mode = mip->INODE.i_mode;
  myst.st_uid = mip->INODE.i_uid;
  myst.st_gid = mip->INODE.i_gid;
  myst.st_atime = mip->INODE.i_atime;
  myst.st_ctime = mip->INODE.i_ctime;
  myst.st_mtime = mip->INODE.i_mtime;
  myst.st_nlink = mip->INODE.i_links_count;
  myst.st_size = mip->INODE.i_size;
  myst.st_mode = mip->INODE.i_mode;
  myst.st_blocks = mip->INODE.i_blocks;
  printf("\n----STAT OF %s----\n",basename(pathname));
  printf("Name: %s\n", basename(pathname));
  printf("Size: %ld\t",  myst.st_size);
  printf("Blocks: %ld\n",myst.st_blocks);
  printf("Device: %llu\t",myst.st_dev);
  printf("Inode: %lu\t", myst.st_ino);
  printf("Links: %d\n", myst.st_nlink);
  printf("Uid: %d\tGid: %d\n", myst.st_uid, myst.st_gid);
  //testing 
  printf("Mode: %o\n", myst.st_mode);

  strcpy(ftime, ctime(&myst.st_atime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  printf("Access: %s\n",ftime);
  strcpy(ftime, ctime(&myst.st_mtime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  printf("Modify: %s\n",ftime);
  strcpy(ftime, ctime(&myst.st_ctime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  printf("Change: %s\n\n",ftime);

  iput(mip);
}


int mychmod(char *mode, char *filename){
  
  int ino = getino(filename);
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);
  char buffer[1024];
  int n = 0;

  if((ip->i_mode & 0xF000) == 0x8000){
    //reg file
    n = sprintf(buffer,"%o",0100000);
    buffer[n - 1] = mode[2];
    buffer[n - 2] = mode[1];
    buffer[n - 3] = mode[0];
  }
  if((ip->i_mode & 0xF000) == 0x4000){
    //dir
    n = sprintf(buffer,"%o",040000);
    buffer[n - 1] = mode[2];
    buffer[n - 2] = mode[1];
    buffer[n - 3] = mode[0];
  }
  if((ip->i_mode & 0xF000) == 0xA000){
    //symlink
    n = sprintf(buffer,"%o",0120000);
    buffer[n - 1] = mode[2];
    buffer[n - 2] = mode[1];
    buffer[n - 3] = mode[0];
  }

  int x = strtol(buffer,NULL,8);
  ip->i_mode = x;
  mip->dirty = 1;
  iput(mip);
  return 1;

}

int myutime(char *pathname)
{
  if (strcmp(pathname, "")==0)
  {
    printf("no pathname provided\n");
    return 0;
  }
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);

  mip->INODE.i_atime = time(0L);
  iput(mip);
  return 1;
}
