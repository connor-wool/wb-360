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
	if(DEBUGGING) printf("tokenizing string `%s` on '/'\n", source);
	char *sacrifice, *token;
	int i;

	sacrifice = make_string(source);
	i = 0;
	token = strtok(sacrifice, "/");
	while(token > 0){
		result[i] = make_string(token);
		if(DEBUGGING) printf("found `%s`\n", result[i]);
		token = strtok(0, "/");
		i++;
	}
	return i;
}

int print_minode(MINODE *mip){
	if(DEBUGGING){
	printf("Printing minode #%d\n", mip->ino);
	INODE *ip = &mip->INODE;
	printf("i_mode=[%x]\n", ip->i_mode);
	printf("i_uid=[%d]\n", ip->i_uid);
	printf("i_size=[%d]\n", ip->i_size);
	printf("i_links_count=[%d]\n", ip->i_links_count);
	printf("i_blocks=[%d] (1024)\n", ip->i_blocks / 2);
	printf("i_block[0]=[%d]\n", ip->i_block[0]);
	printf("dirty=[%d]\n", mip->dirty);
	printf("-----\n");
	}
}

int print_running_fd(){
	if(DEBUGGING){
		OFT *fd = 0;
		printf("printing OPEN FILE TABLE for *running*\n");
		for(int i = 0; i < NFD; i++){
			fd = running->fd[i];
			if(fd > 0){
				printf("mode=%d refCount=%d mip=%d offset=%d\n", fd->mode, fd->refCount, fd->mptr->ino, fd->offset);
			}
		}
	}
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
			if(DEBUGGING) printf("iget: found [dev=%d ino=%d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}
	for(i=0; i < NMINODE; i++){
		mip = &minode[i];
		if(mip->refCount == 0){
			iput(mip);
			if(DEBUGGING) printf("iget: allocating NEW minode[%d] for [dev=%d ino=%d]\n",i,dev,ino);
			mip->refCount = 1;
			mip->dev = dev; mip->ino = ino;
			mip->dirty = mip->mounted = 0;
			//get INODE of ino into buf;
			blk = (ino-1)/8 + iblock;
			offset = (ino -1) % 8;
			if(DEBUGGING) printf("iget: ino=%d blk=%d offset=%d\n",ino,blk,offset);
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

	if(DEBUGGING) printf("iput: ino=[%d] ref=[%d] dirty=[%d]\n", mip->ino, mip->refCount, mip->dirty);

	if(mip->refCount > 0){
		if(DEBUGGING) printf("iput: [%d] remains in array because refcount=%d\n",mip->ino, mip->refCount);
		return 0;
	}

	if(!mip->dirty){
		if(DEBUGGING) printf("iput: [%d] no write to disk because dirty=%d\n",mip->ino, mip->dirty);
		return 0;
	}

	//now we know we have a dirty inode that needs to be written back to disk
	//printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);
	
	//use mip->ino to compute blk of inode, offset of inode
	blk = ((mip->ino)-1)/8 + iblock;
	offset = ((mip->ino) -1) %8;

	if(DEBUGGING) printf("iput: write to disk: dev=[%d] ino=[%d] blk=[%d]\n", mip->dev, mip->ino, blk);

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

	if(DEBUGGING) printf("search: name %s in ino=%d\n", name, mip->ino);
	inode = &mip->INODE;
	nblocks = inode->i_blocks / (BLKSIZE / 512);
	
	if(DEBUGGING){
		printf("search:there are [%d] blocks in this inode\n", nblocks);
		printf("pause for breath, press any key:\n");
		getchar();	
	}

	//this is causing a weird error
	for(i = 0; i < nblocks; i++){
		if(DEBUGGING) printf("search: getting data block %d of %d\n", i+1, nblocks);
		if(DEBUGGING) printf("search: bnum=[%d]\n", inode->i_block[i]);
		get_block(mip->dev, inode->i_block[i], buf);
		char *cp = buf;
		DIR *dp = (DIR*)cp;
		while(cp < &buf[BLKSIZE-1] && dp->rec_len > 0){
			if(DEBUGGING) printf("looking at [ino=%d rlen=%d n=`%s`]\n",dp->inode, dp->rec_len, dp->name);
			if(strcmp(name, dp->name) == 0){
				if(DEBUGGING) printf("found `%s` as ino=%d\n", dp->name, dp->inode);
				return dp->inode;
			}
			if(dp->rec_len == 0)
				break;

			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		
	}
	if(DEBUGGING) printf("not found, returning 0!\n");
	return 0;
}

char *search_bynumber(MINODE *mip, int target_number){
        INODE *inode;
        int nblocks, i, j;
        char buf[BLKSIZE];

        if(DEBUGGING) printf("searching for inode %d in parent %d\n", target_number, mip->ino);
        inode = &mip->INODE;
        nblocks = inode->i_blocks / (BLKSIZE / 512);
        
	if(DEBUGGING){
		printf("pause for breath, press any key to continue\n");
		getchar();
	}
	
	for(i = 0; i < nblocks; i++){
                if(DEBUGGING) printf("getting data block %d of %d\n", i+1, nblocks);
                get_block(mip->dev, inode->i_block[i], buf);
                char *cp = buf;
                DIR *dp = (DIR*)cp;
                while(cp < &buf[BLKSIZE-1]){
                        if(DEBUGGING) printf("looking at %d\n", dp->inode);
                        if(dp->inode == target_number){
                                if(DEBUGGING) printf("found %d, is name %s\n", dp->inode, dp->name);
				return make_string(dp->name);
                        }
                        if(dp->rec_len == 0)
                                break;

                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }

        }
        if(DEBUGGING) printf("not found, returning 0!\n");
        return 0;
}


int getino(int *dev, char *pathname)
{
	int i, ino, blk, offset;
	char buf[BLKSIZE];
	INODE *inp;
	MINODE *mip;

	if(DEBUGGING) printf("getino: pathname=`%s`\n", pathname);
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
		//printf("getino: i=%d\n", i);
		//ino = search(mip, name[i]);

		//search for next file in path
		ino = search(mip, pathbuf[i]);

		if (ino == 0){
			iput(mip);
			if(DEBUGGING)printf("getino: name %s does not exist\n", pathname);
			return 0;
		} 
		
		iput(mip);
		mip = iget(*dev, ino);
	}
	iput(mip);
	return ino;
}

char *getinodename(int ino){
	MINODE *mip, *pmip;
	int i;
	char *result;

	if(DEBUGGING) printf("getting inode name for [%d]\n", ino);
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

	iput(mip);
	iput(pmip);

	//return the name of this inode
	return result;
}
