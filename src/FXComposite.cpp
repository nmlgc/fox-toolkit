/********************************************************************************
*                                                                               *
*                C o m p o s i t e   W i n d o w   O b j e c t                  *
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
* $Id: FXComposite.cpp,v 1.16 1998/09/22 20:33:45 jvz Exp $                   *
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
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXShell.h"

  
/*
  Notes:
  - Rather a slim class.
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXComposite) FXCompositeMap[]={
  FXMAPFUNC(SEL_FOCUS_NEXT,0,FXComposite::onFocusNext),
  FXMAPFUNC(SEL_FOCUS_PREV,0,FXComposite::onFocusPrev),
  FXMAPFUNC(SEL_FOCUS_UP,0,FXComposite::onFocusPrev),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXComposite::onFocusNext),
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXComposite::onFocusPrev),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXComposite::onFocusNext),
  };


// Object implementation
FXIMPLEMENT(FXComposite,FXWindow,FXCompositeMap,ARRAYNUMBER(FXCompositeMap))


// Only used for Root Window
FXComposite::FXComposite(FXApp* a):
  FXWindow(a){
  }


// Only used for Shell Window
FXComposite::FXComposite(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(a,opts,x,y,w,h){
  }


// Create empty composite window
FXComposite::FXComposite(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(p,opts,x,y,w,h){
  }


// Create X window
void FXComposite::create(){
  FXWindow::create();
  for(FXWindow *c=getFirst(); c; c=c->getNext()) c->create();
  }


// Destroy X window
void FXComposite::destroy(){
  for(FXWindow *c=getFirst(); c; c=c->getNext()) c->destroy();
  FXWindow::destroy();
  }


// Get width
FXint FXComposite::getDefaultWidth(){
  FXint t,w=0;
  for(FXWindow* child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      t=child->getX()+child->getWidth();
      if(w<t) w=t;
      }
    }
  return w;
  }


// Get height
FXint FXComposite::getDefaultHeight(){
  FXint t,h=0;
  for(FXWindow* child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      t=child->getY()+child->getHeight();
      if(h<t) h=t;
      }
    }
  return h;
  }


// Get maximum child width
FXint FXComposite::maxChildWidth() const {
  register FXWindow* child;
  register FXuint hints;
  register FXint t,m;
  for(m=0,child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) t=child->getWidth();
      else t=child->getDefaultWidth();
      if(m<t) m=t;
      }
    }
  return m;
  }


// Get maximum child height
FXint FXComposite::maxChildHeight() const {
  register FXWindow* child;
  register FXuint hints;
  register FXint t,m;
  for(m=0,child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_HEIGHT) t=child->getHeight();
      else t=child->getDefaultHeight();
      if(m<t) m=t;
      }
    }
  return m;
  }


// Just tell server where the windows are!
void FXComposite::layout(){
  for(FXWindow *c=getFirst(); c; c=c->getNext()){
    if(c->shown()){
      c->position(c->getX(),c->getY(),c->getWidth(),c->getHeight());
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Focus moved to next
long FXComposite::onFocusNext(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child;
//fprintf(stderr,"%s::onFocusNext\n",getClassName());
  if(getFocus())
    child=getFocus()->getNext();
  else
    child=getFirst();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
    child=child->getNext();
    }
  return 0;
  }


// Focus moved to previous
long FXComposite::onFocusPrev(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child;
//fprintf(stderr,"%s::onFocusPrev\n",getClassName());
  if(getFocus())
    child=getFocus()->getPrev();
  else
    child=getLast();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
    child=child->getPrev();
    }
  return 0;
  }


// Is widget a composite
FXbool FXComposite::isComposite() const { return 1; }


// Dispose of all the children
FXComposite::~FXComposite(){
  while(getLast()){ delete getLast(); }
  }


