#ifndef _GLOBALS_H
# define _GLOBALS_H 1

#ifdef DEBUG
# define _debug(msg) printf("%s\n", msg)
#else
# define _debug(msg)  
#endif

#define IMAGE_NAME "squeak.image"
#define AVAILABLE_MEM 3000000

extern int  getFullScreenFlag(void);
extern void setFullScreenFlag(int i);
extern int  getInterruptCheckCounter(void);
extern void setInterruptCheckCounter(int i);
extern int  getInterruptKeycode(void);
extern void setInterruptKeycode(int i);
extern int  getInterruptPending(void);
extern void setInterruptPending(int i);
extern int  getSavedWindowSize(void);
extern void setSavedWindowSize(int i);

// We can swap windows:
// the active window is the one in the bottom screen.
#define BOTW 0
#define TOPW 1
extern int activeWindow;

#define IMAGE_NAME_SIZE 300
#define SHORTIMAGE_NAME_SIZE 100
#define VMPATH_SIZE 300

char imageName[IMAGE_NAME_SIZE + 1]; // full path to image 
char shortImageName[SHORTIMAGE_NAME_SIZE + 1]; // just the image file name
char vmPath[VMPATH_SIZE + 1]; // full path to interpreter's directory

// We use both nds screens for displaying the Squeak desktop

#define VWIDTH  SCREEN_WIDTH
#define VHEIGHT (SCREEN_HEIGHT*2)

// Define the screen size as two positive 16-bit integers packed 
// into a 32-bit integer.

#define SCREEN_SIZE ((VWIDTH << 16) | VHEIGHT);

#define SCREEN_PREFERED_DEPTH 8

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)<(b))?(b):(a))

extern int nativeDepth;
#endif
