/********************************************************************************
*                                                                               *
*                G r o u p  B o x   W i n d o w   O b j e c t                   *
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
* $Id: FXGroupBox.cpp,v 1.13 1998/08/26 07:41:16 jeroen Exp $                    *
********************************************************************************/
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXGroupBox.h"


/*
  To do:
  
  - FXGroupBox should have radio behaviour
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXGroupBox) FXGroupBoxMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXGroupBox::onPaint),
  FXMAPFUNC(SEL_UNCHECK_OTHER,0,FXGroupBox::onUncheckOther),
  };


// Object implementation
FXIMPLEMENT(FXGroupBox,FXPacker,FXGroupBoxMap,ARRAYNUMBER(FXGroupBoxMap))


// Deserialization
FXGroupBox::FXGroupBox(){
  font=(FXFont*)-1;
  textColor=0;
  }


// Make a horizontal one
FXGroupBox::FXGroupBox(FXComposite* p,const char* text,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXPacker(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs),label(text){
  font=getApp()->normalFont;
  textColor=0;
  }


// Create X window
void FXGroupBox::create(){
  FXPacker::create();
  textColor=acquireColor(getApp()->foreColor);
  font->create();
  }


// Change the font
void FXGroupBox::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Get default width
FXint FXGroupBox::getDefaultWidth(){
  FXint cw=FXPacker::getDefaultWidth();
  if(label.text()){
    FXint tw=font->getTextWidth(label.text(),label.length())+16;
    return FXMAX(cw,tw);
    }
  return cw;
  }


// Get default height
FXint FXGroupBox::getDefaultHeight(){
  FXint ch=FXPacker::getDefaultHeight();
  if(label.text()){ 
    return ch+font->getFontHeight()+4;
    }
  return ch;
  }


// Recompute layout
void FXGroupBox::layout(){
  FXint tmp=padtop;
  if(label.text()){
    padtop=padtop+font->getFontHeight();
    }
  FXPacker::layout();
  padtop=tmp;
  flags&=~FLAG_DIRTY;
  }


// Handle repaint 
long FXGroupBox::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint tw,yy,xx,hh;
  FXASSERT(xid!=0);

  tw=0;
  xx=0;
  yy=0;
  hh=height;
  
  // Paint background
  FXWindow::onPaint(sender,sel,ptr);
  
  // Draw label if there is one
  if(label.text()){
    tw=font->getTextWidth(label.text(),label.length());
    yy=2+font->getFontAscent()/2;
    hh=height-yy;
    }
    
  // We should really just draw what's exposed!
  switch(options&FRAME_MASK) {
    case FRAME_LINE: drawBorderRectangle(0,yy,width,hh); break;
    case FRAME_SUNKEN: drawSunkenRectangle(0,yy,width,hh); break;
    case FRAME_RAISED: drawRaisedRectangle(0,yy,width,hh); break;
    case FRAME_GROOVE: drawGrooveRectangle(0,yy,width,hh); break;
    case FRAME_RIDGE: drawRidgeRectangle(0,yy,width,hh); break;
    case FRAME_SUNKEN|FRAME_THICK: drawDoubleSunkenRectangle(0,yy,width,hh); break;
    case FRAME_RAISED|FRAME_THICK: drawDoubleRaisedRectangle(0,yy,width,hh); break;
    }
  
  // Draw label
  if(label.text()){
    if(options&GROUPBOX_TITLE_RIGHT) xx=width-tw-16;
    else if(options&GROUPBOX_TITLE_CENTER) xx=(width-tw)/2-4;
    else xx=8;
    setForeground(backColor);
    setTextFont(font);
    fillRectangle(xx,yy,tw+8,2);
    setForeground(textColor);
    drawText(xx+4,font->getFontAscent(),label.text(),label.length());
    }
  return 1;
  }


// Uncheck other radio buttons
long FXGroupBox::onUncheckOther(FXObject* sender,FXSelector,void*){
  FXWindow* child;
  for(child=getFirst(); child; child=child->getNext()){
    if(child!=sender){ child->handle(sender,MKUINT(0,SEL_UNCHECK_RADIO),NULL); }
    }
  return 1;
  }


// Change text
void FXGroupBox::setText(const FXchar* text){
  if(label!=text){
    label=text;
    recalc();
    update(0,0,width,height);
    }
  }


// Set text color
void FXGroupBox::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


