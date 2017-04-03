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

//print single indirect
int print_si(int blocknum){
	printf("printing single indirect blocks on block %d\n", blocknum);
	repeat_char('=', 40, 1);
	char buf[BLKSIZE];
	get_block(fd, blocknum, buf);
	int *intp = (int*)buf;
	while(*intp > 0 && intp < &buf[BLKSIZE - 1]){
		printf("%d ", *intp);
		intp++;
	}
	repeat_char('=', 40, 1);
}

//print double indirect block
int print_di(int blocknum){
	repeat_char('=', 40, 1);
	printf("printing double indirect blocks on block %d\n", blocknum);
	repeat_char('=', 40, 1);
	char buf[BLKSIZE];
	get_block(fd, blocknum, buf);
	int *intp = (int*)buf;
	while(*intp > 0 && intp < &buf[BLKSIZE - 1]){
		print_si(*intp);
		intp++;
	}
	printf("\n");
}

//print triple indirect block
int print_ti(int blocknum){
	printf("printing triple indirect blocks on block %d\n", blocknum);
	char buf[BLKSIZE];
	get_block(fd, blocknum, buf);
	int *intp = (int*)buf;
	while(*intp > 0 && intp < &buf[BLKSIZE - 1]){
		print_di(*intp);
		intp++;
	}
}

//print all blocks associated with an inode number
int print_inode_blocks(int ino){
	printf("printing disk blocks for inode #%d\n", ino);
	//get group descriptor block to find inode table
	//run mailmans to find inode
	//read inode into buf
	//read values from inode

	int inode_start = 0;
	int blk, offset, i;
	char buf[BLKSIZE];
	get_block(fd, 2, buf);
	gp = (GD*)buf;
	inode_start = gp->bg_inode_table;
	
	blk = ((ino-1)/8) + inode_start;
	offset = (ino-1) % 8;

	get_block(fd, blk, buf);
	ip = (INODE*)buf + offset;

	printf("direct blocks: ");	
	for(i = 0; i < 12; i++){
		printf("%d ", ip->i_block[i]);
	}
	printf("\n");
	
	//print single indirect blocks
	if(ip->i_block[12] > 0)
		print_si(ip->i_block[12]);

	//print double indirect blocks
	if(ip->i_block[13] > 0)
		print_di(ip->i_block[13]);

	//print triple indirect blocks
	if(ip->i_block[14] > 0)
		print_ti(ip->i_block[14]);
}

int tokenize(char *source, char *result[]){
	printf("tokenizing string\n");
	char *sacrifice, *token; 
	int i;

	sacrifice = make_string(source);
	i = 0;
	token = strtok(sacrifice, "/");
	while (token > 0){
		result[i] = make_string(token);
		token = strtok(0, "/");
		i++;
	}
	return i;
}

//get inode number of a filename
int search (INODE *inode, char *name){
	repeat_char('=', 40, 1);
	printf("!!! ->searching for %s\n", name);
	//check how many blocks inode has
	//read each block in sequence looking for name
	int nblocks, i, j;
	char buf[BLKSIZE];
	
	nblocks = inode->i_blocks;
	for(i = 0; i < nblocks; i++){
		printf("getting data block %d of %d\n", i+1, nblocks);
		get_block(fd, inode->i_block[i], buf);
		char *cp = buf;
		dp = (DIR*)cp;
		while(cp < &buf[BLKSIZE-1]){
			printf("looking at %s\n", dp->name);
			if(strcmp(name, dp->name) == 0){
				printf("found %s, is ino# %d\n", dp->name, dp->inode);
				repeat_char('=', 40, 1);
				return dp->inode;
			}
			if(dp->rec_len == 0)
				break;

			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		printf("push key to continue\n");
		getchar();
	}
	printf("not found, returning 0!\n");
	repeat_char('=', 40, 1);
	return 0;
}

INODE *getino(int ino){
	printf("getting inode #%d\n", ino);
	
	INODE *result = malloc(sizeof(INODE));
	char buf[BLKSIZE];
	int inode_block, blk, offset;
	
	get_block(fd, 2, buf);
	gp = (GD*)buf;
	inode_block = gp->bg_inode_table;
	
	blk = ((ino-1)/8) + inode_block;
	offset = (ino-1) % 8;

	get_block(fd, blk, buf);
	*result = *((INODE*)buf + offset);
	return result;
}

int search_path(int ninodes, char *pathbuf[]){
	//establish pointer to root inode
	//in while loop:
	//	return if search gives 0
	//	if at last path location, and found, print
	//	if more to go, reset pointer and loop

	printf("searching path\n");

	INODE *current;
	int i, nextinode;

	current = getino(2); 
	
	for(i = 0; i < ninodes; i++){
		nextinode = search(current, pathbuf[i]);	
		
		if(nextinode == 0)
			return 0;
		
		current = getino(nextinode);
	}
	print_inode_blocks(nextinode);
}

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
	//this was made redundant by the 'get-inode' function
	/*
	get_block(fd, 2, buf);
	gp = (GD*)buf;
	iblock = gp->bg_inode_table;
	printf("iblock is %d\n", iblock);

	//get pointer to root inode
	get_block(fd, iblock, buf);
	ip = (INODE*)buf + 1;

	//verify some info about root inode
	num_blocks = ip->i_blocks;
	printf("num blocks in root inode: %d\n", num_blocks);
	*/


	char *pathbuf[100] = {0};
	int n = tokenize(path, pathbuf);
	printf("n is %d\n", n);
	
	int i = 0;
	while(pathbuf[i] > 0){
		printf("path: %s\n", pathbuf[i]);
		i++;
	}
	
	//print superblocks info here:
	repeat_char('=', 40, 1);
	printf("   SUPERBLOCK INFO:\n");
	repeat_char('=', 40, 1);
	printf("s_inodes_count = %d\n", sp->s_inodes_count);
	printf("s_blocks_count = %d\n", sp->s_blocks_count);
	printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);
	printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
	printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);


	search_path(n, pathbuf);

}
