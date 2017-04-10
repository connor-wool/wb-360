/******************************
cd_ls_pwd.c

contains code to run the cd (chdir) ls, and pwd functions on our ext2 filesystem
******************************/

int ls_file(char *pathname){

}

int ls_directory(int ino){
        //create pointers for inode and minode
        MINODE *mip; INODE *ip;
        int i;
        char buf[BLKSIZE];
        char *cp; DIR *dp;
        mip = iget(dev, ino);
        ip = &mip->INODE;

        //read all data blocks of inode that are not empty
        i = 0;
        while(ip->i_block[i]){
                get_block(dev, ip->i_block[i], buf);
                cp = buf;
                dp = (DIR*)cp;
                while(cp < &buf[BLKSIZE-1] && dp->rec_len > 0){
			char *namebuf = (char*)malloc(dp->name_len + 1);
			memset(namebuf, 0, dp->name_len +1);
			memcpy(namebuf,dp->name,dp->name_len);
                        printf("+++ %s\n",namebuf);
                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }
                i++;
        }
}


int ls(char *pathname){
        MINODE *mip;
        int ino;

        //determine the initial device to search on
        if( pathname[0] == '/'){
                dev = root->dev;
        }
        else{
                dev = running->cwd->dev;
        }

	//sanatize input from user
	char *sacrifice = make_string(pathname);
	char *sani = (char*) malloc(256);
	memset(sani, 0, 256);
	strcat(sani, dirname(sacrifice));
	free(sacrifice);
	if(strcmp(sani, ".") != 0){
		sacrifice = make_string(pathname);
		strcat(sani, basename(sacrifice));
		free(sacrifice);
	}
	printf("string is `%s`?\n",sani);

        //get the ino for pathname
        ino = getino(&dev, pathname);
        //turn that ino into an minode in core
        mip = iget(dev, ino);

        //check if that inode points to a reg file or directory
        if(mip->INODE.i_mode & 0x8000){
                printf("ls: found a regular file at the end of ls path\n");
        }
        else{
                printf("ls: found a directory at end of ls path\n");
                ls_directory(mip->ino);
        }
}

int chdir(char *pathname){
        printf("entering chdir\n");
        printf("chdir arg string is `%s`\n", pathname);
	int ino; MINODE *mip;
        //determine initial dev
        //convert pathname to (dev, ino)
        //get MINODE pointing to (dev, ino)
        //if mip is not DEV, reject with error message
        //if mip is a DIR, dispose of old dir, set cwd to new mip
        if(pathname[0] == '/'){
                printf("starting chdir search from root\n");
		dev = root->dev;
        }
        else{
		printf("starting chdir search from relative\n");
                dev = running->cwd->dev;
        }

        ino = getino(&dev, pathname);
        mip = iget(dev, ino);

        printf("mip is now ino=[%d]\n", mip->ino);
        printf("mip i_mode =[%x]\n", mip->INODE.i_mode);

        if(mip->INODE.i_mode & 0x8000){
                printf("trying to change directory to a regular file! Rejecting.\n");
                return -1;
        }
        else if(mip->INODE.i_mode & 0x4000){
                printf("setting new working directory\n");
                //dispose of old dir
                iput(running->cwd);
                //set cwd to new mip
                running->cwd = mip;
        }

}

int pwd_helper(MINODE *mip, int child){
        //base case of finding root inode
        char buf[BLKSIZE];
        int ino; char *cp; DIR *dp;
        MINODE *parent;

        //this case short-circuits the base case of root
        if(mip->ino == 2){
                printf("/");
        }
        else{
                //search for parent in directory
                //root shouldn't search for his papa, he is his papa
                get_block(dev, mip->INODE.i_block[0], buf);
                cp = buf;
                dp = (DIR*)cp;
                while(cp < &buf[BLKSIZE-1]){
                        if(strcmp(dp->name, "..") == 0){
                                ino = dp->inode;
                                break;
                        }
                        cp += dp->rec_len;
                        dp = (DIR*)cp;
                }
                parent = iget(dev, ino);
                pwd_helper(parent, mip->ino);
        }

        //our papa printed us, and so we print our child
        get_block(dev, mip->INODE.i_block[0], buf);
        cp = buf;
        dp = (DIR*)cp;
        while(cp < &buf[BLKSIZE-1]){
                if(dp->inode == child){
                        printf("%s", dp->name);
                        break;
                }
                cp += dp->rec_len;
                dp = (DIR*)cp;
        }
}

int pwd(MINODE *mip){
        MINODE *papa;
        DIR *dp; char *cp;
        char buf[BLKSIZE];
        int ino;
        get_block(dev, mip->INODE.i_block[0], buf);
        cp = buf;
        dp = (DIR*)cp;
        while(cp < &buf[BLKSIZE-1]){
                if(strcmp(dp->name, "..") == 0){
                        ino = dp->inode;
                        break;
                }
                cp += dp->rec_len;
                dp = (DIR*)cp;
        }
        papa = iget(dev, ino);
        pwd_helper(papa, mip->ino);
        printf("\n");
}

