/********************************************************************************
*                                                                               *
*                     S h e l l   W i n d o w   O b j e c t                     *
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
* $Id: FXShell.cpp,v 1.15 1998/10/30 15:49:39 jvz Exp $                      *
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
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"

/*

  To do:
  - allow resize option..
  - setting icons
  - Iconified/normal
  - FXApp should keep track of toplevel windows, and if last one is closed,
    end the application.
  - It is not true that all shell windows' sizes are controlled by users;
    popups should resize when contents change...
  - Should Shell be a type of packer?
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXShell) FXShellMap[]={
  FXMAPFUNC(SEL_FOCUS_NEXT,0,FXShell::onFocusNext),
  FXMAPFUNC(SEL_FOCUS_PREV,0,FXShell::onFocusPrev),
  FXMAPFUNC(SEL_CONFIGURE,0,FXShell::onConfigure),
  };


// Object implementation
FXIMPLEMENT(FXShell,FXComposite,FXShellMap,ARRAYNUMBER(FXShellMap))


// Create a toplevel window
FXShell::FXShell(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(a,opts,x,y,w,h){
  }


// Create X window
void FXShell::create(){
//fprintf(stderr,"%s width=%d (%d), height=%d (%d)\n",getClassName(),width,getDefaultWidth(),height,getDefaultHeight());
  FXComposite::create();
//fprintf(stderr,"%s width=%d (%d), height=%d (%d)\n",getClassName(),width,getDefaultWidth(),height,getDefaultHeight());
  if(width<2 || height<2) resize(getDefaultWidth(),getDefaultHeight());
  }


// User determines size of shells
void FXShell::recalc(){
  getApp()->refresh();
  flags|=FLAG_DIRTY;
  }


// Handle configure notify
long FXShell::onConfigure(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXComposite::onConfigure(sender,sel,ptr);
  if((ev->rect.w!=width) || (ev->rect.h!=height)){
    width=ev->rect.w;               // Record new size
    height=ev->rect.h;
    recalc();                       // Need layout later
    }
  return 1;
  }


// Focus moved to next
long FXShell::onFocusNext(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *child;
  if(getFocus()){
    child=getFocus()->getNext();
    while(child){
      if(child->isEnabled() && child->canFocus()){
        child->setFocus();
        return 1;
        }
      if(child->isComposite() && child->handle(sender,sel,ptr)) return 1;
      child=child->getNext();
      }
    getFocus()->killFocus();
    }
  child=getFirst();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(sender,sel,ptr)) return 1;
    child=child->getNext();
    }
  return 0;
  }


// Focus moved to previous
long FXShell::onFocusPrev(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *child;
  if(getFocus()){
    child=getFocus()->getPrev();
    while(child){
      if(child->isEnabled() && child->canFocus()){
        child->setFocus();
        return 1;
        }
      if(child->isComposite() && child->handle(sender,sel,ptr)) return 1;
      child=child->getPrev();
      }
    getFocus()->killFocus();
    }
  child=getLast();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(sender,sel,ptr)) return 1;
    child=child->getPrev();
    }
  return 0;
  }


