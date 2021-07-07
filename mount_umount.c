/*********** mount_umount.c.c file ****************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern MNTABLE table[NTAB];
extern PROC   proc[NPROC], *running;

extern char   gpath[256];
extern char   *name[64];
extern int    n;

extern int    fd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[256];

int mymount(char *filesys, char *mpoint){

  //if no args display all filesys
  if(strlen(filesys) == 0 && strlen(mpoint) == 0){
    printf("\n----CURRENTLY MOUNTED----\n");
    for(int i = 0; i < NTAB; i++){
      if(table[i].dev != 0){
	printf("%s mounted on %s\n",table[i].devname,table[i].path);
      }
    }
    printf("\n");
    return 0;
  }

  //verify file sys is not already mounted
  for(int j = 0; j < NTAB; j++){
    if(strcmp(table[j].devname,filesys)==0){
      printf("----FILE SYSTEM ALREADY MOUNTED----\n");
      return 0;
    }
  }

  //go to next empty entry in mount table
  int k = 0;
  for(k = 0; k < NTAB; k++){
    if(table[k].dev == 0){
      break;
    }
  }

  MNTABLE *mnptr = &table[k];
  if((fd = open(filesys,O_RDWR)) < 0){
    printf("----INVALID FILE SYSTEM----\n");
    return 0;
  }

  

  dev = fd;

    int ino  = getino(mpoint);
  if(ino < 0){
    printf("----INVALID MOUNT POINT----\n");
  }
  MINODE *mip = iget(dev,ino);
  
  char buf[BLKSIZE];

  //check if ext2 filesystem;

  get_block(dev,1,buf);
  
   sp = (SUPER *)buf;
   if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      return 0;
  }
   printf("%s is an ext2 filesystem\n",filesys);

   mnptr->ninodes  = sp->s_inodes_count;
   mnptr->nblocks  = sp->s_blocks_count;

   get_block(dev,2,buf);
   gp = (GD *)buf;

   mnptr->bmap  = gp->bg_block_bitmap;
   mnptr->imap  = gp->bg_inode_bitmap;
   mnptr->inode_start = gp->bg_inode_table;

    
   char temp[64];
   if(mpoint[0] != '/'){
     strcpy(temp,"/");
     strcat(temp,mpoint);
   }
   else{
     strcpy(temp,mpoint);
   }

   strcpy(mnptr->devname,filesys);
   strcpy(mnptr->path,temp);
   mnptr->dev = dev;
   
   mnptr->minodeptr = iget(root->dev,root->ino);
   mip->mounted = 1;
   iput(mip);
   mip->mptr = &table[k];
   
   printf("----MOUNT OK----\n");
   printf("%s mounted on %s Opened at FD[%d]\n",mip->mptr->devname,mip->mptr->path,mip->mptr->dev);

   dev = root->dev;
   return 1;
}


int myumount(char *filesys){

  int i = 0;
  for(i = 0 ; i < NTAB; i++){
    if(strcmp(filesys,table[i].devname)==0){
      printf("File System %s is mounted on %s\n",table[i].devname,table[i].path);
      break;
    }
  }

  if(table[i].dev == root->dev){
    printf("----CANNOT UNMOUNT, BUSY!----\n");
    return 0;
  }

  MINODE *mip = table[i].minodeptr;

  mip->mptr = 0;
  mip->mounted = 0;
  iput(mip);
 
  close(table[i].dev);

  table[i].dev = 0;
  table[i].bmap = 0;
  table[i].imap = 0;
  table[i].minodeptr = 0;
  table[i].ninodes = 0;
  table[i].nblocks = 0;
  table[i].inode_start = 0;
  strcpy(table[i].path,"\0");
  strcpy(table[i].devname,"\0");
  
  
  printf("----UMOUNT OK----\n");
  return 0;
}
