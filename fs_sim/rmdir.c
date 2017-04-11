//FILE TO RUN RMDIR CODE

/***************
remove a directory, to start we don't care if it's empty
***************/

int rm_dir(char *pathname){
	//initial values
	int ino; MINODE *mip; int i;
	
	if(DEBUGGING) printf("rm_dir: got pathname `%s`\n", pathname);
	
	//determine dev value
	if(pathname[0] == '/'){
		dev = root->dev;
	}	
	else{
		dev = running->cwd->dev;
	}

	//get a reference to the directory to delete
	ino = getino(&dev, pathname);
	mip = iget(dev, ino);

	//TODO: check ownership

	//TODO:Check that the inode points to a dir

	//TODO:Check that dir is not busy (refcount = 0)

	//TODO: check that dir is empty

	//deallocate the block and inode for this mip
	for(i = 0; i < 12; i++){
		if(mip->INODE.i_block[i] == 0)
			continue;
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
}


