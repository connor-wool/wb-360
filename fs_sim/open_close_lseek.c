/*
open_close_lseek.c

*/


/*
open algorithms:
http://www.eecs.wsu.edu/~cs360/open_close.html

page 332 in book
*/
int open(char* file, char* given_mode) {
	int mode = 0;
	if(strcmp(mode_str, "RD") == 0)
		mode = RD;
    	else if(strcmp(mode_str, "WR") == 0)
        	mode = WR;
    	else if(strcmp(mode_str, "RW") == 0)
        	mode = RW;
    	else if(strcmp(mode_str, "APPEND") == 0)
		mode = APPEND;
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





	return 1;
}



