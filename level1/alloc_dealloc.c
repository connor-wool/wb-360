/*************************
ALLOC_DEALLOC.c
This file contains the code to allocate and deallocate blocks and inodes.
It is used when adding or removing from the filesystem.
*************************/

/********************
ALLOCATION SECTION: balloc (block allocate) ialloc (inode allocate)
********************/

//subtract 1 from the "free blocks" count in SUPER and GD
int decFreeBlocks(int dev){
        char buf[BLKSIZE];

        //decrement free blocks in SUPER
        get_block(dev, 1, buf);
        sp = (SUPER *)buf;
        sp->s_free_blocks_count--;
        put_block(dev, 1, buf);

        //decrement free blocks in GD
        get_block(dev, 2, buf);
        gp = (GD *)buf;
        gp->bg_free_blocks_count--;
        put_block(dev, 2, buf);
}

//allocate a new block on dev
int balloc(int dev){
        //establish counters and buffer
        int i;
        char buf[BLKSIZE];

        //read block_bitmap block
        get_block(dev, bmap, buf);

        //find first free block and mark as allocated
        for (i = 0; i < nblocks; i++){
                if (tst_bit(buf, i) == 0){
                        set_bit(buf, i);
                        decFreeBlocks(dev);
                        put_block(dev, bmap, buf);
                        return i+1;
                }
        }
}

//decrement the number of free inodes by 1
int decFreeInodes(int dev)
{
        char buf[BLKSIZE];

        // dec free inodes count in SUPER and GD
        get_block(dev, 1, buf);
        sp = (SUPER *)buf;
        sp->s_free_inodes_count--;
        put_block(dev, 1, buf);

        get_block(dev, 2, buf);
        gp = (GD *)buf;
        gp->bg_free_inodes_count--;
        put_block(dev, 2, buf);
}

//allocate a new inode on dev
int ialloc(int dev)
{
        //establish counters and buffer
        int  i;
        char buf[BLKSIZE];

        // read inode_bitmap block
        get_block(dev, imap, buf);

        //find first free inode and mark as allocated
        for (i=0; i < ninodes; i++){
                if (tst_bit(buf, i)==0){
                        set_bit(buf,i);
                        decFreeInodes(dev);

                        put_block(dev, imap, buf);

                        return i+1;
                }
        }

        //return error message if all inodes allocated
        printf("ialloc(): no more free inodes\n");
        return 0;
}

/******************************************
DEALLOCATION: bdealloc (block deallocate) idealloc (inode deallocate)
******************************************/

//add 1 to "free block count" in super and gd
int incFreeBlocks(int dev){
        char buf[BLKSIZE];

        //increment free blocks in SUPER
        get_block(dev, 1, buf);
        sp = (SUPER *)buf;
        sp->s_free_blocks_count++;
        put_block(dev, 1, buf);

        //decrement free blocks in GD
        get_block(dev, 2, buf);
        gp = (GD *)buf;
        gp->bg_free_blocks_count++;
        put_block(dev, 2, buf);
}

//deallocate a block on dev
int balloc(int dev, int blk_num){
        char buf[BLKSIZE];
        
	//read bmap from disk
	//adjust bit in bitmap
	//write back to disk
	//increment free blocks
	//write random data to dealloc'd block to erase
        
	get_block(dev, bmap, buf);
	clr_bit(buf, blk_num);
	put_block(dev, bmap, buf);
	incFreeBlocks(dev);	

	char eraser[BLKSIZE] = {'f'};
	put_block(dev, blk_num, eraser);
}

//increment the number of free inodes by 1
int incFreeInodes(int dev)
{
        char buf[BLKSIZE];

        //inc free inodes count in SUPER and GD
        get_block(dev, 1, buf);
        sp = (SUPER *)buf;
        sp->s_free_inodes_count++;
        put_block(dev, 1, buf);

        get_block(dev, 2, buf);
        gp = (GD *)buf;
        gp->bg_free_inodes_count++;
        put_block(dev, 2, buf);
}

//deallocate an inode on dev
int idealloc(int dev, int ino)
{
        //establish counters and buffer
        //read imap from disk
	//mark ino as free in imap
	//write imap back to disk
	//increment free inodes

	char buf[BLKSIZE];
        get_block(dev, imap, buf);
	clr_bit(buf, ino);
	put_block(dev, imap, buf);
	incFreeInodes(dev);
}
