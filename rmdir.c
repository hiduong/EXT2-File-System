/************* rmdir.c file **************/

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

int rm_child(MINODE *parent, char *name) {
  
   char sbuf[BLKSIZE];
   int i = 0;
   DIR *prev = NULL;
   char *cp;
   DIR *dp;
   int length = 0;
   char temp[256];
   
   for (i = 0; i < 12; i++) {

   get_block(parent->dev, parent->INODE.i_block[i], sbuf);
   dp = (DIR *)sbuf;
   cp = (char *)sbuf;
  
   prev = NULL;

   while(cp < sbuf + BLKSIZE) {
     prev = dp;
     cp += dp->rec_len;
     dp = (DIR *)cp;
     memset(temp,0,256);
     strncpy(temp,dp->name,dp->name_len);
     if(strcmp(temp,name) == 0){
       break;
     }
    }
   if(cp < sbuf + BLKSIZE){
     break;
   }
   }
   //dir is last entry
   if (cp + dp->rec_len == sbuf + BLKSIZE) {
      prev->rec_len += dp->rec_len;
      //completely remove the entry
      dp->rec_len = 0;
      strcpy(dp->name,"\0");
      dp->name_len = 0;
      dp->inode = 0;
      put_block(parent->dev, parent->INODE.i_block[i], sbuf);
      return 0;
   }
     //dir is in the middle of first of block excluding . and ..
   else {
      length = dp->rec_len;
      prev = dp;
      cp += dp->rec_len;
      dp = (DIR *)cp;
      while (cp < sbuf + BLKSIZE) {
	//shift everything from target dir to left
	strcpy(prev->name,dp->name);
        prev->inode = dp->inode;
        prev->name_len = dp->name_len;
	prev->rec_len = dp->rec_len;
	cp += dp->rec_len;
         if(cp >= sbuf + BLKSIZE) {
            break;
         }
	 //target dir will be the last dir 
         prev = dp;
         dp = (DIR *)cp;       
      }
      prev->rec_len += length; // set the rec length of second to last dir + rec_len of target dir
   }
 


   //completely remove the entry
   dp->rec_len = 0;
   strcpy(dp->name,"\0");
   dp->name_len = 0;
   dp->inode = 0;

   put_block(parent->dev, parent->INODE.i_block[i], sbuf);
   return 1;
}

int myrmdir(char *pathname){
  MINODE *start;
  char dname[256], bname[256];
  char *parent, *child;
  strcpy(bname,pathname);
  strcpy(dname,pathname);
  if(pathname[0] == '/'){
    start = root;
    dev = root->dev;
  }
  else{
    start = running->cwd;
    dev = running->cwd->dev;
  }
  
  parent = dirname(dname);
  child = basename(bname);
  printf("dirname = %s basename = %s\n",parent,child);
  
  int ino = getino(pathname);
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);

  int cino = getino(child);
  MINODE *cip = iget(dev,ino);
  if(ino == 0){
    printf("----DIR DOES NOT EXIST, rmdir FAILED----\n");
    return 0;
  }

  if((ip->i_mode & 0xF000) == 0x4000){
    
  }
  else{

    printf("----NOT A DIR, rmdir FAILED----\n");
    return 0;
  }

  if(is_empty(cip) != 0){
    printf("----DIR NOT EMPTY, rmdir FAILED----\n");
    return 0;
  }

  for(int i = 0; i < 12; i++){
    if(mip->INODE.i_block[i] == 0){
      break;
    }
    bdalloc(mip->dev,mip->INODE.i_block[i]);
  }
  idalloc(mip->dev,mip->ino);
  iput(mip);

  int pino = getino(parent);
  MINODE *pip = iget(mip->dev, pino);

  rm_child(pip,child);

  pip->INODE.i_links_count--;
  pip->dirty = 1;
  iput(pip);
  return 0;
}
