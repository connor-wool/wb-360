/*
my_cp_mv.c
*/

int my_cp(char* source, char* destination){
	int sourcef, destf, n = 0;
	char copy_buf[1024];

	if(DEBUGGING) printf("cp: open source file\n");
	sourcef = my_open(source, "R");	//open src for READ
	
	if(DEBUGGING) printf("cp: touch dest file\n");
	my_touch(destination);

	if(DEBUGGING) printf("cp: open dest file\n");
	destf = my_open(destination, "W"); //open dst for WR|CREAT

	if(DEBUGGING) printf("cp: begin copy from source to destination, press enter to continue\n");
	if(DEBUGGING) getchar();
	
	//read from source and write to destination
	while( n=my_read(sourcef, copy_buf, BLKSIZE) ){
		if(DEBUGGING) printf("got %d bytes from read\n", n);
		my_write(destf, copy_buf, n);  // notice the n in write()
	}

	if(DEBUGGING) printf("cp: finished file copy\n");
	if(DEBUGGING) getchar();

	my_close(sourcef);
	my_close(destf);
	return 1;
}

int my_mv(char* source, char* destination){
	int dev = running->cwd->dev;
	int ino = getino(&dev, source);
	if(ino == 0){
		printf("Source does not exist!\n");
		return 0;
	}
	//check whether src is on the same dev as src
	// HOW????
	// if yes,
	link(source, destination);

	// if no,
	// cp(source, destination);

	unlink(source);
	return 1;
}
