/********************************************************************************
*                                                                               *
*                         C o l o r W e l l   C l a s s                         *
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
* $Id: FXColorWell.cpp,v 1.30 1998/10/13 22:27:49 jvz Exp $                   *
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
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXLabel.h"
#include "FXColorWell.h"

/*
  To do:
  - Check drop types.
*/

#define WELLSIZE    12              // Minimum well size
#define FOCUSBORDER 3               // Focus border


/*******************************************************************************/


// Name of the drag type
const FXchar FXColorWell::colorDragTypeName[]="application/x-color";


// Color drag type
FXDragType FXColorWell::colorDragType=0;


// Map
FXDEFMAP(FXColorWell) FXColorWellMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXColorWell::onPaint),
  FXMAPFUNC(SEL_FOCUSIN,0,FXColorWell::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXColorWell::onFocusOut),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXColorWell::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXColorWell::onLeftBtnRelease),
  FXMAPFUNC(SEL_MOTION,0,FXColorWell::onMotion),
  FXMAPFUNC(SEL_DND_ENTER,0,FXColorWell::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXColorWell::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXColorWell::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXColorWell::onDNDMotion),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXColorWell::onSelectionRequest),
  };


// Object implementation
FXIMPLEMENT(FXColorWell,FXCell,FXColorWellMap,ARRAYNUMBER(FXColorWellMap))


// Init
FXColorWell::FXColorWell(){
  wellColor[0]=wellColor[1]=0;
  rgba=0;
  }

  
// Make a color well
FXColorWell::FXColorWell(FXComposite* p,FXColor clr,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXCell(p,opts,x,y,w,h,pl,pr,pt,pb){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  wellColor[0]=wellColor[1]=0;
  rgba=clr;
  dragCursor=getApp()->swatchCursor;
  }


// Create X window
void FXColorWell::create(){
  FXCell::create();
  wellColor[0]=rgbaoverwhite(rgba);
  wellColor[1]=rgbaoverblack(rgba);
  if(!colorDragType){colorDragType=getApp()->registerDragType(colorDragTypeName);}
  dropEnable();
  show();
  }


// If window can have focus
FXbool FXColorWell::canFocus() const { return 1; }


// Compute color over black
FXPixel FXColorWell::rgbaoverblack(FXColor clr){
  FXint r,g,b,mul=FXALPHAVAL(clr);
  r=(FXREDVAL(clr)*mul)/255;
  g=(FXGREENVAL(clr)*mul)/255;
  b=(FXBLUEVAL(clr)*mul)/255;
  return acquireColor(FXRGB(r,g,b));
  }


// Compute color over white
FXPixel FXColorWell::rgbaoverwhite(FXColor clr){
  FXint r,g,b,mul=FXALPHAVAL(clr);
  r=((255-mul)*255+FXREDVAL(clr)*mul)/255;
  g=((255-mul)*255+FXGREENVAL(clr)*mul)/255;
  b=((255-mul)*255+FXBLUEVAL(clr)*mul)/255;
  return acquireColor(FXRGB(r,g,b));
  }


// Get default size
FXint FXColorWell::getDefaultWidth(){
  return WELLSIZE+FOCUSBORDER+padleft+padright+4;
  }


FXint FXColorWell::getDefaultHeight(){
  return WELLSIZE+FOCUSBORDER+padtop+padbottom+4;
  }


// Handle repaint 
long FXColorWell::onPaint(FXObject*,FXSelector,void*){
  FXPoint points[3];
  clearArea(0,0,width,padtop+FOCUSBORDER);
  clearArea(0,padtop+FOCUSBORDER,padleft+FOCUSBORDER,height-padtop-padbottom-(FOCUSBORDER<<1));
  clearArea(width-padright-FOCUSBORDER,padtop+FOCUSBORDER,padright+FOCUSBORDER,height-padtop-padbottom-(FOCUSBORDER<<1));
  clearArea(0,height-padbottom-FOCUSBORDER,width,padbottom+FOCUSBORDER);
  setForeground(wellColor[0]);
  points[0].x=points[1].x=padleft+FOCUSBORDER+2; 
  points[2].x=width-padright-FOCUSBORDER-2;
  points[0].y=points[2].y=padtop+FOCUSBORDER+2;
  points[1].y=height-padbottom-FOCUSBORDER-2;
  fillPolygon(points,3);
  setForeground(wellColor[1]);
  points[0].x=padleft+FOCUSBORDER+2; 
  points[1].x=points[2].x=width-padright-FOCUSBORDER-2;
  points[0].y=points[1].y=height-padbottom-FOCUSBORDER-2;
  points[2].y=padtop+FOCUSBORDER+2;
  fillPolygon(points,3);
  drawDoubleSunkenRectangle(padleft+FOCUSBORDER,padtop+FOCUSBORDER,width-padright-padleft-(FOCUSBORDER<<1),height-padbottom-padtop-(FOCUSBORDER<<1));
  if(hasFocus()){
    drawFocusRectangle(padleft,padtop,width-padright-padleft,height-padbottom-padtop);
    }
  return 1;
  }


// Gained focus
long FXColorWell::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXCell::onFocusIn(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }

  
// Lost focus
long FXColorWell::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXCell::onFocusOut(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }


// Handle drag-and-drop enter
long FXColorWell::onDNDEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXCell::onDNDEnter(sender,sel,ptr);
//fprintf(stderr,"%s::onDNDEnter win(%d)\n",getClassName(),xid);
  return 1;
  }

// Handle drag-and-drop leave
long FXColorWell::onDNDLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXCell::onDNDLeave(sender,sel,ptr);
//fprintf(stderr,"%s::onDNDLeave win(%d)\n",getClassName(),xid);
  return 1;
  }


// Handle drag-and-drop motion
long FXColorWell::onDNDMotion(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onDNDMotion win(%d)\n",getClassName(),xid);
  
  // Handle base class first
  if(FXCell::onDNDMotion(sender,sel,ptr)) return 1;
  setDragRectangle(0,0,width,height,FALSE);
  
  // Is it a color being dropped?
  if(offeredDNDType(FXColorWell::colorDragType)){
    acceptDrop(DRAG_COPY);
    return 1;
    }
  return 0;
  }


// Handle drag-and-drop drop
long FXColorWell::onDNDDrop(FXObject* sender,FXSelector sel,void* ptr){
  FXuchar *data; FXuint len,r,g,b,a;
//fprintf(stderr,"%s::onDNDDrop win(%d)\n",getClassName(),xid);
  
  // Try handling it in base class first
  if(FXCell::onDNDDrop(sender,sel,ptr)) return 1;
  
  // Try handle here
  if(getDNDData(FXColorWell::colorDragType,data,len)){
    sscanf((char*)data,"#%02x%02x%02x%02x",&r,&g,&b,&a);
    setRGBA(FXRGBA(r,g,b,a));
    FXFREE(&data);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)rgba); 
    return 1;
    }
  return 0;
  }


// Somebody wants our selection
long FXColorWell::onSelectionRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXchar *value;
//fprintf(stderr,"%s::onSelRequest win(%d)\n",getClassName(),xid);
  
  // Try handling it in base class first
  if(FXCell::onSelectionRequest(sender,sel,ptr)) return 1;
  
  // Try handle here
  if(getDNDType()==FXColorWell::colorDragType){
    FXMALLOC(&value,FXchar,50);
    sprintf(value,"#%02x%02x%02x%02x",FXREDVAL(rgba),FXGREENVAL(rgba),FXBLUEVAL(rgba),FXALPHAVAL(rgba));
    setDNDData(FXColorWell::colorDragType,(FXuchar*)value,50);
    return 1;
    }
  return 0;
  }



// Drag start
long FXColorWell::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    grab();
    beginDrag(&FXColorWell::colorDragType,1);
    }
  return 1;
  }


// Drop 
long FXColorWell::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    ungrab();
    endDrag(didAccept()==DRAG_COPY);
// {
//   FXEvent *ev=(FXEvent*)ptr;
//   XEvent se;
//   FXint tox,toy;
//   Window tmp;
//   Window www=getWindowAt(ev->root_x,ev->root_y);
//   
//   XTranslateCoordinates(getDisplay(),XDefaultRootWindow(getDisplay()),www,ev->root_x,ev->root_y,&tox,&toy,&tmp);
// 
//   se.xbutton.type=ButtonPress;
//   se.xbutton.serial=0;
//   se.xbutton.send_event=1;
//   se.xbutton.display=getDisplay();
//   se.xbutton.window=www;
//   se.xbutton.root=XDefaultRootWindow(getDisplay());
//   se.xbutton.subwindow=None;
//   se.xbutton.time=ev->time;
//   se.xbutton.x=tox;
//   se.xbutton.y=toy;
//   se.xbutton.x_root=ev->root_x;
//   se.xbutton.y_root=ev->root_y;
//   se.xbutton.state=0;
//   se.xbutton.button=2;
//   se.xbutton.same_screen=TRUE;
// fprintf(stderr,"SendEvent to window %d\n",se.xbutton.window);
//   XSendEvent(getDisplay(),se.xbutton.window,True,NoEventMask,&se);
//   
//   se.xbutton.type=ButtonRelease;
//   se.xbutton.serial=0;
//   se.xbutton.send_event=1;
//   se.xbutton.display=getDisplay();
//   se.xbutton.window=www;
//   se.xbutton.root=XDefaultRootWindow(getDisplay());
//   se.xbutton.subwindow=None;
//   se.xbutton.time=ev->time+1;
//   se.xbutton.x=tox;
//   se.xbutton.y=toy;
//   se.xbutton.x_root=ev->root_x;
//   se.xbutton.y_root=ev->root_y;
//   se.xbutton.state=Button2Mask;
//   se.xbutton.button=2;
//   se.xbutton.same_screen=TRUE;
// fprintf(stderr,"SendEvent to window %d\n",se.xbutton.window);
//   XSendEvent(getDisplay(),se.xbutton.window,True,NoEventMask,&se);
// }
    }
  return 1;
  }


// Moving 
long FXColorWell::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(isEnabled()){
    if(isDragging()){
      handleDrag(event->root_x,event->root_y,DRAG_COPY);
      if(didAccept()==DRAG_COPY){
        setDragCursor(getApp()->swatchCursor);
        }
      else{
        setDragCursor(getApp()->dontdropCursor);
        }
      }
    }
  return 1;
  }


// Change RGBA color 
void FXColorWell::setRGBA(FXColor clr){
  if(clr!=rgba){
    rgba=clr;
    wellColor[0]=rgbaoverwhite(rgba);
    wellColor[1]=rgbaoverblack(rgba);
    update(0,0,width,height);
    }
  }


// Get RGBA color
FXColor FXColorWell::getRGBA() const { 
  return rgba; 
  }

