/*
link_unlink.c
*/


//#include <libgen.h>



// link oldfile newfile
//returns 1 if success, 0 if fail
int link(char* oldfile, char* newfile)
{
    char* parent = NULL;
    char* child  = NULL;
    dev = running->cwd->dev;

	//get oldfile into memory
    int oino = getino(&dev, oldfile);
    MINODE* omip = iget(dev, oino);

    	// Verify that oldfile exists
    if(!omip)
    {
		printf("File does not exist!\n");
		iput(omip);
		return 0; //fail
    }


	//check oldfile type (cannot be DIR)
    if(S_ISDIR(omip->INODE.i_mode))
    {
		printf("File cannot be directory!\n");
		iput(omip);		
		return 0; //fail
    }

//break up newfile into parent(directory) and child(proposed file)
	parent = dirname(make_string(newfile));
	child = basename(make_string(newfile));

	printf("parent: %s\n\n", parent);
	printf("child: %s\n\n", child);

    // Get parent in memory
    int nino = getino(&dev, parent);
    MINODE* nmip = iget(dev, nino);

    	// Verify that newfiles parent directory exists
    if(!nmip)
    {
		printf("Parent directory of new file does not exist!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
    }
//parent must be dir
    if(!S_ISDIR(nmip->INODE.i_mode))
    {
		printf("Parent directory of new file does not exist!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
    }
	// Verify that child (proposed file) does not exist already
    if(getino(dev, newfile) > 0)
    {
		printf("Parent directory of new file does not exist!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
    }

    	// Verify that link is not being made across devices
    if(omip->dev != nmip->dev)
    {
		printf("Parent directory of new file does not exist!\n");
		iput(omip);
		iput(nmip);
		return 0; //fail
    }

	//enter child
 	enter_name(nmip, oino, child);
	

    INODE* ip = &omip->INODE;

//increment links_count
    ip->i_links_count++;
    omip->dirty = 1;

	iput(omip);
	iput(nmip);


    return 1;
}





// unlink file. Unlink command can also be used to delete any regular file
// returns 1 for success, 0 for failure
int unlink(char* filename) {
	
	int dev = running->cwd->dev;	
	char* parent;
	char* child = "init!";
	//No idea why... but initializing child and not parent makes it work...

	int ino = getino(&dev, filename);
	MINODE* mip = iget(dev, ino);

	//verify file exists
	if(!mip) {
		printf("File does not exist!\n");
		iput(mip);
		return 0;	
	}

	//check file type (cannot be DIR)
	if(S_ISDIR(mip->INODE.i_mode)){ //file cannot be dir
		printf("File cannot be directory!\n");
		iput(mip);		
		return 0; //fail
	}

	INODE* ip = &mip->INODE;
	
	// Deallocate its blocks
	for(int b = 0; b < 12 && ip->i_block[b] != 0; b++)
		bdealloc(dev, ip->i_block[b]);
	
	// Deallocate its inode
	idealloc(dev, ino);

	//split filename into parent and child
	child = basename(make_string(filename));
	parent = dirname(make_string(filename));


	//get parent in memory
	int pino = getino(&dev, parent);
	MINODE* pmip = iget(dev, pino);
					
	// remove basename from parent DIR
	printf("\n\n rm_child \n\n");
	rm_child(pmip, child);
	
	pmip->INODE.i_links_count--;

	iput(mip);
	iput(pmip);
	return 1;
}








//------ below is old code... unused ----------------------------



/*
//-------------------------------------------------------


	//get parent in memory
	int pino = getino(&dev, parent);
	MINODE* pmip = iget(dev, pino);

	printf("\nfilname: %s\n\n", filename);
	printf("parent: %s\n\n", parent);
	printf("child: %s\n\n", child);

	// remove basename from parent DIR
	printf("\n\n rm_child \n\n");
	rm_child(pmip, child);
	
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


*/


