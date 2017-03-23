//WRITE YOUR OWN bmap.c to print the blocks bitmap

/**************** bmap.c file *************************/
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

char buf[BLKSIZE];
int fd;

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
   read(fd, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

bmap(){

  printf("=========================================\n");
  printf("PRINTING BLOCK BITMAP\n");
  printf("=========================================\n");



	char buf[BLKSIZE];
	int bmap, nblocks;
	int i;

	//read the SUPER block
	get_block(fd, 1, buf);
	sp = (SUPER*)buf;

	nblocks = sp->s_blocks_count;
	printf("nblocks = %d\n",nblocks);

	//read Group Descriptor 0
	get_block(fd, 2, buf);
	gp = (GD*)buf;

	bmap = gp->bg_block_bitmap;
	printf("bmap = %d\n",bmap);

	//read the block bitmap block
	get_block(fd, bmap, buf);
	
	for(i = 0; i < nblocks; i++){
		(tst_bit(buf, i)) ? putchar('1') : putchar('0');
		if (i && (i % 8) == 0)
			printf(" ");
	}
	printf("\n");

}

char *disk = "mydisk";

main(int argc, char *argv[ ])
{
  if (argc > 1)
    disk = argv[1];

  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  bmap();
}

