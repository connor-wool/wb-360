//CAT FILE


int my_cat(char *pathname){
	int open_fd;
	char buf[BLKSIZE], dummy = 0; //how does this work?
	int n;

	printf("KC'S COOL CAT MEOWS!\n");
	printf("CONNOR'S COOL CAT PURRS!\n");
	printf("CAT OUTPUT:\n");

	//open file for read mode
	open_fd = my_open(pathname, "0");

	print_running_fd();
	printf("pause, press enter key to continue\n");
	getchar();

	//loop until 0 is returned from read
	while( n = my_read(open_fd, buf, BLKSIZE)){
		buf[n] = 0;
		printf("%s", buf);
		getchar();
	}

	close(open_fd);
}
