/************* open_close_lseek.c file **************/

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


int mytruncate(MINODE *mip)
{
  int buf[BLKSIZE], cbuf[BLKSIZE];
  int dbuf[BLKSIZE];
  INODE *ip = &(mip->INODE);
    //direct block numbers
  for(int j = 0; j < 12; j++){
    ip->i_block[j] = 0;  
  }

  //indirect block numbers

  get_block(fd,ip->i_block[12],(char *)buf);
  int *up = (int *)buf;
  while(up < buf + BLKSIZE){
    *up = 0;
    up++;
  }
  put_block(fd,ip->i_block[12],(char *)buf);
  //double indirect block numbers
  get_block(fd,ip->i_block[13],(char *)dbuf); 

  for(int u = 0; u < 256; u++)
    {

      get_block(fd,dbuf[u],(char *)cbuf);
      int *op = (int *)cbuf;

      while(op < cbuf + BLKSIZE){
	*op = 0;
	op++;
      }
      put_block(fd,dbuf[u],(char *)cbuf);
    }
  put_block(fd,ip->i_block[13],(char *)dbuf);

  ip->i_atime = ip->i_mtime = time(0L);
  mip->dirty = 1;
  ip->i_size = 0;
  ip->i_blocks = 0;
  iput(mip);
  
}

//mode 0|1|2|3 fir R|W|RW|APPEND
int open_file(char *pathname, int mode)
{
  int i = 0;
  if(pathname[0] == '/'){
    dev = root->dev;
  }
  else{
    dev = running->cwd->dev;
  }

  int ino = getino(pathname);

  MINODE * mip = iget(dev,ino);
    
  if((mip->INODE.i_mode & 0xF000) != 0x8000){
    printf("----NOT A REGULAR FILE FAILED TO OPEN----\n");
    return 0;
  }

  int j = 0;
  while(running->fd[j] != NULL){
    if(running->fd[j]->mode > 0 && running->fd[j]->mptr->ino == mip->ino){
      printf("FILE OPENED FOR WRITE, READ/WRITE OR APPENED\n");
      return 0;
    }
    j++;
  }

  OFT *oftp = (OFT *)malloc(sizeof(OFT));

  oftp->mode = mode;
  oftp->refCount = 1;
  oftp->mptr = mip;

  while(i < 8){
    if(running->fd[i] == NULL){
      break;
    }
    i++;
  }

  switch (mode){
  case 0:
    oftp->offset = 0;
    break;
  case 1:
    mytruncate(mip);
    oftp->offset = 0;
    break;
  case 2:
    oftp->offset = 0;
    break;
  case 3:
    oftp->offset = mip->INODE.i_size;
    break;
  default:
    printf("invalid mode\n");
    return -1;
  }

  running->fd[i] = oftp;
  printf("OPENED FILE AT - FD[%d]\n",i);
  return i;

  
}

int close_file(int mfd){
  

  if(mfd > 7 || mfd < 0){
    printf("----FD NOT IN RANGE----\n");
    return 0;
  }

  if(running->fd[mfd] != NULL){
  }
  else{
    return 0;
  }

  OFT *oftp = running->fd[mfd];
  running->fd[mfd] = 0;
  oftp->refCount--;
  if(oftp->refCount > 0){
    printf("----FILE IS BUSY----\n");
    return 0;
  }

  MINODE *mip = oftp->mptr;
  iput(mip);
  printf("CLOSED FILE AT - FD[%d]\n",mfd);
  return 0;
  
}

int pfd()
{
 
  OFT *fd;

  printf(" fd     mode    offset    INODE \n");
  printf("----    ----    ------   --------\n");

  for (int i = 0; i < 8; i++)
  {
    fd = running->fd[i];
  
    if (fd == 0)
    {
      continue;
    }

    if (fd->refCount >= 1)
    {
      printf("%d\t", i);
      
      if(fd->mode == 0)
      {
        printf("READ\t");
      }
      else if(fd->mode == 1)
      {
        printf("WRITE\t");
      }
      else if(fd->mode == 2){
	printf("R/W\t");
      }
      else if(fd->mode == 3){
	printf("APPEND\t");
      }
      printf("%d\t[%d, %d]\n", fd->offset, fd->mptr->dev, fd->mptr->ino);
    }
  }
  return 0;
}

int mylseek(int mfd,int position){


  OFT *oftp = running->fd[mfd];
   if(position < 0 || position > oftp->mptr->INODE.i_size){
   printf("Invalid position\n");
   return 0;
   }
   else{
    oftp->offset = position;
    printf("offset successful");
    return 1;
   }
}



int mydup(int mfd){
  if(running->fd[mfd] == NULL){
    printf("----NOT AN OPENED DESCRIPTOR----\n");
    return 0;
  }

  int i = 0;

  while(i < 8){
    if(running->fd[i] == NULL){
      running->fd[i] = running->fd[mfd];
      OFT *oftp =  running->fd[i];
      oftp->refCount++;
      return 0;
    }
    i++;
  }
}

int mydup2(int fd,int gd){
  if(running->fd[gd] != NULL){
    close_file(gd);
  }

  running->fd[gd] = running->fd[fd];
  OFT *oftp = running->fd[gd];
  oftp->refCount++;
  return 0;
}


