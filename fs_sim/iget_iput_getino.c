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

//return a copy of a string, stored in heap memory
char* make_string(char *source){
	char *result = (char*)malloc(strlen(source));
	strcpy(result, source);
	return result;
}

int tokenize(char *source, char *result[]){
	printf("tokenizing string!\n");
	char *sacrifice, *token;
	int i;

	sacrifice = make_string(source);
	i = 0;
	token = strtok(sacrifice, "/");
	while(token > 0){
		result[i] = make_string(token);
		token = strtok(0, "/");
		i++;
	}
	return i;
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
			printf("iget: found [dev=%d ino=%d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}
	for(i=0; i < NMINODE; i++){
		mip = &minode[i];
		if(mip->refCount == 0){
			printf("iget: allocating NEW minode[%d] for [dev=%d ino=%d]\n",i,dev,ino);
			mip->refCount = 1;
			mip->dev = dev; mip->ino = ino;
			mip->dirty = mip->mounted = 0;
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
	printf("iget: PANIC: there are no more free minodes\n");
	return 0;
	
}

//dispose of mionde pointed to by mip
int iput(MINODE *mip){
	int blk, offset;
	char buf[BLKSIZE];

	mip->refCount--;
	if(mip->refCount > 0)
		return 0;
	
	if(!mip->dirty)
		return 0;

	//now we know we have a dirty inode that needs to be written back to disk
	printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);
	
	//use mip->ino to compute blk of inode, offset of inode
	blk = ((mip->ino)-1)/8 + iblock;
	offset = ((mip->ino) -1) %8;

	//read in buffer
	get_block(mip->dev, blk, buf);
	ip = (INODE*)buf+offset;
	*ip = mip->INODE;
	
	//write block back to disk
	put_block(mip->dev, blk, buf);
	return 0;
}

int search(MINODE *mip, char *name){
	INODE *inode;
	int nblocks, i, j;
	char buf[BLKSIZE];

	printf("searching for inode %s in parent %d\n", name, mip->ino);
	inode = &mip->INODE;
	nblocks = inode->i_blocks / (BLKSIZE / 512);
	for(i = 0; i < nblocks; i++){
		printf("getting data block %d of %d\n", i+1, nblocks);
		get_block(mip->dev, inode->i_block[i], buf);
		char *cp = buf;
		DIR *dp = (DIR*)cp;
		while(cp < &buf[BLKSIZE-1]){
			printf("looking at %s\n", dp->name);
			if(strcmp(name, dp->name) == 0){
				printf("found %s, is ino# %d\n", dp->name, dp->inode);
				return dp->inode;
			}
			if(dp->rec_len == 0)
				break;

			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		
	}
	printf("not found, returning 0!\n");
	return 0;
}

char *search_bynumber(MINODE *mip, int target_number){
        INODE *inode;
        int nblocks, i, j;
        char buf[BLKSIZE];

        printf("searching for inode %d in parent %d\n", target_number, mip->ino);
        inode = &mip->INODE;
        nblocks = inode->i_blocks / (BLKSIZE / 512);
        for(i = 0; i < nblocks; i++){
                printf("getting data block %d of %d\n", i+1, nblocks);
                get_block(mip->dev, inode->i_block[i], buf);
                char *cp = buf;
                DIR *dp = (DIR*)cp;
                while(cp < &buf[BLKSIZE-1]){
                        printf("looking at %d\n", dp->inode);
                        if(dp->inode == target_number){
                                printf("found %d, is name %s\n", dp->inode, dp->name);
				return make_string(dp->name);
                        }
                        if(dp->rec_len == 0)
                                break;

                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }

        }
        printf("not found, returning 0!\n");
        return 0;
}


int getino(int *dev, char *pathname)
{
	int i, ino, blk, offset;
	char buf[BLKSIZE];
	INODE *inp;
	MINODE *mip;

	printf("getino: pathname=`%s`\n", pathname);
	if (strcmp(pathname, "/") == 0)
		return 2;

	if (pathname[0] == '/')
		mip = iget(*dev, 2);
	else
		mip = iget(running->cwd->dev, running->cwd->ino);

	strcpy(buf, pathname);
	
	//tokenize path
	//n = number of token strings
	char *pathbuf[100] = {0};
	int n = tokenize(pathname, pathbuf);

	for (i=0; i < n; i++){
		printf("============================\n");
		printf("getino: i=%d\n", i);

		//search for next file in path
		//ino = search(mip, name[i]);
		ino = search(mip, pathbuf[i]);

		if (ino == 0){
			iput(mip);
			printf("name %s does not exist\n", pathbuf[i]);
			return 0;
		} 
		
		iput(mip);
		mip = iget(*dev, ino);
	}
	return ino;
}

char *getinodename(int ino){
	MINODE *mip, *pmip;
	int i;
	char *result;

	//check if root, return "/"
	if(ino == 2){
		result = make_string("/");
		return result;		
	}

	//get minode by inode#
	mip = iget(dev, ino);
	
	//get the parent of this inode
	i = search(mip, "..");
	pmip = iget(dev, i);
	
	//search for this inode number in that inodes directory
	result = search_bynumber(pmip, ino);

	//return the name of this inode
	return result;
}
