/********************************************************************************
*                                                                               *
*                         S c r o l l b a r   O b j e c t s                     *
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
* $Id: FXScrollbar.cpp,v 1.13 1998/10/28 15:09:54 jvz Exp $                   *
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
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXArrowButton.h"
#include "FXScrollbar.h"


#define SCROLLBAR_WIDTH 15

/* 
  To do:
  
  - Analyze app-wide setting of scrollbarWidth
  - Should increase/decrease, and slider get messages instead?
  - Scrollbar items should derive from FXWindow (as they are very simple).
  - Backpixmap:- need FXWindow backtile option
  - Then remove xincs.h
  - Need to fix background tile
  - ArrowButtons should auto-repeat themselves [so we can use regular arrow buttons].
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXScrollbarItem) FXScrollbarItemMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXScrollbarItem::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXScrollbarItem,FXFrame,FXScrollbarItemMap,ARRAYNUMBER(FXScrollbarItemMap))



// Construct and init
FXScrollbarItem::FXScrollbarItem(FXComposite* p,FXuint opts):
  FXFrame(p,opts|FRAME_RAISED|FRAME_THICK,0,0,SCROLLBAR_WIDTH,SCROLLBAR_WIDTH){
  state=STATE_UP;
  }


// Slightly different from Frame border
long FXScrollbarItem::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onPaint(sender,sel,ptr);
  if(options&FRAME_RAISED){
    setForeground(backColor);
    drawLine(0,0,width-2,0);
    drawLine(0,0,0,height-2);
    setForeground(hiliteColor);
    drawLine(1,1,width-3,1);
    drawLine(1,1,1, height-3);
    setForeground(shadowColor);
    drawLine(1,height-2,width-2,height-2);
    drawLine(width-2,height-2,width-2, 1);
    setForeground(borderColor);
    drawLine(0,height-1, width-1, height-1);
    drawLine(width-1,height-1,width-1, 0);
    }
  else if(options&FRAME_SUNKEN){
    setForeground(borderColor);
    drawLine(0,0,width-2,0);
    drawLine(0,0,0, height-2);
    setForeground(shadowColor);
    drawLine(1,1,width-3,1);
    drawLine(1,1,1,height-3);
    setForeground(hiliteColor);
    drawLine(0,height-1,width-1,height-1);
    drawLine(width-1,height-1,width-1,1);
    setForeground(backColor);
    drawLine(1,height-2,width-2,height-2);
    drawLine(width-2,height-2,width-2,2);
    }
  return 1;
  }


// Change state
void FXScrollbarItem::setState(FXint s) {
  if(state != s){
    switch(s){
    case STATE_DOWN:
      options&=~FRAME_RAISED;
      options|=FRAME_SUNKEN;
      break;
    case STATE_UP:
      options&=~FRAME_SUNKEN;
      options|=FRAME_RAISED;
      break;
      }
    state=s;
    update(0,0,width,height);
    }
  }


void FXScrollbarItem::create(){
  FXFrame::create();
  show();
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXScrollbarArrow) FXScrollbarArrowMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXScrollbarArrow::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXScrollbarArrow,FXScrollbarItem,FXScrollbarArrowMap,ARRAYNUMBER(FXScrollbarArrowMap))



// Construct and init
FXScrollbarArrow::FXScrollbarArrow(FXComposite* p,FXuint opts):
  FXScrollbarItem(p,opts){
  }


// Get default size
FXint FXScrollbarArrow::getDefaultWidth(){ 
  return getApp()->scrollbarWidth; 
  }


FXint FXScrollbarArrow::getDefaultHeight(){ 
  return getApp()->scrollbarWidth; 
  }


// Handle repaint 
long FXScrollbarArrow::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXPoint points[3];
  int xx,yy,ww,hh;
  FXScrollbarItem::onPaint(sender,sel,ptr);
  if(options&(ARROW_UP|ARROW_DOWN)){
    ww=9; hh=4;
    }
  else{
    ww=4; hh=9;
    }
  xx=(width-ww)/2;
  yy=(height-hh)/2;
  if(state==STATE_DOWN){ ++xx; ++yy; }
  setForeground(borderColor);
  if(options&ARROW_UP){
    points[0].x=xx+(ww>>1);
    points[0].y=yy-1;
    points[1].x=xx;
    points[1].y=yy+hh;
    points[2].x=xx+ww;
    points[2].y=yy+hh;
    }
  else if(options&ARROW_DOWN){
    points[0].x=xx+1;
    points[0].y=yy;
    points[1].x=xx+ww-1;
    points[1].y=yy;
    points[2].x=xx+(ww>>1);
    points[2].y=yy+hh;
    }
  else if(options&ARROW_LEFT){
    points[0].x=xx+ww;
    points[0].y=yy;
    points[1].x=xx+ww;
    points[1].y=yy+hh-1;
    points[2].x=xx;
    points[2].y=yy+(hh>>1);
    }
  else{ /*options&ARROW_RIGHT*/
    points[0].x=xx;
    points[0].y=yy;
    points[1].x=xx;
    points[1].y=yy+hh-1;
    points[2].x=xx+ww;
    points[2].y=yy+(hh>>1);
    }
  fillPolygon(points,3);
  return 1;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXScrollbar) FXScrollbarMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXScrollbar::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXScrollbar::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXScrollbar::onAnyBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXScrollbar::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXScrollbar::onAnyBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXScrollbar::onMiddleBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXScrollbar::onAnyBtnRelease),
  FXMAPFUNC(SEL_MOTION,0,FXScrollbar::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTOINC_LINE,FXScrollbar::onTimeIncLine),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTOINC_PAGE,FXScrollbar::onTimeIncPage),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTODEC_LINE,FXScrollbar::onTimeDecLine),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTODEC_PAGE,FXScrollbar::onTimeDecPage),
  };


// Object implementation
FXIMPLEMENT(FXScrollbar,FXComposite,FXScrollbarMap,ARRAYNUMBER(FXScrollbarMap))


// Make a text button
FXScrollbar::FXScrollbar(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h){
  slider=new FXScrollbarItem(this);
  if(options&SCROLLBAR_HORIZONTAL){
    decrease=new FXScrollbarArrow(this,ARROW_LEFT);
    increase=new FXScrollbarArrow(this,ARROW_RIGHT);
    thumbpos=decrease->getDefaultWidth();
    thumbsize=slider->getDefaultWidth();
    }
  else{
    decrease=new FXScrollbarArrow(this,ARROW_UP);
    increase=new FXScrollbarArrow(this,ARROW_DOWN);
    thumbpos=decrease->getDefaultHeight();
    thumbsize=slider->getDefaultHeight();
    }
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  timer=NULL;
  dragpoint=0;
  range=100;
  page=1;
  line=1;
  pos=0;
  }


// Get default size
FXint FXScrollbar::getDefaultWidth(){
  return options&SCROLLBAR_HORIZONTAL ? increase->getDefaultWidth()+decrease->getDefaultWidth()+slider->getDefaultWidth() : increase->getDefaultWidth();
  }


FXint FXScrollbar::getDefaultHeight(){
  return options&SCROLLBAR_HORIZONTAL ? increase->getDefaultHeight() : increase->getDefaultHeight()+decrease->getDefaultHeight()+slider->getDefaultHeight();
  }


// Pressed LEFT button in slider
long FXScrollbar::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXWindow *child=getChildAt(event->win_x,event->win_y);
  if(child==slider){
    if(options&SCROLLBAR_HORIZONTAL){
      dragpoint=event->win_x-thumbpos;
      }
    else{
      dragpoint=event->win_y-thumbpos;
      }
    flags|=FLAG_PRESSED;
    }
  else{
    if(child==increase){
      increase->setState(STATE_DOWN);
      autoScrollInc();
      pos+=line;
      }
    else if(child==decrease){
      decrease->setState(STATE_DOWN);
      autoScrollDec();
      pos-=line;
      }
    else{
      if(options&SCROLLBAR_HORIZONTAL){
        if(event->win_x<thumbpos){
          autoScrollPageDec();
          pos-=page;
          }
        else if(event->win_x>(thumbpos+thumbsize)){
          autoScrollPageInc();
          pos+=page;
          }
        }
      else{
        if(event->win_y<thumbpos){
          autoScrollPageDec();
          pos-=page;
          }
        else if(event->win_y>(thumbpos+thumbsize)){
          autoScrollPageInc();
          pos+=page;
          }
        }
      }
    if(pos<0) pos=0;
    if(pos>(range-page)) pos=range-page;
    layout();
    }
  return 1;
  }


// Pressed MIDDLE button in slider
long FXScrollbar::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  register int travel;
  dragpoint=thumbsize/2;
  if(options&SCROLLBAR_HORIZONTAL){
    thumbpos=event->win_x-dragpoint;
    if(thumbpos<decrease->getDefaultWidth()) thumbpos=decrease->getDefaultWidth();
    if(thumbpos>(width-increase->getDefaultWidth()-thumbsize)) thumbpos=width-increase->getDefaultWidth()-thumbsize;
    slider->position(thumbpos,0,thumbsize,height);
    travel=width-decrease->getDefaultWidth()-increase->getDefaultWidth()-thumbsize;
    pos=0;
    if(travel>0){ pos=((thumbpos-decrease->getDefaultWidth())*(range-page))/travel; }
    }
  else{
    thumbpos=event->win_y-dragpoint;
    if(thumbpos<decrease->getDefaultHeight()) thumbpos=decrease->getDefaultHeight();
    if(thumbpos>(height-increase->getDefaultHeight()-thumbsize)) thumbpos=height-increase->getDefaultHeight()-thumbsize;
    slider->position(0,thumbpos,width,thumbsize);
    travel=height-decrease->getDefaultHeight()-increase->getDefaultHeight()-thumbsize;
    pos=0;
    if(travel>0){ pos=((thumbpos-decrease->getDefaultHeight())*(range-page))/travel; }
    }
  flags|=FLAG_PRESSED;
  if(target && target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos)) return 1; 
  return 0;
  }


// Released LEFT or MIDDLE button
long FXScrollbar::onAnyBtnRelease(FXObject*,FXSelector,void*){
  FXASSERT(increase && decrease);
  increase->setState(STATE_UP);
  decrease->setState(STATE_UP);
  stopAutoScroll();
  flags&=~FLAG_PRESSED;
  if(target && target->handle(this,MKUINT(message,SEL_COMMAND),(void*)pos)) return 1; 
  return 0;
  }


// Moving 
long FXScrollbar::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  register int travel;
  if(flags&FLAG_PRESSED){
    if(options&SCROLLBAR_HORIZONTAL){
      thumbpos=event->win_x-dragpoint;
      if(thumbpos<decrease->getDefaultWidth()) thumbpos=decrease->getDefaultWidth();
      if(thumbpos>(width-increase->getDefaultWidth()-thumbsize)) thumbpos=width-increase->getDefaultWidth()-thumbsize;
      slider->position(thumbpos,0,thumbsize,height);
      travel=width-decrease->getDefaultWidth()-increase->getDefaultWidth()-thumbsize;
      pos=0;
      if(travel>0){ pos=((thumbpos-decrease->getDefaultWidth())*(range-page))/travel; }
      }
    else{
      thumbpos=event->win_y-dragpoint;
      if(thumbpos<decrease->getDefaultHeight()) thumbpos=decrease->getDefaultHeight();
      if(thumbpos>(height-increase->getDefaultHeight()-thumbsize)) thumbpos=height-increase->getDefaultHeight()-thumbsize;
      slider->position(0,thumbpos,width,thumbsize);
      travel=height-decrease->getDefaultHeight()-increase->getDefaultHeight()-thumbsize;
      pos=0;
      if(travel>0){ pos=((thumbpos-decrease->getDefaultHeight())*(range-page))/travel; }
      }
    if(target && target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos)) return 1; 
    }
  return 0;
  }


// Increment line timeout
long FXScrollbar::onTimeIncLine(FXObject*,FXSelector,void*){
  pos+=line;
  if(pos>(range-page)){
    pos=range-page;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTOINC_LINE);
    }
  layout();
  if(target && target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos)) return 1;
  return 0;
  }


// Increment page timeout
long FXScrollbar::onTimeIncPage(FXObject*,FXSelector,void*){
  pos+=page;
  if(pos>(range-page)){
    pos=range-page;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTOINC_PAGE);
    }
  layout();
  if(target && target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos)) return 1;
  return 0;
  }


// Decrement line timeout
long FXScrollbar::onTimeDecLine(FXObject*,FXSelector,void*){
  pos-=line;
  if(pos<0){
    pos=0;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTODEC_LINE);
    }
  layout();
  if(target && target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos)) return 1;
  return 0;
  }


// Decrement page timeout
long FXScrollbar::onTimeDecPage(FXObject*,FXSelector,void*){
  pos-=page;
  if(pos<0){
    pos=0;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTODEC_PAGE);
    }
  layout();
  if(target && target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos)) return 1;
  return 0;
  }


// Recalculate layout
void FXScrollbar::layout(){
  register int total,travel;
  FXASSERT(0<page && page<=range);
  FXASSERT(0<=pos && pos<=(range-page));
  if(options&SCROLLBAR_HORIZONTAL){
    decrease->position(0,0,decrease->getDefaultWidth(),height);
    increase->position(width-increase->getDefaultWidth(),0,increase->getDefaultWidth(),height);
    total=width-decrease->getDefaultWidth()-increase->getDefaultWidth();
    thumbsize=(total*page)/range;
    if(thumbsize<slider->getDefaultWidth()) thumbsize=slider->getDefaultWidth();
    travel=total-thumbsize;
    thumbpos=decrease->getDefaultWidth();
    if(range>page){ thumbpos+=(pos*travel)/(range-page); }
    slider->position(thumbpos,0,thumbsize,height);
    }
  else{
    decrease->position(0,0,width,decrease->getDefaultHeight());
    increase->position(0,height-increase->getDefaultWidth(),width,increase->getDefaultHeight());
    total=height-decrease->getDefaultHeight()-increase->getDefaultHeight();
    thumbsize=(total*page)/range;
    if(thumbsize<slider->getDefaultHeight()) thumbsize=slider->getDefaultHeight();
    travel=total-thumbsize;
    thumbpos=decrease->getDefaultHeight();
    if(range>page){ thumbpos+=(pos*travel)/(range-page); }
    slider->position(0,thumbpos,width,thumbsize);
    }
  flags&=~FLAG_DIRTY;
  }


// Create X window
void FXScrollbar::create(){
  FXComposite::create();
  XSetWindowBackgroundPixmap(getDisplay(),xid,getGrayBitmap());
  show();
  }


// Handle repaint 
long FXScrollbar::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXComposite::onPaint(sender,sel,ptr);
  // to do: should be black where pressed
  return 1;
  }
  

// Set range
void FXScrollbar::setRange(FXint r){
  range=r; 
  if(range<1) range=1;
  if(page>range) page=range;
  if(page<line) line=page;
  if(pos>(range-page)) pos=range-page;
  layout();
  }


// Set page size
void FXScrollbar::setPage(FXint p){
  page=p;
  if(page<1) page=1;
  if(page>range) page=range;
  if(pos>(range-page)) pos=range-page;
  if(page<line) line=page;
  layout();
  }


// Set line size
void FXScrollbar::setLine(FXint l){
  line=l;
  if(line<1) line=1;
  if(line>page) line=page;
  }


// Set position
void FXScrollbar::setPosition(FXint p){
  if(p<0) p=0;
  if(p>(range-page)) p=range-page;
  if(p!=pos){
    pos=p;
    layout();
    }
  }


// Start automatic increment
void FXScrollbar::autoScrollInc(){
  if(timer) getApp()->removeTimeout(timer);
  timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTOINC_LINE);
  }


// Start automatic decrement
void FXScrollbar::autoScrollDec(){
  if(timer) getApp()->removeTimeout(timer);
  timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTODEC_LINE);
  }


// Start automatic page increment
void FXScrollbar::autoScrollPageInc(){
  if(timer) getApp()->removeTimeout(timer);
  timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTOINC_PAGE);
  }
  

// Start automatic page decrement
void FXScrollbar::autoScrollPageDec(){
  if(timer) getApp()->removeTimeout(timer);
  timer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXScrollbar::ID_AUTODEC_PAGE);
  }
  

// Stop it
void FXScrollbar::stopAutoScroll(){
  if(timer) getApp()->removeTimeout(timer);
  timer=NULL;
  }


// True if automatic scrolling
FXint FXScrollbar::isAutoScrolling(){
  return timer!=NULL;
  }


// Delete
FXScrollbar::~FXScrollbar(){
  stopAutoScroll();
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXScrollCorner) FXScrollCornerMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXScrollCorner::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXScrollCorner,FXFrame,FXScrollCornerMap,ARRAYNUMBER(FXScrollCornerMap))


// Construct and init
FXScrollCorner::FXScrollCorner(FXComposite* p):FXFrame(p){
  flags|=FLAG_ENABLED;
  }


// Create X Window
void FXScrollCorner::create(){
  FXFrame::create();
  }


// Slightly different from Frame border
long FXScrollCorner::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onPaint(sender,sel,ptr);
  return 1;
  }

void FXScrollCorner::enable(){ }

void FXScrollCorner::disable(){ }

