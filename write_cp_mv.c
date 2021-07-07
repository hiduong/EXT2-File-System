/************* write_cp_mv.c file **************/

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

int mywrite(int mfd, char *buf, int nbytes){
  int lbk;
  int startByte;
  int blk;
  OFT *oftp = running->fd[mfd];
  MINODE *mip = oftp->mptr;
  char *cq = buf;
  
  while(nbytes > 0){
    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;

    if(lbk < 12){
      if(mip->INODE.i_block[lbk] == 0){
	mip->INODE.i_block[lbk] = balloc(mip->dev);
      }
      blk = mip->INODE.i_block[lbk];
    }
    else if(lbk >= 12 && lbk < 256 + 12){
      int ibuf[BLKSIZE];
      if(mip->INODE.i_block[12] == 0){
	mip->INODE.i_block[12] = balloc(mip->dev);
	get_block(mip->dev,mip->INODE.i_block[12],(char *)ibuf);
	int *up =  (int *)ibuf;
	while(up < ibuf + BLKSIZE){
	  *up = 0;
	  up++;
	}
	put_block(mip->dev,mip->INODE.i_block[12],(char *)ibuf);
      }
      get_block(mip->dev,mip->INODE.i_block[12],(char *)ibuf);
      blk = ibuf[lbk - 12];
      if(blk == 0){
        ibuf[lbk - 12] = balloc(mip->dev);
        
	put_block(mip->dev,mip->INODE.i_block[12],(char *)ibuf);

	blk = ibuf[lbk - 12];
      }
    }
    else{
      int dbuf[BLKSIZE];
      int dblk = 0;
      if(mip->INODE.i_block[13] == 0){
	mip->INODE.i_block[13] = balloc(mip->dev);
	get_block(mip->dev,mip->INODE.i_block[13],(char *)dbuf);
	int *up = (int *)dbuf;
	while(up < dbuf + BLKSIZE){
	  *up = 0;
	  up++;
	}
	put_block(mip->dev,mip->INODE.i_block[13],(char *)dbuf);
      }
      get_block(mip->dev,mip->INODE.i_block[13],(char *)dbuf);
      lbk -= (12 - BLKSIZE);
      dblk = dbuf[lbk / BLKSIZE];
      get_block(mip->dev,dblk,(char *)dbuf);
      blk = dbuf[lbk % 256];
      if(blk == 0){
	dbuf[lbk % 256] = balloc(mip->dev);
	put_block(mip->dev,dblk,(char *)dbuf);
	blk = dbuf[lbk % 256];
      }
	
    }
    char wbuf[BLKSIZE];
    get_block(mip->dev,blk,wbuf);
    char *cp = wbuf + startByte;
    int remain = BLKSIZE - startByte;
    int size = 0;
    if(remain > nbytes){
      size = nbytes;
    }
    else{
      size = remain;
    }
  
    memcpy(cp,cq,size);

    cp = cp + size;
    cq = cp + size;
    
    oftp->offset = oftp->offset + size;
    
    if(oftp->offset > mip->INODE.i_size){
      mip->INODE.i_size = oftp->offset;
    }
    
    nbytes = nbytes - size;
    put_block(mip->dev,blk,wbuf);  
  }
  
  mip->dirty = 1;
  return nbytes;
}

int mycp(char *source, char *dest){
  char mybuf[BLKSIZE],dummy = 0;
  int m;
  int fd = open_file(source,0);


  int k = creat_file(dest);
  if(k == 0){
    close_file(fd);
    printf("----FAILED TO CP----\n");
    return 0;
  }
  
  int gd = open_file(dest,2);
  while(m = myread(fd,mybuf,BLKSIZE)){
    mybuf[m] = 0;
    mywrite(gd,mybuf,m);
  }
  close_file(fd);
  close_file(gd);
}

int mymv(char *source, char *dest){

  char path[256];
  int dino = 0;
  int sino = getino(source);
   if(sino == 0){
    printf("----SOURCE DOES NOT EXIST----\n");
    return 0;
  }
  if(source[0] != '/'){
    ls_cwd(running->cwd,path);
    if(strcmp(path,"/")==0){
      strcat(path,source);
    }
    else{
    strcat(path,"/");
    strcat(path,source);
    }
    if(strcmp(dest,"\0") == 0){
      printf("----MV OK----\n");
      return 0;
    }
    if(strcmp(dest,"/")==0){
      strcat(dest,source);
    }
    else{
      strcat(dest,"/");
      strcat(dest,source);
    }

    dino = getino(dest);
    if(dino != 0){
      printf("----SOURCE ALREADY IN DESTINATION----\n");
      return 0;
    }
    mylink(path,dest);
    mycp(path,dest);
    myunlink(path);
    printf("----MV OK----\n");
    return 1;
  }

  char bname[256];
  strcpy(bname,basename(source));
  
  if(strcmp(dest,"\0") == 0){
    printf("----MV OK----\n");
    return 0;
  }
  if(strcmp(dest,"/")==0){
    strcat(dest,bname);
  }
  else{
    strcat(dest,"/");
    strcat(dest,bname);
  }

  dino = getino(dest);
  if(dino != 0){
    printf("----SOURCE ALREADY IN DESTINATION----\n");
    return 0;
  }
  mylink(source,dest);
  mycp(source,dest);
  myunlink(source);
  printf("----MV OK----\n");
  return 1;
}
