/*
 * ioctlqueue.c is 
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include "ioctlqueue.h"

// Global variables and function declarations
static wait_queue_head_t wq;
bool keyboardinterrupt = false;
bool shiftpress = false;
bool ctrlpress = false;
char readcharpressed = 0;

static struct file_operations pseudo_dev_proc_operations;		
static struct proc_dir_entry *proc_entry;
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
								unsigned int cmd, unsigned long arg);


static inline unsigned char inb( unsigned short usPort ) {

    unsigned char uch;
   
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

// Our own IRQ handler for keyboard interrupts
irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	char c = 0;
	char readchar = NOCHAR;
	static char scanref[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	
	// Processing: the only time we care about released vs pressed, is shift or ctrl.
	c = inb(0x60);
	if (c == 0xffffffb6 || c ==  0xffffffaa ) // Shift release is 0xffffffb6 or 0xffffffaa lmao
	{
		keyboardinterrupt = true;
		shiftpress = false;
		readchar = SHIFTCHARRELEASE;
		readcharpressed = readchar; 
		wake_up(&wq);
	}
	else if ( c == 0xffffff9d ) // ctrl release 
	{
		keyboardinterrupt = true;
		ctrlpress = false;
		readchar = CTRLRELEASE;
		readcharpressed = readchar; 
		wake_up(&wq);
		
	}
	else if (!(( ( c = inb( 0x60 ) ) & 0x80 ) ))
	{
		if(c == 0xe) //backspace press
		{
			readchar = BACKSPACECHAR;
		}
		else if (c==0x36 || c==0x2a) //right and left shift press
		{
			if (!shiftpress) //we do not want repeated sticky shift presses
			{
				shiftpress = true;
				readchar = SHIFTCHARPRESS;
			}
		}
		else if (c==0x1d) // ctrl press
		{
			if (!ctrlpress) //we do not want repeated sticky ctrl presses
			{
				ctrlpress = true;
				readchar = CTRLPRESS;
			}
		}
		else if (c==0x3a) //capslock press
		{
			readchar = CAPSLOCKCHAR;
		}
		else if (c==0x45) //numlock press
		{
			readchar = NUMLOCKCHAR;
		}
		else //printable characters
		{
			readchar = scanref[(int) c];
		}	
		readcharpressed = readchar; 
		keyboardinterrupt = true;
		wake_up(&wq);
	}
	
	return (irq_handler_t) IRQ_HANDLED;
}

int init_module()
{
	int result;
	msleep(500); //Without this, your initial 'insmod' enter character breaks the original keyboard handler.
	
	// Freeing the original keyboard handler
	// TLDR: doing free_irq(1,NULL) leads to failures
	// i8042 is the intel chip that controls the keyboard
	// i8042_free_irqs is what frees the irqs in the i8042, found via searching in kallsyms
	// We are taking the i8042_free_irqs function address and making it into our own function
	// Source: https://stackoverflow.com/questions/22157442/how-can-i-reference-non-exported-linux-kernel-functions-from-a-kernel-module
	void (*freehandler)(void) =(int (*)() )  kallsyms_lookup_name("i8042_free_irqs");
	freehandler();
	
	// Requesting the new handler
	result = request_irq(1, 
						(irq_handler_t) irq_handler,
						IRQF_SHARED, "test_keyboard_irq_handler",
						(void *)(irq_handler));
	if (result)
		printk(KERN_ALERT "Can't get handler!");
	else printk(KERN_INFO "Handler initalised");
	
	printk(KERN_INFO "Loading module");
	pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
	proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
	if (!proc_entry)
	{
		printk(KERN_ALERT "Error creating /proc entry!");
		return 1;
	}
	proc_entry->proc_fops = &pseudo_dev_proc_operations;
	
	// initialising waitqueue
	init_waitqueue_head(&wq);
	
	return 0;

}


static int pseudo_device_ioctl(struct inode *inode,
								struct file *file,
								unsigned int cmd,
								unsigned long arg)
{
	char temp = 'a';
	char *tempptr = &temp;

	
	switch (cmd)
	{
		/* IOCTL_TEST will take ioctl calls from userspace ioctl_test.c
		 * It will return the value of the keyboard entered value
		 * This value will be processed in userspace, and printed accordingly.
		 */
		case IOCTL_TEST:
			wait_event_interruptible(wq, keyboardinterrupt == true);
			if(readcharpressed==SHIFTCHARPRESS || readcharpressed==SHIFTCHARRELEASE ||readcharpressed == CAPSLOCKCHAR || 
				readcharpressed == BACKSPACECHAR || readcharpressed == NUMLOCKCHAR || readcharpressed == CTRLPRESS || readcharpressed == CTRLRELEASE)
			{
					keyboardinterrupt = false;
					return readcharpressed;
			}
			if(readcharpressed!=NOCHAR)
			{
				(*tempptr) = readcharpressed;
				// Copy info from kernel to userspace. (usespace, kernelspace, size)
				memcpy( (char *) arg, tempptr, sizeof(char));
				keyboardinterrupt = false;
				return PRINTABLECHAR;
			}
			break;
		default:
			printk(KERN_ALERT "Illegal ioctl call!");
			return -EINVAL;
			break;
	}
	return 0;
}

void cleanup_module()
{
	printk(KERN_INFO "Releasing handler");
	free_irq(1, (void *)(irq_handler));
	remove_proc_entry("ioctl_test", NULL);
}

MODULE_LICENSE("GPL");