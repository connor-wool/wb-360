/*my_write.c
 *
 */


int my_write(char *fd_num_string, char *inputstring){
	int fd_num = -1;
	char buf[BLKSIZE];
	char *buf_ptr;

	fd_num = atoi(fd_num_string);
	if(DEBUGGING) printf("write: converted string `%s` to value `%d`\n", fd_num_string, fd_num);

	//get a reference to the opened file descriptor in the running->fd array
	//verify that file descriptor number is valid
	if(running->fd[fd_num] == 0){
		printf("write: trying to access file descriptor that does not exist\n");
		return 1;
	}
	
	OFT *ofd = running->fd[fd_num];
	INODE *ip = &ofd->mptr->INODE;
	if(DEBUGGING) printf("write: created pointers to ofd and ip\n");

	//check that the file descriptor is open in a mode compatible with a write action
	if(ofd->mode == 0){
		printf("write: cannot write to read-only file!\n");
		return 1;
	}
	if(DEBUGGING) printf("write: verified that file is not write-protected\n");

	//write data into block based on offset, then write block back to disk
	int first_write_block = ofd->offset / BLKSIZE;
	get_block(ofd->mptr->dev, ip->i_block[first_write_block], buf);
	buf_ptr = buf + (ofd->offset % BLKSIZE);
	strcpy(buf_ptr, inputstring);
	put_block(ofd->mptr->dev, ip->i_block[first_write_block], buf);

	ofd->offset += strlen(inputstring);
	//add to offset value based on amount written
	
	ip->i_size += strlen(inputstring);
	//update file size
	
	ip->i_atime = ip->i_mtime = time(0L);
	//update file time
	
	//mark minode as dirty
	ofd->mptr->dirty = 1;
}
