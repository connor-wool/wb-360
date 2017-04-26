/*
my_read.c

*/

int read_file(char* a1, char* a2){
	//verify that fd is open for R or RW
	//HOW?
	int fd = atoi(a1);
	int nbytes = atoi(a2);
	char buf[nbytes + 1];
	printf("\n\n1\n\n");
	return(my_read(fd, buf, nbytes));
}


int my_read(int fd, char *buf, int nbytes)
{
	int count = 0;
    	char *cq = buf;                // cq points at buf[ ]
	int *ip;

	if (DEBUGGING) printf("fd: %d\n buf: %s\n nbytes: %d\n", fd, buf, nbytes);

	char readbuf[1024];

	int avil, lbk, startByte, blk, indirect_blk, indirect_off, remain;

	MINODE *mip;
	OFT *oftp;

	oftp = running->fd[fd];
	mip = oftp->mptr;

	//Calculates the available amount of bytes to be read
	avil = mip->INODE.i_size - oftp->offset;

	printf("\n\n2\n\n");
	while (nbytes && avil){

		//Compute LOGICAL BLOCK number lbk and startByte in that block from offset;
		lbk       = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;
		 
		if (lbk < 12){                     // lbk is a direct block
			if (DEBUGGING) printf("Direct...\n");
			blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
		}
		else if (lbk >= 12 && lbk < 256 + 12) { 
			//  indirect blocks
			if (DEBUGGING) printf("Indirect...\n");
			get_block(mip->dev, mip->INODE.i_block[12], readbuf);
			ip = (int *)readbuf + lbk - 12;
			blk = *ip;
		}
		else{ 
			//  double indirect blocks
			if (DEBUGGING) printf("Double Indirect...\n");
			get_block(mip->dev, mip->INODE.i_block[13], readbuf);

			indirect_blk = (lbk - 256 - 12) / 256;
			indirect_off = (lbk - 256 - 12) % 256;

			ip = (int *)readbuf + indirect_blk;

			get_block(mip->dev, *ip, readbuf);

			ip = (int *)readbuf + indirect_off;
			blk = *ip;
		} 
		/* get the data block into readbuf[BLKSIZE] */
		get_block(mip->dev, blk, readbuf);
		/* copy from startByte to buf[ ], at most remain bytes in this block */
		char *cp = readbuf + startByte;   
		remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
		while (remain > 0){
			*cq++ = *cp++;             // copy byte from readbuf[] into buf[]
			oftp->offset++;           // advance offset 
			count++;                  // inc count as number of bytes read
			avil--; nbytes--;  remain--;
			if (nbytes <= 0 || avil <= 0) 
			break;
		}
 
		// if one data block is not enough, loop back to OUTER while for more ...
      	}
	printf("myread: read %d char from file descriptor %d\n", count, fd);  
	return count;   // count is the actual number of bytes read
}




void my_cat(char *path)
{
	int n, i;
	int fd = 0;

	char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]

	//check for path
	if(!path)
	{
		printf("No file provided!\n");
		return;
	}

	//open with 0 for RD
	fd = my_open(path, "R"); //open filename for READ

	while((n = my_read(fd, mybuf, 1024)))
	{	
		//null terminate the buffer
		mybuf[n] = '\0';
		i = 0;
		//print each char in the buffer, this is to handle \n
		while(mybuf[i])
		{
			putchar(mybuf[i]);
			if(mybuf[i] == '\n')
				putchar('\r');
			i++;
		}
	}
	
	printf("\n\r");
	my_close(fd);

	return;
}




