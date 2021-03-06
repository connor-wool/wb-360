/*
Connor Wool
Matthew Bourland
CS360 - Lab7 - Spring 2017
*/

#include "type.h"
#include "iget_iput_getino.c"


//initialize file system
int init(){
	int i;

	//set all minodes refcount to 0
	for(i = 0; i < NMINODE; i++){
		minode[i].refCount = 0;
	}

	//establish proc0
	//proc[0] pid = 1, uid = 0, cwd = 0, fd[*] = 0
	proc[0].pid = 1;
	proc[0].uid = 0;
	proc[0].cwd = 0;
	for(i = 0; i < NFD; i++){
		proc[0].fd[i]=0;
	}

	//establish proc1
	//proc[1] pid = 2, uid =1, cwd = 0, fd[*]=0
	proc[1].pid = 2;
	proc[1].uid = 1;
	proc[1].cwd = 0;
	for(i = 0; i < NFD; i++){
		proc[1].fd[i]=0;
	}
}


//load root inode and set root pointer to it
int mount_root(){
	printf("mount_root: mounting root!\n");
	root = iget(dev, 2);
	if(root > 0){
		printf("mount_root: root mounted successfully! root = inode[%d]\n",root->ino);
	}
	else{
		printf("mount_root: mount root failed... somehow. Exiting.\n");
		exit(1);
	}
}




//----------------------- ls ----------------------------------
int ls_directory(int ino){
	//create pointers for inode and minode
	MINODE *mip; INODE *ip;
	int i;
	char buf[BLKSIZE];
	char *cp; DIR *dp;
	mip = iget(dev, ino);
	ip = &mip->INODE;
	
	//read all data blocks of inode that are not empty
	i = 0;
	while(ip->i_block[i]){
		get_block(dev, ip->i_block[i], buf);
		cp = buf;
		dp = (DIR*)cp;
		while(cp < &buf[BLKSIZE-1]){
			printf("+++ %s\n",dp->name);
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		i++;
	}
}

int ls(char *pathname){
	MINODE *mip;
	int ino;	

	//determine the initial device to search on
	if( pathname[0] = '/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}

	//get the ino for pathname
	ino = getino(&dev, pathname);
	//turn that ino into an minode in core
	mip = iget(dev, ino);

	//check if that inode points to a reg file or directory
	if(mip->INODE.i_mode & 0x8000){
		printf("ls: found a regular file at the end of ls path\n");
	}
	else{
		printf("ls: found a directory at end of ls path\n");
		ls_directory(mip->ino);
	}		
}



// ------------------------- cd ----------------------------------


int chdir(char *pathname){
	int ino; MINODE *mip;
	//determine initial dev
	//convert pathname to (dev, ino)
	//get MINODE pointing to (dev, ino)
	//if mip is not DEV, reject with error message
	//if mip is a DIR, dispose of old dir, set cwd to new mip
	if(pathname[0] = '/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	ino = getino(&dev, pathname);
	mip = iget(dev, ino);

	if(mip->INODE.i_mode == 0x8000){
		printf("trying to change directory to a regular file! Rejecting.\n");
		return -1;
	}
	else if(mip->INODE.i_mode == 0x4000){
		//dispose of old dir
		iput(running->cwd);
		//set cwd to new mip
		running->cwd = mip;
	}
	
}



//-------------------------- pwd ----------------------------------


int pwd_helper(MINODE *mip, int child){
	//base case of finding root inode
	char buf[BLKSIZE];
	int ino; char *cp; DIR *dp;
	MINODE *parent;	

	//this case short-circuits the base case of root
	if(mip->ino == 2){
		printf("/");
	}
	else{
		//search for parent in directory
		//root shouldn't search for his papa, he is his papa
		get_block(dev, mip->INODE.i_block[0], buf);
		cp = buf;
		dp = (DIR*)cp;
		while(cp < &buf[BLKSIZE-1]){
			if(strcmp(dp->name, "..") == 0){
				ino = dp->inode;
				break;
			}
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		parent = iget(dev, ino);
		pwd_helper(parent, mip->ino);
	}

	//our papa printed us, and so we print our child
	get_block(dev, mip->INODE.i_block[0], buf);
        cp = buf;
	dp = (DIR*)cp;
        while(cp < &buf[BLKSIZE-1]){
        	if(dp->inode == child){
                	printf("%s", dp->name);
                        break;
                }
                cp += dp->rec_len;
                dp = (DIR*)cp;
	}
}

int pwd(MINODE *mip){
	MINODE *papa;
	DIR *dp; char *cp;
	char buf[BLKSIZE];
	int ino;
	get_block(dev, mip->INODE.i_block[0], buf);
        cp = buf;
	dp = (DIR*)cp;
        while(cp < &buf[BLKSIZE-1]){
                if(strcmp(dp->name, "..") == 0){
                        ino = dp->inode;
                        break;
                }
                cp += dp->rec_len;
                dp = (DIR*)cp;
        }
        papa = iget(dev, ino);
	pwd_helper(papa, mip->ino);
}



//---------------- mkdir pathname -------------------------------

//NOT COMPLETE
//returns 1 if success, 0 if fail
int make_dir(char* pathname){
	if(pathname[0] = '/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	
/*
2. Let  
     parent = dirname(pathname);   parent= "/a/b" OR "a/b"
     child  = basename(pathname);  child = "c"
	char *parent = ??  
	char *child = 	??
*/	
	pino  = getino(&dev, parent);
	pip   = iget(dev, pino); 

/*
   Verify : (1). parent INODE is a DIR (HOW?)   AND
            (2). child does NOT exists in the parent directory (HOW?);
*/	
	if(pip->INODE.i_mode == 0x4000) { // if parent INODE is DIR
		if(true) { //child does not exist in parent directory
		
		}
		else{
			printf("Child exists in parent directory!\n");
			return 0; //fail				
		}
	}
	else{
		printf("Parent is not a directory!\n");
		return 0; //fail
	}

	mymkdir(pip, child);
	pip->INODE.i_links_count--;	
	pip->INODE.i_atime = time(0L);  // touch its atime
	pip->dirty = 1;	//mark as dirty (modified)
	
	return 1; //success
}


int mymkdir(MINODE *pip, char *name){
/*	
1. pip points at the parent minode[] of "/a/b", name is a string "c") 

	how to do this?
*/	
	ino = ialloc(dev);
	bno = balloc(dev);

	mip = iget(dev,ino);
	INODE *ip = &mip->INODE;

	mip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
	mip->i_uid  = running->uid;	// Owner uid 
	mip->i_gid  = running->gid;	// Group Id
	mip->i_size = BLKSIZE;		// Size in bytes 
	mip->i_links_count = 2;	        // Links count=2 because of . and ..
	mip->i_atime = i_ctime = i_mtime = time(0L);  // set to current time
	mip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
	mip->i_block[0] = bno;             // new DIR has one data block   
	mip->i_block1] to i_block[14] = 0;
 
	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk


	//6. Write . and .. entries into a buf[ ] of BLKSIZE
	//	Then, write buf[ ] to the disk block bno;

	enter_name(pip, ino, name);
		
}


int enter_name(MINODE *pip, int myino, char *myname){
//read page 326 in textbook for more details/algorithm

}



//------------------------------rmdir pathname ---------------------

int rmdir(char* pathname){
//algorithm on 329
}


int creat(char* pathname){
//algorithm on 327
}


//link oldFilename newFilename
int link(char* oldfile, char* newfile){
// algorithm on page 330
}


//unlink filename
int unlink(char* filename){
// algorithm on page 330
}



// symlink oldname newname;    
int symlink(char* oldfile, char* newfile){
//algorithm on page 331
}  


//readlink
int readlink(char* file, char buffer[]){
// algorithm on page 331
}







//---------------------- main -------------------------------------



int main(int argc, char *argv[]){
	printf("=== Starting: EXT2 Management System ===\n");
	printf("Authors: Matt Bourland, Connor Wool\n\n");

	//open disk	
	if (argc > 1)
		disk = argv[1];
	
	printf("=== Initialization Stage ===\n");
	printf("main: attempting to open disk %s...\n", disk);	
	if((dev = fd = open(disk, O_RDWR)) < 0){
		printf("open on %s failed!\n", disk);
		exit(1);
	}


	printf("main: disk opening complete!\n");
	printf("main: please check that the values for fd and dev seem logical:\n");
	printf("main: \t--> fd=%d dev=%d\n", fd, dev);

	//check for ext2 filesystem and get some info
	printf("main: checking disk for ext2 filesystem...\n");
	get_block(fd, 1, buf);
	sp = (SUPER*)buf;
	if(sp->s_magic != 0xef53){
		printf("main: ERROR! ext2 filesystem not found on disk\n");
		printf("main: exiting program.\n");
		exit(1);
	}
	else{
		printf("main: verified: ext2 filesystem on disk!\n");
	}



}



