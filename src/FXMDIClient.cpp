/********************************************************************************
*                                                                               *
*          M u l t i p l e  D o c u m e n t   C l i e n t   W i n d o w         *
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
* $Id: FXMDIClient.cpp,v 1.39 1998/10/21 22:39:15 jvz Exp $                   *
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
#include "FXLabel.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenu.h"
#include "FXMenuBar.h"
#include "FXMDIChild.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXMDIClient.h"


/*
  Notes:
  - This brings up the question of the cascade design.  Do you want the windows 
    to be cascaded/tiled in the order that they were created, or do you want them 
    to be cascaded in some sort of focus order, with the one that had the focus 
    to be on top,  keeping its focus.
    In the test app, if you click within any of the child windows, it pops to 
    the top with the focus, with the exception of "TEST3", which has a button.  
    If this button is clicked, that window dos not pop to the top when it 
    gets the focus.  Seems like it should...
  - Minor problems with mdi.  When a window is maximized and then
    deleted, the next window that is created is created in "normal" mode.
    When any adjustment is made to the size of the new window, it pops to
    "almost" maximized mode.  (The child frame is visible, but as large as
    possible).  Seems like the child windows should be created maximized if
    the mdi is in maximized mode.  Also, if there are two windows and one
    gets maximized and then deleted, the second window pops to "almost"
    maximixed mode.
  - Need some type of default action when no MDI child exists; all buttons
    grayed out&unchecked??
  - We make MDIClient get a first crack at the messages, so that the MDIChild
    can not shadow any messages really directed at the MDIClient
*/

#define CASCADE_XOFF  24
#define CASCADE_YOFF  24


/*******************************************************************************/


FXDEFMAP(FXMDIClient) FXMDIClientMap[]={
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_TILE_HORIZONTAL,FXMDIClient::onCmdTileHorizontal),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_TILE_VERTICAL,FXMDIClient::onCmdTileVertical),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CASCADE,FXMDIClient::onCmdCascade),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_DELETE,FXMDIClient::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MAXIMIZE,FXMDIClient::onCmdMaximize),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MINIMIZE,FXMDIClient::onCmdMinimize),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_RESTORE,FXMDIClient::onCmdRestore),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_ACTIVATE_NEXT,FXMDIClient::onCmdActivateNext),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_ACTIVATE_PREV,FXMDIClient::onCmdActivatePrev),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_TILE_HORIZONTAL,FXMDIClient::onUpdTileHorizontal),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_TILE_VERTICAL,FXMDIClient::onUpdTileVertical),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_CASCADE,FXMDIClient::onUpdCascade),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_DELETE,FXMDIClient::onUpdDelete),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MAXIMIZE,FXMDIClient::onUpdMaximize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MINIMIZE,FXMDIClient::onUpdMinimize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_RESTORE,FXMDIClient::onUpdRestore),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_ACTIVATE_NEXT,FXMDIClient::onUpdActivateNext),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_ACTIVATE_PREV,FXMDIClient::onUpdActivatePrev),
  };


// Object implementation
FXIMPLEMENT(FXMDIClient,FXScrollArea,FXMDIClientMap,ARRAYNUMBER(FXMDIClientMap))

  
// Construct and init
FXMDIClient::FXMDIClient(){
  active=NULL;
  menubar=NULL;
  xmin=0;
  xmax=0;
  ymin=0;
  ymax=0;
  }


// Construct and init
FXMDIClient::FXMDIClient(FXComposite* p,FXMenuBar* mb,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  active=NULL;
  menubar=mb;
  xmin=0;
  xmax=0;
  ymin=0;
  ymax=0;
  }


// Set other background color
void FXMDIClient::create(){
  FXScrollArea::create();
  setBackColor(acquireColor(getApp()->shadowColor));
  }


// Propagate size change
void FXMDIClient::recalc(){ 
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


// Get first MDI child
FXMDIChild* FXMDIClient::getMDIChildFirst() const { return (FXMDIChild*)corner->getNext(); }

// Get last MDI Child
FXMDIChild* FXMDIClient::getMDIChildLast() const { return (FXMDIChild*)getLast(); }


// Restore the active child
long FXMDIClient::onCmdRestore(FXObject*,FXSelector,void* ptr){
  register FXMDIChild* child;
//fprintf(stderr,"%s::onCmdRestore\n",getClassName());
  if(options&MDICLIENT_MAXIMIZED){
    for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
      if(child->shown()){
        if(child!=active) child->restore();
        }
      }
    if(menubar){
      menubar->setMDIClient(this);
      menubar->hideMDIControls();
      }
    options&=~MDICLIENT_MAXIMIZED;
    }
  if(active) active->restore();
  recalc();
  return 1;
  }


// Update restore
long FXMDIClient::onUpdRestore(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_DISABLE;
  if((options&MDICLIENT_MAXIMIZED) || (active && (active->isMinimized() || active->isMaximized()))) msg=ID_ENABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Maximize the active child
long FXMDIClient::onCmdMaximize(FXObject*,FXSelector,void* ptr){
  register FXMDIChild* child;
//fprintf(stderr,"%s::onCmdMaximize\n",getClassName());
  if(!(options&MDICLIENT_MAXIMIZED)){
    for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
      if(child->shown()){
        child->maximize();
        }
      }
    if(menubar){
      menubar->setMDIClient(this);
      menubar->showMDIControls();
      }
    options|=MDICLIENT_MAXIMIZED;
    }
  recalc();
  return 1;
  }


// Update maximized
long FXMDIClient::onUpdMaximize(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_DISABLE;
  if(!(options&MDICLIENT_MAXIMIZED)) msg=ID_ENABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Minimize the active child
long FXMDIClient::onCmdMinimize(FXObject*,FXSelector,void* ptr){
  register FXMDIChild* child;
//fprintf(stderr,"%s::onCmdMinimize\n",getClassName());
  if(options&MDICLIENT_MAXIMIZED){
    for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
      if(child->shown()){
        if(child!=active) child->restore();
        }
      }
    if(menubar){
      menubar->setMDIClient(this);
      menubar->hideMDIControls();
      }
    options&=~MDICLIENT_MAXIMIZED;
    }
  if(active) active->minimize();
  recalc();
  return 1;
  }


// Update minimized
long FXMDIClient::onUpdMinimize(FXObject* sender,FXSelector,void* ptr){
//fprintf(stderr,"%s::onUpdMinimize\n",getClassName());
  FXuint msg=ID_DISABLE;
  if((options&MDICLIENT_MAXIMIZED) || (active && !active->isMinimized())) msg=ID_ENABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Delete active child
long FXMDIClient::onCmdDelete(FXObject*,FXSelector,void* ptr){
  FXMDIChild *c1,*c2;
  //fprintf(stderr,"%s::onCmdDelete\n",getClassName());
  if(active){
    c1=active;
    c2=(FXMDIChild*)active->getNext();
    if(!c2) c2=(FXMDIChild*)c1->getPrev();
    if(c2==(FXMDIChild*)corner) c2=NULL;
    setActiveChild(c2);
    delete c1;
    if(!corner->getNext()){
      if(options&MDICLIENT_MAXIMIZED){
        if(menubar){
          menubar->setMDIClient(this);
          menubar->hideMDIControls();
          }
        options&=~MDICLIENT_MAXIMIZED;
        }
      }
    recalc();
    }
  return 1;
  }


// Update delete active child
long FXMDIClient::onUpdDelete(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=active ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Set the active child
FXbool FXMDIClient::setActiveChild(FXMDIChild* child){
  if(active!=child){
    if(active){
      active->setInactive();
      active=NULL;
      }
    if(child){
      active=child;
      active->setActive();
      if(menubar){ 
        menubar->setWindowIcon(child->getWindowIcon());
        menubar->setWindowMenu(child->getWindowMenu());
        }
      }
    recalc();
    return TRUE;
    }
  return FALSE;
  }


// Set Menu Bar
void FXMDIClient::setMenuBar(FXMenuBar* mb){
  menubar=mb;
  }


// Get Menu Bar
FXMenuBar* FXMDIClient::getMenuBar() const {
  return menubar;
  }


// Tile horizontally (actually, prefer wider windows)
long FXMDIClient::onCmdTileHorizontal(FXObject*,FXSelector,void* ptr){
  register FXMDIChild* child;
  register FXint n,nr,nc,hroom,vroom,r,c;
  if(options&MDICLIENT_MAXIMIZED){
    if(menubar){
      menubar->setMDIClient(this);
      menubar->hideMDIControls();
      }
    options&=~MDICLIENT_MAXIMIZED;
    }
  for(n=0,child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
    if(child->shown()){
      if(!child->isMinimized()) n++;
      }
    }
  nr=n;
  nc=1;
  if(n>3){
    nc=(int)sqrt((double)n);
    nr=(n+nc-1)/nc;
    }
  hroom=0;
  vroom=0;
  if(nc>0) hroom=width/nc;
  if(nr>0) vroom=height/nr;
  for(child=(FXMDIChild*)corner->getNext(),n=0; child; child=(FXMDIChild*)child->getNext()){
    if(child->shown()){
      if(!child->isMinimized()){
        r=n/nc;
        c=n%nc;
        child->restore();
        child->position(c*hroom,r*vroom,hroom,vroom);
        n++;
        }
      }
    }
  if(active && active->shown()) active->raise();
  recalc();
  return 1;
  }


// Update tile horizontally
long FXMDIClient::onUpdTileHorizontal(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=corner->getNext() ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Tile vertically (actually, prefer taller windows)
long FXMDIClient::onCmdTileVertical(FXObject*,FXSelector,void* ptr){
  register FXMDIChild* child;
  register FXint n,nr,nc,hroom,vroom,r,c;
  if(options&MDICLIENT_MAXIMIZED){
    if(menubar){
      menubar->setMDIClient(this);
      menubar->hideMDIControls();
      }
    options&=~MDICLIENT_MAXIMIZED;
    }
  for(n=0,child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
    if(child->shown()){
      if(!child->isMinimized()) n++;
      }
    }
  nc=n;
  nr=1;
  if(n>3){
    nr=(int)sqrt((double)n);
    nc=(n+nr-1)/nr;
    }
  hroom=0;
  vroom=0;
  if(nc>0) hroom=width/nc;
  if(nr>0) vroom=height/nr;
  for(child=(FXMDIChild*)corner->getNext(),n=0; child; child=(FXMDIChild*)child->getNext()){
    if(child->shown()){
      if(!child->isMinimized()){
        r=n/nc;
        c=n%nc;
        child->restore();
        child->position(c*hroom,r*vroom,hroom,vroom);
        n++;
        }
      }
    }
  if(active && active->shown()) active->raise();
  recalc();
  return 1;
  }


// Update tile vertically
long FXMDIClient::onUpdTileVertical(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=corner->getNext() ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Cascade windows
long FXMDIClient::onCmdCascade(FXObject*,FXSelector,void* ptr){
  register FXMDIChild* child;
  FXint childx,childy,childw,childh;
  childx=5;
  childy=5;
  childw=(2*width)/3;
  childh=(2*height)/3;
  if(options&MDICLIENT_MAXIMIZED){
    if(menubar){
      menubar->setMDIClient(this);
      menubar->hideMDIControls();
      }
    options&=~MDICLIENT_MAXIMIZED;
    }
  for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
    if(child->shown()){
      if(child!=active && !child->isMinimized()){
        child->restore();
        child->position(childx,childy,childw,childh);
        childx+=CASCADE_XOFF;
        childy+=CASCADE_YOFF;
        if(childx+child->getWidth()>width){ childx=5; childy=5; }
        if(childy+child->getHeight()>height){ childy=5; }
        child->raise();
        }
      }
    }
  if(active && active->shown() && !active->isMinimized()){
    active->restore();
    active->position(childx,childy,childw,childh);
    active->raise();
    }
  recalc();
  return 1;
  }


// Update cascade
long FXMDIClient::onUpdCascade(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=corner->getNext() ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }



// Activate next child
long FXMDIClient::onCmdActivateNext(FXObject*,FXSelector,void*){
  if(active && active->getNext()){
    setActiveChild((FXMDIChild*)active->getNext());
    active->raise();
    }
  return 1;
  }


// Activate previous child
long FXMDIClient::onCmdActivatePrev(FXObject*,FXSelector,void*){
  if(active && active->getPrev()!=corner){
    setActiveChild((FXMDIChild*)active->getPrev());
    active->raise();
    }
  return 1;
  }


// Activate next child
long FXMDIClient::onUpdActivateNext(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_DISABLE;
  if(active  && active->getNext()) msg=ID_ENABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Activate previous child
long FXMDIClient::onUpdActivatePrev(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_DISABLE;
  if(active  && active->getPrev()!=corner) msg=ID_ENABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Recompute interior 
void FXMDIClient::recompute(){
  register FXMDIChild* child;
  register FXint xl,yl,xh,yh;
  xmin= 1000000;
  xmax=-1000000;
  ymin= 1000000;
  ymax=-1000000;
  for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
    if(child->shown()){
      if(child->isMinimized()){
        xl=child->getIconX();
        yl=child->getIconY();
        xh=xl+child->getIconWidth();
        yh=yl+child->getIconHeight();
        }
      else{
        xl=child->getNormalX();
        yl=child->getNormalY();
        xh=xl+child->getNormalWidth();
        yh=yl+child->getNormalHeight();
        }
      if(xl<xmin) xmin=xl;
      if(xh>xmax) xmax=xh;
      if(yl<ymin) ymin=yl;
      if(yh>ymax) ymax=yh;
      }
    }
  flags&=~FLAG_RECALC;
  }



// Determine content width of scroll area
FXint FXMDIClient::getContentWidth(){ 
  if(flags&FLAG_RECALC) recompute();
  if(options&MDICLIENT_MAXIMIZED) return width;
  if(xmin>xmax) return 1;
  if(xmin<0 && xmax<=width) return width-xmin;
  if(0<=xmin && width<xmax) return xmax;
  return xmax-xmin;
  }


// Determine content height of scroll area
FXint FXMDIClient::getContentHeight(){ 
  if(flags&FLAG_RECALC) recompute();
  if(options&MDICLIENT_MAXIMIZED) return height;
  if(ymin>ymax) return 1;
  if(ymin<0 && ymax<=height) return height-ymin;
  if(0<=ymin && height<ymax) return ymax;
  return ymax-ymin;
  }


// Move contents; moves child window
void FXMDIClient::moveContents(FXint x,FXint y){
  FXMDIChild *child;
  for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
    child->move(child->getX()+x-pos_x,child->getY()+y-pos_y);
    }
  pos_x=x;
  pos_y=y;
  }


// Recalculate layout
void FXMDIClient::layout(){
  register FXMDIChild* child;
  register FXint xx,yy,ww,hh;
  
  // If maximized mode, you only see one child
  if(options&MDICLIENT_MAXIMIZED){
    for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
      if(child->shown()){
        child->position(0,0,width,height);
        }
      }
    }
  
  // Normal layout otherwise
  else{
    for(child=(FXMDIChild*)corner->getNext(); child; child=(FXMDIChild*)child->getNext()){
      if(child->shown()){
        if(child->isMinimized()){
          xx=child->getIconX();
          yy=child->getIconY();
          ww=child->getIconWidth();
          hh=child->getIconHeight();
          }
        else{
          xx=child->getNormalX();
          yy=child->getNormalY();
          ww=child->getNormalWidth();
          hh=child->getNormalHeight();
          }
        child->position(xx,yy,ww,hh);
        }
      }
    }
  
  // Raise the active window
  // The scrollbars are raised on top of everything else right afterward
  if(active && active->shown()) active->raise();
  
  // Recompute scroll bars
  FXScrollArea::layout();
  
  // With new range, adjust scroll bar position X
  if(xmin<0){
    horizontal->setPosition(-xmin);
    pos_x=-horizontal->getPosition();
    }
  
  // With new range, adjust scroll bar position Y
  if(ymin<0){
    vertical->setPosition(-ymin);
    pos_y=-vertical->getPosition();
    }
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Handle message
long FXMDIClient::handle(FXObject* sender,FXSelector key,void* data){
  
  // Filter out messages for MDIClient; the active MDIChild
  // should not be able to catch any of MDIClient's messages.
  if(FXScrollArea::handle(sender,key,data)) return 1;
  
  // Unknown messages get forwarded to active MDIChild
  return ID_LAST<=SELID(key) && active && active->handle(sender,key,data);
  }


// Return true if MDI Client is maximized
FXbool FXMDIClient::isMaximized() const { 
  return (options&MDICLIENT_MAXIMIZED)!=0; 
  }


// Destruct thrashes object
FXMDIClient::~FXMDIClient(){
  active=(FXMDIChild*)-1;
  menubar=(FXMenuBar*)-1;
  }
