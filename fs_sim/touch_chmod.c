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
	int ino, owner, group, others;
	char own_s[2] = {0};
	char gp_s[2] = {0};
	char otr_s[2] = {0};
	MINODE *mip;
	
	if(DEBUGGING) printf("chmod: val=%s\n");
	if(DEBUGGING) printf("chmod: path=%s\n");

	//parse values of new mode
	own_s[0] = new_val[0];
	gp_s[0] = new_val[1];
}
