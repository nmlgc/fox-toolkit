/********************************************************************************
*                                                                               *
*                 V e r t i c a l   C o n t a i n e r   O b j e c t             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* Contributed by: Charles W. Warren                                             *
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
* $Id: FXShutter.cpp,v 1.14 1998/10/26 15:41:12 jvz Exp $                     *
********************************************************************************/
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
#include "FXPacker.h"
#include "FXVerticalFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXShutter.h"
#include "FXShell.h"

  

/*******************************************************************************/

// Map
FXDEFMAP(FXShutter) FXShutterMap[]={
  FXMAPFUNC(SEL_FOCUS_UP,0,FXShutter::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXShutter::onFocusDown),
  FXMAPFUNC(SEL_TIMEOUT,FXShutter::ID_SHUTTER_TIMEOUT,FXShutter::onTimeout),
  FXMAPFUNC(SEL_COMMAND,FXShutter::ID_OPEN_SHUTTERITEM,FXShutter::onOpenItem),
  };


// Object implementation
FXIMPLEMENT(FXShutter,FXPacker,FXShutterMap,ARRAYNUMBER(FXShutterMap))


// Make shutter
FXShutter::FXShutter(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXVerticalFrame(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  selectedItem=NULL;
  closingItem=NULL;
  heightIncrement=1;
  closingHeight=0;
  closingHadScrollbar=FALSE;
  timer=NULL;
  }


// Focus moved up
long FXShutter::onFocusUp(FXObject* sender,FXSelector sel,void* ptr){
  return FXVerticalFrame::onFocusPrev(sender,sel,ptr);
  }


// Focus moved down
long FXShutter::onFocusDown(FXObject* sender,FXSelector sel,void* ptr){
  return FXVerticalFrame::onFocusNext(sender,sel,ptr);
  }


// The sender of the message is the item to open up
long FXShutter::onOpenItem(FXObject* sender,FXSelector sel,void* ptr){
  FXShutterItem *item=(FXShutterItem*)sender;
  if(!selectedItem) selectedItem=(FXShutterItem*)getFirst();
  if(selectedItem==item) return 1;
  heightIncrement=1;
  closingItem=selectedItem;
  closingHeight=closingItem->getHeight();
  closingHadScrollbar=closingItem->scrollWindow->verticalScrollbar()->shown();
  selectedItem=item;
  timer=getApp()->addTimeout(10,this,FXShutter::ID_SHUTTER_TIMEOUT);
  return 1;
  }


// Shutter Item Animation
long FXShutter::onTimeout(FXObject* sender,FXSelector sel,void*){
  if(!closingItem) return 0;
  closingHeight-=heightIncrement;
  heightIncrement+=5;
  if(closingHeight>0){
    timer=getApp()->addTimeout(10,this,FXShutter::ID_SHUTTER_TIMEOUT);
    }
  else{
    closingItem=NULL;
    timer=NULL;
    }
  recalc();
  return 1;   
  }


// Layout
void FXShutter::layout(){
  register FXShutterItem* child;
  
  // One of the children may have disappeared
  if(selectedItem==NULL) selectedItem=(FXShutterItem*)getFirst();
  
  // Force only one of the children to be open
  for(child=(FXShutterItem*)getFirst(); child; child=(FXShutterItem*)child->getNext()){
    if(child->shown()){
      if(child==selectedItem){
        child->setLayoutHints(LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_LEFT|LAYOUT_TOP);
        child->scrollWindow->setScrollStyle(closingItem ? (VSCROLLER_NEVER|HSCROLLER_NEVER) : HSCROLLER_NEVER);
        child->scrollWindow->show();
        }
      else if(child==closingItem){
        child->setLayoutHints(LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|LAYOUT_LEFT|LAYOUT_TOP);
        child->scrollWindow->setScrollStyle(closingHadScrollbar ? (VSCROLLER_ALWAYS|HSCROLLER_NEVER) : (VSCROLLER_NEVER|HSCROLLER_NEVER));
        child->setHeight(closingHeight);
        }
      else{
        child->setLayoutHints(LAYOUT_FILL_X|LAYOUT_LEFT|LAYOUT_TOP);
        child->scrollWindow->hide();
        }
      }
    }
  
  // Then layout normally
  FXVerticalFrame::layout();
  flags&=~FLAG_DIRTY;
  }


// Clean up
FXShutter::~FXShutter() {
  if(timer) getApp()->removeTimeout(timer);
  timer=(FXTimer*)-1;
  selectedItem=(FXShutterItem*)-1;
  closingItem=(FXShutterItem*)-1;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXShutterItem) FXShutterItemMap[]={
  FXMAPFUNC(SEL_FOCUS_UP,0,FXShutterItem::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXShutterItem::onFocusDown),
  FXMAPFUNC(SEL_COMMAND,FXShutterItem::ID_SHUTTERITEM_BUTTON,FXShutterItem::onCmdButton),
  };


// Object implementation
FXIMPLEMENT(FXShutterItem,FXPacker,FXShutterItemMap,ARRAYNUMBER(FXShutterItemMap))


FXShutterItem::FXShutterItem(FXComposite* p,const char* text,FXIcon* ic,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXVerticalFrame(p,opts,x,y,w,h,0,0,0,0,0,0){
  button=new FXButton(this,text,NULL,this,FXShutterItem::ID_SHUTTERITEM_BUTTON,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,0,0,0,0);
  scrollWindow=new FXScrollWindow(this,VSCROLLER_NEVER|HSCROLLER_NEVER|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_LEFT|LAYOUT_TOP,0,0,0,0);
  content=new FXVerticalFrame(scrollWindow,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,pl,pr,pt,pb,hs,vs);
  }


// We want a different background color...
void FXShutterItem::create(){
  FXVerticalFrame::create();
  content->setBackColor(acquireColor(getApp()->shadowColor)); 
  }


// Button Pressed
long FXShutterItem::onCmdButton(FXObject* sender,FXSelector sel,void* ptr){
  getParent()->handle(this,MKUINT(FXShutter::ID_OPEN_SHUTTERITEM,SEL_COMMAND),ptr);
  return 1;
  }


// Focus moved up
long FXShutterItem::onFocusUp(FXObject* sender,FXSelector sel,void* ptr){
  return FXVerticalFrame::onFocusPrev(sender,sel,ptr);
  }


// Focus moved down
long FXShutterItem::onFocusDown(FXObject* sender,FXSelector sel,void* ptr){
  return FXVerticalFrame::onFocusNext(sender,sel,ptr);
  }


// Thrash it
FXShutterItem::~FXShutterItem(){
  if(((FXShutter*)getParent())->selectedItem==this) ((FXShutter*)getParent())->selectedItem=NULL;
  button=(FXButton*)-1;
  scrollWindow=(FXScrollWindow*)-1;
  content=(FXVerticalFrame*)-1;
  }
  
