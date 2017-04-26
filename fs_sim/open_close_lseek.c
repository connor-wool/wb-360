/*
open_close_lseek.c
*/


/*
open algorithms:
http://www.eecs.wsu.edu/~cs360/open_close.html

page 332 in book
*/


//return 0 if fail, return i (running->fd[i]) if success
//mode = 0|1|2|3 or R|W|RW|APPEND
int my_open(char* file, char* given_mode) {
	if(!given_mode){
		printf("No mode given!\n");
		return 0;
	}
	//set mode from the given user input (accepts 0-3 or R-APPEND)
	int mode = -1;
	if(strcmp(given_mode, "R") == 0)
		mode = 0;
    	else if(strcmp(given_mode, "W") == 0)
        	mode = 1;
    	else if(strcmp(given_mode, "RW") == 0)
        	mode = 2;
    	else if(strcmp(given_mode, "APPEND") == 0)
		mode = 3;
	else {
		mode = given_mode;
	}
	if (mode != (0 | 1 | 2 | 3)) {
		printf("Not a valid mode!\n");
		return 0;
	}	

	if(file[0] == '/')
        	dev = root->dev;
    	else
        	dev = running->cwd->dev;
	
	//get pathname's inumber, Minode pointer
	int ino = getino(&dev, file);
	MINODE* mip = iget(dev,ino);  
	INODE* ip = &mip->INODE; 
// dev may change with mounting
// so this will need updating for level3

	// verify file exists
    	if(!mip)
    	{
		printf("File does not exist!\n");
		return 0;
    	}

    	// Verify file is a regular file
    	if(!S_ISREG(mip->INODE.i_mode))
    	{
		printf("File is not a regular file!\n");
        	iput(mip);
        	return 0; //fail
	}


	//Check whether the file is ALREADY opened with INCOMPATIBLE mode
	OFT* fp = NULL;
	for(int i = 0; i < NOFT; i++){
	fp = running->fd[i];
		
        	if(mode != 0 
			&& fp->refCount > 0
                	&& fp->mptr == mip
                	&& fp->mode != 0)
		{
			printf("File is already open in an incompatible mode!\n");
            		iput(mip);
            		return 0; //fail
		}
	}


	//allocate a free open file table pointer
	OFT* oftp;
	oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->mptr = mip;  // point at the file's minode[]

	switch(mode){
		case 0 : 
			oftp->offset = 0;     // R: offset = 0
                	break;
         	case 1 : 
			truncate(mip);        // W: truncate file to 0 size
                	oftp->offset = 0;
                  	break;
         	case 2 : 
			oftp->offset = 0;     // RW: do NOT truncate file
                	break;
         	case 3 : 
			oftp->offset =  mip->INODE.i_size;  // APPEND mode              
			break;
		default: 
			printf("invalid mode\n");
               		return(0);
      	}

    	// find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
    	int fd;
	for(fd = 0; fd < NFD; fd++){
        	if(running->fd[fd] == NULL)
        		break;

        	if(fd == NFD - 1)
        	{
			printf("Failed to open\n");
            		iput(mip);
            		return 0;
        	}
    	}
	
   	// Let running->fd[i] point at the OFT entry
	running->fd[fd] = oftp;

	// update INODE's time field
	if (mode == 0)
		ip->i_atime = time(0L);
	else{
		ip->i_atime = time(0L);
		ip->i_mtime = time(0L);	
	}		
	mip->dirty = 1; //mark dirty
	iput(mip); 
	printf("fd: %d\n", fd);
	return fd;
}



// return 1 on successs, 0 on failure
int my_close(int fd) {
	// Verify fd is within range.
	if((fd < 0) || (fd >= NFD)) {
		printf("File descriptor out of range!\n");
		return 0;
	}

	OFT* fp;
	//verify running->fd[fd] is pointing at a OFT entry
	for(int i = 0; i < NOFT; i++) {
	fp = &running->fd[i];

		if(fp->mptr == running->fd[fd]->mptr) { // running->fd[fd] is pointing at entry.
			break;
		}
		if(i == NOFT - 1) {
			printf("File not in OpenFileTable!\n");
			return 0;
		}
    	}
	

	// close file
	fp = running->fd[fd];
	running->fd[fd] = 0;
	fp->refCount--;

	if (fp->refCount > 0){ 
		return 1; // file still has another instance open, but success
	}

	// last user of this OFT entry ==> dispose of the Minode[]
	MINODE* mip = fp->mptr;
	iput(mip);
	return 1;
}


int my_lseek() {
	//  From fd, find the OFT entry. 


	//  change OFT entry's offset to position but make sure NOT to over run either end
	//  of the file.


	//return original position


	return 1;
}









