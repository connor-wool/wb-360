/*my_write.c
 *
 */


int my_write(int fd_num, char *buf, int nbytes){
	int bytes_to_write = nbytes;
	int logical_block; int start_byte;
	int disk_block;
	OFT *ofp;
	INODE *ip;
	char wbuf[BLKSIZE];
	char *cp;
	char *read_pointer;

	//make a pointer to the OTF object
	ofp = running->fd[fd_num];
	ip = &ofp->mptr->INODE;

	//make a pointer to keep track of our progress through buf
	read_pointer = buf;

	while(bytes_to_write > 0){
		logical_block = ofp->offset / BLKSIZE;
		start_byte = ofp->offset % BLKSIZE;

		//------FIND BLOCK NUMBER ON DISK---------

		//direct block writes
		if(logical_block < 12){
			
			//check if block exists
			if(ip->i_block[logical_block] == 0){

				//allocate new block if it doesn't exist
				ip->i_block[logical_block] = balloc(ofp->mptr->dev);
				
				//write 0's to block on disk
				memset(wbuf, 0, BLKSIZE);
				put_block(ofp->mptr->dev, ip->i_block[logical_block], wbuf);
			}	
			disk_block = ip->i_block[logical_block];

				
		}
		//single indirect block
		else if (logical_block >= 12 && logical_block < 256 + 12){
			//get the indirect block, and from there find the data block
			char buf2[BLKSIZE];
			//get the indirect listing block into buf2
			get_block(ofp->mptr->dev, ip->i_block[12], buf2);
			//create an int pointer to the start of the indirect block
			int *int_ptr = buf2;
			//move int pointer to the block number we want to read
			int_ptr += logical_block - 12;
			//check if block exists
			if(*int_ptr == 0){
				*int_ptr = balloc(ofp->mptr->dev);
				memset(wbuf, 0, BLKSIZE);
				put_block(ofp->mptr->dev, *int_ptr, wbuf);
			}
			disk_block = *int_ptr;
		}
		//double indirect block TODO
		else{
			//get the double indirect block
			char buf2[BLKSIZE];
			get_block(ofp->mptr->dev, ip->i_block[13], buf2);
			int *int_ptr = buf2;
			//double indirect block points to single indirect blocks, 256 of them, each with 256 entries
			int_ptr += (logical_block - 256 - 12) / 256;
			//int pointer is now value of single indirect block
			get_block(ofp->mptr->dev, *int_ptr, buf2);
			int_ptr = buf2;
			int_ptr += (logical_block - 256 - 12) % 256;
			//int pointer is now value of data block
			if(*int_ptr == 0){
				*int_ptr = balloc(ofp->mptr->dev);
				memset(wbuf, 0, BLKSIZE);
				put_block(ofp->mptr->dev, *int_ptr, wbuf);
			}

		}

		//------WRITE DATA TO FOUND BLOCK NUMBER
		//all cases will fall through to this point
		
		//read in the disk block	
		get_block(ofp->mptr->dev, disk_block, wbuf);
		
		//make a pointer to the start of our write in this block
		cp = wbuf + start_byte;

		//calculate the space remaining to write to in this block
		int remain = BLKSIZE - start_byte;

		//TWO CASES:
		//	remain > bytes_to_write
		//	remain < bytes_to_write
	
		//bytes to write is more than space in block
		if(remain < bytes_to_write){
			//OPTIMIZATION -- USE MEM COPY
			memcpy(cp, read_pointer, remain);

			//decrement bytes to write by bytes written
			bytes_to_write -= remain;

			//advance read pointer by bytes written
			read_pointer += remain;

			//increment offset by bytes written
			ofp->offset += remain;

			//adjust size of file if needed
			if(ofp->offset > ip->i_size){
				ip->i_size = ofp->offset;
			}
		}
		//case where bytes to write is less than space in block
		else{
			memcpy(cp, read_pointer, bytes_to_write);
			read_pointer += bytes_to_write;
			ofp->offset += bytes_to_write;
			if(ofp->offset > ip->i_size){
				ip->i_size = ofp->offset;
			}
			bytes_to_write = 0;
		}

		//write the new data back to disk
		put_block(ofp->mptr->dev, disk_block, wbuf);
	}

	ofp->mptr->dirty = 1;
	printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd_num);
	return nbytes;
}

int write_file(char *fd_num_string, char *string_to_write){
	int fd_num, string_length;
	OFT *opened_file;
	char buf[BLKSIZE];

	//parse number string to a number
	fd_num = atoi(fd_num_string);

	//verify file is open in proper mode
	opened_file = running->fd[fd_num];
	if(opened_file->mode < 1){
		printf("file is opened in write-protected mode!\n");
		return -1;
	}

	//copy the string into a buf and get length in bytes
	strcpy(buf, string_to_write);
	string_length = strlen(string_to_write);

	return( my_write(fd_num, buf, string_length));
}	
