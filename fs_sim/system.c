/*** SYSTEM.C ***/
/*
	This file contains the code for KC Wang's final project.
*/


#include "type.h"
#include "iget_iput_getino.c"

//globals
MINODE minode[NMINODE];		//global minode array
MINODE *root;			//root pointer to '/'
PROC proc[NPROC], *running; 	//PROC, we will only use proc[0]

int fd, dev;				  //file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock; //file system constant values




