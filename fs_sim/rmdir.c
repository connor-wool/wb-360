//FILE TO RUN RMDIR CODE

/***************
remove a directory, to start we don't care if it's empty
***************/

/*


//---------- MATT'S WORK BELOW ------------------------------------


//http://www.eecs.wsu.edu/~cs360/rmdir.html


//returns 1 on success, 0 on failure
int rmdir(char* pathname) {
	
	char* parent, child;
	int uid = running->uid;
	int dev = running->cwd->dev; 	
	
	int ino = getino(&dev, pathname); //get inumber of pathname
	MINODE* mip = iget(dev, ino);	//get its minode[ ] pointer

	//check ownership
        if(uid != SUPER_USER || uid != mip->INODE.i_uid) {
		printf("Permission Denied!\n");
		return 0;
	}

	//check DIR type
        if(!S_ISDIR(mip->INODE.i_mode)) {
		printf("Not a directory!\n");
		iput(mip);            
		return 0;
        }
        // check if not busy
        if(mip->refCount > 1) {
		printf("File is busy\n");
		iput(mip);
		return 0;
	}

	// check if empty dir
	else if(!isEmptyDir(mip)) {
		printf("Directory Must be empty!\n");
		return 0;
	}

	// Deallocate its block and inode
	for (i=0; i<12; i++){
		if (mip->INODE.i_block[i]==0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	iput(mip); // clears mip->refCount = 0


	child = basename(make_string(filename));
	parent = dirname(make_string(filename));

	// get parent DIR's ino and Minode (pointed by pip)
	int pino = getino(&dev, parent); 
	MINODE* pmip = iget(dev, ino);
	
	INODE* pip = iget(pmip->dev, pino);
	
	rm_child(pip, child);
	
        pip->i_links_count--;
        pip->i_atime = time(0L);
        ip->i_mtime = time(0L);
	pmip->dirty = 1;
	iput(pmip);
	
	return 1; //success
}

int rm_child(MINODE* parent, char* name) {
	
	//Search parent INODE's data block(s) for the entry of name
		
	

	return 1; //success
}







*/













//---------- CONNOR'S WORK BELOW ------------------------------------




//remove a childs reference from parent dir listing
int rm_child(MINODE *parent, char *name){
	char *cp; DIR *dp; DIR *prev; char buf[BLKSIZE];
	int i = -1;
	int data_block_num = -1;

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
				data_block_num = parent->INODE.i_block[i];
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
	if(DEBUGGING) printf("rm_child: ino=%d name=%s\n", dp->inode, dp->name);	


	//case: entry is last entry in block
	if((cp + dp->rec_len) > &buf[BLKSIZE-1]){
		if(DEBUGGING) printf("`%s` is last entry in block; rec_len=[%d]\n", dp->name, dp->rec_len);
		// add dp->rec_len to prev->rec_len
		prev->rec_len += dp->rec_len;
		if(DEBUGGING) printf("`%s` is now last entry; rec_len=[%d]\n", prev->name, prev->rec_len);
	}
	
	//case: entry is only entry in block
		// first entry in data block
	else if(dp->rec_len == BLKSIZE){
		if(DEBUGGING) printf("`%s` is only entry in it's block\n", dp->name);
		//deallocate data block
		bdealloc(parent->dev, parent->INODE.i_block[i]);
		parent->INODE.i_block[i] = 0;

		//fix parent data block numbers so all non-zero are contiguous
			//TODO: check this, although it should be done by default
		//change parent filesize
		
		// move parent's NONZERO blocks upward so 
		// that there is no HOLEs in parent's data block numbers	
		parent->INODE.i_size -= BLKSIZE;
	}

	//case: entry has others following in block
		// in the middle of a block
	else{
		if(DEBUGGING) printf("`%s` has following entries in block\n", dp->name);
		//create new pointers to walk to last entry in block
		char *cp2 = cp;
		DIR *dp2 = dp;
		
		//get last entry in block
		while((cp2 + dp2->rec_len) < &buf[BLKSIZE-1]){
			cp2 += dp2->rec_len;
			dp2 = (DIR*)cp2;
		}

		//dp2 now points at last entry
		//add rec_len for target entry to final entry
			// add removed rec_len to the LAST entry of the block
		dp2->rec_len += dp->rec_len;
		
		//move cp to point to entry after target
		cp2 = cp + dp->rec_len;

			//move all entries AFTER this entry LEFT		
		//use cp(target pointer) and cp2(entry after target) to memcpy
		printf("attempting to complete memcpy call\n");
		memcpy(cp, cp2, &buf[BLKSIZE-1] - cp);
		printf("completed memcpy call\n");
		
	}
	
	//write parent data blocks back to disk
	//mark parent dirty
	if(DEBUGGING) printf("rmchild: completed if/else block\n");
	if(DEBUGGING) printf("dev=%d block=%d\n", parent->dev, data_block_num);
	put_block(parent->dev, data_block_num, buf);
	parent->dirty = 1;
	if(DEBUGGING) printf("completed rm_child subroutine\n");
}

int rm_dir(char *pathname){
	//initial values
	int ino; MINODE *mip; int i;
	MINODE *pmip; int pino;
	char *child; char *sacrifice;	

	if(DEBUGGING) printf("rm_dir: got pathname `%s`\n", pathname);
	
	// determine dev value
	if(pathname[0] == '/'){
		dev = root->dev;
	}	
	else{
		dev = running->cwd->dev;
	}

	// get child dir's name
	sacrifice = make_string(pathname);
	child = basename(sacrifice);
	if(DEBUGGING) printf("rmdir: found child name of `%s`\n", child);
	
	// get a reference to the directory to delete
		// get inumber/MINODE* of pathname
	ino = getino(&dev, pathname);
	mip = iget(dev, ino);
	if(DEBUGGING) printf("rmdir: dir to delete is ino=[%d]\n", mip->ino);

	// get a reference to the parent of this directory
		// get parent DIR's ino and Minode for later
	pino = search(mip, "..");
	pmip = iget(dev, pino);
	if(DEBUGGING) printf("rmdir: parent of deleted is ino=[%d]\n", pmip->ino);

	// check if root
	if(mip->ino == 2){
		printf("!!! Error: this system does not allow deletion of root\n");
	}

	//TODO: check ownership
	int uid = running->uid;
        if(uid != SUPER_USER || uid != mip->INODE.i_uid) {
		printf("Permission Denied!\n");
		iput(mip);
		iput(pmip);
		return 0; //fail
	}

	//TODO:Check that the inode points to a dir
        if(!S_ISDIR(mip->INODE.i_mode)) {
		printf("Not a directory!\n");
		iput(mip);
		iput(pmip);            
		return 0;
        }

	//TODO:Check that dir is not busy (refcount = 0)
        if(mip->refCount > 1) {
		printf("File is busy\n");
		iput(mip);
		iput(pmip);
		return 0;
	}

	//TODO: check that dir is empty
	if(mip->INODE.i_links_count > 2) {
		printf("Directory Must be empty!\n");
		iput(mip);
		iput(pmip);
		return 0;
	}


	//deallocate the block and inode for this mip
	if(DEBUGGING) printf("deallocating inode and blocks for directory\n");
	for(i = 0; i < 12; i++){
		if(mip->INODE.i_block[i] == 0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	iput(mip);

	//remove the child entry from the parent's directory
	rm_child(pmip, child);
	
	//decrement parent link by 1, modify atime, mtime, mark dirty, iput
	pmip->INODE.i_links_count--;
	pmip->INODE.i_atime = pmip->INODE.i_mtime = time(0L);
	pmip->dirty = 1;
	iput(pmip); // write inode to disk
}


