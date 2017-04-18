/*
link_unlink.c
*/


//#include <libgen.h>


// link oldfile newfile
//returns 1 if success, 0 if fail
int link(char* oldfile, char* newfile) {

	char* parent, child;
	dev = running->cwd->dev;
	
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
		printf("File cannot be directory!\n");
		iput(omip);		
		return 0; //fail
	}

	//break up newfile into parent(directory) and child(proposed file)
	parent = dirname(make_string(newfile));
	child = basename(make_string(newfile));
	
	//get parent in memory
	int nino = getino(&dev, parent);
	MINODE* nmip = iget(dev, nino);



    	// Verify that newfiles parent directory exists
    	if(!nmip) {
		printf("Parent directory of new file does not exist!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
	}



/*

HERE

returns zero in the following if statement.

commands/files used:
--------------------------
creat file1
link file1 file2
--------------------------

This should work right???

*/


	if(!(nmip->INODE.i_mode == 0x4000)) { //parent must be dir
		printf("Parent of newfile must be a directory!\n");
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
    	if(omip->dev != nmip->dev) {
		printf("Cannot link across devices!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
	}

	//enter child
	//enter_name(nmip, omip->ino, child);
	enter_name(nmip, oino, child);

 	INODE* ip = &omip->INODE;
	ip->i_links_count++;
	omip->dirty = 1;


	iput(omip);
	iput(nmip);

	return 1; //success
}


// unlink file
// returns 1 for success, 0 for failure
int unlink(char* filename) {

	char* parent, child;

	int ino = getino(&dev, filename);
	MINODE* mip = iget(dev, ino);

	//verify file exists
	if(!mip) {
		printf("File does not exist!\n");
		iput(mip);
		return 0;	
	}

	//check file type (cannot be DIR)
	if(mip->INODE.i_mode == 0x4000) { //file cannot be dir
		printf("File cannot be directory!\n");
		iput(mip);		
		return 0; //fail
	}

	//split filename into parent and child
	parent = dirname(make_string(filename));
	child = basename(make_string(filename));
	
	//get parent in memory
	int pino = getino(&dev, parent);
	MINODE* pmip = iget(dev, pino);
	
	// remove basename from parent DIR
	printf("\n\n rm_child \n\n");
	printf("parent: %s\n\n", parent);
	printf("child: %s\n\n", child);

/*

seg faults on the line above, wont print 'child'??

Do these two lines (with filename = file1):
	parent = dirname(make_string(filename));
	child = basename(make_string(filename));
result in:
	parent = .
	child = file1


??????????????
*/



	rm_child(pmip, child); //(MINDODE*, char*)
	pmip->dirty = 1;
	iput(pmip);
 
 	INODE* ip = &mip->INODE;

	// decrement INODE's link_count
	mip->INODE.i_links_count--;
	if (mip->INODE.i_links_count > 0){	// (SLINK file)
		printf("\n\n SLINK \n\n");
		mip->dirty = 1; 
		iput(mip);
	}

	if (mip->INODE.i_links_count == 0){	// (!SLINK file)
		printf("\n\n !SLINK \n\n");
		truncate(mip);
		idealloc(dev, ino);//deallocate INODE;
		iput(mip);
	}

	return 1;

}


// deallocate all data blocks
int truncate(MINODE* m){
        // Deallocate its blocks
        for(int b = 0; b < 12 && ip->i_block[b] != 0; b++) {
		//int bdealloc(int dev, int blk_num)
		bdealloc(dev, ip->i_block[b]);
	}

//in for loop, 12 = number of direct blocks?
	return 1;
}







