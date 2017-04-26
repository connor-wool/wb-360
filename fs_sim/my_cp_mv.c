/*
my_cp_mv.c
*/

int cp(char* source, char* destination){
	int fd, gd;
	
	fd = my_open(source, "R");	//open src for READ
	
	my_touch(destination);
	gd = my_open(destination, "W"); //open dst for WR|CREAT
	
	while( n=read(fd, buf[], BLKSIZE) ){
		write(gd, buf, n);  // notice the n in write()
	}	

	my_close(fd);
	my_close(gd);
	return 1;
}

int mv(char* source, char* destination){
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