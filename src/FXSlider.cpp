/********************************************************************************
*                                                                               *
*                             S l i d e r   O b j e c t s                       *
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
* $Id: FXSlider.cpp,v 1.23 1998/10/29 05:38:14 jeroen Exp $                     *
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
#include "FXSlider.h"


#define OVERHANG 4

#define MINOVERHANG 3


/* 
  Notes:
  - Should we let the slider head catch messages?
  - Add other options for tickmarks and so on
  - Add API's to change sizes and appearance
  - Should sliderhead inherit from FXFrame?
  - Dual/triple head options.
  - Need to be able to disable/enable, head should show/hide.
  - Sliderhead starts too small for SLIDER_INSIDE_BAR mode.
  - What messages should be sent?
  - Should we have a special head window?
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXSliderHead) FXSliderHeadMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXSliderHead::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXSliderHead,FXFrame,FXSliderHeadMap,ARRAYNUMBER(FXSliderHeadMap))



// Construct and init
FXSliderHead::FXSliderHead(FXComposite* p,FXuint opts):
  FXFrame(p,opts){
  }


// Create X Window
void FXSliderHead::create(){
  FXFrame::create();
  show();
  }


// Get default size
FXint FXSliderHead::getDefaultWidth(){
  return 7; 
  }


FXint FXSliderHead::getDefaultHeight(){ 
  return 7; 
  }


// Slightly different from Frame border
long FXSliderHead::onPaint(FXObject*,FXSelector,void*){
  FXint m;
  clearWindow();
  if(options&SLIDER_VERTICAL){
    m=(height>>1);
    if(options&SLIDER_ARROW_LEFT){
      setForeground(hiliteColor);
      drawLine(m,0,width-1,0);
      drawLine(0,m,m,0);
      setForeground(shadowColor);
      drawLine(1,height-m-1,m+1,height-1);
      drawLine(m,height-2,width-2,height-2);
      drawLine(width-2,1,width-2,height-2);
      setForeground(borderColor);
      drawLine(0,height-m-1,m,height-1);
      drawLine(width-1,0,width-1,height-1);
      drawLine(m,height-1,width-1,height-1);
      }
    else if(options&SLIDER_ARROW_RIGHT){
      setForeground(hiliteColor);
      drawLine(0,0,width-m-2,0);
      drawLine(0,1,0,height-1);
      drawLine(width-m-1,0,width-1,m);
      setForeground(shadowColor);
      drawLine(width-m-2,height-1,width-2,height-m-1);
      drawLine(1,height-2,width-m-2,height-2);
      setForeground(borderColor);
      drawLine(width-m-1,height-1,width-1,height-m-1);
      drawLine(0,height-1,width-m-2,height-1);
      }
    else if(options&SLIDER_INSIDE_BAR){
      drawDoubleRaisedRectangle(0,0,width,height);
      setForeground(shadowColor);
      drawLine(1,m-1,width-2,m-1);
      setForeground(hiliteColor);
      drawLine(1,m,width-2,m);
      }
    else{
      drawDoubleRaisedRectangle(0,0,width,height);
      }
    }
  else{
    m=(width>>1);
    if(options&SLIDER_ARROW_UP){
      setForeground(hiliteColor);
      drawLine(0,m,m,0);
      drawLine(0,m,0,height-1);
      setForeground(shadowColor);
      drawLine(width-1,m+1,width-m-1,1);
      drawLine(width-2,m+1,width-2,height-2);
      drawLine(1,height-2,width-2,height-2);
      setForeground(borderColor);
      drawLine(width-1,m,width-m-1,0);
      drawLine(width-1,m,width-1,height-1);
      drawLine(0,height-1,width-1,height-1);
      }
    else if(options&SLIDER_ARROW_DOWN){
      setForeground(hiliteColor);
      drawLine(0,0,width-1,0);
      drawLine(0,1,0,height-m-1);
      drawLine(0,height-m-1,m,height-1);
      setForeground(shadowColor);
      drawLine(width-2,1,width-2,height-m-2);
      drawLine(width-1,height-m-2,width-m-1,height-2);
      setForeground(borderColor);
      drawLine(width-1,0,width-1,height-m-1);
      drawLine(width-1,height-m-1,width-m-1,height-1);
      }
    else if(options&SLIDER_INSIDE_BAR){
      drawDoubleRaisedRectangle(0,0,width,height);
      setForeground(shadowColor);
      drawLine(m-1,1,m-1,height-2);
      setForeground(hiliteColor);
      drawLine(m,1,m,height-1);
      }
    else{
      drawDoubleRaisedRectangle(0,0,width,height);
      }
    }
  return 1;
  }


/*******************************************************************************/


// Map
FXDEFMAP(FXSlider) FXSliderMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXSlider::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXSlider::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXSlider::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXSlider::onAnyBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXSlider::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXSlider::onAnyBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXSlider::onMiddleBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXSlider::onAnyBtnRelease),
  FXMAPFUNC(SEL_TIMEOUT,1,FXSlider::onTimeInc),
  FXMAPFUNC(SEL_TIMEOUT,2,FXSlider::onTimeDec),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXSlider::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXSlider::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALVALUE,FXSlider::onCmdSetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXSlider::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALVALUE,FXSlider::onCmdGetRealValue),
  };


// Object implementation
FXIMPLEMENT(FXSlider,FXComposite,FXSliderMap,ARRAYNUMBER(FXSliderMap))


// Make a text button
FXSlider::FXSlider(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h){
  head=new FXSliderHead(this,opts&~LAYOUT_MASK);
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  timer=NULL;
  range[0]=0;
  range[1]=100;
  slotColor=0;
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  headpos=0;
  dragpoint=0;
  headsize=(options&SLIDER_INSIDE_BAR)?20:9;
  slotsize=5;
  incr=1;
  pos=50;
  }


// Enable the window
void FXSlider::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXComposite::enable();
    update(0,0,width,height);
    }
  }


// Disable the window
void FXSlider::disable(){
  if(flags&FLAG_ENABLED){
    FXComposite::disable();
    update(0,0,width,height);
    }
  }


// If window can have focus
FXbool FXSlider::canFocus() const { return 1; }


// Get default size
FXint FXSlider::getDefaultWidth(){
  FXint w;
  if(options&SLIDER_VERTICAL){
    if(options&SLIDER_INSIDE_BAR) w=4+headsize/2;
    else if(options&(SLIDER_ARROW_LEFT|SLIDER_ARROW_RIGHT)) w=slotsize+MINOVERHANG*2+headsize/2;
    else w=slotsize+MINOVERHANG*2;
    }
  else{
    w=headsize+4;
    }
  return w;
  }


FXint FXSlider::getDefaultHeight(){
  FXint h;
  if(options&SLIDER_VERTICAL){
    h=headsize+4;
    }
  else{
    if(options&SLIDER_INSIDE_BAR) h=4+headsize/2;
    else if(options&(SLIDER_ARROW_UP|SLIDER_ARROW_DOWN)) h=slotsize+2*MINOVERHANG+headsize/2;
    else h=slotsize+MINOVERHANG*2;
    }
  return h;
  }


// Update value from a message
long FXSlider::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setPosition((FXint)ptr);
  return 1;
  }


// Update value from a message
long FXSlider::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setPosition(*((FXint*)ptr)); }
  return 1;
  }


// Update value from a message
long FXSlider::onCmdSetRealValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setPosition((FXint) *((FXdouble*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXSlider::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXint*)ptr)=getPosition(); return 1; }
  return 0;
  }


// Obtain value from text field
long FXSlider::onCmdGetRealValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXdouble*)ptr)=(FXdouble)getPosition(); return 1; }
  return 0;
  }



// Pressed LEFT button
long FXSlider::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXWindow *child;
  if(!isEnabled()) return 0;
  if(canFocus()) setFocus();
  child=getChildAt(event->win_x,event->win_y);
  flags&=~FLAG_UPDATE;
  if(child==head){
    if(options&SLIDER_VERTICAL){
      dragpoint=event->win_y-headpos;
      }
    else{
      dragpoint=event->win_x-headpos;
      }
    flags|=FLAG_PRESSED;
    }
  else{
    if(options&SLIDER_VERTICAL){
      if(event->win_y<headpos){
        autoSlideDec();
        pos-=incr;
        }
      else if(event->win_y>(headpos+headsize)){
        autoSlideInc();
        pos+=incr;
        }
      }
    else{
      if(event->win_x<headpos){
        autoSlideDec();
        pos-=incr;
        }
      else if(event->win_x>(headpos+headsize)){
        autoSlideInc();
        pos+=incr;
        }
      }
    if(pos<range[0]) pos=range[0];
    if(pos>range[1]) pos=range[1];
    layout();
    }
  return 1;
  }


// Released ANY button
long FXSlider::onAnyBtnRelease(FXObject*,FXSelector,void*){
  if(!isEnabled()) return 0;
  stopAutoSlide();
  ///// check if changed, really!
  if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)pos); 
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  return 1;
  }


// Moving 
long FXSlider::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  register int oldpos,travel;
  if(!isEnabled()) return 0;
  if(flags&FLAG_PRESSED){
    oldpos=pos;
    if(options&SLIDER_VERTICAL){
      headpos=event->win_y-dragpoint;
      if(headpos<2) headpos=2;
      if(headpos>(height-headsize-2)) headpos=height-headsize-2;
      travel=height-headsize-4;
      pos=range[0];
      if(travel>0){ pos=range[0]+((range[1]-range[0])*(headpos-2))/travel; }
      }
    else{
      headpos=event->win_x-dragpoint;
      if(headpos<2) headpos=2;
      if(headpos>(width-headsize-2)) headpos=width-headsize-2;
      travel=width-headsize-4;
      pos=range[0];
      if(travel>0){ pos=range[0]+((range[1]-range[0])*(headpos-2))/travel; }
      }
    layout();
    if(oldpos!=pos){
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos); 
      return 1;
      }
    }
  return 0;
  }


// Pressed middle or right
long FXSlider::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  register int travel;
  if(!isEnabled()) return 0;
  if(canFocus()) setFocus();
  dragpoint=headsize/2;
  if(options&SLIDER_VERTICAL){
    headpos=event->win_y-dragpoint;
    if(headpos<2) headpos=2;
    if(headpos>(height-headsize-2)) headpos=height-headsize-2;
    travel=height-headsize-4;
    pos=range[0];
    if(travel>0){ pos=range[0]+((range[1]-range[0])*(headpos-2))/travel; }
    }
  else{
    headpos=event->win_x-dragpoint;
    if(headpos<2) headpos=2;
    if(headpos>(width-headsize-2)) headpos=width-headsize-2;
    travel=width-headsize-4;
    pos=range[0];
    if(travel>0){ pos=range[0]+((range[1]-range[0])*(headpos-2))/travel; }
    }
  layout();
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos); 
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  return 1;
  }


// Increment line timeout
long FXSlider::onTimeInc(FXObject*,FXSelector,void*){
  pos+=incr;
  if(pos>range[1]){
    pos=range[1];
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->scrollSpeed,this,1);
    }
  layout();
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos); 
  return 1;
  }


// Decrement line timeout
long FXSlider::onTimeDec(FXObject*,FXSelector,void*){
  pos-=incr;
  if(pos<range[0]){
    pos=range[0];
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->scrollSpeed,this,2);
    }
  layout();
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos); 
  return 1;
  }


// Recalculate layout
void FXSlider::layout(){
  FXint travel;
  FXASSERT(range[0]<=pos && pos<=range[1]);
  if(options&SLIDER_VERTICAL){
    travel=height-headsize-4;
    headpos=2;
    if(range[1]>range[0]) headpos=2+(travel*(pos-range[0]))/(range[1]-range[0]);
    if(options&SLIDER_INSIDE_BAR)
      sliderHead()->position(2,headpos,width-4,headsize);
    else
      sliderHead()->position(0,headpos,width,headsize);
    }
  else{
    travel=width-headsize-4;
    headpos=2;
    if(range[1]>range[0]) headpos=2+(travel*(pos-range[0]))/(range[1]-range[0]);
    if(options&SLIDER_INSIDE_BAR)
      sliderHead()->position(headpos,2,headsize,height-4);
    else
      sliderHead()->position(headpos,0,headsize,height);
    }
  if(isEnabled())
    sliderHead()->show();
  else
    sliderHead()->hide();
  flags&=~FLAG_DIRTY;
  }


// Create X window
void FXSlider::create(){
  FXComposite::create();
  slotColor=acquireColor(FXRGB(255,255,255));
  baseColor=acquireColor(getApp()->backColor);
  hiliteColor=acquireColor(getApp()->hiliteColor);
  shadowColor=acquireColor(getApp()->shadowColor);
  borderColor=acquireColor(getApp()->borderColor);
  if(options&SLIDER_INSIDE_BAR) XSetWindowBackgroundPixmap(getDisplay(),xid,getGrayBitmap());
  show();
  }


// Draw slider slot
void FXSlider::drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
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


// Handle repaint 
long FXSlider::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint tx,ty,hhs=headsize/2;
  FXWindow::onPaint(sender,sel,ptr);
  if(options&SLIDER_VERTICAL){
    if(options&SLIDER_INSIDE_BAR){
      drawDoubleSunkenRectangle(0,0,width,height);
      setForeground(slotColor);
      //fillRectangle(2,2,width-4,height-4);
      }
    else{
      if(options&SLIDER_ARROW_LEFT) tx=hhs+(width-slotsize-hhs)/2;
      else if(options&SLIDER_ARROW_RIGHT) tx=(width-slotsize-hhs)/2;
      else tx=(width-slotsize)/2;
      drawDoubleSunkenRectangle(tx,0,slotsize,height);
      setForeground(slotColor);
      fillRectangle(tx+2,2,slotsize-4,height-4);
      }
    }
  else{
    if(options&SLIDER_INSIDE_BAR){
      drawDoubleSunkenRectangle(0,0,width,height);
      setForeground(slotColor);
      //fillRectangle(2,2,width-4,height-4);
      }
    else{
      if(options&SLIDER_ARROW_UP) ty=hhs+(height-slotsize-hhs)/2;
      else if(options&SLIDER_ARROW_DOWN) ty=(height-slotsize-hhs)/2;
      else ty=(height-slotsize)/2;
      drawDoubleSunkenRectangle(0,ty,width,slotsize);
      setForeground(slotColor);
      fillRectangle(2,ty+2,width-4,slotsize-4);
      }
    }
  return 1;
  }


// Set slider range  
void FXSlider::setRange(FXint lo,FXint hi){
  if(lo>hi){ fxerror("%s::setRange: trying to set negative range.\n",getClassName()); }
  if(range[0]!=lo || range[1]!=hi){
    range[0]=lo;
    range[1]=hi;
    if(pos<range[0]) pos=range[0];
    if(pos>range[1]) pos=range[1];
    layout();
    }
  }


// Set position
void FXSlider::setPosition(FXint p){
  if(p<range[0]) p=range[0];
  if(p>range[1]) p=range[1];
  if(p!=pos){
    pos=p;
    layout();
    }
  }


// Get slider options
FXuint FXSlider::getSliderStyle() const {
  return (options&SLIDER_MASK); 
  }


// Set slider options
void FXSlider::setSliderStyle(FXuint style){
  FXuint opts=(options&~SLIDER_MASK) | (style&SLIDER_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }



// Set head size
void FXSlider::setHeadSize(FXint hs){
  if(headsize!=hs){
    headsize=hs;
    recalc();
    }
  }

  
// Set slot size
void FXSlider::setSlotSize(FXint bs){
  if(slotsize!=bs){
    slotsize=bs;
    recalc();
    }
  }


// Set slot color
void FXSlider::setSlotColor(FXPixel clr){
  slotColor=clr;
  update(0,0,width,height);
  }


// Set base color
void FXSlider::setBaseColor(FXPixel clr){
  baseColor=clr;
  update(0,0,width,height);
  }


// Set highlight color
void FXSlider::setHiliteColor(FXPixel clr){
  hiliteColor=clr;
  update(0,0,width,height);
  }


// Set shadow color
void FXSlider::setShadowColor(FXPixel clr){
  shadowColor=clr;
  update(0,0,width,height);
  }


// Set border color
void FXSlider::setBorderColor(FXPixel clr){
  borderColor=clr;
  update(0,0,width,height);
  }


// Start automatic increment
void FXSlider::autoSlideInc(){
  if(timer) getApp()->removeTimeout(timer);
  timer=getApp()->addTimeout(getApp()->scrollSpeed,this,1);
  }


// Start automatic decrement
void FXSlider::autoSlideDec(){
  if(timer) getApp()->removeTimeout(timer);
  timer=getApp()->addTimeout(getApp()->scrollSpeed,this,2);
  }



// Stop it
void FXSlider::stopAutoSlide(){
  if(timer) getApp()->removeTimeout(timer);
  timer=NULL;
  }


// True if automatic scrolling
FXbool FXSlider::isAutoSliding(){
  return timer!=NULL;
  }


// Delete
FXSlider::~FXSlider(){
  stopAutoSlide();
  head=(FXSliderHead*)-1;
  timer=(FXTimer*)-1;
  }


