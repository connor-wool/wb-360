/* balloc.c: allocate a free data block, and return it's block number **/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

//define shorter type names
typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR; //newest dir entry for e2fs

#define BLKSIZE 1024

/* I don't know how I feel about this, using global variables across
   the entire project feels very unsafe. But, it's how KC did it. I
   need to ask him why he does it this way.  */
GD	*gp;
SUPER	*sp;
INODE	*ip;
DIR	*dp;

/***** GLOBALS *****/

int fd; //INT value points to fd for the open file system
int imap, bmap; //block numbers for imap and bmap
int ninodes, nblocks, nfreeInodes, nfreeBlocks; //reference values

//read a block of BLKSIZE bytes into a buffer of BLKSIZE bytes
int get_block(int fd, int blk, char buf[])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

//write a block of BLKSIZE bytes from buffer to disk
int put_block(int fd, int blk, char buf[])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

//checks status of a single bit in a byte array
int tst_bit(char *buf, int bit){
	int i,j;		//byte/bit address scheme
	i = bit/8; j=bit%8; 	//(i -> byte#) (j -> bit#)
	
	if (buf[i] & (1 << j))	//check byte
		return 1;	//return 1 if byte is 1

	return 0;		//return 0 if byte is 0
}	

//sets a single bit to equal 1
int set_bit(char *buf, int bit){
	int i,j;		//byte/bit address scheme
	i = bit/8; j=bit%8;	//(i -> byte#) (j -> bit#)
	buf[i] |= (1 << j);	//set bit to 1
}

//set a single bit to equal 0
int clr_bit(char *buf, int bit){
	int i, j;
	i = bit/8; j=bit%8;
	buf[i] &= ~(1 << j);	//set bit to 0;	
}

//subtract 1 from the "free blocks" count in SUPER and GD
int decFreeBlocks(int dev){
	char buf[BLKSIZE];

	//decrement free blocks in SUPER
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_blocks_count--;
	put_block(dev, 1, buf);

	//decrement free blocks in GD
	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_blocks_count--;
	put_block(dev, 2, buf);
}

int balloc(int dev){
	//establish counters and buffer
	int i;
	char buf[BLKSIZE];

	//read block_bitmap block
	get_block(dev, bmap, buf);
	
	//find first free block and mark as allocated
	for (i = 0; i < nblocks; i++){
		if (tst_bit(buf, i) == 0){
			set_bit(buf, i);
			decFreeBlocks(dev);
			put_block(dev, bmap, buf);
			return i+1;
		}
	}
}

//default disk name
char *disk = "mydisk";

main(int argc, char *argv[]){
	//establish local variables and buffer
	int i, block;
	char buf[BLKSIZE];

	//check if user entered command line argument for disk name
	if (argc > 1)
		disk = argv[1];
	
	//open the file system specified as "disk"
	fd = open(disk, O_RDWR);

	//print error message if disk is not opened successfully
	if (fd < 0){
		printf("open %s failed!\n", disk);
		exit(1);
	}

	//read SUPER block and cast to pointer
	get_block(fd, 1, buf);
	sp = (SUPER*)buf;

	//print information about current fs state
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;
	nfreeInodes = sp->s_free_inodes_count;
	nfreeBlocks = sp->s_free_blocks_count;
	printf("ninodes=%d nblocks=%d nfreeInodes=%d nfreeBlocks=%d\n",
		ninodes, nblocks, nfreeInodes, nfreeBlocks);

	//read first group descriptor block
	get_block(fd, 2, buf);
	gp = (GD*)buf;

	//find what block the block bitmap is on
	bmap = gp->bg_block_bitmap;
	printf("bmap = %d\n", bmap);
	getchar();

	//allocate 5 blocks as a test
	for (i = 0; i < 5; i++){
		block = balloc(fd);
		printf("allocated block = %d\n", block);
	}

}





