/********************************************************************************
*                                                                               *
*          M u l t i p l e  D o c u m e n t   C l i e n t   W i n d o w         *
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
* $Id: FXMDIClient.cpp,v 1.27 2002/01/18 22:43:01 jeroen Exp $                  *
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
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXMenuButton.h"
#include "FXComposite.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenuPane.h"
#include "FXMDIButton.h"
#include "FXPacker.h"
#include "FXHorizontalFrame.h"
#include "FXToolbar.h"
#include "FXMenubar.h"
#include "FXMDIChild.h"
#include "FXScrollbar.h"
#include "FXScrollArea.h"
#include "FXMDIClient.h"


/*
  Notes:
  - This brings up the question of the cascade design.  Do you want the windows
    to be cascaded/tiled in the order that they were created, or do you want them
    to be cascaded in some sort of focus order, with the one that had the focus
    to be on top,  keeping its focus.
    In the test app, if you click within any of the child windows, it pops to
    the top with the focus, with the exception of "TEST3", which has a button.
    If this button is clicked, that window dos not pop to the top when it
    gets the focus.  Seems like it should...
  - Minor problems with mdi.  When a window is maximized and then
    deleted, the next window that is created is created in "normal" mode.
    When any adjustment is made to the size of the new window, it pops to
    "almost" maximized mode.  (The child frame is visible, but as large as
    possible).  Seems like the child windows should be created maximized if
    the mdi is in maximized mode.  Also, if there are two windows and one
    gets maximized and then deleted, the second window pops to "almost"
    maximixed mode.
  - We make MDIClient get a first crack at the messages, so that the MDIChild
    can not shadow any messages really directed at the MDIClient.
  - Need ``arrange icons'' feature.
  - When switching active MDIChild windows, we pass the old to the new and vice
    versa; this allows the MDIChild's target to determine if we switched windows
    only, or if we switched between one document and another at the same time
*/

#define CASCADE_XOFF  24
#define CASCADE_YOFF  24


/*******************************************************************************/


FXDEFMAP(FXMDIClient) FXMDIClientMap[]={
  FXMAPFUNC(SEL_CHANGED,0,FXMDIClient::onChanged),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_NEXT,FXMDIClient::onUpdActivateNext),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_PREV,FXMDIClient::onUpdActivatePrev),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_TILEHORIZONTAL,FXMDIClient::onUpdTileHorizontal),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_TILEVERTICAL,FXMDIClient::onUpdTileVertical),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_CASCADE,FXMDIClient::onUpdCascade),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_CLOSE,FXMDIClient::onUpdClose),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MINIMIZE,FXMDIClient::onUpdMinimize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_RESTORE,FXMDIClient::onUpdRestore),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MAXIMIZE,FXMDIClient::onUpdMaximize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENURESTORE,FXMDIClient::onUpdMenuRestore),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENUCLOSE,FXMDIClient::onUpdMenuClose),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENUMINIMIZE,FXMDIClient::onUpdMenuMinimize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENUWINDOW,FXMDIClient::onUpdMenuWindow),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_CLOSE_DOCUMENT,FXMDIClient::onUpdCloseDocument),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_CLOSE_ALL_DOCUMENTS,FXMDIClient::onUpdCloseAllDocuments),
  FXMAPFUNC(SEL_UPDATE,FXMDIClient::ID_MDI_ANY,FXMDIClient::onUpdAnyWindows),
  FXMAPFUNCS(SEL_UPDATE,FXMDIClient::ID_MDI_1,FXMDIClient::ID_MDI_10,FXMDIClient::onUpdWindowSelect),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_NEXT,FXMDIClient::onCmdActivateNext),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_PREV,FXMDIClient::onCmdActivatePrev),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_TILEHORIZONTAL,FXMDIClient::onCmdTileHorizontal),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_TILEVERTICAL,FXMDIClient::onCmdTileVertical),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_CASCADE,FXMDIClient::onCmdCascade),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CLOSE_DOCUMENT,FXMDIClient::onCmdCloseDocument),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CLOSE_ALL_DOCUMENTS,FXMDIClient::onCmdCloseAllDocuments),
  FXMAPFUNCS(SEL_COMMAND,FXMDIClient::ID_MDI_1,FXMDIClient::ID_MDI_10,FXMDIClient::onCmdWindowSelect),
  };


// Object implementation
FXIMPLEMENT(FXMDIClient,FXScrollArea,FXMDIClientMap,ARRAYNUMBER(FXMDIClientMap))


// Construct and init
FXMDIClient::FXMDIClient(){
  mdifirst=NULL;
  mdilast=NULL;
  active=NULL;
  cascadex=CASCADE_XOFF;
  cascadey=CASCADE_YOFF;
  xmin=0;
  xmax=0;
  ymin=0;
  ymax=0;
  }


// Construct and init
FXMDIClient::FXMDIClient(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  backColor=getApp()->getShadowColor();
  mdifirst=NULL;
  mdilast=NULL;
  active=NULL;
  cascadex=CASCADE_XOFF;
  cascadey=CASCADE_YOFF;
  xmin=0;
  xmax=0;
  ymin=0;
  ymax=0;
  }


// Propagate size change
void FXMDIClient::recalc(){
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


// User clicks on one of the window menus
long FXMDIClient::onCmdWindowSelect(FXObject*,FXSelector sel,void*){
  register FXMDIChild *child=getMDIChildFirst();
  register FXint which=SELID(sel)-ID_MDI_1;
  while(child && which){
    child=child->getMDINext();
    which--;
    }
  setActiveChild(child,TRUE);
  return 1;
  }


// Update handler for window menus
long FXMDIClient::onUpdWindowSelect(FXObject *sender,FXSelector sel,void*){
  register FXMDIChild *child=getMDIChildFirst();
  register FXint n=SELID(sel)-ID_MDI_1;
  register FXint which=n+1;
  while(child && n){
    child=child->getMDINext();
    n--;
    }
  if(child){
    FXString string;
    if(which<10)
      string.format("&%d %s",which,child->getTitle().text());
    else
      string.format("1&0 %s",child->getTitle().text());
    sender->handle(this,MKUINT(FXWindow::ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&string);
    sender->handle(this,MKUINT(FXWindow::ID_SHOW,SEL_COMMAND),NULL);
    if(child==active)
      sender->handle(this,MKUINT(FXWindow::ID_CHECK,SEL_COMMAND),NULL);
    else
      sender->handle(this,MKUINT(FXWindow::ID_UNCHECK,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(FXWindow::ID_HIDE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Show or hide depending on whether there are any windows
long FXMDIClient::onUpdAnyWindows(FXObject *sender,FXSelector,void*){
  if(getMDIChildFirst())
    sender->handle(this,MKUINT(FXWindow::ID_SHOW,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(FXWindow::ID_HIDE,SEL_COMMAND),NULL);
  return 1;
  }


// Update restore; gray if no active
long FXMDIClient::onUpdRestore(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update maximized; gray if no active
long FXMDIClient::onUpdMaximize(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update minimized
long FXMDIClient::onUpdMinimize(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update close active child
long FXMDIClient::onUpdClose(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update menu's restore button
long FXMDIClient::onUpdMenuWindow(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
  return 1;
  }


// Update menu's restore button
long FXMDIClient::onUpdMenuRestore(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
  return 1;
  }


// Update menu's minimized button
long FXMDIClient::onUpdMenuMinimize(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
  return 1;
  }


// Update menu's close button
long FXMDIClient::onUpdMenuClose(FXObject* sender,FXSelector sel,void* ptr){
  if(active) return active->handle(sender,sel,ptr);
  sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
  return 1;
  }


// Set the active child
FXbool FXMDIClient::setActiveChild(FXMDIChild* child,FXbool notify){
  FXbool wasmax=FALSE;
  if(active!=child){

    if(active){

      // Was it maximized?
      wasmax=active->isMaximized();

      // Deactivate old MDIChild
      active->handle(this,MKUINT(0,SEL_DESELECTED),(void*)child);     // FIXME should call member function

      // Restore to normal size if it was maximized
      //if(wasmax) active->handle(this,MKUINT(0,SEL_RESTORE),NULL);
      if(wasmax) active->restore(notify);
      }

    if(child){

      // Activate new MDIChild
      child->handle(this,MKUINT(0,SEL_SELECTED),(void*)active);     // FIXME should call member function

      // Maximize because the old MDIChild was maximized
      //if(wasmax) active->handle(this,MKUINT(0,SEL_MAXIMIZE),NULL);
      if(wasmax) child->maximize(notify);
      }

    active=child;

    // Notify target
    //if(notify){handle(this,MKUINT(0,SEL_CHANGED),ptr);}   // FIXME

    recalc();
    return TRUE;
    }
  return FALSE;
  }


// Current child changed
long FXMDIClient::onChanged(FXObject*,FXSelector,void* ptr){
  //return target && target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  setActiveChild((FXMDIChild*)ptr);
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);    // FIXME
  return 1;
  }


// Tile horizontally (actually, prefer wider windows)
long FXMDIClient::onCmdTileHorizontal(FXObject*,FXSelector,void*){
  register FXMDIChild* child;
  register FXint n,nr,nc,hroom,vroom,r,c;
  for(n=0,child=mdifirst; child; child=child->getMDINext()){
    if(child->shown() && !child->isMinimized()) n++;
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
  for(child=mdifirst,n=0; child; child=child->getMDINext()){
    if(child->shown() && !child->isMinimized()){
      r=n/nc;
      c=n%nc;
      child->restore(TRUE);
      child->position(c*hroom,r*vroom,hroom,vroom);
      n++;
      }
    }
  if(active && active->shown()) active->raise();
  recalc();
  return 1;
  }


// Update tile horizontally
long FXMDIClient::onUpdTileHorizontal(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=mdifirst ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Tile vertically (actually, prefer taller windows)
long FXMDIClient::onCmdTileVertical(FXObject*,FXSelector,void*){
  register FXMDIChild* child;
  register FXint n,nr,nc,hroom,vroom,r,c;
  for(n=0,child=mdifirst; child; child=child->getMDINext()){
    if(child->shown() && !child->isMinimized()) n++;
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
  for(child=mdifirst,n=0; child; child=child->getMDINext()){
    if(child->shown() && !child->isMinimized()){
      r=n/nc;
      c=n%nc;
      child->restore(TRUE);
      child->position(c*hroom,r*vroom,hroom,vroom);
      n++;
      }
    }
  if(active && active->shown()) active->raise();
  recalc();
  return 1;
  }


// Update tile vertically
long FXMDIClient::onUpdTileVertical(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=mdifirst ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Pass message to all MDI Child windows
long FXMDIClient::forallWindows(FXObject* sender,FXSelector sel,void* ptr){
  register FXMDIChild *child,*nxtchild;
  register FXbool handled=0;
  for(child=mdifirst; child; child=nxtchild){
    nxtchild=child->getMDINext();
    handled|=child->handle(sender,sel,ptr);
    }
  return handled;
  }


// Pass message to all MDI Child windows whose target is document
long FXMDIClient::forallDocWindows(FXObject* document,FXObject* sender,FXSelector sel,void* ptr){
  register FXMDIChild *child,*nxtchild;
  register FXbool handled=0;
  if(document){
    for(child=mdifirst; child; child=nxtchild){
      nxtchild=child->getMDINext();
      if(document==child->getTarget()){ handled|=child->handle(sender,sel,ptr); }
      }
    }
  return handled;
  }


// Cascade windows
long FXMDIClient::onCmdCascade(FXObject*,FXSelector,void*){
  register FXMDIChild* child;
  FXint childx,childy,childw,childh;
  childx=5;
  childy=5;
  childw=(2*width)/3;
  childh=(2*height)/3;
  for(child=mdifirst; child; child=child->getMDINext()){
    if(child==active) continue;
    if(child->shown() && !child->isMinimized()){
      child->handle(this,MKUINT(0,SEL_RESTORE),NULL);
      child->position(childx,childy,childw,childh);
      child->raise();
      childx+=cascadex;
      childy+=cascadey;
      if(childx+child->getWidth()>width){ childx=5; childy=5; }
      if(childy+child->getHeight()>height){ childy=5; }
      }
    }
  if(active && active->shown() && !active->isMinimized()){
    active->restore(TRUE);
    active->position(childx,childy,childw,childh);
    active->raise();
    }
  recalc();
  return 1;
  }


// Update cascade
long FXMDIClient::onUpdCascade(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=mdifirst ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Activate next child
long FXMDIClient::onCmdActivateNext(FXObject*,FXSelector,void*){
  if(active && active->getMDINext()){
    handle(this,MKUINT(0,SEL_CHANGED),active->getMDINext());
    }
  return 1;
  }


// Activate next child
long FXMDIClient::onUpdActivateNext(FXObject* sender,FXSelector,void*){
  if(active  && active->getMDINext())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Activate previous child
long FXMDIClient::onCmdActivatePrev(FXObject*,FXSelector,void*){
  if(active && active->getMDIPrev()){
    handle(this,MKUINT(0,SEL_CHANGED),active->getMDIPrev());
    }
  return 1;
  }


// Activate previous child
long FXMDIClient::onUpdActivatePrev(FXObject* sender,FXSelector,void*){
  if(active  && active->getMDIPrev())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Close all windows whose document is the same as the active
// window, or, if the active window does not have a document,
// close the active window only.
// Before any windows are closed, a SEL_CLOSEALL message is sent
// to the active window to ask if it is OK to close the windows.
long FXMDIClient::onCmdCloseDocument(FXObject* sender,FXSelector,void* ptr){
  register FXMDIChild *child,*nxtchild;
  register FXObject *document;
  if(active && active->handle(this,MKUINT(0,SEL_CLOSEALL),NULL)){
    document=active->getTarget();
    if(document){
      for(child=mdifirst; child; child=nxtchild){
        nxtchild=child->getMDINext();
        if(document==child->getTarget()){
          child->handle(sender,MKUINT(ID_DELETE,SEL_COMMAND),ptr);
          }
        }
      }
    else{
      active->handle(sender,MKUINT(ID_DELETE,SEL_COMMAND),ptr);
      }
    return 1;
    }
  return 0;
  }


// Close all windows. Before any windows are closed, a SEL_CLOSEALL
// message is sent to the window. The message is sent only once for
// each group of windows having the same document.
// If any single window does not want to be closed, no windows will
// be closed at all.
long FXMDIClient::onCmdCloseAllDocuments(FXObject* sender,FXSelector,void* ptr){
  register FXMDIChild *child,*nxtchild,*ch;
  register FXObject *document;
  for(child=mdifirst; child; child=child->getMDINext()){
    document=child->getTarget();
    if(document){
      for(ch=child->getMDIPrev(); ch; ch=ch->getMDIPrev()){
        if(ch->getTarget()==document) goto nxt;
        }
      }
    if(!child->handle(this,MKUINT(0,SEL_CLOSEALL),NULL)) return 0;
nxt:continue;
    }
  for(child=mdifirst; child; child=nxtchild){
    nxtchild=child->getMDINext();
    child->handle(sender,MKUINT(ID_DELETE,SEL_COMMAND),ptr);
    }
  return 1;
  }


// Update close current document
long FXMDIClient::onUpdCloseDocument(FXObject* sender,FXSelector,void*){
  if(active)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update close all documents
long FXMDIClient::onUpdCloseAllDocuments(FXObject* sender,FXSelector,void*){
  if(mdifirst)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
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
  for(child=mdifirst; child; child=child->getMDINext()){
    if(child->shown()){
      if(child->isMaximized()){
        xl=0;
        yl=0;
        xh=width;
        yh=height;
        }
      else if(child->isMinimized()){
        xl=child->getX();
        yl=child->getY();
        xh=xl+child->getDefaultWidth();
        yh=yl+child->getDefaultHeight();
        }
      else{
        xl=child->getX();
        yl=child->getY();
        xh=xl+child->getWidth();
        yh=yl+child->getHeight();
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
  FXint ww,hh,vw;

  // If active child is maximized, client is non-scrollable
  if(active && active->isMaximized()){
    xmin=0;
    ymin=0;
    xmax=width;
    ymax=height;
    flags&=~FLAG_RECALC;
    return width;
    }

  // Recalculate contents
  if(flags&FLAG_RECALC) recompute();

  // Return the content width
  vw=width;
  hh=FXMAX(ymax,height)-FXMIN(ymin,0);
  if(hh>height) vw-=vertical->getDefaultWidth();
  ww=FXMAX(xmax,vw)-FXMIN(xmin,0);
  return ww;
  }


// Determine content height of scroll area
FXint FXMDIClient::getContentHeight(){
  FXint ww,hh,vh;

  // If active child is maximized, client is non-scrollable
  if(active && active->isMaximized()){
    xmin=0;
    ymin=0;
    xmax=width;
    ymax=height;
    flags&=~FLAG_RECALC;
    return height;
    }

  // Recalculate contents
  if(flags&FLAG_RECALC) recompute();

  // Return the content height
  vh=height;
  ww=FXMAX(xmax,width)-FXMIN(xmin,0);
  if(ww>width) vh-=horizontal->getDefaultHeight();
  hh=FXMAX(ymax,vh)-FXMIN(ymin,0);
  return hh;
  }


// Move contents; moves child window
void FXMDIClient::moveContents(FXint x,FXint y){
  FXMDIChild *child;
  for(child=mdifirst; child; child=child->getMDINext()){
    child->move(child->getX()+x-pos_x,child->getY()+y-pos_y);
    }
  pos_x=x;
  pos_y=y;
  }


// Recalculate layout
void FXMDIClient::layout(){
  register FXMDIChild* child;
  register FXint xx,yy,ww,hh;

  // Place children
  for(child=mdifirst; child; child=child->getMDINext()){
    if(child->shown()){
      if(child->isMaximized()){
        xx=0;
        yy=0;
        ww=width;
        hh=height;
        }
      else if(child->isMinimized()){
        xx=child->getX();
        yy=child->getY();
        ww=child->getDefaultWidth();
        hh=child->getDefaultHeight();
        }
      else{
        xx=child->getX();
        yy=child->getY();
        ww=child->getWidth();
        hh=child->getHeight();
        }
      child->position(xx,yy,ww,hh);
      }
    }

  // Raise active child
  if(active && active->shown()) active->raise();

  FXTRACE((150,"xmin=%4d xmax=%4d\n",xmin,xmax));
  FXTRACE((150,"ymin=%4d ymax=%4d\n",ymin,ymax));

  // Recompute scroll bars, and raise them on top of everything
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

  // Set line size to some arbitrary amount
  vertical->setLine(20);
  horizontal->setLine(20);

  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Delegate all other messages to active child
long FXMDIClient::onDefault(FXObject* sender,FXSelector key,void* data){
  return active && active->handle(sender,key,data);
  }


// Save object to stream
void FXMDIClient::save(FXStream& store) const {
  FXScrollArea::save(store);
  store << mdifirst;
  store << mdilast;
  store << active;
  store << cascadex;
  store << cascadey;
  store << xmin;
  store << xmax;
  store << ymin;
  store << ymax;
  }


// Load object from stream
void FXMDIClient::load(FXStream& store){
  FXScrollArea::load(store);
  store >> mdifirst;
  store >> mdilast;
  store >> active;
  store >> cascadex;
  store >> cascadey;
  store >> xmin;
  store >> xmax;
  store >> ymin;
  store >> ymax;
  }


// Destruct thrashes object
FXMDIClient::~FXMDIClient(){
  mdifirst=(FXMDIChild*)-1;
  mdilast=(FXMDIChild*)-1;
  active=(FXMDIChild*)-1;
  }
