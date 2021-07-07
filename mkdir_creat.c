/************* mkdir_creat.c file **************/

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

int enter_name(MINODE *pip, int ino, char *mname,int n_len)
{
    int i;
    char buf[BLKSIZE];
    memset(buf,0,BLKSIZE);
    INODE *ip = &pip->INODE;

    for(i = 0; i < 12; i++)
    {
        if (ip->i_block[i] == 0)
        {
            break;
        }
        
    }
    i = i - 1;
    get_block(dev,ip->i_block[i],buf);
    DIR *dp = (DIR *)buf;
    char *cp = buf;

    while (cp + dp->rec_len < (buf) + BLKSIZE){
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    // dp NOW points at last entry in block
    int ideal_length = 4*( (8 + dp->name_len + 3)/4 );
    
    int remain = dp->rec_len - ideal_length;
    
    int need_length = 4*( (8 + n_len + 3)/4 );

    if (remain >= need_length)
      {
        dp->rec_len = ideal_length;

        cp += dp->rec_len;
        dp = (DIR *)cp;

        strncpy(dp->name,mname,n_len);
        dp->rec_len =  remain;
        dp->name_len = n_len;
        dp->inode = ino;

        put_block(dev,ip->i_block[i],buf);
	return 1;
    }

    dp->rec_len = ideal_length;

    cp += dp->rec_len;
    dp = (DIR *)cp;

    strncpy(dp->name,mname,n_len);
    dp->rec_len =  remain;
    dp->name_len = n_len;
    dp->inode = ino;

    put_block(dev,ip->i_block[i],buf);
    
    return 1;
}


int mymkdir(MINODE *pip, char *mname, int pino){
  MINODE *mip;

  int ino = ialloc(dev);
  int bno = balloc(dev);
  printf("ino = %d bno = %d\n",ino, bno);

  mip = iget(dev,ino);

  INODE *ip = &(mip->INODE);

  ip->i_mode = 040775;
  ip->i_uid = running->uid;
  ip->i_gid = running->pid;
  ip->i_size = BLKSIZE;
  ip->i_links_count = 2;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2;
  ip->i_block[0] = bno;
  for(int i = 1; i < 15; i++){
    ip->i_block[i] = 0;
  }
  mip->dirty = 1;
  iput(mip);

  char buf[BLKSIZE];
  bzero(buf,BLKSIZE);
  DIR *dp = (DIR *)buf;
  char *cp = buf;
  //making . entry

  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';

  //mkaing .. entry
  cp += 12;
  dp = (DIR *)cp;
  dp->inode = pino;
  dp->rec_len = BLKSIZE-12;
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  put_block(dev,bno,buf);
  int n_len = strlen(mname);
  enter_name(pip,ino,mname,n_len);
}

int make_dir(char *pathname){
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

  int pino = getino(parent);
  MINODE * pip = iget(dev,pino);
  INODE *ip = &(pip->INODE);
   if((ip->i_mode & 0xF000) == 0x4000){
    
  }
   else{

     printf("---NOT A DIR---\n");
     return 0;
   }
   
   printf("===========================================\n");
   int check = search(pip,child);
   if(check == 0){
     printf("---child does not exist, ok to mkdir---\n");
   }
   else{

     printf("---child exists, mkdir failed---\n");
     return 0;
   }

   mymkdir(pip,child,pino);

   ip->i_links_count = ip->i_links_count + 1;
   ip->i_atime = time(0L);
   pip->dirty = 1;
   
   iput(pip);
  
}

int mycreat(MINODE *pip, char *mname, int pino){
 MINODE *mip;

  int ino = ialloc(dev);
  printf("ino = %d\n",ino);

  mip = iget(dev,ino);

  INODE *ip = &(mip->INODE);

  ip->i_mode = 0100644;
  ip->i_uid = running->uid;
  ip->i_gid = running->pid;
  ip->i_size = 0;
  ip->i_links_count = 1;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 0;
  ip->i_block[0] = 0;
  for(int i = 1; i < 15; i++){
    ip->i_block[i] = 0;
  }
  mip->dirty = 1;
  iput(mip);
  int n_len = strlen(mname);
  enter_name(pip,ino,mname,n_len);

  return ino;
}

int creat_file(char *pathname){

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

  int pino = getino(parent);
  MINODE * pip = iget(dev,pino);
  INODE *ip = &(pip->INODE);
   if((ip->i_mode & 0xF000) == 0x4000){
    
  }
   else{

     printf("---NOT A DIR---\n");
     return 0;
   }
   
   printf("===========================================\n");
   int check = search(pip,child);
   if(check == 0){
     printf("---file does not exist, ok to creat---\n");
   }
   else{

     printf("---file exists, creat failed---\n");
     return 0;
   }

  int x =  mycreat(pip,child,pino);

   ip->i_atime = time(0L);
   pip->dirty = 1;
   
   iput(pip);

   return x;
  
}
