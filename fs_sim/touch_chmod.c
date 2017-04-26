/*Code file for the touch and chmod commands
 */


int my_touch(char *path){
	int ino;
	MINODE *mip;
	
	if(DEBUGGING) printf("touch: starting my_touch\n");
	if(DEBUGGING) printf("touch: setting default dev\n");

	if(path[0] == '/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}

	if(DEBUGGING) printf("touch: dev set to %d\n", dev);
	if(DEBUGGING) printf("touch: get ino\n");

	ino = getino(&dev, path);

	if(ino == 0){
	//creat file that doesn't already exist
		creat_file(path);
		return 1;
	}

	if(DEBUGGING) printf("touch: ino=%d\n", ino);
	if(DEBUGGING) printf("touch: getting minode\n");
	
	mip = iget(dev, ino);
	
	if(DEBUGGING) printf("touch: got inode#[%d]\n", mip->ino);
	if(DEBUGGING) printf("touch: setting new time for mip (check with ls)\n");
	
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	mip->dirty = 1;
	iput(mip);
	return 1;

}

int my_chmod(char *new_val, char *path){
	int ino = 0;
	int owner = 0;
	int group = 0;
	int others = 0;
	MINODE *mip;
	
	char copy[10] = {0};
	strcpy(copy, new_val);
	
	if(DEBUGGING) printf("copy: `%s`\n", copy);
	if(DEBUGGING) printf("chmod: val=%s\n",new_val);
	if(DEBUGGING) printf("chmod: path=%s\n", path);
	
	//parse values of new mode	
	char *copy_p = &copy[2];
	others = atoi(copy_p);
	copy[2] = 0;
	copy_p--;
	group = atoi(copy_p);
	copy[1] = 0;
	copy_p--;
	owner = atoi(copy_p);

	if(DEBUGGING) printf("owner=%d group=%d others=%d\n", owner, group, others);

	//get reference to minode
	ino = getino(&dev, path);
	mip = iget(dev, ino);

	//update permissions for inode with new values
	INODE *ip = &mip->INODE;
	if(DEBUGGING) printf("mode before or-flash: [%x]\n", ip->i_mode);
	//first flash all 0 permissions into the mode using bitwise or to preserve filetype
	ip->i_mode = ip->i_mode & 0xfe00;
	if(DEBUGGING) printf("mode after or-flash: [%x]\n", ip->i_mode);
	
	ip->i_mode = ip->i_mode | (owner << 6);
	if(DEBUGGING) printf("mode after and-flash owner: [%x]\n", ip->i_mode);
	
	ip->i_mode = ip->i_mode | (group << 3);
	if(DEBUGGING) printf("mode after and-flash group: [%x]\n", ip->i_mode);
	
	ip->i_mode = ip->i_mode | (others << 0);
	if(DEBUGGING) printf("mode after and-flash others: [%x]\n", ip->i_mode);

	//mark mip as dirty
	mip->dirty = 1;
	//write mip back
	iput(mip);
}
