/*** TYPE.H FILE ***/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;

#define BLKSIZE 1024
#define NMINODE 100
#define NFD     16
#define NPROC   2

typedef struct minode{
	INODE INODE;
	int dev, ino;
	int refCount;
	int dirty;
	int mounted;
	struct mntable *mptr;
}MINODE;

typedef struct oft{
	int mode;
	int refCount;
	MINODE *mptr;
	int offset;
}OFT;

typedef struct proc{
	struct proc *next;
	int pid;
	int uid;
	MINODE *cwd;
	OFT *fd[NFD];
}PROC;

//global var stuff
MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;

int fd, dev;
int nblocks, ninodes, bmap, imap, iblock;
