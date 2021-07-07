/************* link_unlink_symlink.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256], pathname2[256];

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

int mylink(char *old_file, char *new_file){
  char dname[256], bname[256];
  char *parent, *child;
  int oino = getino(old_file);
  MINODE *omip = iget(dev,oino);
  INODE *ip = &(omip->INODE);

  if(oino == 0){
    printf("---OLD FILE DOES NOT EXISTS CANNOT LINK---\n");
    return 0;
  }

   if((ip->i_mode & 0xF000) == 0x4000){
     printf("---IS A DIR CANNOT LINK---\n");
     return 0;
  }

   int nino = getino(new_file);
   if(nino != 0){
     printf("---NEW FILE EXISTS CANNOT LINK---\n");
     return 0;
   }
   strcpy(bname,new_file);
   strcpy(dname,new_file);

   parent = dirname(dname);
   child = basename(bname);
   int pino = getino(parent);
   MINODE *pmip = iget(dev,pino);
   int n_len = strlen(child);
   enter_name(pmip,oino,child,n_len);
   omip->INODE.i_links_count++;
   omip->dirty = 1;
   iput(omip);
   iput(pmip);
   return 1;
   
}

int myunlink(char *filename){

  int ino = getino(filename);
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);
  char dname[256], bname[256];
  char *parent, *child;

  
   if((ip->i_mode & 0xF000) == 0x4000){
     printf("---IS A DIR CANNOT UNLINK---\n");
     return 0;
  }

   
   strcpy(bname,filename);
   strcpy(dname,filename);

   parent = dirname(dname);
   child = basename(bname);

   int pino = getino(parent);
   MINODE *pmip = iget(dev,pino);
   rm_child(pmip,child);
   pmip->dirty = 1;
   iput(pmip);
   mip->INODE.i_links_count--;
   if(mip->INODE.i_links_count > 0){
     mip->dirty = 1;
   }
   else{
     bdalloc(dev,ino);
     idalloc(dev,ino);     
   }
   iput(mip);
   return 1;
}

int mysymlink(char *old_file, char *new_file){
  int oino = getino(old_file);
  MINODE *omip = iget(dev,oino);
  INODE *ip = &(omip->INODE);

  if(oino == 0){
    printf("---OLD FILE DOES NOT EXISTS CANNOT SYMLINK---\n");
    return 0;
  }

    int nino = getino(new_file);
   MINODE *nmip = iget(dev,nino);
   INODE *nip = &(nmip->INODE);

   if(nino != 0){
     printf("---NEW FILE EXISTS CANNOT SYMLINK---\n");
     return 0;
   }
 

  int x =  creat_file(new_file);

   nmip = iget(dev,x);
   nip = &(nmip->INODE);

   
   nip->i_mode = 0120777;
   strncpy((char *)nip->i_block,old_file,60);
   nip->i_size = strlen(old_file);
   nmip->dirty = 1;
   iput(nmip);
   
   omip->dirty = 1;
   iput(omip);
   return 1;
}

int myreadlink(char *file,char *buffer){
  int ino = getino(file);
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);
  strcpy(buffer,(char *)ip->i_block);
  iput(mip);
}
