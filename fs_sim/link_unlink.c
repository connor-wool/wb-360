/*
link_unlink.c
*/


// link oldfile newfile
//returns 1 if success, 0 if fail
int link(char* oldfile, char* newfile) {

	char* parent, child;
	int dev = running->cwd->device;
	
	//get 
	int oino = getino(&dev, oldfile);
	MINODE* omip = iget(dev, oino);

    	// Verify that oldfile exists
    	if(!omip) {
		printf("File does not exist!\n");
		iput(omip);
		return 0; //fail
	}

	//check oldfile type (cannot be DIR)
	if(omip->INODE.i_mode == 0x4000) { //file cannot be dir
		printf("File cannot be directory!\n")
		iput(omip);		
		return 0; //fail
	}

	//break up newfile into parent(directory) and child(proposed file)
	parent = dirname(make_string(newfile));
	child = basename(make_string(newfile));
	
	//get parent in memory
	nino = getino(&dev, parent);
	MINODE* nmip = iget(dev, nino);



    	// Verify that newfiles parent directory exists
    	if(!nmip) {
		printf("Parent directory of new file does not exist!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
	}

	if(!(nmip->INODE.i_mode == 0x4000)) { //parent directory must be dir
		printf("Parent of newfile must be a directory!\n")
		iput(omip);
		iput(nmip);	
		return 0; //fail
	}
	

	// Verify that child (proposed file) does not exist already
    	if(getino(&dev, newfile) > 0) {
		printf("Newfile already exists!\n");
		iput(omip);
		iput(nmip);
		return 0;
	}


    	// Verify that link is not being made across devices
    	if(omip->device != nmip->device) {
		printf("Cannot link across devices!\n")
		iput(omip);
		iput(nmip);
		return 0; //fail
	}

	
	//enter child
	enter_name(nmip, omip->ino, child);

 	INODE* ip = &omip->inode;
	ip->i_links_count++;
	omip->dirty = 1;


	iput(omip);
	iput(nmip);

	return 1; //success
}


// unlink file
// returns 1 for success, 0 for failure
int unlink(char* filename) {

	ino = getino(&dev, filename);
	mip = iget(dev, ino);

	//verify file exists
	if(!mip) {
		printf("File does not exist!\n");
		iput(mip);
		return 0;	
	}

	//check oldfile type (cannot be DIR)
	if(mip->INODE.i_mode == 0x4000) { //file cannot be dir
		printf("File cannot be directory!\n")
		iput(mip);		
		return 0; //fail
	}

 	INODE* ip = &mip->inode;



	

return 1;
}


/*********** Algorithm of unlink ************
unlink(char *filename)
{
1. get filenmae's minode:
ino = getino(&dev, filename);
mip = iget(dev, ino);
check it's a REG or SLINK file
2. // remove basename from parent DIR
rm_child(pmip, mip->ino, basename);
pmip->dirty = 1;
iput(pmip);
3. // decrement INODE's link_count
mip->INODE.i_links_count--;
if (mip->INODE.i_links_count > 0){
mip->dirty = 1; iput(mip);
}
4. if (!SLINK file)
// assume:SLINK file has no data block
truncate(mip); // deallocate all data blocks
deallocate INODE;
iput(mip);
}

/*







