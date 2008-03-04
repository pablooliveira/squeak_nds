/* Copyright (C) 2008 by Pablo Oliveira 
 *  
 * based on code from the unix squeak vm:
 * Copyright (C) 1996-2004 by Ian Piumarta and other authors/contributors
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
#include "sq.h"
#include "globals.h"

#define IEB_SIZE 64	/* must be power of 2 */

typedef struct
{
  int x, y;
} SqPoint;

SqPoint mousePosition= { 0, 0 };

static sqInputEvent inputEventBuffer[IEB_SIZE];

int iebIn=  0;	/* next IEB location to write */
int iebOut= 0;	/* next IEB location to read  */

#define iebEmptyP()	(iebIn == iebOut)
#define iebAdvance(P)	(P= ((P + 1) & (IEB_SIZE - 1)))

int buttonState= 0;		/* mouse button state or 0 if not pressed */
int modifierState= 0;		/* modifier key state or 0 if none pressed */

int inputEventSemaIndex= 0;

int ioSetInputSemaphore(int semaIndex)
{ 
  _debug("Set Input Semaphore");
  if (semaIndex == 0) {
    success(false);
  }
  else
    inputEventSemaIndex= semaIndex;
  return true;
}

sqInputEvent *allocateInputEvent(int eventType)
{
  sqInputEvent *evt= &inputEventBuffer[iebIn];
  iebAdvance(iebIn);
  if (iebEmptyP())
    {
      /* overrun: discard oldest event */
      iebAdvance(iebOut);
    }
  evt->type= eventType;
  evt->timeStamp= ioMSecs();
  return evt;
}

#define allocateMouseEvent() ( \
  (sqMouseEvent *)allocateInputEvent(EventTypeMouse) \
)

#define allocateKeyboardEvent() ( \
  (sqKeyboardEvent *)allocateInputEvent(EventTypeKeyboard) \
)

#define allocateDragEvent() ( \
  (sqDragDropFilesEvent *)allocateInputEvent(EventTypeDragDropFiles) \
)

void signalInputEvent(void)
{
  if (inputEventSemaIndex > 0) 
    signalSemaphoreWithIndex(inputEventSemaIndex);
}
int ioMousePoint(void) {
  ioProcessEvents();
  // return the mouse point two 16-bit positive integers packed into a 32-bit integer 
  // x is high 16 bits; y is low 16 bits
  return (mousePosition.x << 16) | (mousePosition.y & 0xFFFF);  
}

int ioGetButtonState(void) {
  ioProcessEvents();
  return buttonState;
}

int ioGetKeystroke(void) {
  ioProcessEvents();
  // we could use a virtual keyboard
  return -1; // pretend the buffer is empty
}

int ioPeekKeystroke(void) {
  ioProcessEvents();
  // we could use a virtual keyboard
  return -1; // pretend the buffer is empty
}

void recordMouseEvent(void)
{
  _debug("recordMouseEvent");
  sqMouseEvent *evt= allocateMouseEvent();
  evt->x= mousePosition.x;
  evt->y= mousePosition.y;
  evt->buttons= (buttonState & 0x7);
  evt->modifiers= (buttonState >> 3);
  evt->reserved1=
    evt->windowIndex= 0;
  signalInputEvent();
}

void recordKeyboardEvent(int keyCode, int pressCode, int modifiers)
{
  sqKeyboardEvent *evt= allocateKeyboardEvent();
  evt->charCode= keyCode;
  evt->pressCode= pressCode;
  evt->modifiers= modifiers;
  evt->reserved1=
    evt->windowIndex= 0;
  signalInputEvent();
}

/* retrieve the next input event from the queue */

int ioGetNextEvent(sqInputEvent *evt)
{
  _debug("ioGetNextEvent");
  if (iebEmptyP()) 
    ioProcessEvents();
  if (iebEmptyP()) {
    _debug("No events");
    return false;
  }
  _debug("An event"); 
  *evt= inputEventBuffer[iebOut];
  iebAdvance(iebOut);
  return true;
}


int wasDown = false;
int lastx = 0;
int lasty = 0;

int ioProcessEvents(void) {
  // Mapping of the button states to the nds controls:
  //  6 command 
  //  5 option
  //  4 control
  //  3 shift   
  //  2 left   <- TOUCH
  //  1 middle <- LEFT  + TOUCH
  //  0 right  <- RIGHT + TOUCH
  scanKeys();
  int isDown = keysHeld() & KEY_TOUCH;
  int recordMouse = false;

  if (isDown) {
    touchPosition p = touchReadXY();
    mousePosition.x = p.px;
    mousePosition.y = p.py;
    if (activeWindow == BOTW)
      mousePosition.y += VHEIGHT/2;
  }

  if ((isDown) && (!wasDown)) {
    wasDown = true;
    if (keysHeld() & KEY_RIGHT) buttonState |= 1;
    else if (keysHeld() & KEY_LEFT) buttonState |= 2;
    else buttonState |= 4;
    recordMouse = true;
  } 
  
  if ((!isDown) && (wasDown)) {
    buttonState = 0;
    recordMouse = true;
    wasDown = false;
  } 
  
  if (lastx != mousePosition.x || lasty != mousePosition.y) {
    recordMouse = true;
    lastx = mousePosition.x;
    lasty = mousePosition.y;
  }

  if (recordMouse) recordMouseEvent();

  return false;
}


