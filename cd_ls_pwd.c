/************* cd_ls_pwd.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern MNTABLE table[NTAB];
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];

int j = 0;

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007


char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";


int change_dir(char *pathname)
{
  char temp[64];
  char bname[64];
  char n[64];
  strcpy(temp,pathname);
  strcpy(bname,basename(temp));
  strcpy(n,"/");
  strcat(n,bname);
  if(strlen(pathname) == 0){
    iput(running->cwd);
    running->cwd = root;
    return 0;
  }
  else{
    int ino = getino(pathname);
    if(ino == 0){
      return 0;
    }
    MINODE *mip = iget(dev,ino);
    INODE *ip = &(mip->INODE);
     if((ip->i_mode & 0xF000) == 0x4000){
       
     }else{
       return 0;
     }
     if(mip == root){
       if(ino == 2){
	 root = iget(root->mptr->minodeptr->dev,2);
	 running->cwd = root;
	 return 0;
       }
     }
     int i = 0;
     int k = 0;
     for(i = 0; i < NTAB; i++){
       if(strcmp(n,table[i].path)==0){
	 k = 1;
	 break;
       }
     }
     if(k == 1){
       root = iget(table[i].dev,2);
       running->cwd = iget(table[i].dev,2);
       return 0;
     }
    iput(running->cwd);
    running->cwd = mip;
  }
}


int ls_rpwd(MINODE *wd,char *buffer){
  char myname[256];
  if(wd == root){
    return 0;
  }
  int parentino = get_myino(wd,"..");
  int myino = get_myino(wd,".");
  MINODE *pip = iget(dev,parentino);
  get_myname(pip,myino,myname);
  ls_rpwd(pip,buffer);
  j += sprintf(buffer+j,"/%s",myname);
}


int ls_cwd(MINODE *wd,char *buffer)
{
  j = 0;
  if(wd == root){
    sprintf(buffer,"/");
  }
  else{
    ls_rpwd(wd,buffer);
  }
}


int ls_file(int ino, char *fname,int len)
{
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);
  char ftime[64];
  int i;
  char temp[256];

  if((ip->i_mode & 0xF000) == 0x8000){
    printf("%c",'-');
  }
  if((ip->i_mode & 0xF000) == 0x4000){
    printf("%c",'d');
  }
  if((ip->i_mode & 0xF000) == 0xA000){
    printf("%c",'l');
  }
  for(i = 8; i >= 0; i--){
    if(ip->i_mode & (1 << i)){
      printf("%c",t1[i]);
    }
    else{
      printf("%c",t2[i]);
    }
  }
  printf("%4d ", ip->i_links_count);
  printf("%4d ", ip->i_gid);
  printf("%4d ", ip->i_uid);
  printf("%8d ", ip->i_size);

  strcpy(ftime,ctime((const long int *)&ip->i_ctime));
  ftime[strlen(ftime) - 1] = 0;
  printf("%s  ", ftime);
  char tname[256];
  memset(tname,0,256);
  strncpy(tname,fname,len);
  printf("%s ",tname);

  if((ip->i_mode & 0xF000)==0xA000){
    printf("-> %s",(char *)ip->i_block);

  }
  
  printf("\n");
}

int ls_dir(char *dname){

  char buf[BLKSIZE], *cp;
  int ino;
  MINODE *mip;
  ino = getino(dname);
  mip = iget(dev,ino);
  DIR *dp;
  int i;
  INODE *ip = &(mip->INODE);
  for(i = 0; i < 12; i++){
    if(mip->INODE.i_block[i] == 0){
      return 0;
    }
    get_block(dev,mip->INODE.i_block[i],buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf + BLKSIZE){
      if(strcmp(dp->name,"\0")==0){
	return 0;
      }
      ls_file(dp->inode,dp->name,dp->name_len);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

int list_file(char *pathname)
{
  char path[256];
  char temp[256];
  char bname[256];
  strcpy(temp,pathname);
  strcpy(bname,basename(temp));

  if(pathname[0] == '/'){
    dev = root->dev;
  }
  else{
    dev = running->cwd->dev;
  }

  int ino = getino(pathname);
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);

  if(pathname[0] == '/'){
    if(ino == 0){
      printf("----FILE DOES NOT EXIST---\n");
      return 0;
    }
  }
 
  if((ip->i_mode & 0xF000) == 0x8000){
   int k = strlen(bname);
   ls_file(ino,bname,k);
   return 0;
  }
  
  if(strlen(pathname) == 0){
    ls_cwd(running->cwd,path);
    ls_dir(path);
  }
  else if(pathname[0] == '/'){
    ls_dir(pathname);
  }
  else{
    ls_cwd(running->cwd,path);
    strcat(path,"/");
    strcat(path,pathname);
    ls_dir(path);
  }

}

int rpwd(MINODE *wd){

  MNTABLE *mptr = root->mptr;
  char myname[256];
  if(wd == root){
    if(strcmp(mptr->path,"/") != 0){
      printf("%s",mptr->path);
    }
    return 0;
  }
  int parentino = get_myino(wd,"..");
  int myino = get_myino(wd,".");
  MINODE *pip = iget(dev,parentino);
  get_myname(pip,myino,myname);
  rpwd(pip);
  printf("/%s",myname);
}


int pwd(MINODE *wd)
{
  MNTABLE *mptr = root->mptr;
  if(wd == root){
    printf("%s\n",mptr->path);
  }
  else{
    rpwd(wd);
    printf("\n");
  }
}




