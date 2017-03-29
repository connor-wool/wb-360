/*
This file attempts to create an exploit where we compile our own code
but it simply calls KC's code (in another directory) and passes our
command line arguments to it.
*/

/*
So how do we plan to do this?
	1) Fork a child process
	2) in that process, find kc's code at a known location
	3) change to KC's process image, passing in our own command line arguments
	4) we should still retain our cwd because we copy parent's env
	5) kc's code now runs in the current directory on arguments we pass
*/





