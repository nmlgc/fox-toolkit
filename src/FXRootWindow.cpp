/********************************************************************************
*                                                                               *
*                       R o o t   W i n d o w   O b j e c t                     *
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
* $Id: FXRootWindow.cpp,v 1.5 1998/10/27 06:29:38 jeroen Exp $                  *
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
#include "FXRootWindow.h"
#include "FXShell.h"

  

/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXRootWindow,FXComposite,NULL,0)


// This constructor is only used for the root window
FXRootWindow::FXRootWindow(FXApp* a):
  FXComposite(a){
  }


// When created, create subwindows ONLY
void FXRootWindow::create(){
  if(!xid){
    xid=RootWindow(getApp()->display,DefaultScreen(getApp()->display));
    width=DisplayWidth(getApp()->display,DefaultScreen(getApp()->display));
    height=DisplayHeight(getApp()->display,DefaultScreen(getApp()->display));
    depth=DefaultDepth(getApp()->display,DefaultScreen(getApp()->display));
    
    // Normally create children
    for(FXWindow *c=getFirst(); c; c=c->getNext()) c->create();
    }
  }


// When deleted, delete subwindows ONLY
void FXRootWindow::destroy(){
  if(xid){
    for(FXWindow *c=getFirst(); c; c=c->getNext()) c->destroy();
    xid=0;
    }
  }


// Get default width
FXint FXRootWindow::getDefaultWidth(){ 
  return DisplayWidth(getApp()->display,DefaultScreen(getApp()->display)); 
  }


// Get default height
FXint FXRootWindow::getDefaultHeight(){
  return DisplayHeight(getApp()->display,DefaultScreen(getApp()->display));
  }


// Just in case it gets called, it does nothing
void FXRootWindow::layout(){
  flags&=~FLAG_DIRTY; 
  }


// Just in case it gets called, it does nothing
void FXRootWindow::recalc(){ 
  flags&=~FLAG_DIRTY; 
  }


// Does not destroy root window
FXRootWindow::~FXRootWindow(){
  xid=0;
  }
