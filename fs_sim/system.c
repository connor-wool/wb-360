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
	printf("mounting root!\n");
	root = iget(dev, 2);
}

char *disk = "mydisk";
char line[128], cmd[64], pathname[64];
char buf[BLKSIZE];

int main(int argc, char *argv[]){
	printf("starting ext2 management system\n");
	
	if (argc > 1)
		disk = argv[1];

	printf("attempting to open disk %s...\n", disk);	
	if((dev = fd = open(disk, O_RDWR)) < 0){
		printf("open on %s failed!\n", disk);
		exit(1);
	}

	printf("disk opening complete!\n");
	printf("please check that the values for fd and dev seem logical:\n");
	printf("\t--> fd=%d dev=%d\n", fd, dev);

	//check for ext2 filesystem and get some info
	printf("checking disk for ext2 filesystem...\n");
	get_block(fd, 1, buf);
	sp = (SUPER*)buf;
	if(sp->s_magic != 0xef53){
		printf("ERROR! ext2 filesystem not found on disk\n");
		printf("exiting program.\n");
		exit(1);
	}
	else{
		printf("verified: ext2 filesystem on disk!\n");
	}
}
