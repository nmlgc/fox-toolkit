/********************************************************************************
*                                                                               *
*                     P o p u p   W i n d o w   O b j e c t                     *
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
* $Id: FXPopup.cpp,v 1.20 1998/10/30 15:49:38 jvz Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXPopup.h"

/*

  To do:
  - allow resize option..
  - setting icons
  - Iconified/normal
  - FXApp should keep track of toplevel windows, and if last one is closed,
    end the application
*/

/*******************************************************************************/


// Map
FXDEFMAP(FXPopup) FXPopupMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXPopup::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXPopup::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXPopup::onLeave),
  FXMAPFUNC(SEL_MOTION,0,FXPopup::onMotion),
  FXMAPFUNC(SEL_MAP,0,FXPopup::onMap),
  FXMAPFUNC(SEL_FOCUS_UP,0,FXPopup::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXPopup::onFocusDown),
  FXMAPFUNC(SEL_FOCUS_NEXT,0,FXPopup::onDefault),
  FXMAPFUNC(SEL_FOCUS_PREV,0,FXPopup::onDefault),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXPopup::onDefault),
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXPopup::onDefault),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXPopup::onButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXPopup::onButtonRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXPopup::onButtonPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXPopup::onButtonRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXPopup::onButtonPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXPopup::onButtonRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXPopup::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXPopup::onKeyRelease),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXPopup::onCmdUnpost),
  };


// Object implementation
FXIMPLEMENT(FXPopup,FXShell,FXPopupMap,ARRAYNUMBER(FXPopupMap))


// Transient window used for popups
FXPopup::FXPopup(FXApp* a,FXWindow* own,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXShell(a,opts,x,y,w,h){
  defaultCursor=getApp()->rarrowCursor;
  dragCursor=getApp()->rarrowCursor;
  flags|=FLAG_ENABLED;
  owner=own;
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  border=(options&FRAME_THICK)?2:(options&(FRAME_SUNKEN|FRAME_RAISED))?1:0;
  }


// Popups do override-redirect
FXbool FXPopup::doesOverrideRedirect() const { return 1; }


// Popups do save-unders
FXbool FXPopup::doesSaveUnder() const { return 1; }


// Get owner; if it has none, it's owned by itself
FXWindow* FXPopup::getOwner() const { return owner ? owner : (FXWindow*)this; }


// Create X window
void FXPopup::create(){
  FXShell::create();
  baseColor=acquireColor(getApp()->backColor);
  hiliteColor=acquireColor(getApp()->hiliteColor);
  shadowColor=acquireColor(getApp()->shadowColor);
  borderColor=acquireColor(getApp()->borderColor);
  }


// Get width
FXint FXPopup::getDefaultWidth(){
  register FXWindow* child;
  register FXint w,wmax,wcum,mw=0;
  register FXuint hints;
  wmax=wcum=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(options&PACK_UNIFORM_WIDTH) w=mw;
      else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else w=child->getDefaultWidth();
      if(wmax<w) wmax=w;
      wcum+=w;
      }
    }
  if(options&POPUP_HORIZONTAL) wmax=wcum;
  return wmax+(border<<1);
  }


// Get height
FXint FXPopup::getDefaultHeight(){
  register FXWindow* child;
  register FXint h,hmax,hcum,mh=0;
  register FXuint hints;
  hmax=hcum=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else h=child->getDefaultHeight();
      if(hmax<h) hmax=h;
      hcum+=h;
      }
    }
  if(!(options&POPUP_HORIZONTAL)) hmax=hcum;
  return hmax+(border<<1);
  }


// Recalculate layout
void FXPopup::layout(){
  register FXWindow *child;
  register FXuint hints;
  register FXint w,h,x,y;
  register FXint mh=0,mw=0;
  if(options&POPUP_HORIZONTAL){
    if(options&PACK_UNIFORM_WIDTH) mh=maxChildWidth();
    for(x=border,child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(options&PACK_UNIFORM_WIDTH) w=mw;
        else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
        else w=child->getDefaultWidth();
        child->position(x,border,w,height-(border<<1));
        x+=w;
        }
      }
    }
  else{
    if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
    for(y=border,child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(options&PACK_UNIFORM_HEIGHT) h=mh;
        else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
        else h=child->getDefaultHeight();
        child->position(border,y,width-(border<<1),h);
        y+=h;
        }
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Show it 
void FXPopup::show(){
  FXShell::show();
  }


// Hide it
void FXPopup::hide(){
  FXShell::hide();
  }


void FXPopup::drawBorderRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(borderColor);
  drawRectangle(x,y,w-1,h-1);
  }


void FXPopup::drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(shadowColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }

void FXPopup::drawSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(hiliteColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }

void FXPopup::drawRidgeRectangle(FXint x,FXint y,FXint w,FXint h){
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

void FXPopup::drawGrooveRectangle(FXint x,FXint y,FXint w,FXint h){
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


void FXPopup::drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
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

void FXPopup::drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
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


// Draw border
void FXPopup::drawFrame(FXint x,FXint y,FXint w,FXint h){
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
long FXPopup::onPaint(FXObject*,FXSelector,void*){
  FXASSERT(xid);
  drawFrame(0,0,width,height);
  return 1;
  }


// Focus moved down; wrap back to begin if at end
long FXPopup::onFocusDown(FXObject*,FXSelector,void*){
  FXWindow *child;
  if(getFocus()){
    child=getFocus()->getNext();
    while(child){
      if(child->isEnabled() && child->canFocus()){
        child->setFocus();
        return 1;
        }
      child=child->getNext();
      }
    }
  child=getFirst();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    child=child->getNext();
    }
  return 0;
  }


// Focus moved up; wrap back to end if at begin
long FXPopup::onFocusUp(FXObject*,FXSelector,void*){
  FXWindow *child;
  if(getFocus()){
    child=getFocus()->getPrev();
    while(child){
      if(child->isEnabled() && child->canFocus()){
        child->setFocus();
        return 1;
        }
      child=child->getPrev();
      }
    }
  child=getLast();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    child=child->getPrev();
    }
  return 0;
  }


// Moved into the popup:- tell the target
long FXPopup::onEnter(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
//fprintf(stderr,"%s::onEnter %lx \n",getClassName(),this);
  if(ev->code==CROSSINGNORMAL){ 
    if(getOwner()->grabbed()) getOwner()->ungrab();
    }
  return 1;
  }


// Moved outside the popup:- tell the target
long FXPopup::onLeave(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
//fprintf(stderr,"%s::onLeave %lx \n",getClassName(),this);
  if(ev->code==CROSSINGNORMAL && shown()){ 
    if(!getOwner()->contains(ev->root_x,ev->root_y)){///////// Should be in owners parent coords
      if(!getOwner()->grabbed()) getOwner()->grab();
      }
    }
  return 1;
  }


// Moved (while outside the popup):- tell the target
long FXPopup::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
//fprintf(stderr,"%s::onMotion %lx\n",getClassName(),this);
  if(contains(ev->root_x,ev->root_y)){
    if(getOwner()->grabbed()) getOwner()->ungrab();
    }
  else{
    if(!getOwner()->contains(ev->root_x,ev->root_y)){///////// Should be in owners parent coords
      if(!getOwner()->grabbed()) getOwner()->grab();
      }
    }
  return 1;
  }


// Window may have appeared under the cursor, so ungrab if it was grabbed
long FXPopup::onMap(FXObject* sender,FXSelector sel,void* ptr){
  FXint x,y; FXuint buttons;
//fprintf(stderr,"%s::onMap %lx\n",getClassName(),this);
  FXShell::onMap(sender,sel,ptr);
  getCursorPosition(x,y,buttons);
  if(0<=x && 0<=y && x<width && y<height){
    if(getOwner()->grabbed()) getOwner()->ungrab();
    }
  return 1;
  }


// Pressed button outside popup
long FXPopup::onButtonPress(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onButtonPress %lx\n",getClassName(),this);
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  return 1;
  }


// Released button outside popup
long FXPopup::onButtonRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
//fprintf(stderr,"%s::onButtonRelease %lx\n",getClassName(),this);
  if(event->moved){handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);}
  return 1;
  }


// Key press; escape cancels popup
long FXPopup::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(event->code==KEY_Escape || event->code==KEY_Cancel){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    return 1;
    }
  return FXShell::onKeyPress(sender,sel,ptr);
  }


// Key release; escape cancels popup
long FXPopup::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(event->code==KEY_Escape || event->code==KEY_Cancel){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    return 1;
    }
  return FXShell::onKeyRelease(sender,sel,ptr);
  }


// Unpost menu in case it was its own owner; otherwise
// tell the owner to do so.
long FXPopup::onCmdUnpost(FXObject* sender,FXSelector,void* ptr){
//fprintf(stderr,"%s::onCmdUnpost %lx\n",getClassName(),this);
  if(owner==NULL){
    popdown();
    if(grabbed()) ungrab();
    }
  else{
    owner->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
    }
  return 1;
  }


// Popup the menu at some location
void FXPopup::popup(FXWindow* own,FXint x,FXint y,FXint w,FXint h){
  FXint rw=getRoot()->getWidth();
  FXint rh=getRoot()->getHeight();
  owner=own;
  if(w<=0) w=getDefaultWidth();
  if(h<=0) h=getDefaultHeight();
  if(x+w>rw) x=rw-w;
  if(y+h>rh) y=rh-h;
  if(x<0) x=0; 
  if(y<0) y=0; 
  position(x,y,w,h);
  show();
  raise();
  if(!owner) grab();// Perhaps should NOT grab here!
  }


// Pops down menu and its submenus
void FXPopup::popdown(){
  if(getFocus()) getFocus()->killFocus();
//  killFocus();
  hide();
  }


// Set base color
void FXPopup::setBaseColor(FXPixel clr){
  baseColor=clr;
  update(0,0,width,height);
  }


// Set highlight color
void FXPopup::setHiliteColor(FXPixel clr){
  hiliteColor=clr;
  update(0,0,width,height);
  }


// Set shadow color
void FXPopup::setShadowColor(FXPixel clr){
  shadowColor=clr;
  update(0,0,width,height);
  }


// Set border color
void FXPopup::setBorderColor(FXPixel clr){
  borderColor=clr;
  update(0,0,width,height);
  }


// Get popup orientation
FXuint FXPopup::getOrientation() const {
  return (options&POPUP_MASK);
  }


// Set popup orientation
void FXPopup::setOrientation(FXuint orient){
  FXuint opts=(options&~POPUP_MASK) | (orient&POPUP_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Zap
FXPopup::~FXPopup(){
  owner=(FXWindow*)-1;
  }
