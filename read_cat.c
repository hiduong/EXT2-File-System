/************* read_cat.c file **************/

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

int myread(int mfd, char *buf, int nbytes){
  int lbk;
  int startByte;
  int blk;
  OFT *oftp = running->fd[mfd];
  int filesize = oftp->mptr->INODE.i_size;
  int count = 0;
  int avil = filesize - oftp->offset; 
  char *cq = buf;
  MINODE *mip = oftp->mptr;

  while(nbytes && avil){
    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;

    if(lbk < 12){
      blk = mip->INODE.i_block[lbk];
    }
    else if(lbk >= 12 && lbk < 256 + 12){
      u32 ibuf[256];
      get_block(mip->dev,mip->INODE.i_block[12],(char *)ibuf);
      blk = ibuf[lbk-12];
    }
    else{
      u32 dbuf[256];
      int dblk;
      get_block(mip->dev,mip->INODE.i_block[13],(char *)dbuf);
      lbk -= (12+256);
      dblk = dbuf[lbk / 256];
      get_block(mip->dev,dblk,(char *)dbuf);
      blk = dbuf[lbk % 256];
      
    }

    char readbuf[nbytes];
    get_block(mip->dev,blk,readbuf);
   
    char *cp = readbuf + startByte;
    int remain = BLKSIZE - startByte;

    if(avil < remain){
      remain = avil;
    }
    int size = 0;
    
    if(remain > nbytes){
      size = nbytes;
    }
    else{
      size = remain;
    }

    memcpy(cq,cp,size);

    count = size;
    oftp->offset = oftp->offset + size;
    avil = avil - size;
    nbytes = nbytes - size;
  }
  return count;
}

int mycat(char *pathname){
  char mybuf[BLKSIZE],dummy = 0;
  int n;
  int fd = open_file(pathname,0);
  while(n = myread(fd,mybuf,BLKSIZE)){
    mybuf[n] = 0;
    printf("%s",mybuf);
  }
  close_file(fd);
}
