/*** Connor's type.h file ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

//define shorter types
typedef struct ext2_super_block	SUPER;
typedef struct ext2_group_desc	GD;
typedef struct ext2_inode	INODE;
typedef struct ext2_dir_entry_2	DIR;

//global constant definitions
#define BLKSIZE 1024

//establish global pointers
GD	*gp;
SUPER	*sp;
INODE	*ip;
DIR	*dp;

//establish global variables
int fd;

/*** COMMONLY USED FUNCTIONS ***/

//move block from disk to buf
int get_block(int fd, int blk, char buf[ ])
{
        lseek(fd, (long)blk*BLKSIZE, 0);
        read(fd, buf, BLKSIZE);
}

//move block from buf to disk
int put_block(int fd, int blk, char buf[ ])
{
        lseek(fd, (long)blk*BLKSIZE, 0);
        write(fd, buf, BLKSIZE);
}

//check status of bit
int tst_bit(char *buf, int bit)
{
        int i, j;
        i = bit/8; j=bit%8;
        if (buf[i] & (1 << j))
        return 1;
        return 0;
}

//set bit to 1
int set_bit(char *buf, int bit)
{
        int i, j;
        i = bit/8; j=bit%8;
        buf[i] |= (1 << j);
}

//set bit to 0
int clr_bit(char *buf, int bit)
{
        int i, j;
        i = bit/8; j=bit%8;
        buf[i] &= ~(1 << j);
}

char *make_string(char *argument){
	char *result = (char*)malloc(strlen(argument));
	strcpy(result, argument);
	return result;
}

void repeat_char(char c, int times, int add_newline){
	int i;
	for(i = 0; i < times; i++){
		printf("%c", c);
	}
	if(add_newline)
		printf("\n");
}
