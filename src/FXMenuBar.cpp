/********************************************************************************
*                                                                               *
*                              M e n u  B a r                                   *
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
* $Id: FXMenuBar.cpp,v 1.6 1998/10/27 22:57:36 jvz Exp $                     *
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
#include "FXGIFIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXMenuButton.h"
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenu.h"
#include "FXMenuBar.h"
#include "FXMDIButton.h"
#include "FXMDIChild.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXMDIClient.h"

/*
  Notes:
  - Mnemonics, accelerators.
  - Help text from constructor is third part; second part should be
    accelerator key combination.
  - When menu label changes, hotkey might have to be adjusted.
  - Fix it so menu stays up when after Alt-F, you press Alt-E.
  - MenuItems should be derived from FXLabel.
  - Perhaps use auto-hide feature to show/hide MDI buttons
*/

/*******************************************************************************/


// Map
FXDEFMAP(FXMenuBar) FXMenuBarMap[]={
  FXMAPFUNC(SEL_ENTER,0,FXMenuBar::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuBar::onLeave),
  FXMAPFUNC(SEL_MOTION,0,FXMenuBar::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuBar::onBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuBar::onBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuBar::onBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuBar::onBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuBar::onBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuBar::onBtnRelease),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXMenuBar::onFocusRight),
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXMenuBar::onFocusLeft),
  FXMAPFUNC(SEL_FOCUS_NEXT,0,FXMenuBar::onDefault),     // Not handled!
  FXMAPFUNC(SEL_FOCUS_PREV,0,FXMenuBar::onDefault),
  FXMAPFUNC(SEL_FOCUS_UP,0,FXMenuBar::onDefault),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXMenuBar::onDefault),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXMenuBar::onCmdUnpost),
  };


// Object implementation
FXIMPLEMENT(FXMenuBar,FXComposite,FXMenuBarMap,ARRAYNUMBER(FXMenuBarMap))


// Make a menu title button
FXMenuBar::FXMenuBar(FXComposite* p,FXuint opts):
  FXComposite(p,opts){
  flags|=FLAG_ENABLED;
  windowbtn=new FXMenuButton(this,NULL,NULL,NULL,MENUBUTTON_DOWN|MENUBUTTON_WINDOW|LAYOUT_LEFT,0,0,0,0,0,0,0,0);
  deletebtn=new FXMDIButton(this,NULL,FXMDIClient::ID_DELETE,FRAME_RAISED|MDIBTN_DELETE|LAYOUT_RIGHT);
  restorebtn=new FXMDIButton(this,NULL,FXMDIClient::ID_RESTORE,FRAME_RAISED|MDIBTN_RESTORE|LAYOUT_RIGHT);
  minimizebtn=new FXMDIButton(this,NULL,FXMDIClient::ID_MINIMIZE,FRAME_RAISED|MDIBTN_MINIMIZE|LAYOUT_RIGHT);
  dragCursor=getApp()->rarrowCursor;
  }


// Create menu bar
void FXMenuBar::create(){
  FXComposite::create();
  windowbtn->setBackColor(backColor);
  show();
  }


// Get width
FXint FXMenuBar::getDefaultWidth(){
  register FXWindow *child;
  register FXint n=0,w=0;
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      w+=child->getDefaultWidth();
      n++;
      }
    }
  if(n>2) w+=3*(n-1);
  return w+4;
  }


// Get height
FXint FXMenuBar::getDefaultHeight(){
  register FXWindow *child;
  register FXint t,h=0;
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      t=child->getDefaultHeight();
      if(h<t) h=t;
      }
    }
  return h+2;
  }


// Set MDI client
void FXMenuBar::setMDIClient(FXObject* client){
  windowbtn->setTarget(client);
  minimizebtn->setTarget(client);
  restorebtn->setTarget(client);
  deletebtn->setTarget(client);
  }


// Get MDI client
FXObject* FXMenuBar::getMDIClient() const {
  return deletebtn->getTarget();
  }


// Get icon used for the menu button
FXIcon *FXMenuBar::getWindowIcon() const {
  return windowbtn->getIcon();
  }


// Change icon used for window menu button
void FXMenuBar::setWindowIcon(FXIcon* ic){
  windowbtn->setIcon(ic);
  }


// Obtain window menu
FXMenuPane* FXMenuBar::getWindowMenu() const {
  return windowbtn->getPopup();
  }


// Change window menu
void FXMenuBar::setWindowMenu(FXMenuPane* menu){
  windowbtn->setPopup(menu);
  }


// Display MDI controls
void FXMenuBar::showMDIControls(){
  options|=MENUBAR_MDI;
  recalc();
  }


// Hide MDI controls
void FXMenuBar::hideMDIControls(){
  options&=~MENUBAR_MDI;
  recalc();
  }


// Layout from left to right
void FXMenuBar::layout(){
  register FXWindow *child;
  register FXint w,h,l,r;
  if(options&MENUBAR_MDI){
    windowbtn->show();
    deletebtn->show();
    restorebtn->show();
    minimizebtn->show();
    }
  else{
    windowbtn->hide();
    deletebtn->hide();
    restorebtn->hide();
    minimizebtn->hide();
    }
  for(l=2,r=width-2,child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      w=child->getDefaultWidth();
      h=child->getDefaultHeight();
      if(child->getLayoutHints()&LAYOUT_RIGHT){
        r-=w;
        child->position(r,(height-h)/2,w,h);
        r-=3;
        }
      else{
        child->position(l,(height-h)/2,w,h);
        l+=w;
        l+=3;
        }
      }
    }
  flags&=~FLAG_DIRTY;
  }


// We're considered inside the menu bar when either 
// in the bar or in any active menus
FXbool FXMenuBar::contains(FXint parentx,FXint parenty) const {
  FXint x,y;
  if(FXComposite::contains(parentx,parenty)) return 1;
  if(getFocus()){
    getParent()->translateCoordinatesTo(x,y,this,parentx,parenty);
    if(getFocus()->contains(x,y)) return 1;
    }
  return 0;
  }


// Focus moved to right
long FXMenuBar::onFocusRight(FXObject*,FXSelector,void*){
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
    child=getFirst();
    while(child){
      if(child->isEnabled() && child->canFocus()){
        child->setFocus();
        return 1;
        }
      child=child->getNext();
      }
    }
  return 0;
  }


// Focus moved to left
long FXMenuBar::onFocusLeft(FXObject*,FXSelector,void*){
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
    child=getLast();
    while(child){
      if(child->isEnabled() && child->canFocus()){
        child->setFocus();
        return 1;
        }
      child=child->getPrev();
      }
    }
  return 0;
  }


// Enter:- when inside the popup, all is normal!
long FXMenuBar::onEnter(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint px,py;
//fprintf(stderr,"%s::onEnter %lx x=%d y=%d\n",getClassName(),this,ev->root_x,ev->root_y);
  if(!getFocus() || !getFocus()->isActive()) return 1;
  if(ev->code==CROSSINGNORMAL){
    translateCoordinatesTo(px,py,getParent(),ev->win_x,ev->win_y);
    if(contains(px,py)){
      if(grabbed()) ungrab(); 
      }
    }
  return 1;
  }


// Leave:- when outside the popup, a click will hide the popup!
long FXMenuBar::onLeave(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint px,py;
//fprintf(stderr,"%s::onLeave %lx x=%d y=%d\n",getClassName(),this,ev->root_x,ev->root_y);
  if(!getFocus() || !getFocus()->isActive()) return 1;
  if(ev->code==CROSSINGNORMAL){
    translateCoordinatesTo(px,py,getParent(),ev->win_x,ev->win_y);
    if(!contains(px,py)){
      if(!grabbed()) grab();
      }
    }
  return 1;
  }


// Moved while outside
// We need to do this because the definition of ``inside'' means
// that we're inside even though possibly we're not in THIS window!!!
long FXMenuBar::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint px,py;
  if(!getFocus() || !getFocus()->isActive()) return 0;
//fprintf(stderr,"%s::onMotion %lx \n",getClassName(),this);
  translateCoordinatesTo(px,py,getParent(),ev->win_x,ev->win_y);
  if(contains(px,py)){
    if(grabbed()) ungrab(); 
    }
  else{
    if(!grabbed()) grab();
    }
  return 1;
  }


// Button pressed
long FXMenuBar::onBtnPress(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onBtnPress %lx \n",getClassName(),this);
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  return 1;
  }


// Button released
long FXMenuBar::onBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
//fprintf(stderr,"%s::onBtnRelease %lx \n",getClassName(),this);
  if(ev->moved){ handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL); }
  return 1;
  }


// Unpost the menu
long FXMenuBar::onCmdUnpost(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onCmdUnpost %lx \n",getClassName(),this);
  if(getFocus()) getFocus()->killFocus();
  //killFocus();
  return 1;
  }


// // Force focus to this window if it will accept
// long FXMenuBar::onHotKey(FXObject*,FXSelector,void*){
//   FXWindow *child;
// fprintf(stderr,"%s::onHotKey %08x\n",getClassName(),this);
//   child=getFirst();
//   while(child){
//     if(child->isEnabled() && child->canFocus()){
//       child->setFocus();
//       return 1;
//       }
//     child=child->getNext();
//     }
//   return 1;
//   }
// 


// Destroy thrashes object
FXMenuBar::~FXMenuBar(){
  windowbtn=(FXMenuButton*)-1;
  minimizebtn=(FXButton*)-1;
  restorebtn=(FXButton*)-1;
  deletebtn=(FXButton*)-1;
  }
