/********************************************************************************
*                                                                               *
*                                D i a l   W i d g e t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* Contributed by: Guoqing Tian                                                  *
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
* $Id: FXDial.cpp,v 1.16 1998/09/18 22:07:14 jvz Exp $                        *
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
#include "FXDial.h"


/*
  Notes:
  - Position decoupled from angle.
  - Add some API's.
  - Properly handle cyclic/non cyclic stuff.
  - Callbacks should report position in the void* ptr.
  - Keep notchangle>=0, as % of negative numbers is implementation defined.
*/

#define DIALWIDTH    12
#define DIALDIAMETER 40


/*******************************************************************************/

// Map
FXDEFMAP(FXDial) FXDialMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXDial::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXDial::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXDial::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXDial::onLeftBtnRelease),
FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXDial::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXDial::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALVALUE,FXDial::onCmdSetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXDial::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALVALUE,FXDial::onCmdGetRealValue),
  };


// Object implementation
FXIMPLEMENT(FXDial,FXCell,FXDialMap,ARRAYNUMBER(FXDialMap))


// Make a window
FXDial::FXDial(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXCell(p,opts,x,y,w,h,pl,pr,pt,pb){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  range[0]=0;
  range[1]=360;
  notchangle=0;
  notchspacing=90;
  notchoffset=0;
  dragpoint=0;
  dragpos=0;
  incr=360;
  pos=0;
  }


// Get minimum width
FXint FXDial::getDefaultWidth(){
  FXint w;
  if(options&DIAL_HORIZONTAL) w=DIALDIAMETER; else w=DIALWIDTH;
  return w+padleft+padright+(border<<1); 
  }


// Get minimum height
FXint FXDial::getDefaultHeight(){ 
  FXint h;
  if(options&DIAL_HORIZONTAL) h=DIALWIDTH; else h=DIALDIAMETER; 
  return h+padtop+padbottom+(border<<1); 
  }


// Can be focused upon
FXbool FXDial::canFocus() const { return 1; }


// Create X window; makes a few colors
void FXDial::create(){
  FXuint rmax,gmax,bmax,r,g,b,i;
  FXCell::create();
  notchColor=acquireColor(FXRGB(255,128,0));
  rmax=(126*FXREDVAL(getApp()->backColor))/100;
  gmax=(126*FXGREENVAL(getApp()->backColor))/100;
  bmax=(126*FXBLUEVAL(getApp()->backColor))/100;
  rmax=FXMIN(rmax,255);
  gmax=FXMIN(gmax,255);
  bmax=FXMIN(bmax,255);
  for(i=0; i<ARRAYNUMBER(sideColor); i++){
    r=(rmax*i)/(ARRAYNUMBER(sideColor)-1);
    g=(gmax*i)/(ARRAYNUMBER(sideColor)-1);
    b=(bmax*i)/(ARRAYNUMBER(sideColor)-1);
    sideColor[i]=acquireColor(FXRGB(r,g,b));
    }
  }


// Update value from a message
long FXDial::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setPosition((FXint)ptr);
  return 1;
  }


// Update value from a message
long FXDial::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setPosition(*((FXint*)ptr)); }
  return 1;
  }


// Update value from a message
long FXDial::onCmdSetRealValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setPosition((FXint) *((FXdouble*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXDial::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXint*)ptr)=getPosition(); return 1; }
  return 0;
  }


// Obtain value from text field
long FXDial::onCmdGetRealValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXdouble*)ptr)=(FXdouble)getPosition(); return 1; }
  return 0;
  }


// Pressed LEFT button
long FXDial::onLeftBtnPress(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  FXWindow::onLeftBtnPress(sender,sel,ptr);
  if(options&DIAL_HORIZONTAL) 
    dragpoint=event->win_x; 
  else 
    dragpoint=event->win_y;
  dragpos=pos;
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  return 1;
  }


// Released LEFT button
long FXDial::onLeftBtnRelease(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onLeftBtnRelease(sender,sel,ptr);
  if(flags&FLAG_CHANGED){
    if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)pos);
    }
  flags&=~FLAG_PRESSED;
  flags&=~FLAG_CHANGED;
  flags|=FLAG_UPDATE;
  return 1;
  }


// Moving 
long FXDial::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint travel,size,delta,newpos,tmp;
  if(flags&FLAG_PRESSED){
    if(options&DIAL_HORIZONTAL){
      size=width-(border<<1);
      travel=event->win_x-dragpoint;
      }
    else{
      size=height-(border<<1);
      travel=dragpoint-event->win_y;
      }
    if(travel){
      delta=(incr*travel)/(2*size);
      if(options&DIAL_CYCLIC){
        tmp=dragpos+delta-range[0];
        while(tmp<0) tmp+=(range[1]-range[0]+1);
        newpos=range[0]+tmp%(range[1]-range[0]+1);
        }
      else{
        if(dragpos+delta<range[0]) newpos=range[0];
        else if(dragpos+delta>range[1]) newpos=range[1];
        else newpos=dragpos+delta;
        }
      if(pos!=newpos){
        pos=newpos;
        FXASSERT(range[0]<=pos && pos<=range[1]);
        notchangle=(notchoffset+(3600*(pos-range[0]))/incr)%3600;
//fprintf(stderr,"pos=%d notchangle=%d\n",pos,notchangle);
        update(border+padleft+1,border+padtop+1,width-(border<<1)-padleft-padright-2,height-(border<<1)-padtop-padbottom-2);
        flags|=FLAG_CHANGED;
        if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)pos);
        return 1;
        }
      }
    }
  return 0;
  }



// Handle repaint 
long FXDial::onPaint(FXObject*,FXSelector,void* ptr){
  const FXdouble fac=0.5*PI/((FXdouble)(ARRAYNUMBER(sideColor)-1));
  FXEvent* event=(FXEvent*)ptr;
  FXint i,size,u,d,lu,ld,t,r,fm,to,off,ang;
  FXint lt,rt,tp,bm;
  FXdouble mid,tmp;
  
  setClipRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);

  // Paint background
  clearArea(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  
  FXASSERT(0<=notchangle && notchangle<3600);
  off=(notchangle+3600)%notchspacing;
  fm=off/notchspacing;
  to=(off+1800-notchspacing+1)/notchspacing;
  
  // Rectangle of dial
  lt=border+padleft+1;
  rt=width-border-padright-2;
  tp=border+padtop+1;
  bm=height-border-padbottom-2;
  
  // Horizontal dial
  if(options&DIAL_HORIZONTAL){
    size=rt-lt;
    r=size/2-1;
    mid=0.5*(lt+rt);
    for(i=fm; i<=to; i++){
      ang=i*notchspacing+off;
      t=(FXint)(mid-r*cos(0.1*DTOR*ang));
      if((options&DIAL_HAS_NOTCH) && (ang+3600)%3600==notchangle){
        setForeground(hiliteColor);
        drawLine(t-1,tp,t-1,bm);
        setForeground(notchColor);
        drawLine(t,tp,t,bm);
        drawLine(t+1,tp,t+1,bm);
        setForeground(borderColor);
        drawLine(t+2,tp,t+2,bm);
        }
      else{
        if(ang<200){
          setForeground(shadowColor);
          drawLine(t,tp,t,bm);
          setForeground(borderColor);
          drawLine(t+1,tp,t+1,bm);
          }
        else if(ang<300){
          setForeground(borderColor);
          drawLine(t,tp,t,bm);
          }
        else if(ang<600){
          setForeground(hiliteColor);
          drawLine(t,tp,t,bm);
          setForeground(borderColor);
          drawLine(t+1,tp,t+1,bm);
          }
        else if(ang<1200){
          setForeground(hiliteColor);
          drawLine(t-1,tp,t-1,bm);
          drawLine(t,tp,t,bm);
          setForeground(borderColor);
          drawLine(t+1,tp,t+1,bm);
          }
        else if(ang<1500){
          setForeground(hiliteColor);
          drawLine(t,tp,t,bm);
          setForeground(borderColor);
          drawLine(t+1,tp,t+1,bm);
          }
        else if(ang<1600){
          setForeground(borderColor);
          drawLine(t,tp,t,bm);
          }
        else{
          setForeground(shadowColor);
          drawLine(t,tp,t,bm);
          setForeground(borderColor);
          drawLine(t-1,tp,t-1,bm);
          }
        }
      }
    drawLine(lt,tp,lt,bm);
    drawLine(rt,tp,rt,bm);
    lu=lt;
    ld=rt;
    for(i=0; i<ARRAYNUMBER(sideColor); i++){
      tmp=r*cos(fac*i);
      u=(FXint)(mid-tmp);
      d=(FXint)(mid+tmp);
      setForeground(sideColor[i]);
      drawLine(lu,tp,u,tp);
      drawLine(ld,tp,d,tp);
      drawLine(lu,bm,u,bm);
      drawLine(ld,bm,d,bm);
      lu=u;
      ld=d;
      }
    drawLine(lu,tp,ld,tp);
    drawLine(lu,bm,ld,bm);
    }
  
  // Vertical dial
  else{
    size=bm-tp;
    r=size/2-1;
    mid=0.5*(tp+bm);
    for(i=fm; i<=to; i++){
      ang=i*notchspacing+off;
      t=(FXint)(mid+r*cos(0.1*DTOR*ang));
      if((options&DIAL_HAS_NOTCH) && (ang+3600)%3600==notchangle){
        setForeground(hiliteColor);
        drawLine(lt,t-1,rt,t-1);
        setForeground(notchColor);
        drawLine(lt,t,rt,t);
        drawLine(lt,t+1,rt,t+1);
        setForeground(borderColor);
        drawLine(lt,t+2,rt,t+2);
        }
      else{
        if(ang<200){
          setForeground(borderColor);
          drawLine(lt,t,rt,t);
          setForeground(shadowColor);
          drawLine(lt,t-1,rt,t-1);
          }
        else if(ang<300){
          setForeground(borderColor);
          drawLine(lt,t,rt,t);
          }
        else if(ang<600){
          setForeground(hiliteColor);
          drawLine(lt,t,rt,t);
          setForeground(borderColor);
          drawLine(lt,t+1,rt,t+1);
          }
        else if(ang<1200){
          setForeground(hiliteColor);
          drawLine(lt,t-1,rt,t-1);
          drawLine(lt,t,rt,t);
          setForeground(borderColor);
          drawLine(lt,t+1,rt,t+1);
          }
        else if(ang<1500){
          setForeground(hiliteColor);
          drawLine(lt,t,rt,t);
          setForeground(borderColor);
          drawLine(lt,t+1,rt,t+1);
          }
        else if(ang<1600){
          setForeground(borderColor);
          drawLine(lt,t,rt,t);
          }
        else{
          setForeground(borderColor);
          drawLine(lt,t,rt,t);
          setForeground(shadowColor);
          drawLine(lt,t+1,rt,t+1);
          }
        }
      }
    drawLine(lt,tp,rt,tp);
    drawLine(lt,bm,rt,bm);
    lu=tp;
    ld=bm;
    for(i=0; i<ARRAYNUMBER(sideColor); i++){
      tmp=r*cos(fac*i);
      u=(FXint)(mid-tmp);
      d=(FXint)(mid+tmp);
      setForeground(sideColor[i]);
      drawLine(lt,lu,lt,u);
      drawLine(lt,ld,lt,d);
      drawLine(rt,lu,rt,u);
      drawLine(rt,ld,rt,d);
      lu=u;
      ld=d;
      }
    drawLine(lt,lu,lt,ld);
    drawLine(rt,lu,rt,ld);
    }
  
  // Border
  drawFrame(0,0,width,height);

  // Inner rectangle
  setForeground(shadowColor);
  drawRectangle(lt-1,tp-1,rt-lt+2,bm-tp+2);
  
  clearClipRectangle();
  return 1;
  }


// Set dial range  
void FXDial::setRange(FXint lo,FXint hi){
  if(lo>hi){ fxerror("%s::setRange: trying to set negative range.\n",getClassName()); }
  if(range[0]!=lo || range[1]!=hi){
    range[0]=lo;
    range[1]=hi;
    if(pos<range[0]) pos=range[0];
    if(pos>range[1]) pos=range[1];
    notchangle=(notchoffset+(3600*(pos-range[0]))/incr)%3600;
    update(0,0,width,height);
    }
  }


// Set dial position
void FXDial::setPosition(FXint p){
  if(p<range[0]) p=range[0];
  if(p>range[1]) p=range[1];
  if(p!=pos){
    pos=p;
    notchangle=(notchoffset+(3600*(pos-range[0]))/incr)%3600;
    update(0,0,width,height);
    }
  }


// Change increment, i.e. the amount of pos change per revolution
void FXDial::setRevolutionIncrement(FXint i){
  incr=FXMIN(1,i);
  notchangle=(notchoffset+(3600*(pos-range[0]))/incr)%3600;
  update(0,0,width,height);
  }


// Change notch spacing
void FXDial::setNotchSpacing(FXint spacing){
  if(spacing<1) spacing=1;
  if(spacing>3600) spacing=3600;
  while(3600%spacing) spacing--;    // Should be a divisor of 3600
  if(notchspacing!=spacing){
    notchspacing=spacing;
    update(0,0,width,height);
    }
  }


// Change notch offset
void FXDial::setNotchOffset(FXint offset){
  if(offset>3600) offset=3600;
  if(offset<-3600) offset=-3600;
  offset=(offset+3600)%3600;
  if(offset!=notchoffset){
    notchangle=(notchoffset+(3600*(pos-range[0]))/incr)%3600;
    update(0,0,width,height);
    }
  }


// Get dial options
FXuint FXDial::getDialStyle() const {
  return (options&DIAL_MASK); 
  }


// Set dial options
void FXDial::setDialStyle(FXuint style){
  FXuint opts=(options&~DIAL_MASK) | (style&DIAL_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Save object to stream
void FXDial::save(FXStream& store) const {
  FXCell::save(store);
  store << range[0] << range[1];
  store << notchangle;
  store << notchspacing;
  store << notchoffset;
  store << incr;
  store << pos;
  }


// Load object from stream
void FXDial::load(FXStream& store){
  FXCell::load(store);
  store >> range[0] >> range[1];
  store >> notchangle;
  store >> notchspacing;
  store >> notchoffset;
  store >> incr;
  store >> pos;
  }  

