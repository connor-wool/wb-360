/***********************************
mkdir_creat.c

This file contains code to make new directories and files
***********************************/

/********************************************************
=== PUT NAME OF A FILE INTO PARENT DIR RECORD ENTRIES ===
********************************************************/
int enter_name(MINODE *pip, int myino, char *myname){
    printf("entering name %s into parent ino=[%d]\n", myname, pip->ino);
    int i;
    DIR *dp;
    char *cp;

    printf("search for last data block:\n");
    i = 1;
    for(int i = 1; i < 12; i++){
        printf("looking at data block [%d] val=[%d]\n", i, pip->INODE.i_block[i]);
        if (pip->INODE.i_block[i] == 0){
            break;
        }
    }
    
    //read last data block of parent into buf
    char buf[BLKSIZE];
    printf("reading in i_block[%d]\n", i-1);
    get_block(pip->dev, pip->INODE.i_block[i-1], buf);

    dp = (DIR*)buf;
    cp = buf;

    printf("searching for last dir entry\n");
    while(cp + dp->rec_len < buf + BLKSIZE && dp->rec_len > 0){
        printf("moving past %s\n", dp->name);
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }

    //now we should be looking at the last inode;
    printf("last dir entry is: [%s]?\n", dp->name);
    printf("last dir size is: [%d] bytes\n",dp->rec_len);

    int ideal_length = 4*((8 + dp->name_len + 3)/4);
    printf("new ideal length is: [%d]\n", ideal_length);

    int remain = dp->rec_len - ideal_length;
    printf("there is [%d] bytes remaining in block\n", remain);

    int need_length = 4*((8 + strlen(myname) + 3)/4);
    printf("we need [%d] bytes to enter new inode\n", need_length);

    if(remain >= need_length){
        printf("there is enough space, creating entry\n");
	dp->rec_len = ideal_length;
        cp += dp->rec_len;
        dp = (DIR*)cp;
        dp->inode = myino;
        dp->rec_len = remain;
        dp->name_len = strlen(myname);
        strcpy(dp->name, myname);
	printf("final entry now: [ino=%d rlen=%d n=%s]\n", dp->inode, dp->rec_len, dp->name);
	put_block(pip->dev, pip->INODE.i_block[i-1], buf);
    }    
    else{
	printf("not enough space, making new data block\n");
        //we need to allocate a new data block for this entry
        int bno = balloc(pip->dev);
        pip->INODE.i_size += BLKSIZE;
        pip->INODE.i_block[i] = bno;
        get_block(pip->dev, bno, buf);
        dp = (DIR*)buf;
        dp->inode = myino;
        dp->rec_len = BLKSIZE;
        dp->name_len = strlen(myname);
        strcpy(dp->name, myname);
	put_block(pip->dev, bno, buf);
    }
}

/*********************************************
=== CREATE NEW DIR ENTRY WITH 1 DATA BLOCK ===
*********************************************/

int mymkdir(MINODE *pip, char *name){
    //pip points to parent minode, name is string
    //allocate an inode and a disk block for directory
    int ino = ialloc(dev);
    int bno = balloc(dev);
    if(DEBUGGING) printf("make new dir using: ino=%d bno=%d\n", ino, bno);

    //load the new inode into memory
    MINODE *mip = iget(dev, ino);

    //write to mip->INODE to make it a dir
    INODE *ip = &mip->INODE;
    ip->i_mode = 0x41ED;
    ip->i_uid = running->uid;
    //ip->i_gid = running->gid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2;
    ip->i_block[0] = bno;
    for(int i = 1; i < 15; i++){
        ip->i_block[i] = 0;
    }
    mip->dirty = 1;

    print_minode(mip);

    iput(mip);

    char buf[BLKSIZE];
    if(DEBUGGING) printf("reading in bno=[%d]\n", bno);
    get_block(dev, bno, buf);
    memset(buf, 0, BLKSIZE);
    char *cp = buf;
    DIR *dp = (DIR*)cp;
    dp->inode = mip->ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    strcpy(dp->name, ".");
    cp += 12;
    dp = (DIR*)cp;
    dp->inode = pip->ino;
    dp->rec_len = 1012;
    dp->name_len = 2;
    strcpy(dp->name, "..");
    put_block(dev, bno, buf);
    
    get_block(dev, bno, buf);
    cp = buf;
	dp = (DIR*)cp;
	while(cp < &buf[BLKSIZE-1]){
		dp = (DIR*)cp;
		if(DEBUGGING) printf("dir entry [ino=%d rlen=%d name=`%s`]\n", dp->inode, dp->rec_len, dp->name);
		cp += dp->rec_len;
	}

    enter_name(pip, ino, name);
}

int make_dir(char *pathname){
    if(DEBUGGING) printf("make_dir: got pathname `%s`\n", pathname);

    MINODE *mip, *pip;
    int pino;
    char *parent, *child;
    if(pathname[0] == '/'){
	mip = root;
        dev = root->dev;
    }
    else{
        mip = running->cwd;
        dev = running->cwd->dev;
    }

    parent = dirname(make_string(pathname));
    child = basename(make_string(pathname));

    if(DEBUGGING) printf("make_dir: split parent=`%s` child=`%s`\n", parent,child);

    //get MINODE of parent
    pino = getino(&dev, parent);
    pip = iget(dev, pino);

    //verify parent inode is dir
    if(DEBUGGING) printf("make_dir: parent i_mode=[%x]\n", pip->INODE.i_mode);

    if( !(pip->INODE.i_mode & 0x4000)){
        printf("Cannot create dir under reg. file (mode != 0x4000)\n");
        return -1;
    }

    //verify child doesn't already exist
    if(getino(&dev, pathname)){
        printf("directory %s already exists!\n", child);
        return -1;
    }

    mymkdir(pip, child);

    //inc parent inodes link count by 1
    //touch atime and mark dirty
    pip->dirty = 1;
    pip->INODE.i_links_count++;
    //how to touch atime?
    iput(pip);
}

/******************************************
=== CREATE NEW FILE WITH NO DATA BLOCKS ===
******************************************/

int mycreat(MINODE *pip, char *name){
    //pip points to parent minode, name is string
    //allocate an inode
    int ino = ialloc(dev);
    if(DEBUGGING) printf("make new file using ino=[%d]\n", ino);

    //load the new inode into memory
    MINODE *mip = iget(dev, ino);

    //write to mip->INODE to make it a file
    INODE *ip = &mip->INODE;
    ip->i_mode = 0x81A4;
    ip->i_uid = running->uid;
    //ip->i_gid = running->gid;
    ip->i_size = 0;
    ip->i_links_count = 1;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 0;
    for(int i = 0; i < 15; i++){
        ip->i_block[i] = 0;
    }
    mip->dirty = 1;

    if(DEBUGGING) print_minode(mip);

    iput(mip);

    //put this file's name into it's parent's dir record
    enter_name(pip, ino, name);
}

int creat_file(char *pathname){
    if(DEBUGGING) printf("creat_file: got pathname `%s`\n", pathname);

    MINODE *mip, *pip;
    int pino;
    char *parent, *child;
    if(pathname[0] == '/'){
        mip = root;
        dev = root->dev;
    }
    else{
        mip = running->cwd;
        dev = running->cwd->dev;
    }

    parent = dirname(make_string(pathname));
    child = basename(make_string(pathname));

    if(DEBUGGING) printf("creat_file: split parent=`%s` child=`%s`\n", parent,child);

    //get MINODE of parent
    pino = getino(&dev, parent);
    pip = iget(dev, pino);

    //verify parent inode is dir
    if(DEBUGGING) printf("creat_file: parent i_mode=[%x]\n", pip->INODE.i_mode);

    if( !(pip->INODE.i_mode & 0x4000)){
        printf("Cannot create file under reg. file (mode != 0x4000)\n");
        return -1;
    }

    //verify child doesn't already exist
    if(getino(&dev, pathname)){
        printf("file %s already exists!\n", child);
        return -1;
    }

    //call my_creat here
    mycreat(pip, child);

    //touch atime and mark dirty
    pip->dirty = 1;
    //how to touch atime?
    iput(pip);
}

