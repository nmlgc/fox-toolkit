/********************************************************************************
*                                                                               *
*                         S c r o l l b a r   O b j e c t s                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXScrollbar.cpp,v 1.40.4.2 2003/06/20 19:02:07 fox Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxkeys.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXScrollbar.h"

/*
  Notes:
  - Should increase/decrease, and slider get messages instead?
  - Scrollbar items should derive from FXWindow (as they are very simple).
  - If non-scrollable, but drawn anyway, don't draw thumb!
  - In case of a coarse range, we have rounding also.
  - The API's setPosition(), setRange() and setPage() should probably have
    an optional notify callback.
*/


#define THUMB_MINIMUM        8
#define BAR_SIZE            15
#define PRESSED_INC          1
#define PRESSED_DEC          2
#define PRESSED_PAGEINC      4
#define PRESSED_PAGEDEC      8
#define PRESSED_THUMB       16
#define PRESSED_FINETHUMB   32
#define SCROLLBAR_MASK      SCROLLBAR_HORIZONTAL


/*******************************************************************************/

// Map
FXDEFMAP(FXScrollbar) FXScrollbarMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXScrollbar::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXScrollbar::onMotion),
  FXMAPFUNC(SEL_MOUSEWHEEL,0,FXScrollbar::onMouseWheel),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXScrollbar::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXScrollbar::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXScrollbar::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXScrollbar::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXScrollbar::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXScrollbar::onRightBtnRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXScrollbar::onUngrabbed),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_TIMEWHEEL,FXScrollbar::onTimeWheel),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTOINC_PIX,FXScrollbar::onTimeIncPix),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTOINC_LINE,FXScrollbar::onTimeIncLine),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTOINC_PAGE,FXScrollbar::onTimeIncPage),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTODEC_PIX,FXScrollbar::onTimeDecPix),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTODEC_LINE,FXScrollbar::onTimeDecLine),
  FXMAPFUNC(SEL_TIMEOUT,FXScrollbar::ID_AUTODEC_PAGE,FXScrollbar::onTimeDecPage),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXScrollbar::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXScrollbar::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXScrollbar::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTRANGE,FXScrollbar::onCmdSetIntRange),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTRANGE,FXScrollbar::onCmdGetIntRange),
  };


// Object implementation
FXIMPLEMENT(FXScrollbar,FXWindow,FXScrollbarMap,ARRAYNUMBER(FXScrollbarMap))


// For deserialization
FXScrollbar::FXScrollbar(){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  thumbpos=BAR_SIZE;
  thumbsize=THUMB_MINIMUM;
  timer=NULL;
  dragpoint=0;
  dragjump=0;
  pressed=0;
  }


// Make a scrollbar
FXScrollbar::FXScrollbar(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  backColor=getApp()->getBaseColor();
  hiliteColor=getApp()->getHiliteColor();
  shadowColor=getApp()->getShadowColor();
  borderColor=getApp()->getBorderColor();
  thumbpos=BAR_SIZE;
  thumbsize=THUMB_MINIMUM;
  target=tgt;
  message=sel;
  timer=NULL;
  dragpoint=0;
  dragjump=0;
  range=100;
  page=1;
  line=1;
  pos=0;
  pressed=0;
  }


// Get default size
FXint FXScrollbar::getDefaultWidth(){
  return (options&SCROLLBAR_HORIZONTAL) ? BAR_SIZE+BAR_SIZE+THUMB_MINIMUM : BAR_SIZE;
  }


FXint FXScrollbar::getDefaultHeight(){
  return (options&SCROLLBAR_HORIZONTAL) ? BAR_SIZE : BAR_SIZE+BAR_SIZE+THUMB_MINIMUM;
  }


// Layout changed
void FXScrollbar::layout(){
  setPosition(pos);
  flags&=~FLAG_DIRTY;
  }


// Update value from a message
long FXScrollbar::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setPosition((FXint)(FXival)ptr);
  return 1;
  }


// Update value from a message
long FXScrollbar::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  setPosition(*((FXint*)ptr));
  return 1;
  }



// Obtain value with a message
long FXScrollbar::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  *((FXint*)ptr)=getPosition();
  return 1;
  }


// Update range from a message
long FXScrollbar::onCmdSetIntRange(FXObject*,FXSelector,void* ptr){
  setRange(((FXint*)ptr)[1]);
  return 1;
  }


// Get range with a message
long FXScrollbar::onCmdGetIntRange(FXObject*,FXSelector,void* ptr){
  ((FXint*)ptr)[0]=0;
  ((FXint*)ptr)[1]=getRange();
  return 1;
  }


// Pressed LEFT button in slider
// Note we don't move the focus to the scrollbar widget!
long FXScrollbar::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  register FXEvent *event=(FXEvent*)ptr;
  register FXint p=pos;
  if(isEnabled()){
    grab();
    if(timer) timer=getApp()->removeTimeout(timer);
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    flags&=~FLAG_UPDATE;
    if(options&SCROLLBAR_HORIZONTAL){     // Horizontal scrollbar
      if(event->win_x<height){                   // Left arrow
        pressed=PRESSED_DEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_LINE);
        p=pos-line;
        update();
        }
      else if(width-height<=event->win_x){       // Right arrow
        pressed=PRESSED_INC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_LINE);
        p=pos+line;
        update();
        }
      else if(event->win_x<thumbpos){             // Page left
        pressed=PRESSED_PAGEDEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_PAGE);
        p=pos-page;
        update();
        }
      else if(thumbpos+thumbsize<=event->win_x){  // Page right
        pressed=PRESSED_PAGEINC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_PAGE);
        p=pos+page;
        update();
        }
      else{                                       // Grabbed the puck
        pressed=PRESSED_THUMB;
        dragpoint=event->win_x-thumbpos;
        flags|=FLAG_PRESSED;
        }
      }
    else{                                 // Vertical scrollbar
      if(event->win_y<width){                   // Up arrow
        pressed=PRESSED_DEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_LINE);
        p=pos-line;
        update();
        }
      else if(height-width<=event->win_y){      // Down arrow
        pressed=PRESSED_INC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_LINE);
        p=pos+line;
        update();
        }
      else if(event->win_y<thumbpos){             // Page up
        pressed=PRESSED_PAGEDEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_PAGE);
        p=pos-page;
        update();
        }
      else if(thumbpos+thumbsize<=event->win_y){  // Page down
        pressed=PRESSED_PAGEINC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_PAGE);
        p=pos+page;
        update();
        }
      else{                                       // Grabbed the puck
        pressed=PRESSED_THUMB;
        if(event->state&(CONTROLMASK|SHIFTMASK|ALTMASK)) pressed=PRESSED_FINETHUMB;
        dragpoint=event->win_y-thumbpos;
        flags|=FLAG_PRESSED;
        }
      }
    if(p<0) p=0;
    if(p>(range-page)) p=range-page;
    if(p!=pos){
      setPosition(p);
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      }
    return 1;
    }
  return 0;
  }


// Released LEFT button
long FXScrollbar::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    dragpoint=0;
    pressed=0;
    setPosition(pos);
    update();
    if(timer){ timer=getApp()->removeTimeout(timer); }
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      }
    return 1;
    }
  return 0;
  }


// Pressed MIDDLE button in slider
long FXScrollbar::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  register FXint p=pos;
  register int travel,lo,hi,t;
  if(isEnabled()){
    grab();
    if(timer) timer=getApp()->removeTimeout(timer);
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    pressed=PRESSED_THUMB;
    flags|=FLAG_PRESSED;
    flags&=~FLAG_UPDATE;
    dragpoint=thumbsize/2;
    if(options&SCROLLBAR_HORIZONTAL){
      travel=width-height-height-thumbsize;
      t=event->win_x-dragpoint;
      if(t<height) t=height;
      if(t>(width-height-thumbsize)) t=width-height-thumbsize;
      if(t!=thumbpos){
        FXMINMAX(lo,hi,t,thumbpos);
        update(lo,0,hi+thumbsize-lo,height);
        thumbpos=t;
        }
      if(travel>0){ p=(((FXdouble)(thumbpos-height))*(range-page))/travel; } else { p=0; }
      }
    else{
      travel=height-width-width-thumbsize;
      t=event->win_y-dragpoint;
      if(t<width) t=width;
      if(t>(height-width-thumbsize)) t=height-width-thumbsize;
      if(t!=thumbpos){
        FXMINMAX(lo,hi,t,thumbpos);
        update(0,lo,width,hi+thumbsize-lo);
        thumbpos=t;
        }
      if(travel>0){ p=(((FXdouble)(thumbpos-width))*(range-page))/travel; } else { p=0; }
      }
    if(p<0) p=0;
    if(p>(range-page)) p=range-page;
    if(pos!=p){
      pos=p;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      }
    return 1;
    }
  return 0;
  }


// Released MIDDLE button
long FXScrollbar::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    dragpoint=0;
    pressed=0;
    setPosition(pos);
    update();
    if(timer){ timer=getApp()->removeTimeout(timer); }
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      }
    return 1;
    }
  return 0;
  }


// Pressed RIGHT button in slider
long FXScrollbar::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  register FXEvent *event=(FXEvent*)ptr;
  register FXint p=pos;
  if(isEnabled()){
    grab();
    if(timer) timer=getApp()->removeTimeout(timer);
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    flags&=~FLAG_UPDATE;
    if(options&SCROLLBAR_HORIZONTAL){     // Horizontal scrollbar
      if(event->win_x<height){                   // Left arrow
        pressed=PRESSED_DEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_PIX);
        p=pos-1;
        update();
        }
      else if(width-height<=event->win_x){       // Right arrow
        pressed=PRESSED_INC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_PIX);
        p=pos+1;
        update();
        }
      else if(event->win_x<thumbpos){             // Page left
        pressed=PRESSED_PAGEDEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_LINE);
        p=pos-line;
        update();
        }
      else if(thumbpos+thumbsize<=event->win_x){  // Page right
        pressed=PRESSED_PAGEINC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_LINE);
        p=pos+line;
        update();
        }
      else{                                       // Grabbed the puck
        pressed=PRESSED_FINETHUMB;
        dragpoint=event->win_x;
        flags|=FLAG_PRESSED;
        }
      }
    else{                                 // Vertical scrollbar
      if(event->win_y<width){                   // Up arrow
        pressed=PRESSED_DEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_PIX);
        p=pos-1;
        update();
        }
      else if(height-width<=event->win_y){      // Down arrow
        pressed=PRESSED_INC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_PIX);
        p=pos+1;
        update();
        }
      else if(event->win_y<thumbpos){             // Page up
        pressed=PRESSED_PAGEDEC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC_LINE);
        p=pos-line;
        update();
        }
      else if(thumbpos+thumbsize<=event->win_y){  // Page down
        pressed=PRESSED_PAGEINC;
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC_LINE);
        p=pos+line;
        update();
        }
      else{                                       // Grabbed the puck
        pressed=PRESSED_FINETHUMB;
        flags|=FLAG_PRESSED;
        }
      }
    if(p<0) p=0;
    if(p>(range-page)) p=range-page;
    if(p!=pos){
      setPosition(p);
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      }
    return 1;
    }
  return 0;
  }


// Released RIGHT button
long FXScrollbar::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    dragpoint=0;
    pressed=0;
    setPosition(pos);
    update();
    if(timer){ timer=getApp()->removeTimeout(timer); }
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      }
    return 1;
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXScrollbar::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onUngrabbed(sender,sel,ptr);
  if(timer){ timer=getApp()->removeTimeout(timer); }
  flags&=~FLAG_PRESSED;
  flags&=~FLAG_CHANGED;
  flags|=FLAG_UPDATE;
  dragpoint=0;
  pressed=0;
  return 1;
  }


// Moving
long FXScrollbar::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint travel,hi,lo,t,p;
  if(!isEnabled()) return 0;
  if(flags&FLAG_PRESSED){
    p=0;
    if(event->state&(CONTROLMASK|SHIFTMASK|ALTMASK)) pressed=PRESSED_FINETHUMB;
    if(pressed==PRESSED_THUMB){             // Coarse movements
      if(options&SCROLLBAR_HORIZONTAL){
        travel=width-height-height-thumbsize;
        t=event->win_x-dragpoint;
        if(t<height) t=height;
        if(t>(width-height-thumbsize)) t=width-height-thumbsize;
        if(t!=thumbpos){
          FXMINMAX(lo,hi,t,thumbpos);
          update(lo,0,hi+thumbsize-lo,height);
          thumbpos=t;
          }
        if(travel>0){ p=(((FXdouble)(thumbpos-height))*(range-page)+travel/2)/travel; }
        }
      else{
        travel=height-width-width-thumbsize;
        t=event->win_y-dragpoint;
        if(t<width) t=width;
        if(t>(height-width-thumbsize)) t=height-width-thumbsize;
        if(t!=thumbpos){
          FXMINMAX(lo,hi,t,thumbpos);
          update(0,lo,width,hi+thumbsize-lo);
          thumbpos=t;
          }
        if(travel>0){ p=(((FXdouble)(thumbpos-width))*(range-page)+travel/2)/travel; }
        }
      }
    else if(pressed==PRESSED_FINETHUMB){    // Fine movements
      if(options&SCROLLBAR_HORIZONTAL){
        travel=width-height-height-thumbsize;
        p=pos+event->win_x-event->last_x;
        if(p<0) p=0;
        if(p>(range-page)) p=range-page;
        if(range>page){ t=height+(((FXdouble)pos)*travel)/(range-page); } else { t=height; }
        if(t!=thumbpos){
          FXMINMAX(lo,hi,t,thumbpos);
          update(lo,0,hi+thumbsize-lo,height);
          thumbpos=t;
          }
        }
      else{
        travel=height-width-width-thumbsize;
        p=pos+event->win_y-event->last_y;
        if(p<0) p=0;
        if(p>(range-page)) p=range-page;
        if(range>page){ t=width+(((FXdouble)pos)*travel)/(range-page); } else { t=width; }
        if(t!=thumbpos){
          FXMINMAX(lo,hi,t,thumbpos);
          update(0,lo,width,hi+thumbsize-lo);
          thumbpos=t;
          }
        }
      }
    if(p<0) p=0;
    if(p>(range-page)) p=range-page;
    if(pos!=p){
      pos=p;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      return 1;
      }
    }
  return 0;
  }


// Mouse wheel
long FXScrollbar::onMouseWheel(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint jump;
  if(isEnabled()){
    if(timer) timer=getApp()->removeTimeout(timer);
    if(!(ev->state&(LEFTBUTTONMASK|MIDDLEBUTTONMASK|RIGHTBUTTONMASK))){
      if(ev->state&ALTMASK) jump=line;                      // Fine scrolling
      else if(ev->state&CONTROLMASK) jump=page;             // Coarse scrolling
      else jump=FXMIN(page,getApp()->getWheelLines()*line); // Normal scrolling
      if(dragpoint==0) dragpoint=pos;                       // Were not scrolling already?
      dragpoint-=ev->code*jump/120;                         // Move scroll position
      if(dragpoint<0) dragpoint=0;
      if(dragpoint>(range-page)) dragpoint=range-page;
      if(dragpoint!=pos){
        dragjump=(dragpoint-pos);
        if(FXABS(dragjump)>16) dragjump/=16;
        timer=getApp()->addTimeout(5,this,ID_TIMEWHEEL);
        }
      return 1;
      }
    }
  return 0;
  }


// Smoothly scroll to desired value as determined by wheel
long FXScrollbar::onTimeWheel(FXObject*,FXSelector,void*){
  register FXint p;
  timer=NULL;
  if(dragpoint<pos){
    p=pos+dragjump;
    if(p<=dragpoint){
      setPosition(dragpoint);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      dragpoint=0;
      }
    else{
      setPosition(p);
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      timer=getApp()->addTimeout(5,this,ID_TIMEWHEEL);
      }
    }
  else{
    p=pos+dragjump;
    if(p>=dragpoint){
      setPosition(dragpoint);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      dragpoint=0;
      }
    else{
      setPosition(p);
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      timer=getApp()->addTimeout(5,this,ID_TIMEWHEEL);
      }
    }
  return 1;
  }


// Increment pixel timeout
long FXScrollbar::onTimeIncPix(FXObject*,FXSelector,void*){
  FXint p=pos+1;
  if(p>=(range-page)){
    p=range-page;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTOINC_PIX);
    }
  if(p!=pos){
    setPosition(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Increment line timeout
long FXScrollbar::onTimeIncLine(FXObject*,FXSelector,void*){
  FXint p=pos+line;
  if(p>=(range-page)){
    p=range-page;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTOINC_LINE);
    }
  if(p!=pos){
    setPosition(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Increment page timeout
long FXScrollbar::onTimeIncPage(FXObject*,FXSelector,void*){
  FXint p=pos+page;
  if(p>=(range-page)){
    p=range-page;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTOINC_PAGE);
    }
  if(p!=pos){
    setPosition(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Decrement pixel timeout
long FXScrollbar::onTimeDecPix(FXObject*,FXSelector,void*){
  FXint p=pos-1;
  if(p<=0){
    p=0;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTODEC_PIX);
    }
  if(p!=pos){
    setPosition(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Decrement line timeout
long FXScrollbar::onTimeDecLine(FXObject*,FXSelector,void*){
  FXint p=pos-line;
  if(p<=0){
    p=0;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTODEC_LINE);
    }
  if(p!=pos){
    setPosition(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Decrement page timeout
long FXScrollbar::onTimeDecPage(FXObject*,FXSelector,void*){
  FXint p=pos-page;
  if(p<=0){
    p=0;
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTODEC_PAGE);
    }
  if(p!=pos){
    setPosition(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Draw button in scrollbar; this is slightly different from a raised rectangle
void FXScrollbar::drawButton(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down){
  dc.setForeground(backColor);
  dc.fillRectangle(x+2,y+2,w-4,h-4);
  if(!down){
    dc.setForeground(backColor);
    dc.fillRectangle(x,y,w-1,1);
    dc.fillRectangle(x,y,1,h-1);
    dc.setForeground(hiliteColor);
    dc.fillRectangle(x+1,y+1,w-2,1);
    dc.fillRectangle(x+1,y+1,1,h-2);
    dc.setForeground(shadowColor);
    dc.fillRectangle(x+1,y+h-2,w-2,1);
    dc.fillRectangle(x+w-2,y+1,1,h-2);
    dc.setForeground(borderColor);
    dc.fillRectangle(x,y+h-1,w,1);
    dc.fillRectangle(x+w-1,y,1,h);
    }
  else{
    dc.setForeground(borderColor);
    dc.fillRectangle(x,y,w-2,1);
    dc.fillRectangle(x,y,1,h-2);
    dc.setForeground(shadowColor);
    dc.fillRectangle(x+1,y+1,w-3,1);
    dc.fillRectangle(x+1,y+1,1,h-3);
    dc.setForeground(hiliteColor);
    dc.fillRectangle(x,y+h-1,w-1,1);
    dc.fillRectangle(x+w-1,y+1,1,h-1);
    dc.setForeground(backColor);
    dc.fillRectangle(x+1,y+h-2,w-1,1);
    dc.fillRectangle(x+w-2,y+2,1,h-2);
    }
  }


// Draw left arrow
void FXScrollbar::drawLeftArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down){
  FXPoint points[3];
  FXint ah,ab;
  ab=(h-7)|1;
  ah=ab>>1;
  x=x+((w-ah)>>1);
  y=y+((h-ab)>>1);
  if(down){ ++x; ++y; }
  points[0].x=x+ah;
  points[0].y=y;
  points[1].x=x+ah;
  points[1].y=y+ab-1;
  points[2].x=x;
  points[2].y=y+(ab>>1);
  dc.setForeground(borderColor);
  dc.fillPolygon(points,3);
  }


// Draw right arrow
void FXScrollbar::drawRightArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down){
  FXPoint points[3];
  FXint ah,ab;
  ab=(h-7)|1;
  ah=ab>>1;
  x=x+((w-ah)>>1);
  y=y+((h-ab)>>1);
  if(down){ ++x; ++y; }
  points[0].x=x;
  points[0].y=y;
  points[1].x=x;
  points[1].y=y+ab-1;
  points[2].x=x+ah;
  points[2].y=y+(ab>>1);
  dc.setForeground(borderColor);
  dc.fillPolygon(points,3);
  }


// Draw up arrow
void FXScrollbar::drawUpArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down){
  FXPoint points[3];
  FXint ah,ab;
  ab=(w-7)|1;
  ah=ab>>1;
  x=x+((w-ab)>>1);
  y=y+((h-ah)>>1);
  if(down){ ++x; ++y; }
  points[0].x=x+(ab>>1);
  points[0].y=y-1;
  points[1].x=x;
  points[1].y=y+ah;
  points[2].x=x+ab;
  points[2].y=y+ah;
  dc.setForeground(borderColor);
  dc.fillPolygon(points,3);
  }


// Draw down arrow
void FXScrollbar::drawDownArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down){
  FXPoint points[3];
  FXint ah,ab;
  ab=(w-7)|1;
  ah=ab>>1;
  x=x+((w-ab)>>1);
  y=y+((h-ah)>>1);
  if(down){ ++x; ++y; }
  points[0].x=x+1;
  points[0].y=y;
  points[1].x=x+ab-1;
  points[1].y=y;
  points[2].x=x+(ab>>1);
  points[2].y=y+ah;
  dc.setForeground(borderColor);
  dc.fillPolygon(points,3);
  }


// Handle repaint
long FXScrollbar::onPaint(FXObject*,FXSelector,void* ptr){
  register FXEvent *ev=(FXEvent*)ptr;
  register int total;
  FXDCWindow dc(this,ev);
  if(options&SCROLLBAR_HORIZONTAL){
    total=width-height-height;
    if(thumbsize<total){                                    // Scrollable
      drawButton(dc,thumbpos,0,thumbsize,height,0);
      dc.setStipple(STIPPLE_GRAY);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      if(pressed&PRESSED_PAGEDEC){
        dc.setForeground(backColor);
        dc.setBackground(shadowColor);
        }
      else{
        dc.setForeground(hiliteColor);
        dc.setBackground(backColor);
        }
      dc.fillRectangle(height,0,thumbpos-height,height);
      if(pressed&PRESSED_PAGEINC){
        dc.setForeground(backColor);
        dc.setBackground(shadowColor);
        }
      else{
        dc.setForeground(hiliteColor);
        dc.setBackground(backColor);
        }
      dc.fillRectangle(thumbpos+thumbsize,0,width-height-thumbpos-thumbsize,height);
      }
    else{                                                   // Non-scrollable
      dc.setStipple(STIPPLE_GRAY);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      dc.setForeground(hiliteColor);
      dc.setBackground(backColor);
      dc.fillRectangle(height,0,total,height);
      }
    dc.setFillStyle(FILL_SOLID);
    drawButton(dc,width-height,0,height,height,(pressed&PRESSED_INC));
    drawRightArrow(dc,width-height,0,height,height,(pressed&PRESSED_INC));
    drawButton(dc,0,0,height,height,(pressed&PRESSED_DEC));
    drawLeftArrow(dc,0,0,height,height,(pressed&PRESSED_DEC));
    }
  else{
    total=height-width-width;
    if(thumbsize<total){                                    // Scrollable
      drawButton(dc,0,thumbpos,width,thumbsize,0);
      dc.setStipple(STIPPLE_GRAY);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      if(pressed&PRESSED_PAGEDEC){
        dc.setForeground(backColor);
        dc.setBackground(shadowColor);
        }
      else{
        dc.setForeground(hiliteColor);
        dc.setBackground(backColor);
        }
      dc.fillRectangle(0,width,width,thumbpos-width);
      if(pressed&PRESSED_PAGEINC){
        dc.setForeground(backColor);
        dc.setBackground(shadowColor);
        }
      else{
        dc.setForeground(hiliteColor);
        dc.setBackground(backColor);
        }
      dc.fillRectangle(0,thumbpos+thumbsize,width,height-width-thumbpos-thumbsize);
      }
    else{                                                   // Non-scrollable
      dc.setStipple(STIPPLE_GRAY);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      dc.setForeground(hiliteColor);
      dc.setBackground(backColor);
      dc.fillRectangle(0,width,width,total);
      }
    dc.setFillStyle(FILL_SOLID);
    drawButton(dc,0,height-width,width,width,(pressed&PRESSED_INC));
    drawDownArrow(dc,0,height-width,width,width,(pressed&PRESSED_INC));
    drawButton(dc,0,0,width,width,(pressed&PRESSED_DEC));
    drawUpArrow(dc,0,0,width,width,(pressed&PRESSED_DEC));
    }
  return 1;
  }


// Set range
void FXScrollbar::setRange(FXint r){
  if(r<1) r=1;
  if(range!=r){
    range=r;
    setPage(page);
    }
  }


// Set page size
void FXScrollbar::setPage(FXint p){
  if(p<1) p=1;
  if(p>range) p=range;
  if(page!=p){
    page=p;
    setPosition(pos);
    }
  }


// Set line size
void FXScrollbar::setLine(FXint l){
  if(l<1) l=1;
  line=l;
  }


// Set position; tricky because the thumb size may have changed
// as well; we do the minimal possible update to repaint properly.
void FXScrollbar::setPosition(FXint p){
  FXint total,travel,lo,hi,l,h;
  pos=p;
  if(pos<0) pos=0;
  if(pos>(range-page)) pos=range-page;
  lo=thumbpos;
  hi=thumbpos+thumbsize;
  if(options&SCROLLBAR_HORIZONTAL){
    total=width-height-height;
    thumbsize=(total*page)/range;
    if(thumbsize<THUMB_MINIMUM) thumbsize=THUMB_MINIMUM;
    travel=total-thumbsize;
    if(range>page){ thumbpos=height+(((FXdouble)pos)*travel)/(range-page); } else { thumbpos=height; }
    l=thumbpos;
    h=thumbpos+thumbsize;
    if(l!=lo || h!=hi){
      update(FXMIN(l,lo),0,FXMAX(h,hi)-FXMIN(l,lo),height);
      }
    }
  else{
    total=height-width-width;
    thumbsize=(total*page)/range;
    if(thumbsize<THUMB_MINIMUM) thumbsize=THUMB_MINIMUM;
    travel=total-thumbsize;
    if(range>page){ thumbpos=width+(((FXdouble)pos)*travel)/(range-page); } else { thumbpos=width; }
    l=thumbpos;
    h=thumbpos+thumbsize;
    if(l!=lo || h!=hi){
      update(0,FXMIN(l,lo),width,FXMAX(h,hi)-FXMIN(l,lo));
      }
    }
  }


// Set highlight color
void FXScrollbar::setHiliteColor(FXColor clr){
  if(hiliteColor!=clr){
    hiliteColor=clr;
    update();
    }
  }


// Set shadow color
void FXScrollbar::setShadowColor(FXColor clr){
  if(shadowColor!=clr){
    shadowColor=clr;
    update();
    }
  }


// Set border color
void FXScrollbar::setBorderColor(FXColor clr){
  if(borderColor!=clr){
    borderColor=clr;
    update();
    }
  }


// Change the scrollbar style
FXuint FXScrollbar::getScrollbarStyle() const {
  return (options&SCROLLBAR_MASK);
  }


// Get the current scrollbar style
void FXScrollbar::setScrollbarStyle(FXuint style){
  FXuint opts=(options&~SCROLLBAR_MASK) | (style&SCROLLBAR_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    update();
    }
  }


// Save object to stream
void FXScrollbar::save(FXStream& store) const {
  FXWindow::save(store);
  store << hiliteColor;
  store << shadowColor;
  store << borderColor;
  store << range;
  store << page;
  store << line;
  store << pos;
  }


// Load object from stream
void FXScrollbar::load(FXStream& store){
  FXWindow::load(store);
  store >> hiliteColor;
  store >> shadowColor;
  store >> borderColor;
  store >> range;
  store >> page;
  store >> line;
  store >> pos;
  }


// Delete
FXScrollbar::~FXScrollbar(){
  if(timer){getApp()->removeTimeout(timer);}
  timer=(FXTimer*)-1;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXScrollCorner) FXScrollCornerMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXScrollCorner::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXScrollCorner,FXWindow,FXScrollCornerMap,ARRAYNUMBER(FXScrollCornerMap))


// Deserialization
FXScrollCorner::FXScrollCorner(){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  }


// Construct and init
FXScrollCorner::FXScrollCorner(FXComposite* p):FXWindow(p){
  backColor=getApp()->getBaseColor();
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  }


// Slightly different from Frame border
long FXScrollCorner::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  return 1;
  }


void FXScrollCorner::enable(){ }


void FXScrollCorner::disable(){ }

