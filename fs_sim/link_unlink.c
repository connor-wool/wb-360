/*
link_unlink.c
*/


// link oldfile newfile
//returns 1 if success, 0 if fail
int link(char* oldfile, char* newfile) {

	int oino = getino(&odev, old_file);
	MINODE* omip = iget(odev, oino);

    	// Verify that oldfile exists
    	if(!mip) {
		printf("File does not exist!\n");
		return 0; //fail
	}

	//check file type (cannot be DIR)
	if(omip->INODE.i_mode == 0x4000) { //file cannot be dir
		printf("File cannot be directory!\n")		
		return 0; //fail
	}
	nion = get(&ndev, new_file);
	if(nion != 0) { //new_file must not exist yet
		return 0; //fail
	}
	if (ndev != odev){ //ndev of dirname(newfile) must be same as odev
		return 0; //fail
	}
	
	//3.
	//creat entry in new_parent DIR with same ino
	//pmip -> minode of dirname(new_file);
	enter_name(pmip, omip->ino, basename(new_file));


	//4.
	omip->INODE.i_links_count++;
	omip->dirty = 1;
	iput(omip);
	iput(pmip);


return 1; // success
}


int unlink(char* oldfile) {

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







