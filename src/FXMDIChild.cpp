/********************************************************************************
*                                                                               *
*          M u l t i p l e   D o c u m e n t   C h i l d   W i n d o w          *
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
* $Id: FXMDIChild.cpp,v 1.47 1998/10/30 15:49:38 jvz Exp $                   *
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
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenu.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXMDIButton.h"
#include "FXMDIChild.h"
#include "FXMDIClient.h"

  
/*
  Notes:
  - You should be able to just delete MDIChild windows!!
  - When minimized, minimize icon should change to restore.
  - When maximized, maximize icons should change to restore.
  - When clicking on title, focus should go to content...
  - Stacking order changes should be performed by MDIClient!
  - We make the MDIChild get the first crack at the messages, so that the
    contents of the MDIChild can not shadow any messages really directed at the
    MDIChild.
  - This means message ID's for the content window will have to start counting from
    numbers higher than both MDIClient and MDIChild. We hauled the MDIClient message
    ID's up to FXWindow to make sure that is the case.
  - Need options for MDI child decorations (close btn, window menu, min btn, max btn, title etc).
*/

#define BORDERWIDTH      6                                // MDI Child border width
#define HANDLESIZE       20                               // Resize handle length
#define MINWIDTH         80                               // Minimum width
#define MINHEIGHT        30                               // Minimum height



#define DRAGNONE         0            // No drag
#define DRAGTOP          1
#define DRAGBOTTOM       2
#define DRAGLEFT         4
#define DRAGRIGHT        8
#define DRAGTOPLEFT      (DRAGTOP|DRAGLEFT)
#define DRAGTOPRIGHT     (DRAGTOP|DRAGRIGHT)
#define DRAGBOTTOMLEFT   (DRAGBOTTOM|DRAGLEFT)
#define DRAGBOTTOMRIGHT  (DRAGBOTTOM|DRAGRIGHT)
#define DRAGTITLE        (DRAGTOP|DRAGBOTTOM|DRAGLEFT|DRAGRIGHT)
#define DRAGINVERTED     16


/*******************************************************************************/

// Map
FXDEFMAP(FXMDIChild) FXMDIChildMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMDIChild::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXMDIChild::onMotion),
  FXMAPFUNC(SEL_ENTER,0,FXMDIChild::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMDIChild::onLeave),
  FXMAPFUNC(SEL_FOCUSIN,0,FXMDIChild::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXMDIChild::onFocusOut),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMDIChild::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMDIChild::onLeftBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMDIChild::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMDIChild::onRightBtnRelease),
  };


// Object implementation
FXIMPLEMENT(FXMDIChild,FXComposite,FXMDIChildMap,ARRAYNUMBER(FXMDIChildMap))

  
// Create MDI Child Window
FXMDIChild::FXMDIChild(){
  windowbtn=(FXMenuButton*)-1;
  minimizebtn=(FXButton*)-1;
  restorebtn=(FXButton*)-1;
  maximizebtn=(FXButton*)-1;
  deletebtn=(FXButton*)-1;
  font=(FXFont*)-1;
  flags|=FLAG_ENABLED;
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  titleColor=0;
  titleBackColor=0;
  xoff=0;
  yoff=0;
  oldw=0;
  oldh=0;
  iconPosX=0;
  iconPosY=0;
  iconWidth=0;
  iconHeight=0;
  normalPosX=0;
  normalPosY=0;
  normalWidth=0;
  normalHeight=0;
  action=DRAGNONE;
  }


// Create MDI Child Window
FXMDIChild::FXMDIChild(FXMDIClient* p,const char* name,FXIcon* ic,FXMenuPane* mn,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h),title(name){
  flags|=FLAG_ENABLED;
  windowbtn=new FXMenuButton(this,NULL,ic,mn,MENUBUTTON_DOWN|MENUBUTTON_WINDOW,0,0,0,0,0,0,0,0);
  minimizebtn=new FXMDIButton(this,p,FXMDIClient::ID_MINIMIZE,FRAME_RAISED|MDIBTN_MINIMIZE);
  restorebtn=new FXMDIButton(this,p,FXMDIClient::ID_RESTORE,FRAME_RAISED|MDIBTN_RESTORE);
  maximizebtn=new FXMDIButton(this,p,FXMDIClient::ID_MAXIMIZE,FRAME_RAISED|MDIBTN_MAXIMIZE);
  deletebtn=new FXMDIButton(this,p,FXMDIClient::ID_DELETE,FRAME_RAISED|MDIBTN_DELETE);
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  titleColor=0;
  titleBackColor=0;
  font=getApp()->normalFont;
  xoff=0;
  yoff=0;
  oldw=0;
  oldh=0;
  iconPosX=xpos;
  iconPosY=ypos;
  iconWidth=0;
  iconHeight=0;
  normalPosX=xpos;
  normalPosY=ypos;
  normalWidth=width;
  normalHeight=height;
  action=DRAGNONE;
  }


// Create X window
void FXMDIChild::create(){
  FXComposite::create();
  baseColor=acquireColor(getApp()->backColor);
  hiliteColor=acquireColor(getApp()->hiliteColor);
  shadowColor=acquireColor(getApp()->shadowColor);
  borderColor=acquireColor(getApp()->borderColor);
  titleColor=acquireColor(getApp()->selforeColor);
  titleBackColor=acquireColor(getApp()->selbackColor);
  font->create();
  if(((FXMDIClient*)getParent())->isMaximized()) maximize();
  ((FXMDIClient*)getParent())->setActiveChild(this);
  recalc();
  raise();
  show();
  }


// Get content window (if any!)
FXWindow *FXMDIChild::contentWindow() const {
  return deletebtn->getNext();
  }


// Get title height
FXint FXMDIChild::getTitleHeight() const {
  FXint t,h;
  h=font->getFontHeight();
  if((t=windowbtn->getDefaultHeight())>h) h=t;
  if((t=deletebtn->getDefaultHeight())>h) h=t;
  return h+4;
  }


// Get title width
FXint FXMDIChild::getTitleWidth() const {
  FXint w;
  w=windowbtn->getDefaultWidth()+3*deletebtn->getDefaultWidth();
  if(title.text()) w+=font->getTextWidth(title.text(),title.length());
  return w+16;
  }


// Get width
FXint FXMDIChild::getDefaultWidth(){
  FXWindow *contents=contentWindow();
  FXint tw=getTitleWidth();
  FXint cw=0;
  if(!isMinimized() && contents) cw=contents->getWidth();
  return FXMAX(cw,tw)+(BORDERWIDTH<<1);
  }


// Get height
FXint FXMDIChild::getDefaultHeight(){
  FXWindow *contents=contentWindow();
  FXint th=getTitleHeight();
  FXint ch=0;
  if(!isMinimized() && contents) ch=contents->getHeight();
  return th+ch+(BORDERWIDTH<<1);
  }


// Just tell server where the windows are!
void FXMDIChild::layout(){
  FXWindow *contents=contentWindow();
  FXint th,bw,bh,by,bx;
  th=getTitleHeight();
  bw=deletebtn->getDefaultWidth();
  bh=deletebtn->getDefaultHeight();
  bx=width-BORDERWIDTH-bw-1;
  by=BORDERWIDTH+(th-bh)/2;
  windowbtn->position(BORDERWIDTH+1,BORDERWIDTH+(th-windowbtn->getDefaultHeight())/2,windowbtn->getDefaultWidth(),windowbtn->getDefaultHeight());
  if(options&MDI_MAXIMIZED){
    deletebtn->hide();
    maximizebtn->hide();
    minimizebtn->hide();
    restorebtn->hide();
    if(contents){
      contents->position(0,0,width,height);
      contents->raise();
      contents->show();
      }
    }
  else if(options&MDI_MINIMIZED){
    deletebtn->position(bx,by,bw,bh); bx-=bw+3;
    maximizebtn->position(bx,by,bw,bh); bx-=bw+3;
    restorebtn->position(bx,by,bw,bh);
    deletebtn->show();
    maximizebtn->show();
    minimizebtn->hide();
    restorebtn->show();
    if(contents){
      contents->hide();
      }
    }
  else{
    deletebtn->position(bx,by,bw,bh); bx-=bw+3;
    maximizebtn->position(bx,by,bw,bh); bx-=bw+3;
    minimizebtn->position(bx,by,bw,bh);
    deletebtn->show();
    maximizebtn->show();
    minimizebtn->show();
    restorebtn->hide();
    if(contents){
      contents->position(BORDERWIDTH,BORDERWIDTH+th,width-(BORDERWIDTH<<1),height-th-(BORDERWIDTH<<1)-1);
      contents->show();
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Is it maximized?
FXbool FXMDIChild::isMaximized() const {
  return (options&MDI_MAXIMIZED)!=0;
  }


// Is it minimized
FXbool FXMDIChild::isMinimized() const {
  return (options&MDI_MINIMIZED)!=0;
  }


// Maximize window
void FXMDIChild::maximize(){
  if(!(options&MDI_MAXIMIZED)){
    options|=MDI_MAXIMIZED;
    options&=~MDI_MINIMIZED;
    recalc();
    }
  }


// Minimize window
void FXMDIChild::minimize(){
  if(!(options&MDI_MINIMIZED)){
    options|=MDI_MINIMIZED;
    options&=~MDI_MAXIMIZED;
    recalc();
    }
  }


// Restore window
void FXMDIChild::restore(){
  if(options&(MDI_MINIMIZED|MDI_MAXIMIZED)){
    options&=~(MDI_MINIMIZED|MDI_MAXIMIZED);
    recalc();
    }
  }


// Move and resize
void FXMDIChild::position(FXint x,FXint y,FXint w,FXint h){
  if(options&MDI_MINIMIZED){
    iconPosX=x;
    iconPosY=y;
    iconWidth=w=getDefaultWidth();
    iconHeight=h=getDefaultHeight();
    }
  else if(!(options&MDI_MAXIMIZED)){
    normalPosX=x;
    normalPosY=y;
    normalWidth=w;
    normalHeight=h;
    }
  FXWindow::position(x,y,w,h);
  }


// Move window
void FXMDIChild::move(int x,int y){
  FXWindow::move(x,y);
  if(options&MDI_MINIMIZED){
    iconPosX=xpos;
    iconPosY=ypos;
    }
  else if(!(options&MDI_MAXIMIZED)){
    normalPosX=xpos;
    normalPosY=ypos;
    }
  }


// Resize
void FXMDIChild::resize(int w,int h){
  if(options&MDI_MINIMIZED){
    iconWidth=w=getDefaultWidth();
    iconHeight=h=getDefaultHeight();
    }
  else if(!(options&MDI_MAXIMIZED)){
    normalWidth=width;
    normalHeight=height;
    }
  FXWindow::resize(w,h);
  }



// If window can have focus
FXbool FXMDIChild::canFocus() const { 
  return TRUE; 
  }


// Change cursor based on location over window
void FXMDIChild::changeCursor(FXint x,FXint y){
  switch(where(x,y)){
    case DRAGTOP:
    case DRAGBOTTOM:
      setDefaultCursor(getApp()->dragHCursor);
      setDragCursor(getApp()->dragHCursor);
      break;
    case DRAGLEFT:
    case DRAGRIGHT:
      setDefaultCursor(getApp()->dragVCursor);
      setDragCursor(getApp()->dragVCursor);
      break;
    case DRAGTOPLEFT:
    case DRAGBOTTOMRIGHT:
      setDefaultCursor(getApp()->dragTLCursor);
      setDragCursor(getApp()->dragTLCursor);
      break;
    case DRAGTOPRIGHT:
    case DRAGBOTTOMLEFT:
      setDefaultCursor(getApp()->dragTRCursor);
      setDragCursor(getApp()->dragTRCursor);
      break;
    default:
      setDefaultCursor(getApp()->arrowCursor);
      setDragCursor(getApp()->arrowCursor);
      break;
    }
  }


// Revert cursor to normal one
void FXMDIChild::revertCursor(){
  setDefaultCursor(getApp()->arrowCursor);
  setDragCursor(getApp()->arrowCursor);
  }


// Enter
long FXMDIChild::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXComposite::onEnter(sender,sel,ptr);
  return 1;
  }


// Leave
long FXMDIChild::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXComposite::onLeave(sender,sel,ptr);
  return 1;
  }


// Make active
void FXMDIChild::setActive(){
  flags|=FLAG_ACTIVE;
  windowbtn->setBackColor(hasFocus() ? titleBackColor : shadowColor);
  update(0,0,width,height);
  }


// Make inactive
void FXMDIChild::setInactive(){
  flags&=~FLAG_ACTIVE;
  windowbtn->setBackColor(backColor);
  update(0,0,width,height);
  }


// Setting focus to the MDI Child will also make it active
void FXMDIChild::setFocus(){
  ((FXMDIClient*)getParent())->setActiveChild(this);
  FXComposite::setFocus();
  }


// Gained focus
long FXMDIChild::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onFocusIn\n",getClassName());
  FXComposite::onFocusIn(sender,sel,ptr);
  windowbtn->setBackColor(titleBackColor);
  update(BORDERWIDTH,BORDERWIDTH,width-(BORDERWIDTH<<1),BORDERWIDTH+font->getFontHeight()+1);
  return 1;
  }

  
// Lost focus
long FXMDIChild::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onFocusOut\n",getClassName());
  FXComposite::onFocusOut(sender,sel,ptr);
  windowbtn->setBackColor(shadowColor);
  update(BORDERWIDTH,BORDERWIDTH,width-(BORDERWIDTH<<1),BORDERWIDTH+font->getFontHeight()+1);
  return 1;
  }


// Draw 2 pixel double raised border
void FXMDIChild::drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(baseColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(hiliteColor);
  drawLine(x+1,y+1,x+w-3,y+1);
  drawLine(x+1,y+1,x+1,y+h-3);
  setForeground(shadowColor);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+h-2,x+w-2,y+1);
  setForeground(borderColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }


// Draw 1 pixel raised border
void FXMDIChild::drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  }


// Draw rubberband box
void FXMDIChild::drawRubberBox(FXint x,FXint y,FXint w,FXint h){
  FXint xx,yy;
  //getParent()->setFunction(BLT_NOT_DST);     // Does this always show up?
  getParent()->setFunction(BLT_SRC_XOR_DST);     // Does this always show up?
  setForeground(getParent()->getBackColor());
  getParent()->clipByChildren(FALSE);
  //getParent()->setClipRectangle(0,0,((FXMDIClient*)getParent())->viewport_w,((FXMDIClient*)getParent())->viewport_h);
  translateCoordinatesTo(xx,yy,getParent(),x,y);
  getParent()->drawHashBox(xx,yy,w,h,BORDERWIDTH);
  //getParent()->drawLine(xx,yy,xx+w-1,yy);
  //getParent()->drawLine(xx,yy,xx,yy+h-1);
  //getParent()->drawLine(xx,yy+h-1,xx+w-1,yy+h-1);
  //getParent()->drawLine(xx+w-1,yy,xx+w-1,yy+h-1);
  getParent()->clipByChildren(TRUE);
  getParent()->clearClipRectangle();
  getParent()->setFunction(BLT_SRC);
  }


// Handle repaint 
long FXMDIChild::onPaint(FXObject*,FXSelector,void*){
  FXint xx,yy,th,fh;
  th=getTitleHeight();
  fh=font->getFontHeight();
  xx=BORDERWIDTH+windowbtn->getDefaultWidth()+4;
  yy=BORDERWIDTH+font->getFontAscent()+(th-fh)/2;
  drawDoubleRaisedRectangle(0,0,width,height);
  if(isActive()){
    setForeground(hasFocus() ? titleBackColor : shadowColor);
    fillRectangle(BORDERWIDTH,BORDERWIDTH,width-BORDERWIDTH*2,th);
    if(title.text()){
      setTextFont(font);
      setForeground(titleColor);
      drawText(xx,yy,title.text(),title.length());
      }
    }
  else{
    setForeground(backColor);
    fillRectangle(BORDERWIDTH,BORDERWIDTH,width-BORDERWIDTH*2,height-BORDERWIDTH*2);
    setForeground(borderColor);
    if(title.text()){
      setTextFont(font);
      drawText(xx,yy,title.text(),title.length());
      }
    }
  return 1;
  }


// Find out where window was grabbed
FXuchar FXMDIChild::where(FXint x,FXint y){
  FXint th=getTitleHeight();
  FXuchar code=DRAGNONE;
  if(!isMinimized() && x<HANDLESIZE) code|=DRAGLEFT; 
  if(!isMinimized() && width-HANDLESIZE<=x) code|=DRAGRIGHT; 
  if(!isMinimized() && y<HANDLESIZE) code|=DRAGTOP; 
  if(!isMinimized() && height-HANDLESIZE<=y) code|=DRAGBOTTOM; 
  if(BORDERWIDTH<=x && x<=width-BORDERWIDTH && BORDERWIDTH<=y && y<BORDERWIDTH+th) code=DRAGTITLE;
  return code;
  }


// Pressed LEFT button 
long FXMDIChild::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(canFocus()) setFocus();
  //raise();
  //getApp()->flush();/////////
  if(action==DRAGNONE){
    action=where(event->win_x,event->win_y);
    oldx=0;
    oldy=0;
    oldw=width;
    oldh=height;
    switch(action){
      case DRAGTOP:
        yoff=event->win_y;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGBOTTOM:
        yoff=event->win_y-oldh;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGLEFT:
        xoff=event->win_x;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGRIGHT:
        xoff=event->win_x-oldw;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGTOPLEFT:
        xoff=event->win_x;
        yoff=event->win_y;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGTOPRIGHT:
        xoff=event->win_x-oldw;
        yoff=event->win_y;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGBOTTOMLEFT:
        xoff=event->win_x;
        yoff=event->win_y-oldh;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGBOTTOMRIGHT:
        xoff=event->win_x-oldw;
        yoff=event->win_y-oldh;
        drawRubberBox(0,0,oldw,oldh);
        action|=DRAGINVERTED;
        flags|=FLAG_PRESSED;
        grab();
        break;
      case DRAGTITLE:
        xoff=event->win_x;
        yoff=event->win_y;
        flags|=FLAG_PRESSED;
        grab();
        break;
      }
    }
  return 1;
  }


// Released LEFT button 
long FXMDIChild::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(flags&FLAG_PRESSED){
    if(action!=DRAGNONE){
      FXint xx,yy;
      translateCoordinatesTo(xx,yy,getParent(),oldx,oldy);
      if(action&DRAGINVERTED) drawRubberBox(oldx,oldy,oldw,oldh);
      position(xx,yy,oldw,oldh);
      action=DRAGNONE;
      recalc();
      ungrab();
      }
    flags&=~FLAG_PRESSED;
    }
  if(event->click_count==2){
    if(isMinimized()){
      getParent()->handle(this,MKUINT(FXMDIClient::ID_RESTORE,SEL_COMMAND),ptr);
      }
    else if(!isMaximized()){
      getParent()->handle(this,MKUINT(FXMDIClient::ID_MAXIMIZE,SEL_COMMAND),ptr);
      }
    }
  return 1;
  }


// Moved
long FXMDIChild::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint tmp;
  if(flags&FLAG_PRESSED){
    if(action&DRAGINVERTED) drawRubberBox(oldx,oldy,oldw,oldh);
    action&=~DRAGINVERTED;
    switch(action){
      case DRAGTOP:
        tmp=oldh+oldy-event->win_y+yoff;
        if(tmp>=MINHEIGHT){ oldh=tmp; oldy=event->win_y-yoff; }
        break;
      case DRAGBOTTOM:
        tmp=event->win_y-yoff-oldy;
        if(tmp>=MINHEIGHT){ oldh=tmp; }
        break;
      case DRAGLEFT:
        tmp=oldw+oldx-event->win_x+xoff;
        if(tmp>=MINWIDTH){ oldw=tmp; oldx=event->win_x-xoff; }
        break;
      case DRAGRIGHT:
        tmp=event->win_x-xoff-oldx;
        if(tmp>=MINWIDTH){ oldw=tmp; }
        break;
      case DRAGTOPLEFT:
        tmp=oldw+oldx-event->win_x+xoff;
        if(tmp>=MINWIDTH){ oldw=tmp; oldx=event->win_x-xoff; }
        tmp=oldh+oldy-event->win_y+yoff;
        if(tmp>=MINHEIGHT){ oldh=tmp; oldy=event->win_y-yoff; }
        break;
      case DRAGTOPRIGHT:
        tmp=event->win_x-xoff-oldx;
        if(tmp>=MINWIDTH){ oldw=tmp; }
        tmp=oldh+oldy-event->win_y+yoff;
        if(tmp>=MINHEIGHT){ oldh=tmp; oldy=event->win_y-yoff; }
        break;
      case DRAGBOTTOMLEFT:
        tmp=oldw+oldx-event->win_x+xoff;
        if(tmp>=MINWIDTH){ oldw=tmp; oldx=event->win_x-xoff; }
        tmp=event->win_y-yoff-oldy;
        if(tmp>=MINHEIGHT){ oldh=tmp; }
        break;
      case DRAGBOTTOMRIGHT:
        tmp=event->win_x-xoff-oldx;
        if(tmp>=MINWIDTH){ oldw=tmp; }
        tmp=event->win_y-yoff-oldy;
        if(tmp>=MINHEIGHT){ oldh=tmp; }
        break;
      case DRAGTITLE:
        if(!event->moved) return 1;       // Have not yet moved enough to qualify as a drag
        oldx=event->win_x-xoff;
        oldy=event->win_y-yoff;
        setDragCursor(getApp()->moveCursor);
        break;
      }
    drawRubberBox(oldx,oldy,oldw,oldh);
    action|=DRAGINVERTED;
    return 1;
    }
  else{
    changeCursor(event->win_x,event->win_y);
    }
  return 0;
  }


// Pressed RIGHT button 
long FXMDIChild::onRightBtnPress(FXObject*,FXSelector,void*){
  if(canFocus()) setFocus();
  lower();
  getApp()->flush();/////////
  return 1;
  }


// Released RIGHT button 
long FXMDIChild::onRightBtnRelease(FXObject*,FXSelector,void*){
  return 1;
  }


// Set base color
void FXMDIChild::setBaseColor(FXPixel clr){
  baseColor=clr;
  update(0,0,width,height);
  }


// Set highlight color
void FXMDIChild::setHiliteColor(FXPixel clr){
  hiliteColor=clr;
  update(0,0,width,height);
  }


// Set shadow color
void FXMDIChild::setShadowColor(FXPixel clr){
  shadowColor=clr;
  update(0,0,width,height);
  }


// Set new window title
void FXMDIChild::setTitle(const char* name){
  title=name;
  update(0,0,width,height);
  }


// Handle message
long FXMDIChild::handle(FXObject* sender,FXSelector key,void* data){
  FXWindow *contents=contentWindow();
  
  // Filter out messages for MDIChild; the contents window
  // should not be able to catch any of MDIChild's messages
  if(FXComposite::handle(sender,key,data)) return 1;
  
  // Unknown messages are forwarded to contents of MDIChild
  return ID_LAST<=SELID(key) && contents && contents->handle(sender,key,data);
  }


// Get icon used for the menu button
FXIcon *FXMDIChild::getWindowIcon() const {
  return windowbtn->getIcon();
  }


// Change icon used for window menu button
void FXMDIChild::setWindowIcon(FXIcon* ic){
  windowbtn->setIcon(ic);
  }


// Obtain window menu
FXMenuPane* FXMDIChild::getWindowMenu() const {
  return windowbtn->getPopup();
  }


// Change window menu
void FXMDIChild::setWindowMenu(FXMenuPane* menu){
  windowbtn->setPopup(menu);
  }


// Change the font
void FXMDIChild::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update(0,0,width,height);
    }
  }


// Destruct thrashes the pointers
FXMDIChild::~FXMDIChild(){
  //if(getParent()->active==this) getParent()->active=NULL;
  windowbtn=(FXMenuButton*)-1;
  minimizebtn=(FXButton*)-1;
  restorebtn=(FXButton*)-1;
  maximizebtn=(FXButton*)-1;
  deletebtn=(FXButton*)-1;
  font=(FXFont*)-1;
  }
