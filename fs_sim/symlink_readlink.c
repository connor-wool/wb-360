/*
symlink_readlink.c

*/

//returns 1 on success, 0 on failure. creat(newfile) of link type that contains
//	the string oldfile in its iblock
int symlink(char* oldfile, char* newfile) {
	char* parent, child;
	dev = running->cwd->dev;
	
	//get oldfile into memory
	int oino = getino(&dev, oldfile);
	MINODE* omip = iget(dev, oino);

    	// Verify that oldfile exists
    	if(!omip) {
		printf("File does not exist!\n");
		iput(omip);
		return 0; //fail
	}

	// Verify that newfile does not exist already
    	if(getino(&dev, newfile) > 0) {
		printf("Newfile already exists!\n");
		iput(omip);
		return 0;
	}

	//create newfile; change new_file to SLINK type;
	creat_file(newfile); //creat newfile
	int lino = getino(&dev, newfile);
	MINODE* link_mip = iget(dev, lino);
	INODE* link_ip = &link_mip->INODE;
	
	link_ip->i_mode = 0120000; // LINK_MODE = 0120777

	//write the string oldfile to newfile's iblock	
	strcpy((char*)(link_ip->i_block), oldfile);
	link_ip->i_size = strlen(oldfile);

	//mark new_file's minode dirty
	link_mip->dirty = 1;
	iput(link_mip);
	
	return 1;
}


// returns the contents of the i_block of filename (which is the link filename)
char* readlink(char* filename) {

	dev = running->cwd->dev;
	char* result = NULL;

	// get filename's INODE into memory
	int ino = getino(&dev, filename);
	MINODE* mip = iget(dev, ino);
	INODE* ip = &mip->INODE;

	// verify it's a link file
	if(ip->i_mode != 0120000){
		printf("File is not a link file!\n");
		return 0; //fail
	}

	// get contents of filenames iblock; print/return it.
	char* contents = (char*)(ip->i_block);
	result = (char*)malloc((strlen(contents) + 1) * sizeof(char));
	strcpy(result, contents);
	printf("\n%s\n", result);
	iput(mip);

	return result;
}


