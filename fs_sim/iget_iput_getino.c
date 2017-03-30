/*** important functions for the file system ***/

//read a block from a disk into a buffer
int get_block(int dev, int blk, char buf[]){
	lseek(fd, (long)blk*BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

//write a block from buffer to disk
int put_block(int dev, int blk, char buf[]){
	lseek(fd, (long)blk*BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

//get value of a single bit in a buffer
int tst_bit(char *buf, int bit){
	int i,j;
	i = bit/8; j = bit%8;
	if (buf[i] & (1 << j))
		return 1;

	return 0;
}

//set a bit in a buffer to 1
int set_bit(char *buf, int bit){
	int i,j;
	i = bit/8; j = bit%8;
	buf[i] |= (1 << j);
}

//set a bit in a buffer to 0
int clr_bit(char *buf, int bit){
	int i,j;
	i = bit/8; j = bit%8;
	buf[i] &= ~(1 << j);
}


MINODE *iget(int dev, int ino){
	int i, blk, offset;
	char buf[BLKSIZE];
	MINODE *mip;
	INODE *ip;
	//search for minode already exisiting in array 
	for (i=0; i < NMINODE; i++){
		mip = &minode[i];
		if(mip->dev == dev && mip->ino == ino){
			mip->refCount++;
			printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}
	for(i=0; i < NMINODE; i++){
		mip = &minode[i];
		if(mip->refCount == 0){
			printf("allocating NEW minode[%d] for [%d %d]\n",i,dev,ino);
			mip->refCount = 1;
			mip->dev = dev; mip->ino = ino;
			mip->dirty = mip->mounted = mip->mountPtr = 0;
			//get INODE of ino into buf;
			blk = (ino-1)/8 + iblock;
			offset = (ino -1) % 8;
			printf("iget: ino=%d blk=%d offset=%d\n",ino,blk,offset);
			get_block(dev, blk, buf);
			ip = (INODE*)buf + offset;
			//copy IONDE to mp->INODE
			mip->INODE = *ip;
			return mip;
		}
	}
	printf("PANIC: there are no more free minodes\n");
	return 0;
	
}

int iput(MINODE *mip){

}

int search(MINODE *mip, char *name){

}

int getino(int *dev, char *pathname)
{

}
