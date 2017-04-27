/*
my_read.c
*/

int my_read(int fd_num, char *buf, int nbytes)
{
	int total_bytes_read = 0;
    	char *return_buf;
	char read_buf[BLKSIZE];
	int avaliable; int logical_block; int start_byte; int disk_block;
	INODE *ip; OFT *ofp;
	char *cp;

	if(DEBUGGING)printf("read: starting read\n");
	if(DEBUGGING)printf("args: fd=%d buf=%x nbytes=%d\n", fd, buf, nbytes);

	//establish pointers to OFT and INODE for this file
	ofp = running->fd[fd_num];
	ip = &ofp->mptr->INODE;
	if(DEBUGGING)printf("read: ofp=%x ip=%x\n", ofp, ip);

	//establish pointer where we're writing read info to
	return_buf = buf;
	if(DEBUGGING)printf("read: return_buf=%x\n", return_buf);

	//Calculates the available amount of bytes to be read
	avaliable = ip->i_size - ofp->offset;
	if(DEBUGGING)printf("read: avaliable=%d\n", avaliable);

	while (nbytes > 0 && avaliable > 0){

		//compute logical_block and start_byte
		logical_block = ofp->offset / BLKSIZE;
		start_byte = ofp->offset % BLKSIZE;
		
	        if(DEBUGGING)printf("read: logical=%d\n", logical_block);	

		//logical block is direct block
		if (logical_block < 12){
			if(DEBUGGING)printf("read: direct block\n");
			disk_block = ip->i_block[logical_block];
		}
		//logical block is single indirect block
		else if (logical_block >= 12 && logical_block < 256 + 12){
			char single_indirect_buf[BLKSIZE];
			int *single_indirect_pointer;

			if(DEBUGGING)printf("read: single indirect\n");
			
			//read in single indirect block to buf
			get_block(ofp->mptr->dev, ip->i_block[12], single_indirect_buf);
			single_indirect_pointer = (int *) single_indirect_buf + (logical_block - 12);
			disk_block = *single_indirect_pointer;
		}
		//logical block is double indirect block
		else{ 
			char double_indirect_buf[BLKSIZE];
			char single_indirect_buf[BLKSIZE];
			int *double_indirect_pointer;
			int *single_indirect_pointer;

			if(DEBUGGING)printf("read: double indirect\n");
	
			//read in the double indirect block
			//create a pointer to the proper offset in the double indirect block
			get_block(ofp->mptr->dev, ip->i_block[13], double_indirect_buf);
			double_indirect_pointer = (int*) double_indirect_buf;
			double_indirect_pointer += ((logical_block - 256 - 12) / 256);

			//use value from double indirect to get single indirect block
			get_block(ofp->mptr->dev, *double_indirect_pointer, single_indirect_buf);
			single_indirect_pointer = (int*) single_indirect_buf;
			single_indirect_pointer += ((logical_block - 256 - 12) % 256);

			//use this value as the disk_block
			disk_block = *single_indirect_pointer;
		} 


		//place disk_block into read_buf
		get_block(ofp->mptr->dev, disk_block, read_buf);
		
		//make a pointer to the start of our read in this block
		cp = read_buf + start_byte;

		//calulate the data remaining to be read in this block
		int remain = BLKSIZE - start_byte;

		//two cases:
		//	remain > bytes_to_read 
		//	remain < bytes_to_read
		
		if(remain < nbytes){
			//OPTIMIZATION use memcopy to complete in one action
			memcpy(cp, return_buf, remain);

			nbytes -= remain;

			return_buf += remain;

			ofp->offset += remain;

			total_bytes_read += remain;
		}
		else{
			memcpy(cp, return_buf, nbytes);
			return_buf += nbytes;
			ofp->offset += nbytes;
			total_bytes_read += nbytes;
			nbytes = 0;
		}
		avaliable = ip->i_size - ofp->offset;
 
		// if one data block is not enough, loop back to OUTER while for more ...
      	}
	printf("myread: read %d char from file descriptor %d\n", total_bytes_read, fd_num);  
	return total_bytes_read;   // count is the actual number of bytes read
}

int read_file(char* fd_num_string, char* bytes_to_read_string){
	int fd_num; int bytes_to_read;
	OFT *open_file;
	char buf[BLKSIZE];

	//parse values to int
	fd_num = atoi(fd_num_string);
	bytes_to_read = atoi(bytes_to_read_string);

	//create pointer to OFT struct
	open_file = running->fd[fd_num];

	//verify file is open in proper mode
	if((open_file->mode == 1) || (open_file->mode == 3)){
		printf("cannot read file, mode=%d\n", open_file->mode);
		return -1;
	}

	return (my_read(fd_num, buf, bytes_to_read));
}


