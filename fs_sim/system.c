/*** SYSTEM.C ***/
/*
	This file contains the code for KC Wang's final project.
*/


#include "type.h"
#include "iget_iput_getino.c"

//globals
/*
MINODE minode[NMINODE];		//global minode array
MINODE *root;			//root pointer to '/'
PROC proc[NPROC], *running; 	//PROC, we will only use proc[0]

int fd, dev;				  //file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock; //file system constant values
*/

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

//quit the program gracefully
int quit(){
	int i;
	MINODE *mip;

	for(i = 0; i < NMINODE; i++){
		mip = &minode[i];
		if(mip->refCount > 0){
			mip->refCount = 1;
			mip->dirty = 1;
		}
		iput(mip);
	}
	printf("\n=== Shutting down... I don't hate you... ===\n\n");
	exit(0);
}

int ls_file(char *pathname){
	
}

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

//variables used for command processing
char *disk = "mydisk";
char line[128], cmd[64], pathname[64];
char buf[BLKSIZE];

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

	//read in GD to get info on filesystem
	get_block(fd, 2, buf);
	gp = (GD*)buf;
	nblocks = gp->bg_free_blocks_count;
	ninodes = gp->bg_free_inodes_count;
	bmap = gp->bg_block_bitmap;
	imap = gp->bg_inode_bitmap;
	iblock = gp->bg_inode_table;

	printf("main: File System Info:\n");
	printf("main: nblocks=%d ninodes=%d\n", nblocks, ninodes);
	printf("main: bmap=%d imap=%d iblock=%d\n", bmap, imap, iblock);	

	init();
	mount_root();
	
	//create p0 as the running process
	printf("main: Creating P0 as the running process...\n");
	running = &proc[0];
	printf("main: Set running->cwd to be root\n");
	running->cwd = iget(dev, 2);
	printf("main: running->cwd is ino[%d] (should be 2)\n", running->cwd->ino);
	printf("main: ref count of minode_0=[%d] (should be 2)\n", minode[0].refCount);
	printf("=== Initialization Stage Complete ===\n");

	printf("\n=== entering command processing loop ===\n\n");
	//command processing loop
	while(1){
		//clear both cmd and pathname buffer
		cmd[0] = 0;
		pathname[0] = 0;
		printf("loop complete. CWD=[%d]\n", running->cwd->ino);
		printf("input command: [ls|cd|pwd|quit] ");
		fgets(line, 128, stdin);
		line[strlen(line) - 1] = 0;
		printf("read input: `%s`\n", line);
		if(strcmp(line, "") == 0) {
			continue; }
		else{
			sscanf(line, "%s %s", cmd, pathname);
			printf("cmd=%s pathname=%s\n", cmd, pathname);
		}
		
		//execute the commands
		if(strcmp(cmd, "quit") == 0){
			quit();
		}
		if(strcmp(cmd, "ls") == 0){
			ls(pathname);
		}
		if(strcmp(cmd, "cd") == 0){
			chdir(pathname);
		}
		if(strcmp(cmd, "pwd") == 0){
			pwd(running->cwd);
		}
	}	
}
