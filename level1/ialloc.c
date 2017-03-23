/******* ialloc.c: allocate a free INODE, return its inode number ******/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

#define BLKSIZE 1024

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

/********** globals *************/
int fd;
int imap, bmap;  // IMAP and BMAP block number
int ninodes, nblocks, nfreeInodes, nfreeBlocks;

int get_block(int fd, int blk, char buf[ ])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
 	read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[ ])
{
 	lseek(fd, (long)blk*BLKSIZE, 0);
 	write(fd, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
 	int i, j;
 	i = bit/8; j=bit%8;
	if (buf[i] & (1 << j))
	return 1;
	return 0;
}

int set_bit(char *buf, int bit)
{
	int i, j;
	i = bit/8; j=bit%8;
	buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
	int i, j;
	i = bit/8; j=bit%8;
	buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev)
{
	char buf[BLKSIZE];

	// dec free inodes count in SUPER and GD
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count--;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
 	gp = (GD *)buf;
	gp->bg_free_inodes_count--;
	put_block(dev, 2, buf);
}

int ialloc(int dev)
{
	//establish counters and buffer
	int  i;
	char buf[BLKSIZE];

	// read inode_bitmap block
	get_block(dev, imap, buf);

	//find first free inode and mark as allocated
	for (i=0; i < ninodes; i++){
   		if (tst_bit(buf, i)==0){
       			set_bit(buf,i);
       			decFreeInodes(dev);

       			put_block(dev, imap, buf);

       			return i+1;
    		}
  	}

	//return error message if all inodes allocated
  	printf("ialloc(): no more free inodes\n");
  	return 0;
}

//default disk name
char *disk = "mydisk";


main(int argc, char *argv[ ])
{
	//establish local variables and buffer
	int i, ino;
	char buf[BLKSIZE];

	//check if user entered command line argument for disk name
	if (argc > 1)
		disk = argv[1];

	//open the file system specified as "disk"
	fd = open(disk, O_RDWR);
	if (fd < 0){ //print error message if open fails
		printf("open %s failed\n", disk);
		exit(1);
	}

	// read SUPER block
	get_block(fd, 1, buf);
	sp = (SUPER *)buf;

	//print information about superblock
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;
	nfreeInodes = sp->s_free_inodes_count;
	nfreeBlocks = sp->s_free_blocks_count;
	printf("ninodes=%d nblocks=%d nfreeInodes=%d nfreeBlocks=%d\n", 
	 ninodes, nblocks, nfreeInodes, nfreeBlocks);

	//read first group descriptor block
	get_block(fd, 2, buf);
	gp = (GD *)buf;

	//find what block the inode bitmap is on
	imap = gp->bg_inode_bitmap;
	printf("imap = %d\n", imap);
	getchar();

	//allocate 5 inodes as a test
	for (i=0; i < 5; i++){  
		ino = ialloc(fd);
		printf("allocated ino = %d\n", ino);
	}
}


