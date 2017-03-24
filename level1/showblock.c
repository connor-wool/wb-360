/*** showblock.c : print blocks associated with a file on disk
	get device and pathname from user as command line arguments
	print direct, indirect, and double-indirect disk blocks
***/

/*** 
PLAN:
	1) Open device, and check for EXT2 file system
	2) Get pointer to root inode
	3) In function, parse path and search fs for file
		ret) inumber of file or 0
	4) Get pointer to found inode
	5) Print direct data blocks
	6) Print single-indirect data blocks
	7) Print double-indirect data blocks
***/

#include "type.h"

int main(int argc, char *argv[]){
	char *device, *path;
	char buf[BLKSIZE];
	int  iblock, num_blocks;

	repeat_char('=', 40, 1);
	printf("   RUNNING SHOWBLOCK.C\n");
	repeat_char('=', 40, 1);
	
	if(argc < 3){
		printf("Please use format 'a.out DEVICE PATH'\n");
		exit(1);
	}

	device = make_string(argv[1]);
	path = make_string(argv[2]);	

	printf("searching for file `%s` on device `%s`\n", path,device);

	//open device and verify success
	fd = open(device, O_RDWR);
	if (fd < 0){
		printf("open of %s failed\n", device);
		exit(1);
	}

	//read SUPER and check for ext2
	get_block(fd, 1, buf);
	sp = (SUPER*)buf;
	if (sp->s_magic != 0xef53){
		printf("EXT2 FILE SYSTEM NOT FOUND ON %s!\n", device);
		exit(1);
	}
	printf("verified: ext2 file system on disk.\n");
	
	//read GD and find where inodes begin
	get_block(fd, 2, buf);
	gp = (GD*)buf;
	iblock = gp->bg_inode_table;
	printf("iblock is %d\n", iblock);

	//get pointer to root inode
	get_block(fd, iblock, buf);
	ip = (INODE*)buf + 1;

	//verify some info about root inode
	num_blocks = ip->i_blocks;
	printf("blocks of root inode: %d\n", num_blocks);
	
	

}
