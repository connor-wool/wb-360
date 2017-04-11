//FILE TO RUN RMDIR CODE

/***************
remove a directory, to start we don't care if it's empty
***************/

int rm_child(MINODE *parent, char *name){
	char *cp; DIR *dp; DIR *prev; char buf[BLKSIZE];
	int i;

	//search parent data blocks for entry of name
	for(int i = 0; i < 12; i++){
		if(parent->INODE.i_block[i] == 0){
			if(DEBUGGING) printf("!!! child `%s` not found in parent ino=[%d]\n", name, parent->ino);
			return 0;
		}
		get_block(parent->dev, parent->INODE.i_block[i], buf);
		cp = buf;
		dp = (DIR*)cp;
		prev = dp;
		while(cp < &buf[BLKSIZE-1]){
			if(strcmp(dp->name, name) == 0){
				if(DEBUGGING) printf("found `%s` as ino=[%d]\n", dp->name, dp->inode);
				break;
			}
			prev = dp;
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		if(strcmp(dp->name, name) == 0){
			break;
		}
	}		

	//at this point, we have dp pointing to an entry of 'name'
	
	//case: entry is last entry in block
	if((cp + dp->rec_len) > &buf[BLKSIZE-1]){
		if(DEBUGGING) printf("`%s` is last entry in block; rec_len=[%d]\n", dp->name, dp->rec_len);
		prev->rec_len += dp->rec_len;
		if(DEBUGGING) printf("`%s` is now last entry; rec_len=[%d]\n", prev->name, prev->rec_len);
	}
	
	//case: entry is only entry in block
	else if(dp->rec_len == BLKSIZE){
		//deallocate data block
		bdealloc(parent->dev, parent->INODE.i_block[i]);
		parent->INODE.i_block[i] = 0;
		//fix parent data block numbers so all non-zero are contiguous
			//TODO: check this, although it should be done by default
		//change parent filesize
		parent->INODE.i_size -= BLKSIZE;
	}

	//case: entry has others following in block
	else{
		//create new pointers to walk to last entry in block
		char *cp2 = cp;
		DIR *dp2 = dp;
		while((cp2 + dp2->rec_len) < &buf[BLKSIZE-1]){
			cp2 += dp2->rec_len;
			dp2 = (DIR*)cp2;
		}

		//dp2 now points at last entry
		//add rec_len for target entry to final entry
		dp2->rec_len += dp->rec_len;
		
		//move cp to point to entry after target
		cp2 = cp + dp->rec_len;
		
		//use cp(target pointer) and cp2(entry after target) to memcpy
		memcpy(cp, cp2, &buf[BLKSIZE-1] - cp);
		
	}
	
	//write parent data blocks back to disk
	//mark parent dirty
	put_block(parent->dev, parent->INODE.i_block[i], buf);
	parent->dirty = 1;
}

int rm_dir(char *pathname){
	//initial values
	int ino; MINODE *mip; int i;
	MINODE *pmip; int pino;
	char *child; char *sacrifice;	

	if(DEBUGGING) printf("rm_dir: got pathname `%s`\n", pathname);
	
	//determine dev value
	if(pathname[0] == '/'){
		dev = root->dev;
	}	
	else{
		dev = running->cwd->dev;
	}

	//get child dir's name
	sacrifice = make_string(pathname);
	child = basename(sacrifice);
	if(DEBUGGING) printf("rmdir: found child name of `%s`\n", child);
	
	//get a reference to the directory to delete
	ino = getino(&dev, pathname);
	mip = iget(dev, ino);
	if(DEBUGGING) printf("rmdir: dir to delete is ino=[%d]\n", mip->ino);

	//get a reference to the parent of this directory
	pino = search(mip, "..");
	pmip = iget(dev, pino);
	if(DEBUGGING) printf("rmdir: parent of deleted is ino=[%d]\n", pmip->ino);

	//check if root
	if(mip->ino == 2){
		printf("!!! Error: this system does not allow deletion of root\n");
	}

	//TODO: check ownership

	//TODO:Check that the inode points to a dir

	//TODO:Check that dir is not busy (refcount = 0)

	//TODO: check that dir is empty

	//deallocate the block and inode for this mip
	if(DEBUGGING) printf("deallocating inode and blocks for directory\n");
	for(i = 0; i < 12; i++){
		if(mip->INODE.i_block[i] == 0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	iput(mip);

	//remove the child entry from the parent's directory listing
	rm_child(pmip, child);
	
	//decrement parent link by 1, modify atime, mtime, mark dirty, iput
	pmip->INODE.i_links_count--;
	pmip->INODE.i_atime = pmip->INODE.i_mtime = time(0L);
	pmip->dirty = 1;
	iput(pmip);
}


