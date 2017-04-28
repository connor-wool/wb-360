/*
open_close_lseek.c
*/


//return 0 if fail, return i (running->fd[i]) if success
//mode = 0|1|2|3 or R|W|RW|APPEND
int my_open(char* file, char* given_mode) {
	
	if(!given_mode){
		printf("No mode given!\n");
		return 0;
	}
	if(DEBUGGING) printf("file=`%s` mode=`%s`\n", file, given_mode);

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
		mode = atoi(given_mode);
	}
	if ((mode < 0) || (mode > 3)) {
		printf("Not a valid mode! (mode=%d)\n",mode);
		return 0;
	}	

	if(file[0] == '/')
        	dev = root->dev;
    	else
        	dev = running->cwd->dev;
	
	if(DEBUGGING) printf("open: get ino, open minode, get inode pointer\n");	
	//get pathname's inumber, Minode pointer
	int ino = getino(&dev, file);
	MINODE* mip = iget(dev,ino);  
	INODE* ip = &mip->INODE; 
	
	// dev may change with mounting
	// so this will need updating for level3

	if(DEBUGGING) printf("open: verify that file exists (mip != 0)\n");
	// verify file exists
    	if(!mip)
    	{
		printf("File does not exist!\n");
		return 0;
    	}

	if(DEBUGGING) printf("open: verify that file is regular file\n");
    	// Verify file is a regular file
    	if(!S_ISREG(mip->INODE.i_mode))
    	{
		printf("File is not a regular file!\n");
        	iput(mip);
        	return 0; //fail
	}

	if(DEBUGGING) printf("open: check if file is already opened\n");
	
	//first we create a null pointer that will be used to point to each OFT entry in the proc's array in turn.
	OFT* fp = 0;

	//then we create our for loop that will look at each entry in the OFT table of the current proc.
	for(int i = 0; i < NFD; i++){

		//we set our OFT pointer (called fp) to whatever is in the array in the proc at this index.
		if(DEBUGGING) printf("running->fd[%d]=%d\n",i, running->fd[i]);
		fp = running->fd[i];
		if(DEBUGGING) printf("fp=%d\n", fp);
		
		//print some info about the current fp pointer
		if(DEBUGGING && fp > 0){
			printf("fp=0x%x\n");
			printf("mode=%d\n", mode);
			printf("fp->refCount=%d\n", fp->refCount);
			printf("fp->mptr=%x mip=%x\n", fp->mptr, mip);
			printf("fp->mode=%d\n", fp->mode);
		}

		//now we check:
		//	is fp not null?
		//	is the fp minode the same as our open mip?
		//	is the mode argument something other than read?
		//	is the refcount of this file table object greater than 0?
		//	is the mode of this file table object something other than read?
        	if(fp > 0 && fp->mptr == mip && mode != 0 && fp->refCount > 0  && fp->mode != 0)
		{
			printf("File is already open in an incompatible mode!\n");
            		iput(mip);
            		return 0; //fail
		}
	}


	if(DEBUGGING) printf("make a new OFT struct to store our data in (using malloc)\n");
	//allocate a free open file table pointer
	OFT* oftp = (OFT*)malloc(sizeof(OFT));
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
    	if(DEBUGGING) printf("finding open space in OFT array for new file\n");
	
	/*
	 *THIS CHUNK OF CODE IS THE BANE OF MY FUCKING EXISTENCE,
	  IF THE VARIABLE IS NON-ZERO, THEN WHY DOES (running->fd[i] == 0) EVAL TO TRUE???????
	 */

	int free_OFT_slot = 0;
	for(int i = 0; i < NFD; i++){
		if(DEBUGGING) printf("running->fd[%d]=0x%x=%d\n", i, running->fd[i], running->fd[i]);
        	if(running->fd[i] == 0 || running->fd[i]->mptr->refCount < 1){
			if(DEBUGGING) printf("found open space at running->fd[%d]=%d\n", i, running->fd[i]);
        		free_OFT_slot = i;
			break;
		}
        	if(i == NFD - 1){
			printf("Failed to open\n");
            		iput(mip);
            		return 0;
        	}
    	}

	if(DEBUGGING) printf("creating new OFT entry at running->fd[%d]\n", free_OFT_slot);
	
   	// Let running->fd[i] point at the OFT entry
	running->fd[free_OFT_slot] = oftp;

	// update INODE's time field
	if (mode == 0)
		ip->i_atime = time(0L);
	else{
		ip->i_atime = time(0L);
		ip->i_mtime = time(0L);	
	}		
	mip->dirty = 1; //mark dirty
	//don't run iput yet, because we want to retain a reference to this minode so it doesn't get overwritten
	//iput(mip); 
	printf("fd: %d\n", free_OFT_slot);
	return free_OFT_slot;
}



// return 1 on successs, 0 on failure
int my_close(int fd_val) {

	// Verify fd is within range.
	if((fd_val < 0) || (fd_val >= NFD)) {
		printf("File descriptor out of range!\n");
		return 0;
	}

	//make a pointer to reference the OFT object for this file
	OFT *ofd;
	ofd = running->fd[fd_val];

	//verify running->fd[fd] is pointing at a OFT entry
	if(ofd == 0 || ofd->refCount < 1){
		printf("no such file descriptor to close!\n");
		return 1;
	}	

	//if the refCount is greater than 1, just decrement
	if(ofd->refCount > 1){
		ofd->refCount--;
		return 0;
	}

	//at this point refcount is 1 and we want to close the file.
	//push the minode back to the disk
	iput(ofd->mptr);

	//remove the entry from the OFT listing
	free(ofd);
	running->fd[fd_val] = 0;
	return 0;
}


int my_lseek(char *fd_number_string, char *seek_value_string) {
	//  From fd, find the OFT entry. 
	int fd_number;
	int seek_value;

	fd_number = atoi(fd_number_string);
	seek_value = atoi(seek_value_string);

	if(DEBUGGING) printf("lseek: fd=[%d] seek=[%d]\n", fd_number, seek_value);

	//get pointer to file
	OFT *ofd = running->fd[fd_number];

	//check if seek is larger than size
	if(seek_value > ofd->mptr->INODE.i_size){
		printf("TRYING TO SEEK PAST END OF FILE! NO CAKE FOR YOU!\n");
		return 1;
	}
	if(seek_value < 0){
		printf("TRYING TO SEEK TO POINT BEFORE FILE! NO CAKE FOR YOU!\n");
		return 1;
	}
	
	int original_offset = ofd->offset;
	ofd->offset = seek_value;

	if(DEBUGGING) printf("lseek: original_offset=[%d] new_offset=[%d]\n", original_offset, ofd->offset);

	return original_offset;
}

