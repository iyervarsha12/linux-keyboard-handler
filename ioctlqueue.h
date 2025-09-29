/*
	Header file shared by ioctlqueue.c and ioctlqueuetest.c
	Contains the shared defines
*/

#ifndef IOCTLTEST_H
#define IOCTLTEST_H

#define IOCTL_TEST _IOR(0,6, char) // ioctl which copies info from kernel to userspace

#define NOCHAR 0
#define PRINTABLECHAR 1
#define SHIFTCHARPRESS 2
#define SHIFTCHARRELEASE 3
#define CAPSLOCKCHAR 4
#define NUMLOCKCHAR 5
#define BACKSPACECHAR 6
#define CTRLPRESS 7
#define CTRLRELEASE 8
#define RECORDARRMAX 100

#endif