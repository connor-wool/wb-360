/*
symlink_readlink.c

*/

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

	//create new_file; change new_file to SLINK type;
	int lino = creat_file(newfile); //creat newfile
	MINODE* link_mip = iget(dev, lino);
	INODE* link_ip = &link_mip->INODE;

	link_ip->i_mode = 0120777; // LINK_MODE = 0120777

	//mark new_file's minode dirty;
	link_mip->dirty = 1;
	iput(link_mip);
		
	//get new_files parent into memory	
	parent = dirname(make_string(newfile));
	child = basename(make_string(newfile));

	//get new_files parent into memory	
	int pino = getino(&dev, parent);
	MINODE* pmip = iget(dev, pino);

	//mark new_file parent minode dirty;
	pmip->dirty = 1;
	//put(new_file's parent minode);
	iput(pmip);
	
return 1;
}


// returns the length of the target file
char* readlink(char* filename) {	//file, buffer

	dev = running->cwd->dev;
	char* result = NULL;

	// get file's INODE into memory
	int ino = getino(&dev, filename);
	MINODE* mip = iget(dev, ino);
	INODE* ip = &mip->INODE;
	

	// verify it's a SLINK file
	//if(!S_ISLNK(ip->i_mode)){
	if(ip->i_mode != 0120777){
		printf("File is not a link file!\n");
		return 0; //fail
	}


	char* contents = (char*)(ip->i_block);
	result = (char*)malloc((strlen(contents) + 1) * sizeof(char));
	strcpy(result, contents);

	iput(mip);

	return result;
}


