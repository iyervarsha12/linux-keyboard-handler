VARSHA'S KEYBOARD HANDLER

This handler, done as a part of CS552 in BU, replaces the Linux Keyboard driver (tested on Puppylinux version 2.6.33). 
There is support for shift and capslock capitalisation, shift special characters, backspace and numlock. 
It additionally supports recording entered characters. 


File information: 
	ioctlqueue.c contains the kernel module programming
	ioctlqueuetest.c contains user level programming
	ioctlqueue.h contains shared defines
	keyboardscript.sh has a script to insert kernel module, compile ioctlqueuetest and run its executable, and remove kernel module after it
	Makefile is to create module files from ioctlqueue.c

Instructions to use the handler:
1. Run "make -C /usr/src/linux-2.6.33.2 SUBDIRS=$PWD modules" to create .ko file for ioctlqueue
2. chmox +x keyboardscript.sh for executable permission
3. Run "./keyboardscript.sh"
4. The welcome text "Vi's keyboard driver! Enter ctrl + e to exit :)" should show up, you're free to use the handler now

Additional info
Ctrl + r starts record
Ctrl + N changes newlines to spaces in the recording
Ctrl + p plays back the recorded text
Ctrl + e exits the custom handler
There is support for shift and capslock capitalisation, shift special characters, backspace and numlock

Sources in web used (outside of those listed in the assignment):
To understand IRQ requests:
	https://www.science.smith.edu/~nhowe/262/oldlabs/keyboard.html
To understand kallsyms better, to get how to free the linux native keyboard handler: 
	https://cleveruptime.com/docs/files/proc-kallsyms
To understand using the address from kallsyms to call an unexported function, to get how to free the linux native keyboard handler:

	https://stackoverflow.com/questions/22157442/how-can-i-reference-non-exported-linux-kernel-functions-from-a-kernel-module
