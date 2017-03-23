//Search for a named file in a directory inode

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

#define BLKSIZE 1024

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

int fd;

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd,(long)blk*BLKSIZE, 0);
   read(fd, buf, BLKSIZE);
}

/*
INODE get_inode(int fd, int ino, char buf[])
{
	int iblock;
		
	//establish local buffer
	char buf[BLKSIZE];

	//read the GD block to find inodes
	get_block(fd, 2, buf);
	gp = (GD*)buf;
	iblock = gp->bg_inode_table;
	printf("inode block = %d\n", iblock);

	//complete mailmans calc to find block containing inode
	int block_number = ino / 8;
	int inode_offset = (ino % 8) * (sizeof(INODE));

	//read block and find inode in that block
	get_block(fd, block_number, buf);
	char cp = buf;
	cp += inode_offset;
	ip = (INODE*)cp;
	return *ip;
}
*/

//search a directory inode for specified file name
int search(INODE *inode, char *name)
{
	//read the inode's data array, and find first block
	int first_block = inode->i_block[0];
	char buf[BLKSIZE];
	get_block(fd, first_block, buf);

	//establish pointers to the dir entries in data block
	char *cp = buf;
	dp = (DIR*) cp;
	
	//read through the dir entries trying to find name
	while(cp < &buf[1023]){
		if(strcmp(name, dp->name) == 0){
			return dp->inode;
		}
		cp += dp->rec_len;
		dp = (DIR*) cp;
	}
	return 0;
	
	
}

char *disk = "mydisk";
int main(int argc, char *argv[])
{
	printf("Test Driver for 'search(ino, name)' function\n");
	if (argc > 1)
		disk = argv[1];

	fd = open(disk, O_RDONLY);
	if (fd < 0){
		printf("open %s failed\n", disk);
		exit(1);
	}	

	//get the root inode
	//get gd block to find inodes
	char buf[BLKSIZE];
	get_block(fd, 2, buf);
	gp = (GD *)buf;
	//get first inode block number
	int iblock = gp->bg_inode_table;
	//get inode block
	get_block(fd, iblock, buf);
	ip = (INODE *)buf + 1;

	//get a value of the file to find
	printf("search for 'f1' in root inode\n");
	int value = search(ip, "z");
	printf("search value = %d\n", value);
}






