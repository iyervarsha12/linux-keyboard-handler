#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "ioctlqueue.h"

// Global variables for functional presses
bool shiftpress = false;
bool capslockpress = false;
bool numlockpress = false;
bool ctrlpress = false;

// Global variables for the bonus
bool recordplay = false;
bool ctrlnplay = false;
char recordarray[RECORDARRMAX];


int main()
{
	char s = 'p'; 
	char *sptr = &s; // To hold the character to print
	int res = 0; // Hold the results from the custom ioctl
	int i = 0; // For recordplay index iterations
	char recordplayspecialchar;
	int recordArrayIdx = 0; // For recordplay playback endpoint index
	int fd = open("/proc/ioctl_test", O_RDONLY);
	printf("Vi's keyboard driver! Enter ctrl + e to exit :)\n");
	fflush(stdout);

	while(1) 
	{
		res = NOCHAR;
		res = ioctl(fd, IOCTL_TEST, sptr); //this only comes back when waitqueue says fuck yea
		if(res!=NOCHAR) //key is pressed!
		{
				// Update backspace, shift, capslock, numlock
				switch(res)
				{
					case BACKSPACECHAR: 
						if(!recordplay)
						{
							printf("\b \b");
							fflush(stdout);
						}
						break;			
					case SHIFTCHARPRESS: 
						shiftpress = true;
						break;
					case SHIFTCHARRELEASE: 
						shiftpress = false;
						break;
					case CAPSLOCKCHAR:
						capslockpress = !capslockpress;
						break;
					case NUMLOCKCHAR:
						numlockpress = !numlockpress;
						break;
					case CTRLPRESS:
						ctrlpress = true;
						break;
					case CTRLRELEASE:
						ctrlpress = false;
						break;
					default: 
						if (ctrlpress && *sptr=='e')
						{
							printf("\nThank you for trying Vi's keyboard driver :)");
							fflush(stdout);
							goto final_label;
						}
						else if (ctrlpress && (*sptr=='r')) // CTRL + r, record time
						{
							printf("\nStarting to record.");
							fflush(stdout);
							recordplay = true;
							recordArrayIdx = 0;
							// Clear the recordplay array before recording
							for (i = 0; i< RECORDARRMAX; i++)
								recordarray[i] = '\0';
						}
						else if (ctrlpress && ((capslockpress || shiftpress)&&(*sptr =='n'))) //CTRL + N
						{
							ctrlnplay = true;
						}
						else if (recordplay && ctrlpress && (*sptr == 'p')) // CTRL + p, playback time. Needs recordplay to have been active!
						{
							if(!ctrlnplay)
								printf("\nPrinting recorded:");
							else printf("\nPrinting recorded (with ctrl + N):");
							fflush(stdout);
							recordplay = false;
							ctrlnplay = false;
							for (i = 0; i< recordArrayIdx && i < RECORDARRMAX; i++)
							{
								printf("%c",recordarray[i]);
								fflush(stdout);
							}
							printf("\n");
							fflush(stdout);
							recordArrayIdx = 0; //resetting the playback array bounds.
						}
						else if((capslockpress || shiftpress)&& (*sptr)>='a' && (*sptr)<='z') // Capital letters
						{
							*sptr = (*sptr) - 32;
							if(recordplay) 
								recordarray[recordArrayIdx++] = (*sptr);
							else 
							{
								printf("%c", (*sptr));
								fflush(stdout); //printf() is buffered by default and may not work for single characters
							}
						}
						else if (shiftpress && ((*sptr>='0' && *sptr<='9')||(*sptr=='-')||(*sptr=='=')||(*sptr=='[')||(*sptr==']')||(*sptr=='\\')||(*sptr==';')||(*sptr=='\'')||(*sptr=='-')||(*sptr==',')||(*sptr=='.')||(*sptr=='/') ))
						{
							// special character time
							switch((*sptr))
							{
								case '0': recordplayspecialchar = ')';
										break;
								case '1': recordplayspecialchar = '!';
										break;
								case '2': recordplayspecialchar = '@';
										break;
								case '3': recordplayspecialchar = '#';
										break;
								case '4': recordplayspecialchar = '$';
										break;
								case '5': recordplayspecialchar = '%';
										break;
								case '6': recordplayspecialchar = '^';
										break;
								case '7': recordplayspecialchar = '&';
										break;
								case '8': recordplayspecialchar = '*';
										break;
								case '9': recordplayspecialchar = '(';
										break;
								case '-': recordplayspecialchar = '_';
										break;
								case '=': recordplayspecialchar = '+';
										break;
								case '[': recordplayspecialchar = '{';
										break;
								case ']': recordplayspecialchar = '}';
										break;
								case '\\': recordplayspecialchar = '|';
										break;
								case ';': recordplayspecialchar = ':';
										break;
								case '\'': recordplayspecialchar = '\"';
										break;
								case ',': recordplayspecialchar = '<';
										break;
								case '.': recordplayspecialchar = '>';
										break;
								case '/': recordplayspecialchar = '?';
										break;
							}
							if(recordplay) 
								recordarray[recordArrayIdx++] = recordplayspecialchar;
							else 
							{
								printf("%c",recordplayspecialchar);
								fflush(stdout);
							}
						}
						// if it's anything but a numlocked number
						else if (!(numlockpress && (*sptr)>='0' && (*sptr)<='9'))
						{
							if(recordplay) 
							{
								if ((*sptr)=='\n' && ctrlnplay)
									recordarray[recordArrayIdx++] = ' ';
								else recordarray[recordArrayIdx++] = (*sptr);	
							}
							else
							{
								printf("%c", (*sptr));
								fflush(stdout);
							}
						}		
				}
				
		}
	}
	final_label: 
	;

	return 0;
}