/********************************************************************************
*                                                                               *
*                        F r a m e   W i n d o w   O b j e c t                  *
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
* $Id: FXFrame.cpp,v 1.13 1998/10/23 06:20:09 jeroen Exp $                      *
********************************************************************************/
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXShell.h"

/*
  To do:
  - FXGLCanvas should just derive straight from FXWindow
  - FXCanvas too, btw.
  - pad stuff should move down to FXLabel
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXFrame) FXFrameMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXFrame::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXFrame,FXWindow,FXFrameMap,ARRAYNUMBER(FXFrameMap))


// Deserialization
FXFrame::FXFrame(){
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  border=0;
  }
  

// Create child frame window
FXFrame::FXFrame(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(p,opts,x,y,w,h){
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  border=(options&FRAME_THICK)?2:(options&(FRAME_SUNKEN|FRAME_RAISED))?1:0;
  }


// Create window
void FXFrame::create(){
  FXWindow::create();
  baseColor=acquireColor(getApp()->backColor);
  hiliteColor=acquireColor(getApp()->hiliteColor);
  shadowColor=acquireColor(getApp()->shadowColor);
  borderColor=acquireColor(getApp()->borderColor);
  show();
  }


void FXFrame::drawBorderRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(borderColor);
  drawRectangle(x,y,w,h);
  }


void FXFrame::drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  }

void FXFrame::drawSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(hiliteColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }

void FXFrame::drawRidgeRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+1,x+w-2,y+h-2);
  setForeground(shadowColor);
  drawLine(x+1,y+1,x+w-3,y+1);
  drawLine(x+1,y+1,x+1,y+h-3);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }

void FXFrame::drawGrooveRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+1,x+w-2,y+h-2);
  setForeground(hiliteColor);
  drawLine(x+1,y+1,x+w-2,y+1);
  drawLine(x+1,y+1,x+1,y+h-2);
  drawLine(x+1,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y+1,x+w-1,y+h-1);
  }


void FXFrame::drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(baseColor);
  drawLine(x+1,y+1,x+w-3,y+1);
  drawLine(x+1,y+1,x+1,y+h-3);
  setForeground(shadowColor);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+h-2,x+w-2,y+1);
  setForeground(borderColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }

void FXFrame::drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  setForeground(borderColor);
  drawLine(x+1,y+1,x+w-2,y+1);
  drawLine(x+1,y+1,x+1,y+h-2);
  setForeground(hiliteColor);
  drawLine(x+1,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y+h-1,x+w-1,y+1);
  setForeground(baseColor);
  drawLine(x+2,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+2,x+w-2,y+h-2);
  }


// Draw dashed focus rectangle
void FXFrame::drawFocusRectangle(FXint x,FXint y,FXint w,FXint h){
  static const char onoff[]={1,2};
  FXASSERT(xid);
  setForeground(borderColor);
  setDashes(0,onoff,2);
  setLineAttributes(0,LINE_ONOFF_DASH,CAP_BUTT,JOIN_MITER);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  setLineAttributes(0,LINE_SOLID,CAP_BUTT,JOIN_MITER);
  }
  


// Draw border
void FXFrame::drawFrame(FXint x,FXint y,FXint w,FXint h){
  switch(options&FRAME_MASK) {
    case FRAME_LINE: drawBorderRectangle(x,y,w,h); break;
    case FRAME_SUNKEN: drawSunkenRectangle(x,y,w,h); break;
    case FRAME_RAISED: drawRaisedRectangle(x,y,w,h); break;
    case FRAME_GROOVE: drawGrooveRectangle(x,y,w,h); break;
    case FRAME_RIDGE: drawRidgeRectangle(x,y,w,h); break;
    case FRAME_SUNKEN|FRAME_THICK: drawDoubleSunkenRectangle(x,y,w,h); break;
    case FRAME_RAISED|FRAME_THICK: drawDoubleRaisedRectangle(x,y,w,h); break;
    }
  }


// Handle repaint 
long FXFrame::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onPaint(sender,sel,ptr);
  drawFrame(0,0,width,height);
  return 1;
  }


// Change frame border style
void FXFrame::setFrameStyle(FXuint style){
  FXuint opts=(options&~FRAME_MASK) | (style&FRAME_MASK);
  if(options!=opts){
    FXint b=(opts&FRAME_THICK) ? 2 : (opts&(FRAME_SUNKEN|FRAME_RAISED)) ? 1 : 0;
    options=opts;
    if(border!=b){
      border=b;
      recalc();
      }
    update(0,0,width,height);
    }
  }


// Get frame style
FXuint FXFrame::getFrameStyle() const { 
  return (options&FRAME_MASK); 
  }


// Set base color
void FXFrame::setBaseColor(FXPixel clr){
  baseColor=clr;
  update(0,0,width,height);
  }


// Set highlight color
void FXFrame::setHiliteColor(FXPixel clr){
  hiliteColor=clr;
  update(0,0,width,height);
  }


// Set shadow color
void FXFrame::setShadowColor(FXPixel clr){
  shadowColor=clr;
  update(0,0,width,height);
  }


// Set border color
void FXFrame::setBorderColor(FXPixel clr){
  borderColor=clr;
  update(0,0,width,height);
  }


// Get default width
FXint FXFrame::getDefaultWidth(){ 
  return (border<<1); 
  }


// Get default height
FXint FXFrame::getDefaultHeight(){ 
  return (border<<1); 
  }
