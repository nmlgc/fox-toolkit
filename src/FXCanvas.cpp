/********************************************************************************
*                                                                               *
*                   C a n v a s   W i n d o w   O b j e c t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: FXCanvas.cpp,v 1.5 1998/10/23 06:20:09 jeroen Exp $                      *
********************************************************************************/
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXCanvas.h"

  

/*******************************************************************************/

// Map
FXDEFMAP(FXCanvas) FXCanvasMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXCanvas::onPaint),
  FXMAPFUNC(SEL_CONFIGURE,0,FXCanvas::onConfigure),
  FXMAPFUNC(SEL_MOTION,0,FXCanvas::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXCanvas::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXCanvas::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXCanvas::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXCanvas::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXCanvas::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXCanvas::onRightBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXCanvas::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXCanvas::onKeyRelease),
  };


// Object implementation
FXIMPLEMENT(FXCanvas,FXWindow,FXCanvasMap,ARRAYNUMBER(FXCanvasMap))

  

// Make a canvas
FXCanvas::FXCanvas(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  }


// Create X window
void FXCanvas::create(){
  FXWindow::create();
  setBackColor(acquireColor(FXRGB(255,255,255)));
  show();
  }


// Canvas is an object drawn by another
long FXCanvas::onPaint(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_PAINT),ptr);
  }


// Handle configure notify
long FXCanvas::onConfigure(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onConfigure(sender,sel,ptr);
  return target && target->handle(this,MKUINT(message,SEL_CONFIGURE),ptr);
  }


// Handle buttons if not handled in derived class
long FXCanvas::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr);
  }

long FXCanvas::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr);
  }

long FXCanvas::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr);
  }

long FXCanvas::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr);
  }

long FXCanvas::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr);
  }

long FXCanvas::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr);
  }


// Mouse moved
long FXCanvas::onMotion(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_MOTION),ptr);
  }


// Handle keyboard press/release 
long FXCanvas::onKeyPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr);
  }

long FXCanvas::onKeyRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr);
  }

