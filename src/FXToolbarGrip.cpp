/********************************************************************************
*                                                                               *
*                       T o o l b a r   G r i p   W i d g e t                   *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXToolbarGrip.cpp,v 1.26 2002/01/18 22:43:06 jeroen Exp $                *
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
#include "FXToolbar.h"
#include "FXToolbarGrip.h"


/*
  Notes:
  - This pretty much works as desired now.
  - Don't repaint while dragging, as it clobbers the inverting rectangle
    being drawn to indicate the future toolbar position.
  - It draws horizontally if its wider than its tall, and vertically otherwise.
  - The mouse is ungrabbed AFTER sending SEL_ENDDRAG, so the highlight is
    removed before any other application gets an event.
*/


// Size
#define GRIP_SINGLE  3          // Single grip for arrangable toolbars
#define GRIP_DOUBLE  7          // Double grip for dockable toolbars

/*******************************************************************************/

// Map
FXDEFMAP(FXToolbarGrip) FXToolbarGripMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXToolbarGrip::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXToolbarGrip::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXToolbarGrip::onLeave),
  FXMAPFUNC(SEL_MOTION,0,FXToolbarGrip::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXToolbarGrip::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXToolbarGrip::onLeftBtnRelease),
  };


// Object implementation
FXIMPLEMENT(FXToolbarGrip,FXWindow,FXToolbarGripMap,ARRAYNUMBER(FXToolbarGripMap))


// Deserialization
FXToolbarGrip::FXToolbarGrip(){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  activeColor=0;
  hiliteColor=0;
  shadowColor=0;
  }


// Construct and init
FXToolbarGrip::FXToolbarGrip(FXToolbar* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(p,opts,x,y,w,h){
  flags|=FLAG_SHOWN;
  if(!(options&TOOLBARGRIP_SEPARATOR)) flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  backColor=getApp()->getBaseColor();
  activeColor=FXRGB(0,0,255);
  if(options&TOOLBARGRIP_SEPARATOR){ // Suggested by Tim Alexeevsky <realtim@mail.ru>
    hiliteColor=getApp()->getShadowColor();
    shadowColor=getApp()->getHiliteColor();
    }
  else{
    hiliteColor=getApp()->getHiliteColor();
    shadowColor=getApp()->getShadowColor();
    }
  }


// Get default width
FXint FXToolbarGrip::getDefaultWidth(){
  return (options&TOOLBARGRIP_DOUBLE)?GRIP_DOUBLE:GRIP_SINGLE;
  }


// Get default height
FXint FXToolbarGrip::getDefaultHeight(){
  return (options&TOOLBARGRIP_DOUBLE)?GRIP_DOUBLE:GRIP_SINGLE;
  }


// Change toolbar orientation
void FXToolbarGrip::setDoubleBar(FXbool dbl){
  FXuint opts=dbl?(options|TOOLBARGRIP_DOUBLE):(options&~TOOLBARGRIP_DOUBLE);
  if(opts!=options){
    options=opts;
    recalc();
    }
  }


// Get toolbar orientation
FXuint FXToolbarGrip::getDoubleBar() const {
  return (options&TOOLBARGRIP_DOUBLE)!=0;
  }


// Handle repaint
long FXToolbarGrip::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  if(width>height){
    if(options&TOOLBARGRIP_DOUBLE){     // =
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,1,2);
      dc.fillRectangle(0,4,1,2);
      dc.fillRectangle(0,0,width-1,1);
      dc.fillRectangle(0,4,width-1,1);
      dc.setForeground(shadowColor);
      dc.fillRectangle(width-1,0,1,3);
      dc.fillRectangle(width-1,4,1,3);
      dc.fillRectangle(0,2,width-1,1);
      dc.fillRectangle(0,6,width-1,1);
      if(flags&FLAG_ACTIVE){
        dc.setForeground(activeColor);
        dc.fillRectangle(1,1,width-2,1);
        dc.fillRectangle(1,5,width-2,1);
        }
      }
    else{                               // -
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,1,2);
      dc.fillRectangle(0,0,width-1,1);
      dc.setForeground(shadowColor);
      dc.fillRectangle(width-1,0,1,3);
      dc.fillRectangle(0,2,width-1,1);
      if(flags&FLAG_ACTIVE){
        dc.setForeground(activeColor);
        dc.fillRectangle(1,1,width-2,1);
        }
      }
    }
  else{
    if(options&TOOLBARGRIP_DOUBLE){     // ||
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,2,1);
      dc.fillRectangle(4,0,2,1);
      dc.fillRectangle(0,0,1,height-1);
      dc.fillRectangle(4,0,1,height-1);
      dc.setForeground(shadowColor);
      dc.fillRectangle(0,height-1,3,1);
      dc.fillRectangle(4,height-1,3,1);
      dc.fillRectangle(2,0,1,height-1);
      dc.fillRectangle(6,0,1,height-1);
      if(flags&FLAG_ACTIVE){
        dc.setForeground(activeColor);
        dc.fillRectangle(1,1,1,height-2);
        dc.fillRectangle(5,1,1,height-2);
        }
      }
    else{                               // |
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,2,1);
      dc.fillRectangle(0,0,1,height-1);
      dc.setForeground(shadowColor);
      dc.fillRectangle(0,height-1,3,1);
      dc.fillRectangle(2,0,1,height-1);
      if(flags&FLAG_ACTIVE){
        dc.setForeground(activeColor);
        dc.fillRectangle(1,1,1,height-2);
        }
      }
    }
  return 1;
  }


// Entered button
long FXToolbarGrip::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onEnter(sender,sel,ptr);
  if(isEnabled() && !(flags&(FLAG_DODRAG|FLAG_TRYDRAG))){ flags|=FLAG_ACTIVE; update(); }
  return 1;
  }


// Leave button
long FXToolbarGrip::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onLeave(sender,sel,ptr);
  if(isEnabled() && !(flags&(FLAG_DODRAG|FLAG_TRYDRAG))){ flags&=~FLAG_ACTIVE; update(); }
  return 1;
  }


// Moved
long FXToolbarGrip::onMotion(FXObject*,FXSelector,void* ptr){
  if(flags&FLAG_DODRAG){
    handle(this,MKUINT(0,SEL_DRAGGED),ptr);
    return 1;
    }
  if((flags&FLAG_TRYDRAG) && ((FXEvent*)ptr)->moved){
    if(handle(this,MKUINT(0,SEL_BEGINDRAG),ptr)) flags|=FLAG_DODRAG;
    flags&=~FLAG_TRYDRAG;
    return 1;
    }
  return 0;
  }


// Pressed LEFT button
long FXToolbarGrip::onLeftBtnPress(FXObject*,FXSelector,void*){
  if(isEnabled()){
    grab();
    flags|=FLAG_TRYDRAG;
    flags&=~FLAG_UPDATE;
    }
  return 1;
  }


// Released LEFT button
long FXToolbarGrip::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(flags&FLAG_DODRAG){handle(this,MKUINT(0,SEL_ENDDRAG),ptr);}
    ungrab();
    flags&=~(FLAG_TRYDRAG|FLAG_DODRAG);
    flags|=FLAG_UPDATE;
    }
  return 1;
  }


// Set highlight color
void FXToolbarGrip::setHiliteColor(FXColor clr){
  if(clr!=hiliteColor){
    hiliteColor=clr;
    update();
    }
  }


// Set shadow color
void FXToolbarGrip::setShadowColor(FXColor clr){
  if(clr!=shadowColor){
    shadowColor=clr;
    update();
    }
  }


// Set active color
void FXToolbarGrip::setActiveColor(FXColor clr){
  if(clr!=activeColor){
    activeColor=clr;
    update();
    }
  }


// Save data
void FXToolbarGrip::save(FXStream& store) const {
  FXWindow::save(store);
  store << activeColor;
  store << hiliteColor;
  store << shadowColor;
  }


// Load data
void FXToolbarGrip::load(FXStream& store){
  FXWindow::load(store);
  store >> activeColor;
  store >> hiliteColor;
  store >> shadowColor;
  }




