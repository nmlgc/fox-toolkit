/********************************************************************************
*                                                                               *
*                     S c r o l l W i n d o w   W i d g e t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXScrollWindow.cpp,v 1.17 2002/01/18 22:43:04 jeroen Exp $               *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"


/*
  Notes:
  - Perhaps scroll windows should observe FRAME_SUNKEN etc.
  - Perhaps need clip-window to be a parent of the content window.
  - Need margins [and item spacing]
  - Intercepts pagedn/pageup to scroll.
*/



/*******************************************************************************/

// Map
FXDEFMAP(FXScrollWindow) FXScrollWindowMap[]={
  FXMAPFUNC(SEL_KEYPRESS,0,FXScrollWindow::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXScrollWindow::onKeyRelease),
  FXMAPFUNC(SEL_FOCUS_SELF,0,FXScrollWindow::onFocusSelf),
  };


// Object implementation
FXIMPLEMENT(FXScrollWindow,FXScrollArea,FXScrollWindowMap,ARRAYNUMBER(FXScrollWindowMap))



// Construct and init
FXScrollWindow::FXScrollWindow(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  }


// Get content window; may be NULL
FXWindow* FXScrollWindow::contentWindow() const {
  return corner->getNext();
  }


// Determine content width of scroll area
FXint FXScrollWindow::getContentWidth(){
  return contentWindow() ? contentWindow()->getDefaultWidth() : 1;
  }


// Determine content height of scroll area
FXint FXScrollWindow::getContentHeight(){
  return contentWindow() ? contentWindow()->getDefaultHeight() : 1;
  }


// Move contents; moves child window
void FXScrollWindow::moveContents(FXint x,FXint y){
  FXWindow* contents=contentWindow();
  if(contents) contents->move(x,y);
  pos_x=x;
  pos_y=y;
  }


// Recalculate layout
void FXScrollWindow::layout(){

  // Layout scroll bars and viewport
  FXScrollArea::layout();

  // Resize contents
  if(contentWindow()){

    // Reposition content window
    contentWindow()->position(pos_x,pos_y,content_w,content_h);

    // Make sure its under the scroll bars
    contentWindow()->lower();
    }
  flags&=~FLAG_DIRTY;
  }


// When focus moves to scroll window, we actually force the
// focus to the content window or a child thereof.
long FXScrollWindow::onFocusSelf(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *child=contentWindow();
  if(child){
    if(child->isEnabled() && child->canFocus()){
      child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
      return 1;
      }
    if(child->isComposite() && child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr)) return 1;
    }
  return FXComposite::onFocusSelf(sender,sel,ptr);
  }


// Keyboard press
long FXScrollWindow::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  if(FXScrollArea::onKeyPress(sender,sel,ptr)) return 1;
  switch(((FXEvent*)ptr)->code){
    case KEY_Page_Up:
    case KEY_KP_Page_Up:
      setPosition(pos_x,pos_y+verticalScrollbar()->getPage());
      return 1;
    case KEY_Page_Down:
    case KEY_KP_Page_Down:
      setPosition(pos_x,pos_y-verticalScrollbar()->getPage());
      return 1;
    }
  return 0;
  }


// Keyboard release
long FXScrollWindow::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  if(FXScrollArea::onKeyRelease(sender,sel,ptr)) return 1;
  switch(((FXEvent*)ptr)->code){
    case KEY_Page_Up:
    case KEY_KP_Page_Up:
    case KEY_Page_Down:
    case KEY_KP_Page_Down:
      return 1;
    }
  return 0;
  }

