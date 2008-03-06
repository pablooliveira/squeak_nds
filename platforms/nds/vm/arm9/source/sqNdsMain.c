/* Copyright (C) 2008 by Pablo Oliveira 
 *  
 * based on code from the unix squeak vm:
 * Copyright (C) 1996-2005 by Ian Piumarta and other authors/contributors
 *                              listed elsewhere in this file.
 *
 * This program is free software.  You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * See the file COPYRIGHT for more details.
 */

#include <nds.h>
#include <fat.h>
#include <math.h>
#include <sys/dir.h>
#include <time.h>
#include <unistd.h>
#include <sys/iosupport.h>

#include "sq.h"
#include "globals.h"

int activeWindow = BOTW;
int nativeDepth = SCREEN_PREFERED_DEPTH;

unsigned int timer = 0;
#define SQ_TIMER_FREQ 50
#define SQ_TIMER_RES (1000/SQ_TIMER_FREQ)

// No os_exports for NDS
void* os_exports[][3] = {};

// NOT IMPLEMENTED //
/////////////////////////////////////////////////////
void* ioLoadModule(char *pluginName) { return false; }
void* ioFindExternalFunctionIn(char *lookupName, void *moduleHandle) { return false; }
int ioFreeModule(void *moduleHandle) { return false; }
sqInt ioDisablePowerManager(sqInt disableIfNonZero) {return false;}
int ioSetCursor(int cursorBitsIndex, int offsetX, int offsetY) {return false;}
int ioSetCursorWithMask(int cursorBitsIndex, int cursorMaskIndex, 
int offsetX, int offsetY) {return false;}
int ioBeep(void) {_debug("beep!");}
int ioFormPrint(int bitsAddr, int width, int height,
int depth, double hScale, double vScale, int landscapeFlag)	{}
int clearProfile(void) {}
int dumpProfile(void)	{}
int startProfiling(void) {}
int stopProfiling(void)	{}
int clipboardReadIntoAt(int count, int byteArrayIndex, int startIndex) {_debug("clipr");return 0;}
int clipboardSize(void) {return 0;}
int clipboardWriteFromAt(int count, int byteArrayIndex, int startIndex) {return 0;}
////////////////////////////////////////////////////

int ioScreenSize(void) {return SCREEN_SIZE;}
int ioScreenDepth(void) {return nativeDepth;}

time_t convertToSqueakTime(time_t unixTime)
{
  /* Squeak epoch is Jan 1, 1901.  Unix epoch is Jan 1, 1970: 17 leap years
   *      and 52 non-leap years later than Squeak. */
  return unixTime + ((52*365UL + 17*366UL) * 24*60*60UL);
}

int ioSeconds(void) {
  // Return time in seconds since the epoch
  return convertToSqueakTime(time(NULL));
}

char *getImageName(void)
{return imageName;}

inline int ioMicroMSecs(void) {
  // the timer is incremented for each VBlank
  // and the nds has a refresh rate of 60 Hz.
  return timer;
}

int ioLowResMSecs(void) 
{return ioMicroMSecs();}
int ioMSecs(void)
{return ioMicroMSecs();}


char * GetAttributeString(sqInt id) {
  // This is a hook for getting various status strings back from
  // the OS. In particular, it allows Squeak to be passed arguments
  // such as the name of a file to be processed. Command line options
  // are reported this way as well, on platforms that support them.

  if (id == 0) return "";
  if (id == 1) return imageName; 
  if (id == 1001) return "NDS";
  if (id == 1002) return "";
  if (id == 1003) return "ARM9 / ARM7";

  // attribute undefined by this platform 
  success(false);
  return "";
}

sqInt sqGetFilenameFromString(char * aCharBuffer, char * aFilenameString, 
  sqInt filenameLength, sqInt aBoolean)
{
  // No symlinks to resolve in nds filesystem.
  sq2uxPath(aFilenameString, filenameLength, aCharBuffer, MAXPATHLEN, 1);
  return 0;
}

int getAttributeIntoLength(int id, int byteArrayIndex, int length) {
  // copy the attribute with the given id into a Squeak string
  char *srcPtr, *dstPtr, *end;
  int charsToMove;

  srcPtr = GetAttributeString(id);
  charsToMove = strlen(srcPtr);
  if (charsToMove > length) {
    charsToMove = length;
  }

  dstPtr = (char *) byteArrayIndex;
  end = srcPtr + charsToMove;
  while (srcPtr < end) {
    *dstPtr++ = *srcPtr++;
  }
  return charsToMove;
}

int attributeSize(sqInt id) {return strlen( GetAttributeString(id) );}
int vmPathSize(void) {return strlen(vmPath);}
int imageNameSize(void) {return strlen(imageName);}

int genericGetLength(char* name, int index, int length) {
  char *stName = (char *) index;
  int count, i;
  count = strlen(name);
  count = (length < count) ? length : count;
  for (i = 0; i<count; i++)
    stName[i] = name[i];
  return count;
}

int vmPathGetLength(int sqVMPathIndex, int length) {
  return genericGetLength(vmPath, sqVMPathIndex, length);
}

int imageNameGetLength(int sqImageNameIndex, int length) {
  return genericGetLength(imageName, sqImageNameIndex, length);
}

int imageNamePutLength(int sqImageNameIndex, int length) {
  // copy from the given Squeak string into the imageName variable. 
  char *sqImageName = (char *) sqImageNameIndex;
  int count, i, ch, j;
  int lastColonIndex = -1;

  count = (IMAGE_NAME_SIZE < length) ? IMAGE_NAME_SIZE : length;

  // copy the file name into a null-terminated C string 
  for (i = 0; i < count; i++) {
    ch = imageName[i] = sqImageName[i];
    if (ch == ':') {
      lastColonIndex = i;
    }
  }
  imageName[count] = 0;

  // copy short image name into a null-terminated C string
  for (i = lastColonIndex + 1, j = 0; i < count; i++, j++) {
    shortImageName[j] = imageName[i];
  }
  shortImageName[j] = 0;

  return count;
}

inline void displayLine(u16* out, int* bits, int l, int r, int bpw, int y) {
  int x,i;
  const int depth = 32 / bpw;
  const int mask = (1 << depth) - 1;

  if (bpw > 2) {
    for(x=l/bpw; x<=r/bpw; x+=1) {
      int word = bits[x];
      u16* end = out + (VWIDTH * y + (x+1)*bpw)/2 - 1;
      for (i=0; i < bpw; i+=2) {
        u16 px1 = word & mask;
        word = word >> depth; 

        u16 px2 = word & mask;
        word = word >> depth;

        *end-- = (px1 << 8) | px2; 
      }
    }
  }
  else 
    // 16 bis depths
    for(x=l/bpw; x<=r/bpw; x+=1) {
      int word = bits[x];
      out[VWIDTH*y + 2*x] = (word >> 16) & 0x00007FFF | BIT(15);
      out[VWIDTH*y + 2*x+1] = word & 0x00007FFF | BIT(15);
    }
}

int ioShowDisplay(int bits, int width, int height, int depth,int l, int r, int t, int b) {
  if (depth == 32) 
    error("32 bits depth is not supported");
  if (depth != nativeDepth)
    ioSetDisplayMode(VWIDTH, VHEIGHT, depth, true);

  if (l >= r || t >= b) return true;

  u16* out;
  int y;
  int bpw = 32/nativeDepth;

#ifndef DEBUG
  for(y=t; y<MIN(SCREEN_HEIGHT,b); y++) {
    displayLine((u16*)BG_BMP_RAM_SUB(0), (int*)bits + y*VWIDTH/bpw, l, r, bpw, y);
  }
#endif

  for(y=MAX(t,SCREEN_HEIGHT); y<=b; y++){ 
    displayLine((u16*)BG_BMP_RAM(0), (int*)bits + y*VWIDTH/bpw, l, r, bpw, y-SCREEN_HEIGHT);
  }

  return true;
}

int ioExit(void) {
  irqDisable(IRQ_VBLANK);
  while(1) {
    swiWaitForVBlank();
  }
}

int ioForceDisplayUpdate(void) {}

int ioHasDisplayDepth(int depth) {return (depth <= 16);}

int ioSetDisplayMode(int width, int height, int depth, int fullscreenFlag) 
{
  if ((width == VWIDTH) && (height == VHEIGHT)) {
    switch (depth) {
      case 16: 
        // Banks A and C should just be enought for two 256*256*16 images (128K each).
        BG3_CR = BG_BMP16_256x256;
#ifndef DEBUG
        SUB_BG3_CR = BG_BMP16_256x256;
#endif
        nativeDepth = 16;
        return true;
      case 8:
      case 4:
      case 2:
      case 1:
        BG3_CR = BG_BMP8_256x256;
#ifndef DEBUG
        SUB_BG3_CR = BG_BMP8_256x256;
#endif
        nativeDepth = depth;
        return true;
      default:
        return false;
    }
  }
  return false;
}


int ioSetFullScreen(int fullScreen) {return true;}

sqInt ioRelinquishProcessorForMicroseconds(sqInt us)
{
  ioProcessEvents();
}

int RL_Was_Up = true;

void VBlank() {
  if ((keysHeld() & (KEY_R | KEY_L))) {
    if (RL_Was_Up) {
      lcdSwap();
      activeWindow = 1 - activeWindow;
      RL_Was_Up = false;
    }
  } else RL_Was_Up = true;
}

void timerI() {
  timer += SQ_TIMER_RES;
}

void initNds() {
#ifdef DEBUG
  // Install an Exception Handler 
  defaultExceptionHandler();
#endif
  // Install VBlank interruption
  irqInit();
  irqSet(IRQ_VBLANK, VBlank);
  irqEnable(IRQ_VBLANK);
  
  // Install timer interruption
  TIMER0_CR = 0;
  TIMER0_DATA = TIMER_FREQ_1024(SQ_TIMER_FREQ);
  TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1024 | TIMER_IRQ_REQ;
  irqSet(IRQ_TIMER0, timerI);
  irqEnable(IRQ_TIMER0);

  // Init the fat library 
  fatInitDefault();
}

void initPalette(u16* palette){
  int i,r,g,b;

  // XXX This palette should be precalculated and loaded directly.

  palette[0] = RGB15(31,31,31); // transparent or white (in 1 depth)
  palette[1] = RGB15(0,0,0); // black
  palette[2] = RGB15(31,31,31); // white
  palette[3] = RGB15(16,16,16); // grey 
  palette[4] = RGB15(31,0,0); // red
  palette[5] = RGB15(0,31,0); // green
  palette[6] = RGB15(0,0,31); // blue
  palette[7] = RGB15(0,31,31); // cyan
  palette[8] = RGB15(31,31,0); // yellow
  palette[9] = RGB15(31,0,31); // magenta
  palette[10] = RGB15(4,4,4); // 1/8 grey
  palette[11] = RGB15(8,8,8); // 2/8 grey
  palette[12] = RGB15(12,12,12); // 3/8 grey
  palette[13] = RGB15(20,20,20); // 5/8 grey
  palette[14] = RGB15(24,24,24); // 6/8 grey
  palette[15] = RGB15(28,28,28); // 7/8 grey

  // 1/32 -> 31/32 greys without n/8 greys
  const int greys[24] = {1,2,3,5,6,7,9,10,11,13,14,15,17,18,19,21,22,23,25,26,27,29,30,31};
  for (i = 16; i < 40; i++)
    palette[i] = RGB15(greys[i],greys[i],greys[i]);

  // 6x6x6 color cube (216 colors)
  for (r = 0; r < 6; r++)
    for (g = 0; g < 6; g++)
      for (b = 0; b < 6; b++)
        palette[36*r + 6*b + g + 40] = RGB15(6*r,6*g,6*b);
}

void initNdsConsole() {
  videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE);
  vramSetBankC(VRAM_C_SUB_BG);
  SUB_BG0_CR = BG_MAP_BASE(31);
  BG_PALETTE_SUB[255] = RGB15(31,31,31);
  consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
  lcdMainOnBottom();
}

const devoptab_t dotab_null = {
  "null",
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void disableNdsConsole() {
  // libnds console uses newlib's devoptab to catch stdout and sterr
  // we are using both screens so we do not want the console to pollute
  // our output. Thus we ask newlib to ignore writes to stdout and stdin.
  devoptab_list[STD_OUT] = &dotab_null;   
  devoptab_list[STD_ERR] = &dotab_null;
}

void initNdsDisplay() {
  // The main and sub modes display each half of the squeak
  // deskstop. We use 2D rotoscale backgrounds on mode 5.

  vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
  memset((void *)BG_BMP_RAM(0), 0, 256*256*2);
  initPalette(BG_PALETTE);
  videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);

  // Set transformations
  BG3_XDX = 1 << 8;
  BG3_XDY = 0;
  BG3_YDX = 0;
  BG3_YDY = 1 << 8;
  BG3_CY = 0 << 8;
  BG3_CX = 0 << 8;

#ifndef DEBUG
  vramSetBankC(VRAM_C_SUB_BG_0x06200000);
  memset((void *)BG_BMP_RAM_SUB(0), 0, 256*256*2);
  memcpy(BG_PALETTE_SUB, BG_PALETTE, 2*256);
  videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

  // Set transformations
  SUB_BG3_XDX = 1 << 8;
  SUB_BG3_XDY = 0;
  SUB_BG3_YDX = 0;
  SUB_BG3_YDY = 1 << 8;
  SUB_BG3_CY = 0 << 8;
  SUB_BG3_CX = 0 << 8;
  
  disableNdsConsole();
#endif
  
  ioSetDisplayMode(VWIDTH, VHEIGHT, SCREEN_PREFERED_DEPTH, true);
  lcdMainOnBottom();
}

int main(void) {
  sqImageFile f;
  int availableMemory;

  initNdsConsole();

  printf("Starting Squeak VM...\n");
  
  initNds();

  imageName[0] = shortImageName[0] = vmPath[0] = 0;
  strcpy(imageName, "/" IMAGE_NAME);
  strcpy(shortImageName, IMAGE_NAME);
  strcpy(vmPath, "/");

  availableMemory = AVAILABLE_MEM; 

  printf("Reading image '%s'\n", imageName);

  f = sqImageFileOpen(imageName, "rb");
  if (f == NULL) {
    printf("Could not open image '%s'\n", imageName);
    ioExit();
  }

  readImageFromFileHeapSize(f, availableMemory);
  sqImageFileClose(f);

  _debug("Starting the image...");

  initNdsDisplay();

  // run Squeak 
  interpret();
}

