/********************************************************************************
*                                                                               *
*                             S l i d e r   O b j e c t s                       *
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
* $Id: FXSlider.cpp,v 1.33.4.2 2003/06/20 19:02:07 fox Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXSlider.h"




/*
  Notes:
  - Add API's to change sizes and appearance.
  - Dual/triple head options.
  - Need to be able to disable/enable, head should show/hide.
  - Sliderhead starts too small for SLIDER_INSIDE_BAR mode.
  - What messages should be sent?
  - Slider should work in double, not int.
  - setValue should not move head unless current headpos does not
    reflect pos (due to resolution issues, several headpos's might
    map to the same pos).
  - If changing pos, you may or may not have to move the head;
    if changing the head, you may or may not get a new pos.
  - In case of a coarse range, we have rounding also.
*/

#define TICKSIZE        4           // Length of ticks
#define OVERHANG        4           // Default amount of overhang
#define MINOVERHANG     3           // Minimal amount of overhang
#define HEADINSIDEBAR   20          // Default for inside bar head size
#define HEADOVERHANGING 9           // Default for overhanging head size

#define SLIDER_MASK (SLIDER_VERTICAL|SLIDER_ARROW_UP|SLIDER_ARROW_DOWN|SLIDER_INSIDE_BAR|SLIDER_TICKS_TOP|SLIDER_TICKS_BOTTOM)

/*******************************************************************************/


// Map
FXDEFMAP(FXSlider) FXSliderMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXSlider::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXSlider::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXSlider::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXSlider::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXSlider::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXSlider::onMiddleBtnRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXSlider::onUngrabbed),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXSlider::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXSlider::onQueryHelp),
  FXMAPFUNC(SEL_TIMEOUT,FXSlider::ID_AUTOINC,FXSlider::onTimeInc),
  FXMAPFUNC(SEL_TIMEOUT,FXSlider::ID_AUTODEC,FXSlider::onTimeDec),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXSlider::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXSlider::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALVALUE,FXSlider::onCmdSetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXSlider::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALVALUE,FXSlider::onCmdGetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTRANGE,FXSlider::onCmdSetIntRange),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTRANGE,FXSlider::onCmdGetIntRange),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALRANGE,FXSlider::onCmdSetRealRange),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALRANGE,FXSlider::onCmdGetRealRange),
  };


// Object implementation
FXIMPLEMENT(FXSlider,FXFrame,FXSliderMap,ARRAYNUMBER(FXSliderMap))


// Make a slider
FXSlider::FXSlider(){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  timer=NULL;
  headpos=0;
  dragpoint=0;
  }


// Make a slider
FXSlider::FXSlider(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  target=tgt;
  message=sel;
  timer=NULL;
  range[0]=0;
  range[1]=100;
  pos=50;
  incr=1;
  delta=0;
  slotColor=getApp()->getBackColor();
  baseColor=getApp()->getBaseColor();
  hiliteColor=getApp()->getHiliteColor();
  shadowColor=getApp()->getShadowColor();
  borderColor=getApp()->getBorderColor();
  headpos=0;
  dragpoint=0;
  headsize=(options&SLIDER_INSIDE_BAR)?HEADINSIDEBAR:HEADOVERHANGING;
  slotsize=5;
  }


// Enable the window
void FXSlider::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXFrame::enable();
    update();
    }
  }


// Disable the window
void FXSlider::disable(){
  if(flags&FLAG_ENABLED){
    FXFrame::disable();
    update();
    }
  }


// Return head position from value
FXint FXSlider::headPos(FXint v) const {
  register FXint travel,rr,p;
  if(v<range[0]) v=range[0];
  if(v>range[1]) v=range[1];
  rr=range[1]-range[0];
  if(options&SLIDER_VERTICAL){
    p=border+padtop+2;
    travel=height-(border<<1)-padtop-padbottom-headsize-4;
    if(rr>0) p+=(travel*(range[1]-v))/rr;
    }
  else{
    p=border+padleft+2;
    travel=width-(border<<1)-padleft-padright-headsize-4;
    if(rr>0) p+=(travel*(v-range[0]))/rr;
    }
  return p;
  }


// Return value from head position
FXint FXSlider::headVal(FXint p) const {
  register FXint travel,rr,v;
  rr=range[1]-range[0];
  if(options&SLIDER_VERTICAL){
    v=range[0];
    travel=height-(border<<1)-padtop-padbottom-headsize-4;
    if(travel>0) v+=(rr*(border+padtop+2+travel-p)+travel/2)/travel;
    }
  else{
    v=range[0];
    travel=width-(border<<1)-padleft-padright-headsize-4;
    if(travel>0) v+=(rr*(p-border-padleft-2)+travel/2)/travel;
    }
  if(v<range[0]) v=range[0];
  if(v>range[1]) v=range[1];
  return v;
  }


// Get default size
FXint FXSlider::getDefaultWidth(){
  FXint w;
  if(options&SLIDER_VERTICAL){
    if(options&SLIDER_INSIDE_BAR) w=4+headsize/2;
    else if(options&(SLIDER_ARROW_LEFT|SLIDER_ARROW_RIGHT)) w=slotsize+MINOVERHANG*2+headsize/2;
    else w=slotsize+MINOVERHANG*2;
    if(options&SLIDER_TICKS_LEFT) w+=TICKSIZE;
    if(options&SLIDER_TICKS_RIGHT) w+=TICKSIZE;
    }
  else{
    w=headsize+4;
    }
  return w+padleft+padright+(border<<1);
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
    if(options&SLIDER_TICKS_TOP) h+=TICKSIZE;
    if(options&SLIDER_TICKS_BOTTOM) h+=TICKSIZE;
    }
  return h+padtop+padbottom+(border<<1);
  }


// Layout changed; even though the position is still
// the same, the head may have to be moved.
void FXSlider::layout(){
  setValue(pos);
  flags&=~FLAG_DIRTY;
  }


// We were asked about status text
long FXSlider::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(!help.empty() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXSlider::onQueryTip(FXObject* sender,FXSelector,void*){
  if(!tip.empty() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&tip);
    return 1;
    }
  return 0;
  }


// Update value from a message
long FXSlider::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setValue((FXint)(FXival)ptr);
  return 1;
  }


// Update value from a message
long FXSlider::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  setValue(*((FXint*)ptr));
  return 1;
  }


// Update value from a message
long FXSlider::onCmdSetRealValue(FXObject*,FXSelector,void* ptr){
  setValue((FXint)*((FXdouble*)ptr));
  return 1;
  }


// Obtain value from text field
long FXSlider::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  *((FXint*)ptr)=getValue();
  return 1;
  }


// Obtain value with a message
long FXSlider::onCmdGetRealValue(FXObject*,FXSelector,void* ptr){
  *((FXdouble*)ptr)=(FXdouble)getValue();
  return 1;
  }


// Update range from a message
long FXSlider::onCmdSetIntRange(FXObject*,FXSelector,void* ptr){
  setRange(((FXint*)ptr)[0],((FXint*)ptr)[1]);
  return 1;
  }


// Get range with a message
long FXSlider::onCmdGetIntRange(FXObject*,FXSelector,void* ptr){
  getRange(((FXint*)ptr)[0],((FXint*)ptr)[1]);
  return 1;
  }


// Update range from a message
long FXSlider::onCmdSetRealRange(FXObject*,FXSelector,void* ptr){
  setRange((FXint) ((FXdouble*)ptr)[0],(FXint) ((FXdouble*)ptr)[1]);
  return 1;
  }


// Get range with a message
long FXSlider::onCmdGetRealRange(FXObject*,FXSelector,void* ptr){
  ((FXdouble*)ptr)[0]=(FXdouble)range[0];
  ((FXdouble*)ptr)[1]=(FXdouble)range[1];
  return 1;
  }


// Pressed LEFT button
long FXSlider::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  register FXEvent *event=(FXEvent*)ptr;
  register FXint p=pos;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    grab();
    if(timer) timer=getApp()->removeTimeout(timer);
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(options&SLIDER_VERTICAL){
      if(event->win_y<headpos){
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC);
        p=pos+incr;
        }
      else if(event->win_y>(headpos+headsize)){
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC);
        p=pos-incr;
        }
      else{
        dragpoint=event->win_y-headpos;
        flags|=FLAG_PRESSED;
        }
      }
    else{
      if(event->win_x<headpos){
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTODEC);
        p=pos-incr;
        }
      else if(event->win_x>(headpos+headsize)){
        timer=getApp()->addTimeout(getApp()->getScrollDelay(),this,ID_AUTOINC);
        p=pos+incr;
        }
      else{
        dragpoint=event->win_x-headpos;
        flags|=FLAG_PRESSED;
        }
      }
    if(p<range[0]) p=range[0];
    if(p>range[1]) p=range[1];
    if(p!=pos){
      setValue(p);
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      }
    flags&=~FLAG_UPDATE;
    return 1;
    }
  return 0;
  }


// Released Left button
long FXSlider::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    if(timer) timer=getApp()->removeTimeout(timer);
    setValue(pos);                                                 // Hop to exact position
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      }
    return 1;
    }
  return 0;
  }


// Moving
long FXSlider::onMotion(FXObject*,FXSelector,void* ptr){
  register FXEvent *event=(FXEvent*)ptr;
  register FXint xx,yy,ww,hh,lo,hi,p,h,travel;
  if(!isEnabled()) return 0;
  if(flags&FLAG_PRESSED){
    yy=border+padtop+2;
    xx=border+padleft+2;
    hh=height-(border<<1)-padtop-padbottom-4;
    ww=width-(border<<1)-padleft-padright-4;
    if(options&SLIDER_VERTICAL){
      h=event->win_y-dragpoint;
      travel=hh-headsize;
      if(h<yy) h=yy;
      if(h>yy+travel) h=yy+travel;
      if(h!=headpos){
        FXMINMAX(lo,hi,headpos,h);
        headpos=h;
        update(border,lo-1,width-(border<<1),hi+headsize+2-lo);
        }
      if(travel>0)
        p=range[0]+((range[1]-range[0])*(yy+travel-h)+travel/2)/travel;    // Use rounding!!
      else
        p=range[0];
      }
    else{
      h=event->win_x-dragpoint;
      travel=ww-headsize;
      if(h<xx) h=xx;
      if(h>xx+travel) h=xx+travel;
      if(h!=headpos){
        FXMINMAX(lo,hi,headpos,h);
        headpos=h;
        update(lo-1,border,hi+headsize+2-lo,height-(border<<1));
        }
      if(travel>0)
        p=range[0]+((range[1]-range[0])*(h-xx)+travel/2)/travel;    // Use rounding!!
      else
        p=range[0];
      }
    if(p<range[0]) p=range[0];
    if(p>range[1]) p=range[1];
    if(pos!=p){
      pos=p;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      }
    return 1;
    }
  return 0;
  }


// Pressed middle or right
long FXSlider::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  register FXEvent *event=(FXEvent*)ptr;
  register FXint xx,yy,ww,hh,lo,hi,p,h,travel;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    dragpoint=headsize/2;
    yy=border+padtop+2;
    xx=border+padleft+2;
    hh=height-(border<<1)-padtop-padbottom-4;
    ww=width-(border<<1)-padleft-padright-4;
    if(options&SLIDER_VERTICAL){
      h=event->win_y-dragpoint;
      travel=hh-headsize;
      if(h<yy) h=yy;
      if(h>yy+travel) h=yy+travel;
      if(h!=headpos){
        FXMINMAX(lo,hi,headpos,h);
        headpos=h;
        update(border,lo-1,width-(border<<1),hi+headsize+2-lo);
        }
      if(travel>0)
        p=range[0]+((range[1]-range[0])*(yy+travel-h)+travel/2)/travel;    // Use rounding!!
      else
        p=range[0];
      }
    else{
      h=event->win_x-dragpoint;
      travel=ww-headsize;
      if(h<xx) h=xx;
      if(h>xx+travel) h=xx+travel;
      if(h!=headpos){
        FXMINMAX(lo,hi,headpos,h);
        headpos=h;
        update(lo-1,border,hi+headsize+2-lo,height-(border<<1));
        }
      if(travel>0)
        p=range[0]+((range[1]-range[0])*(h-xx)+travel/2)/travel;    // Use rounding!!
      else
        p=range[0];
      }
    if(p<range[0]) p=range[0];
    if(p>range[1]) p=range[1];
    if(p!=pos){
      pos=p;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
      flags|=FLAG_CHANGED;
      }
    flags|=FLAG_PRESSED;
    flags&=~FLAG_UPDATE;
    return 1;
    }
  return 0;
  }


// Released middle button
long FXSlider::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint flgs=flags;
  if(isEnabled()){
    ungrab();
    if(timer) timer=getApp()->removeTimeout(timer);
    flags&=~FLAG_PRESSED;
    flags&=~FLAG_CHANGED;
    flags|=FLAG_UPDATE;
    setValue(pos);                                                 // Hop to exact position
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    if(flgs&FLAG_CHANGED){
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(FXival)pos);
      }
    return 1;
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXSlider::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onUngrabbed(sender,sel,ptr);
  if(timer) timer=getApp()->removeTimeout(timer);
  flags&=~FLAG_PRESSED;
  flags&=~FLAG_CHANGED;
  flags|=FLAG_UPDATE;
  return 1;
  }


// Increment line timeout
long FXSlider::onTimeInc(FXObject*,FXSelector,void*){
  FXint p=pos+incr;
  if(p>=range[1]){
    p=range[1];
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTOINC);
    }
  if(p!=pos){
    setValue(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Decrement line timeout
long FXSlider::onTimeDec(FXObject*,FXSelector,void*){
  FXint p=pos-incr;
  if(p<=range[0]){
    p=range[0];
    timer=NULL;
    }
  else{
    timer=getApp()->addTimeout(getApp()->getScrollSpeed(),this,ID_AUTODEC);
    }
  if(p!=pos){
    setValue(p);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)pos);
    flags|=FLAG_CHANGED;
    return 1;
    }
  return 0;
  }


// Draw horizontal ticks
void FXSlider::drawHorzTicks(FXDCWindow& dc,FXint,FXint y,FXint,FXint){
  FXint v,d,p;
  if(range[0]<range[1]){
    d=delta;
    if(d==0) d=incr;
    dc.setForeground(FXRGB(0,0,0));
    for(v=range[0]; v<=range[1]; v+=d){
      p=headPos(v)+headsize/2;
      dc.fillRectangle(p,y,1,TICKSIZE);
      }
    }
  }


// Draw vertical ticks
void FXSlider::drawVertTicks(FXDCWindow& dc,FXint x,FXint,FXint,FXint){
  FXint v,d,p;
  if(range[0]<range[1]){
    d=delta;
    if(d==0) d=incr;
    dc.setForeground(FXRGB(0,0,0));
    for(v=range[0]; v<=range[1]; v+=d){
      p=headPos(v)+headsize/2;
      dc.fillRectangle(x,p,TICKSIZE,1);
      }
    }
  }


// Draw slider head
void FXSlider::drawSliderHead(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h){
  FXint m;
  dc.setForeground(backColor);
  dc.fillRectangle(x,y,w,h);
  if(options&SLIDER_VERTICAL){
    m=(h>>1);
    if(options&SLIDER_ARROW_LEFT){
      dc.setForeground(hiliteColor);
      dc.drawLine(x+m,y,x+w-1,y);
      dc.drawLine(x,y+m,x+m,y);
      dc.setForeground(shadowColor);
      dc.drawLine(x+1,y+h-m-1,x+m+1,y+h-1);
      dc.drawLine(x+m,y+h-2,x+w-1,y+h-2);
      dc.drawLine(x+w-2,y+1,x+w-2,y+h-1);
      dc.setForeground(borderColor);
      dc.drawLine(x,y+h-m-1,x+m,y+h-1);
      dc.drawLine(x+w-1,y+h-1,x+w-1,y);
      dc.fillRectangle(x+m,y+h-1,w-m,1);
      }
    else if(options&SLIDER_ARROW_RIGHT){
      dc.setForeground(hiliteColor);
      dc.drawLine(x,y,x+w-m-1,y);
      dc.drawLine(x,y+1,x,y+h-1);
      dc.drawLine(x+w-1,y+m,x+w-m-1,y);
#ifndef WIN32
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-2,y+h-m-1,x+w-m-2,y+h-1);
      dc.drawLine(x+1,y+h-2,x+w-m-1,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w-1,y+h-m-1,x+w-m-1,y+h-1);
      dc.drawLine(x,y+h-1,x+w-m-1,y+h-1);
#else
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-1,y+h-m-2,x+w-m-2,y+h-1);
      dc.drawLine(x+1,y+h-2,x+w-m-1,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w,y+h-m-2,x+w-m-1,y+h-1);
      dc.drawLine(x,y+h-1,x+w-m-1,y+h-1);
#endif
      }
    else if(options&SLIDER_INSIDE_BAR){
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      dc.setForeground(shadowColor);
      dc.drawLine(x+1,y+m-1,x+w-2,y+m-1);
      dc.setForeground(hiliteColor);
      dc.drawLine(x+1,y+m,x+w-2,y+m);
      }
    else{
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      }
    }
  else{
    m=(w>>1);
    if(options&SLIDER_ARROW_UP){
      dc.setForeground(hiliteColor);
      dc.drawLine(x,y+m,x+m,y);
      dc.drawLine(x,y+m,x,y+h-1);
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-1,y+m+1,x+w-m-1,y+1);
      dc.drawLine(x+w-2,y+m+1,x+w-2,y+h-1);
      dc.drawLine(x+1,y+h-2,x+w-2,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w-1,y+m,x+w-m-1,y);
      dc.drawLine(x+w-1,y+m,x+w-1,y+h-1);
      dc.fillRectangle(x,y+h-1,w,1);
      }
    else if(options&SLIDER_ARROW_DOWN){
      dc.setForeground(hiliteColor);
      dc.drawLine(x,y,x+w-1,y);
      dc.drawLine(x,y+1,x,y+h-m-1);
      dc.drawLine(x,y+h-m-1,x+m,y+h-1);
      dc.setForeground(shadowColor);
      dc.drawLine(x+w-2,y+1,x+w-2,y+h-m-1);
      dc.drawLine(x+w-1,y+h-m-2,x+w-m-1,y+h-2);
      dc.setForeground(borderColor);
      dc.drawLine(x+w-1,y+h-m-1,x+w-m-1,y+h-1);
      dc.fillRectangle(x+w-1,y,1,h-m);
      }
    else if(options&SLIDER_INSIDE_BAR){
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      dc.setForeground(shadowColor);
      dc.drawLine(x+m-1,y+1,x+m-1,y+h-2);
      dc.setForeground(hiliteColor);
      dc.drawLine(x+m,y+1,x+m,y+h-1);
      }
    else{
      drawDoubleRaisedRectangle(dc,x,y,w,h);
      }
    }
  }


// Handle repaint
long FXSlider::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXint tx,ty,hhs=headsize/2;
  FXint xx,yy,ww,hh;
  FXDCWindow dc(this,ev);

  // Repaint border
  drawFrame(dc,0,0,width,height);

  // Slot placement
  xx=border+padleft;
  yy=border+padtop;
  ww=width-(border<<1)-padleft-padright;
  hh=height-(border<<1)-padtop-padbottom;
  FXASSERT(range[0]<=pos && pos<=range[1]);

  dc.setForeground(backColor);
  dc.fillRectangle(xx,yy,ww,hh);

  // Draw the slot
  if(options&SLIDER_VERTICAL){

    // Adjust slot placement for tickmarks
    if(options&SLIDER_TICKS_LEFT){ xx+=TICKSIZE; ww-=TICKSIZE; }
    if(options&SLIDER_TICKS_RIGHT){ ww-=TICKSIZE; }

    // Draw slider
    if(options&SLIDER_INSIDE_BAR){
//       dc.setForeground(backColor);
//       dc.fillRectangle(border,border,width-(border<<1),padtop);
//       dc.fillRectangle(border,height-border-padbottom,width-(border<<1),padbottom);
//       dc.fillRectangle(border,border+padtop,padleft,hh);
//       dc.fillRectangle(width-border-padright,border+padtop,padright,hh);
      drawDoubleSunkenRectangle(dc,xx,yy,ww,hh);
      dc.setStipple(STIPPLE_GRAY);
      dc.setForeground(slotColor);
      dc.setBackground(baseColor);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      dc.fillRectangle(xx+2,yy+2,ww-4,hh-4);
      dc.setFillStyle(FILL_SOLID);
      if(options&SLIDER_TICKS_LEFT) drawVertTicks(dc,border+padleft,yy,ww,hh);
      if(options&SLIDER_TICKS_RIGHT) drawVertTicks(dc,width-padright-border-TICKSIZE,yy,ww,hh);
      if(isEnabled()) drawSliderHead(dc,xx+2,headpos,ww-4,headsize);
      }
    else{
      if(options&SLIDER_ARROW_LEFT) tx=xx+hhs+(ww-slotsize-hhs)/2;
      else if(options&SLIDER_ARROW_RIGHT) tx=xx+(ww-slotsize-hhs)/2;
      else tx=xx+(ww-slotsize)/2;
//       dc.setForeground(backColor);
//       dc.fillRectangle(border,border,width-(border<<1),padtop);
//       dc.fillRectangle(border,height-border-padbottom,width-(border<<1),padbottom);
//       dc.fillRectangle(border,border+padtop,tx-border,hh);
//       dc.fillRectangle(tx+slotsize,border+padtop,width-border-tx-slotsize,hh);
      drawDoubleSunkenRectangle(dc,tx,yy,slotsize,hh);
      dc.setForeground(slotColor);
      dc.fillRectangle(tx+2,yy+2,slotsize-4,hh-4);
      if(options&SLIDER_TICKS_LEFT) drawVertTicks(dc,border+padleft,yy,ww,hh);
      if(options&SLIDER_TICKS_RIGHT) drawVertTicks(dc,width-padright-border-TICKSIZE,yy,ww,hh);
      if(isEnabled()) drawSliderHead(dc,xx,headpos,ww,headsize);
      }
    }
  else{

    // Adjust slot placement for tickmarks
    if(options&SLIDER_TICKS_TOP){ yy+=TICKSIZE; hh-=TICKSIZE; }
    if(options&SLIDER_TICKS_BOTTOM){ hh-=TICKSIZE; }

    // Draw slider
    if(options&SLIDER_INSIDE_BAR){
//       dc.setForeground(backColor);
//       dc.fillRectangle(border,border,width-(border<<1),padtop);
//       dc.fillRectangle(border,height-border-padbottom,width-(border<<1),padbottom);
//       dc.fillRectangle(border,border+padtop,padleft,hh);
//       dc.fillRectangle(width-border-padright,border+padtop,padright,hh);
      drawDoubleSunkenRectangle(dc,xx,yy,ww,hh);
      dc.setForeground(slotColor);
      dc.setStipple(STIPPLE_GRAY);
      dc.setForeground(slotColor);
      dc.setBackground(baseColor);
      dc.setFillStyle(FILL_OPAQUESTIPPLED);
      dc.fillRectangle(xx+2,yy+2,ww-4,hh-4);
      dc.setFillStyle(FILL_SOLID);
      if(options&SLIDER_TICKS_TOP) drawHorzTicks(dc,xx,border+padtop,ww,hh);
      if(options&SLIDER_TICKS_BOTTOM) drawHorzTicks(dc,xx,height-border-padbottom-TICKSIZE,ww,hh);
      if(isEnabled()) drawSliderHead(dc,headpos,yy+2,headsize,hh-4);
      }
    else{
      if(options&SLIDER_ARROW_UP) ty=yy+hhs+(hh-slotsize-hhs)/2;
      else if(options&SLIDER_ARROW_DOWN) ty=yy+(hh-slotsize-hhs)/2;
      else ty=yy+(hh-slotsize)/2;
//       dc.setForeground(backColor);
//       dc.fillRectangle(border,border,width-(border<<1),ty-border);
//       dc.fillRectangle(border,ty+slotsize,width-(border<<1),height-border-ty-slotsize);
//       dc.fillRectangle(border,border+padtop,padleft,hh);
//       dc.fillRectangle(width-border-padright,border+padtop,padright,hh);
      drawDoubleSunkenRectangle(dc,xx,ty,ww,slotsize);
      dc.setForeground(slotColor);
      dc.fillRectangle(xx+2,ty+2,ww-4,slotsize-4);
      if(options&SLIDER_TICKS_TOP) drawHorzTicks(dc,xx,border+padtop,ww,hh);
      if(options&SLIDER_TICKS_BOTTOM) drawHorzTicks(dc,xx,height-border-padbottom-TICKSIZE,ww,hh);
      if(isEnabled()) drawSliderHead(dc,headpos,yy,headsize,hh);
      }
    }
  return 1;
  }


// Set slider range; this also revalidates the position,
// and possibly moves the head [even if the position was still
// OK, the head might still have to be moved to the exact position].
void FXSlider::setRange(FXint lo,FXint hi){
  if(lo>hi){ fxerror("%s::setRange: trying to set negative range.\n",getClassName()); }
  if(range[0]!=lo || range[1]!=hi){
    range[0]=lo;
    range[1]=hi;
    setValue(pos);
    }
  }


// Set position; this should always cause the head to reflect
// the exact [discrete] value representing pos, even if several
// head positions may represent the same position!
// Also, the minimal amount is repainted, as one sometimes as very
// large/wide sliders.
void FXSlider::setValue(FXint p){
  FXint travel,lo,hi,h;
  if(p<range[0]) p=range[0];
  if(p>range[1]) p=range[1];
  if(options&SLIDER_VERTICAL){
    travel=height-(border<<1)-padtop-padbottom-headsize-4;
    if(range[1]>range[0])
      h=border+padtop+2+(travel*(range[1]-p))/(range[1]-range[0]);
    else
      h=border+padtop+2;
    if(h!=headpos){
      FXMINMAX(lo,hi,headpos,h);
      headpos=h;
      update(border,lo-1,width-(border<<1),hi+headsize+2-lo);
      }
    }
  else{
    travel=width-(border<<1)-padleft-padright-headsize-4;
    if(range[1]>range[0])
      h=border+padleft+2+(travel*(p-range[0]))/(range[1]-range[0]);
    else
      h=border+padleft+2;
    if(h!=headpos){
      FXMINMAX(lo,hi,headpos,h);
      headpos=h;
      update(lo-1,border,hi+headsize+2-lo,height-(border<<1));
      }
    }
  pos=p;
  }


// Get slider options
FXuint FXSlider::getSliderStyle() const {
  return (options&SLIDER_MASK);
  }


// Set slider options
void FXSlider::setSliderStyle(FXuint style){
  FXuint opts=(options&~SLIDER_MASK) | (style&SLIDER_MASK);
  if(options!=opts){
    headsize=(opts&SLIDER_INSIDE_BAR)?HEADINSIDEBAR:HEADOVERHANGING;
    options=opts;
    recalc();
    update();
    }
  }


// Set head size
void FXSlider::setHeadSize(FXint hs){
  if(headsize!=hs){
    headsize=hs;
    recalc();
    update();
    }
  }


// Set slot size
void FXSlider::setSlotSize(FXint bs){
  if(slotsize!=bs){
    slotsize=bs;
    recalc();
    update();
    }
  }


// Set increment
void FXSlider::setIncrement(FXint inc){
  incr=inc;
  }


// Set slot color
void FXSlider::setSlotColor(FXColor clr){
  if(slotColor!=clr){
    slotColor=clr;
    update();
    }
  }


// Change help text
void FXSlider::setHelpText(const FXString& text){
  help=text;
  }


// Change tip text
void FXSlider::setTipText(const FXString& text){
  tip=text;
  }


// Change the delta between ticks
void FXSlider::setTickDelta(FXint dist){
  if(dist<0) dist=0;
  if(delta!=dist){
    delta=dist;
    if(options&(SLIDER_TICKS_TOP|SLIDER_TICKS_BOTTOM)){
      recalc();
      }
    }
  }


// Save object to stream
void FXSlider::save(FXStream& store) const {
  FXFrame::save(store);
  store << range[0] << range[1];
  store << pos;
  store << incr;
  store << delta;
  store << slotColor;
  store << headsize;
  store << slotsize;
  store << help;
  store << tip;
  }


// Load object from stream
void FXSlider::load(FXStream& store){
  FXFrame::load(store);
  store >> range[0] >> range[1];
  store >> pos;
  store >> incr;
  store >> delta;
  store >> slotColor;
  store >> headsize;
  store >> slotsize;
  store >> help;
  store >> tip;
  }


// Delete
FXSlider::~FXSlider(){
  if(timer){getApp()->removeTimeout(timer);}
  timer=(FXTimer*)-1;
  }


