/*
open_close_lseek.c
*/


// OpenFileTable
typedef struct open_file 
{
    int     mode;
    int     refCount;
    int     offset;
    MINODE* mip;
}OPEN_FILE;

/*
open algorithms:
http://www.eecs.wsu.edu/~cs360/open_close.html

page 332 in book
*/


//return 0 if fail. mode = 0|1|2|3 for R|W|RW|APPEND
int open(char* file, char* given_mode) {
	
	//set mode from the given user input
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
		printf("Not a valid mode\n");
		return 0;
	}
	if(pathname[0] == '/')
        	dev = root->dev;
    	else
        	dev = running->cwd->dev;
	
	//get pathname's inumber, Minode pointer
	int ino = getino(&dev, file);
	MINODE* mip = iget(dev,ino);  
// dev may change with mounting
// so this will need updating for level3

    	if(!mip)
    	{
		printf("File does not exist!\n");
		return 0;
    	}
    	// Verify it is a regular file
    	if(!S_ISREG(mip->INODE.i_mode))
    	{
		printf("File is not a regular file!\n");
        	iput(mip);
        	return 0;
	}		
	
/*
Check whether the file is ALREADY opened with INCOMPATIBLE mode:
           If it's already opened for W, RW, APPEND : reject.
           (that is, only multiple R are OK)
*/

//how to do this^^^^ ?????

	//allocate a free open file table pointer
	OPEN_FILE* oftp;
	oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->mip = mip;  // point at the file's minode[]

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
               		return(-1);
      	}

   // find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
   // Let running->fd[i] point at the OFT entry
	

    // Find process's first open fd 
    	int fd;
	for(fd = 0; fd < NFD; fd++)
    	{
        if(running->fd[fd] == NULL)
        	break;

        	if(fd == 10 - 1)
        	{
			printf("Failed to open\n");
            		iput(mip);
            		return 0;
        	}
    	}

    	OPEN_FILE* fp = NULL;
    	for(int i = 0; i < NOFT; i++)
    	{
        	fp = &OpenFileTable[i];

        // Check if the file is already open
        // and verify it is open with a compatible mode
        	if(mode != RD 
                	&& fp->refCount > 0
                	&& fp->mip == mip
                	&& fp->mode != RD)
     		{
			printf("Failed to open!\n");
            		iput(mip);
            		return 0;
        	}

        // Make entry in first not in use OpenFileTable entry
        	if(fp->refCount == 0)
        	{
            		fp->mode = mode;
            		fp->offset = offset;
            		fp->refCount++;
            		fp->mip = mip;

            // Add fd to process
            running->fd[fd] = fp;

            break;
        }

        // No more available space in the OpenFileTable
        if(i == NOFT - 1)
        {
            fprintf(stderr, "open: failed to open '%s':"
                    " Too many files open\n", pathname);
            iput(mip);
            return FILE_LIMIT;
        }
    }
	
	ip->i_atime = time(0L);
    	mip->dirty = true;

	return fd;
}



int close(int fd) {
	// Verify fd is within range.
	if((fd < 0) || (fd >= NFD)) {
		printf("File descriptor out of range!\n");
		return 0;
	}

	OPEN_FILE* fp = NULL;

	//verify running->fd[fd] is pointing at a OFT entry
	for(int i = 0; i < NOFT; i++) {
		fp = &OpenFileTable[i];

		if(fp->mip == running->fd[fd]->mip) { // running->fd[fd] is pointing at entry.
			break;
		}
		if(i == 99) {
			printf("File not in OpenFileTable!\n");
			return 0;
		}
    	}
	
	fp = running->fd[fd];
	running->fd[fd] = NULL;
	fp->refCount--;

	if (oftp->refCount > 0){ 
		return 0;
	}


	// last user of this OFT entry ==> dispose of the Minode[]
	mip = oftp->inodeptr;
	iput(mip);

	return 1;
}


int lseek() {
	//  From fd, find the OFT entry. 


	//  change OFT entry's offset to position but make sure NOT to over run either end
	//  of the file.


	//return original position


	return 1;
}








