/********************************************************************************
*                                                                               *
*                           T o o l t i p   W i d g e t                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXTooltip.cpp,v 1.23 1998/10/31 00:00:32 jvz Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXTooltip.h"

/*
  To do:
  - Need hide-timer, perhaps, to hide it again after a while.
  - Tip briefly flashes, dont know why.
  - Need to make sure tooltip positioned inside screen.
*/

#define HSPACE  4
#define VSPACE  2


/*******************************************************************************/

// Map
FXDEFMAP(FXTooltip) FXTooltipMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTooltip::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXTooltip::onUpdate),
  FXMAPFUNC(SEL_TIMEOUT,FXTooltip::ID_TIP_SHOW,FXTooltip::onTipShow),
  FXMAPFUNC(SEL_TIMEOUT,FXTooltip::ID_TIP_HIDE,FXTooltip::onTipHide),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXTooltip::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXTooltip::onCmdGetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXTooltip,FXShell,FXTooltipMap,ARRAYNUMBER(FXTooltipMap))

  
// Deserialization
FXTooltip::FXTooltip(){
  font=NULL;
  textColor=0;
  timer=NULL;
  }

// Create a toplevel window
FXTooltip::FXTooltip(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXShell(a,opts,x,y,w,h),label("Tooltip"){
  font=getApp()->normalFont;
  textColor=0;
  timer=NULL;
  }


// Tooltips do override-redirect
FXbool FXTooltip::doesOverrideRedirect() const { return 1; }


// Tooltips do save-unders
FXbool FXTooltip::doesSaveUnder() const { return 1; }


// Create X window
void FXTooltip::create(){
  FXShell::create();
  textColor=acquireColor(getApp()->foreColor);
  setBackColor(acquireColor(FXRGB(255,255,192)));
  font->create();
  }


void FXTooltip::show(){
//   FXint x,y; FXuint state;
//   getRoot()->getCursorPosition(x,y,state);
//   position(x+10,y+10,getDefaultWidth(),getDefaultHeight());
//   FXShell::show();
//   raise();
  FXShell::show();
  raise();
  }


// Get default width
FXint FXTooltip::getDefaultWidth(){
  const FXchar *beg,*end;
  FXint w,tw=0;
  beg=label.text(); 
  if(beg){
    do{
      end=beg;
      while(*end!='\0' && *end!='\n') end++;
      if((w=font->getTextWidth(beg,end-beg))>tw) tw=w;
      beg=end+1;
      }
    while(*end!='\0');
    }
  return tw+HSPACE+HSPACE+2;
  }


// Get default height
FXint FXTooltip::getDefaultHeight(){
  const FXchar *beg,*end;
  FXint th=0;
  beg=label.text();
  if(beg){
    do{
      end=beg;
      while(*end!='\0' && *end!='\n') end++;
      th+=font->getFontHeight();
      beg=end+1;
      }
    while(*end!='\0');
    }
  return th+VSPACE+VSPACE+2;
  }


// Handle repaint 
long FXTooltip::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  const FXchar *beg,*end;
  FXint tx,ty;
  FXShell::onPaint(sender,sel,ptr);
  setForeground(textColor);
  setTextFont(font);
  drawRectangle(0,0,width-1,height-1);
  beg=label.text();
  if(beg){
    tx=1+HSPACE;
    ty=1+VSPACE+font->getFontAscent();
    do{
      end=beg;
      while(*end!='\0' && *end!='\n') end++;
      drawText(tx,ty,beg,end-beg);
      ty+=font->getFontHeight();
      beg=end+1;
      }
    while(*end!='\0');
    }
  return 1;
  }


// Place the tool tip
void FXTooltip::place(FXint x,FXint y){
  FXint rw=getRoot()->getWidth();
  FXint rh=getRoot()->getHeight();
  FXint w=getDefaultWidth();
  FXint h=getDefaultHeight();
  FXint px,py;
  px=x-w/3;
  py=y+10;
  if(px+w>rw) px=rw-w;
  if(px<0) px=0; 
  if(py+h>rh){ py=rh-h; if(py<=y && y<py+h) py=y-h-10; }
  if(py<0) py=0;
  position(px,py,w,h);
  }


// Automatically place tooltip
void FXTooltip::autoplace(){
  FXint x,y; FXuint state;
  getRoot()->getCursorPosition(x,y,state);
  place(x,y);
  }


// Update tooltip based on widget under cursor
long FXTooltip::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *helpsource=getApp()->getCursorWindow();
  FXWindow::onUpdate(sender,sel,ptr);
  if(!helpsource || !helpsource->handle(this,MKUINT(FXWindow::ID_QUERY_TIP,SEL_UPDATE),ptr)){
    if(timer){getApp()->removeTimeout(timer);timer=NULL;}
    label=0;
    hide();
    }
  return 1;
  }
////////////// Tooltips should be popped/unpopped differently...

// Pop the tool tip now
long FXTooltip::onTipShow(FXObject*,FXSelector,void*){
  timer=NULL;
  autoplace();
  show();
  if(!(options&TOOLTIP_PERMANENT)){
    timer=getApp()->addTimeout(getApp()->tooltipTime,this,ID_TIP_HIDE);
    }
  return 1;
  }


// Tip should hide now
long FXTooltip::onTipHide(FXObject*,FXSelector,void*){
  timer=NULL;
  hide();
  return 1;
  }


// Update value from a message
long FXTooltip::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setText(*((FXString*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXTooltip::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXString*)ptr) = getText(); }
  return 1;
  }



// Change text
void FXTooltip::setText(const FXchar* text){
  if(label!=text){
    label=text;
    if(!shown()){
      if(timer) getApp()->removeTimeout(timer);
      timer=getApp()->addTimeout(getApp()->tooltipPause,this,ID_TIP_SHOW);
      }
    else{
      autoplace();
      }
    recalc();
    update(0,0,width,height);
    }
  }


// Change the font
void FXTooltip::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Set text color
void FXTooltip::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Destroy label
FXTooltip::~FXTooltip(){
  if(timer) getApp()->removeTimeout(timer);
  font=(FXFont*)-1;
  timer=(FXTimer*)-1;
  }
