/********************************************************************************
*                                                                               *
*          M u l t i p l e   D o c u m e n t   C h i l d   W i n d o w          *
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
* $Id: FXMDIChild.cpp,v 1.36.4.1 2002/07/17 01:58:32 fox Exp $                   *
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
#include "FXDCWindow.h"
#include "FXFont.h"
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
#include "FXMenuPane.h"
#include "FXScrollbar.h"
#include "FXScrollArea.h"
#include "FXMDIButton.h"
#include "FXMDIChild.h"
#include "FXMDIClient.h"


/*
  Notes:
  - Stacking order changes should be performed by MDIClient!
  - Need options for MDI child decorations (close btn, window menu, min btn, max btn,
    title etc).
  - Iconified version should be fixed size, showing as much of title as feasible
    (tail with ...'s)
  - Initial icon placement on the bottom of the MDIClient somehow...
*/

#define BORDERWIDTH      5                          // MDI Child border width
#define HANDLESIZE       20                         // Resize handle length
#define MINWIDTH         80                         // Minimum width
#define MINHEIGHT        30                         // Minimum height
#define TITLESPACE       120                        // Width of title when minimized


#define DRAGNONE         0                          // No drag
#define DRAGTOP          1                          // Drag top side
#define DRAGBOTTOM       2                          // Drag bottom side
#define DRAGLEFT         4                          // Drag left side
#define DRAGRIGHT        8                          // Drag right side
#define DRAGTOPLEFT      (DRAGTOP|DRAGLEFT)         // Drag top left
#define DRAGTOPRIGHT     (DRAGTOP|DRAGRIGHT)        // Drag top right
#define DRAGBOTTOMLEFT   (DRAGBOTTOM|DRAGLEFT)      // Drag bottom left
#define DRAGBOTTOMRIGHT  (DRAGBOTTOM|DRAGRIGHT)     // Drag bottom right
#define DRAGTITLE        (DRAGTOP|DRAGBOTTOM|DRAGLEFT|DRAGRIGHT)    // Drag title
#define DRAGINVERTED     16                         // Drag rectangle is inverted

#define MDI_MASK         (MDI_MAXIMIZED|MDI_MINIMIZED)


/*******************************************************************************/

// Map
FXDEFMAP(FXMDIChild) FXMDIChildMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMDIChild::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXMDIChild::onMotion),
  FXMAPFUNC(SEL_FOCUSIN,0,FXMDIChild::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXMDIChild::onFocusOut),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMDIChild::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMDIChild::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMDIChild::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMDIChild::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMDIChild::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMDIChild::onRightBtnRelease),
  FXMAPFUNC(SEL_CLOSE,0,FXMDIChild::onClose),
  FXMAPFUNC(SEL_CLOSEALL,0,FXMDIChild::onCloseAll),
  FXMAPFUNC(SEL_DELETE,0,FXMDIChild::onDelete),
  FXMAPFUNC(SEL_SELECTED,0,FXMDIChild::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXMDIChild::onDeselected),
  FXMAPFUNC(SEL_MINIMIZE,0,FXMDIChild::onMinimize),
  FXMAPFUNC(SEL_RESTORE,0,FXMDIChild::onRestore),
  FXMAPFUNC(SEL_MAXIMIZE,0,FXMDIChild::onMaximize),
  FXMAPFUNC(SEL_FOCUS_SELF,0,FXMDIChild::onFocusSelf),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_CLOSE,FXMDIChild::onUpdClose),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MAXIMIZE,FXMDIChild::onUpdMaximize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MINIMIZE,FXMDIChild::onUpdMinimize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_RESTORE,FXMDIChild::onUpdRestore),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_WINDOW,FXMDIChild::onUpdWindow),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENUCLOSE,FXMDIChild::onUpdMenuClose),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENUMINIMIZE,FXMDIChild::onUpdMenuMinimize),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENURESTORE,FXMDIChild::onUpdMenuRestore),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_MDI_MENUWINDOW,FXMDIChild::onUpdMenuWindow),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_DELETE,FXMDIChild::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_CLOSE,FXMDIChild::onCmdClose),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_MAXIMIZE,FXMDIChild::onCmdMaximize),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_MINIMIZE,FXMDIChild::onCmdMinimize),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_RESTORE,FXMDIChild::onCmdRestore),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_MENUMINIMIZE,FXMDIChild::onCmdMinimize),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_MENURESTORE,FXMDIChild::onCmdRestore),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_MDI_MENUCLOSE,FXMDIChild::onCmdClose),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXMDIChild::onCmdSetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXMDIChild,FXComposite,FXMDIChildMap,ARRAYNUMBER(FXMDIChildMap))


/*******************************************************************************/


// Serialization
FXMDIChild::FXMDIChild(){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  mdinext=(FXMDIChild*)-1;
  mdiprev=(FXMDIChild*)-1;
  windowbtn=(FXMenuButton*)-1;
  minimizebtn=(FXButton*)-1;
  restorebtn=(FXButton*)-1;
  maximizebtn=(FXButton*)-1;
  deletebtn=(FXButton*)-1;
  font=(FXFont*)-1;
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  titleColor=0;
  titleBackColor=0;
  iconPosX=0;
  iconPosY=0;
  iconWidth=0;
  iconHeight=0;
  normalPosX=0;
  normalPosY=0;
  normalWidth=0;
  normalHeight=0;
  xoff=0;
  yoff=0;
  oldx=0;
  oldy=0;
  oldw=0;
  oldh=0;
  action=DRAGNONE;
  }


// Create MDI Child Window
FXMDIChild::FXMDIChild(FXMDIClient* p,const FXString& name,FXIcon* ic,FXMenuPane* mn,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h),title(name){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  mdiprev=p->mdilast;
  mdinext=NULL;
  p->mdilast=this;
  if(mdiprev) mdiprev->mdinext=this; else p->mdifirst=this;
  windowbtn=new FXMDIWindowButton(this,this,FXWindow::ID_MDI_WINDOW);
  minimizebtn=new FXMDIMinimizeButton(this,this,FXWindow::ID_MDI_MINIMIZE,FRAME_RAISED);
  restorebtn=new FXMDIRestoreButton(this,this,FXWindow::ID_MDI_RESTORE,FRAME_RAISED);
  maximizebtn=new FXMDIMaximizeButton(this,this,FXWindow::ID_MDI_MAXIMIZE,FRAME_RAISED);
  deletebtn=new FXMDIDeleteButton(this,this,FXWindow::ID_MDI_CLOSE,FRAME_RAISED);
  windowbtn->setMenu(mn);
  windowbtn->setIcon(ic);
  baseColor=getApp()->getBaseColor();
  hiliteColor=getApp()->getHiliteColor();
  shadowColor=getApp()->getShadowColor();
  borderColor=getApp()->getBorderColor();
  titleColor=getApp()->getSelforeColor();
  titleBackColor=getApp()->getSelbackColor();
  font=getApp()->getNormalFont();
  iconPosX=xpos;
  iconPosY=ypos;
  iconWidth=width;
  iconHeight=height;
  normalPosX=xpos;
  normalPosY=ypos;
  normalWidth=width;
  normalHeight=height;
  if(options&(MDI_MAXIMIZED|MDI_MINIMIZED)){
    normalWidth=p->getWidth()*2/3;
    normalHeight=p->getHeight()*2/3;
    if(normalWidth<8) normalWidth=200;
    if(normalHeight<8) normalHeight=160;
    }
  xoff=0;
  yoff=0;
  oldx=0;
  oldy=0;
  oldw=0;
  oldh=0;
  action=DRAGNONE;
  }


// Create window
void FXMDIChild::create(){
  FXComposite::create();
  font->create();
  recalc();
  }


// Detach window
void FXMDIChild::detach(){
  FXComposite::detach();
  font->detach();
  }


// Get content window (if any!)
FXWindow *FXMDIChild::contentWindow() const {
  return deletebtn->getNext();
  }


// Get width
FXint FXMDIChild::getDefaultWidth(){
  FXint mw,bw;
  mw=windowbtn->getDefaultWidth();
  bw=deletebtn->getDefaultWidth();
  return TITLESPACE+mw+3*bw+(BORDERWIDTH<<1)+2+4+4+6+2;
  }


// Get height
FXint FXMDIChild::getDefaultHeight(){
  FXint fh,mh,bh;
  fh=font->getFontHeight();
  mh=windowbtn->getDefaultHeight();
  bh=deletebtn->getDefaultHeight();
  return FXMAX3(fh,mh,bh)+(BORDERWIDTH<<1)+4;
  }


// Just tell server where the windows are!
void FXMDIChild::layout(){
  FXWindow *contents=contentWindow();
  FXint th,fh,mw,mh,bw,bh,by,bx;
  fh=font->getFontHeight();
  mw=windowbtn->getDefaultWidth();
  mh=windowbtn->getDefaultHeight();
  bw=deletebtn->getDefaultWidth();
  bh=deletebtn->getDefaultHeight();
  th=FXMAX3(fh,mh,bh)+4;
  bx=width-BORDERWIDTH-bw-2;
  by=BORDERWIDTH+(th-bh)/2;
  windowbtn->position(BORDERWIDTH+2,BORDERWIDTH+(th-mh)/2,mw,mh);
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
void FXMDIChild::maximize(FXbool notify){
  if(!(options&MDI_MAXIMIZED)){
    if(options&MDI_MINIMIZED){
      iconPosX=xpos;
      iconPosY=ypos;
      iconWidth=width;
      iconHeight=height;
      }
    else{
      normalPosX=xpos;
      normalPosY=ypos;
      normalWidth=width;
      normalHeight=height;
      }
    xpos=0;
    ypos=0;
    width=getParent()->getWidth();
    height=getParent()->getHeight();
    options|=MDI_MAXIMIZED;
    options&=~MDI_MINIMIZED;
    if(notify){handle(this,MKUINT(0,SEL_MAXIMIZE),NULL);}
    recalc();
    }
  }


// Minimize window
void FXMDIChild::minimize(FXbool notify){
  if(!(options&MDI_MINIMIZED)){
    if(!(options&MDI_MAXIMIZED)){
      normalPosX=xpos;
      normalPosY=ypos;
      normalWidth=width;
      normalHeight=height;
      }
    xpos=iconPosX;
    ypos=iconPosY;
    width=getDefaultWidth();
    height=getDefaultHeight();
    options|=MDI_MINIMIZED;
    options&=~MDI_MAXIMIZED;
    if(notify){handle(this,MKUINT(0,SEL_MINIMIZE),NULL);}
    recalc();
    }
  }


// Restore window
void FXMDIChild::restore(FXbool notify){
  if(options&(MDI_MINIMIZED|MDI_MAXIMIZED)){
    if(options&MDI_MINIMIZED){
      iconPosX=xpos;
      iconPosY=ypos;
      iconWidth=width;
      iconHeight=height;
      }
    xpos=normalPosX;
    ypos=normalPosY;
    width=normalWidth;
    height=normalHeight;
    options&=~(MDI_MINIMIZED|MDI_MAXIMIZED);
    if(notify){handle(this,MKUINT(0,SEL_RESTORE),NULL);}
    recalc();
    }
  }


// Move this window to the specified position in the parent's coordinates
void FXMDIChild::move(FXint x,FXint y){
  FXComposite::move(x,y);
  if(!(options&(MDI_MAXIMIZED|MDI_MINIMIZED))){
    normalPosX=x;
    normalPosY=y;
    }
  else if(options&MDI_MINIMIZED){
    iconPosX=x;
    iconPosY=y;
    }
  }


// Resize this window to the specified width and height
void FXMDIChild::resize(FXint w,FXint h){
  FXComposite::resize(w,h);
  if(!(options&(MDI_MAXIMIZED|MDI_MINIMIZED))){
    normalWidth=w;
    normalHeight=h;
    }
  else if(options&MDI_MINIMIZED){
    iconWidth=w;
    iconHeight=h;
    }
  }


// Move and resize this window in the parent's coordinates
void FXMDIChild::position(FXint x,FXint y,FXint w,FXint h){
  FXComposite::position(x,y,w,h);
  if(!(options&(MDI_MAXIMIZED|MDI_MINIMIZED))){
    normalPosX=x;
    normalPosY=y;
    normalWidth=w;
    normalHeight=h;
    }
  else if(options&MDI_MINIMIZED){
    iconPosX=x;
    iconPosY=y;
    iconWidth=w;
    iconHeight=h;
    }
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
      setDefaultCursor(getApp()->getDefaultCursor(DEF_DRAGH_CURSOR));
      setDragCursor(getApp()->getDefaultCursor(DEF_DRAGH_CURSOR));
      break;
    case DRAGLEFT:
    case DRAGRIGHT:
      setDefaultCursor(getApp()->getDefaultCursor(DEF_DRAGV_CURSOR));
      setDragCursor(getApp()->getDefaultCursor(DEF_DRAGV_CURSOR));
      break;
    case DRAGTOPLEFT:
    case DRAGBOTTOMRIGHT:
      setDefaultCursor(getApp()->getDefaultCursor(DEF_DRAGTL_CURSOR));
      setDragCursor(getApp()->getDefaultCursor(DEF_DRAGTL_CURSOR));
      break;
    case DRAGTOPRIGHT:
    case DRAGBOTTOMLEFT:
      setDefaultCursor(getApp()->getDefaultCursor(DEF_DRAGTR_CURSOR));
      setDragCursor(getApp()->getDefaultCursor(DEF_DRAGTR_CURSOR));
      break;
    default:
      setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
      setDragCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
      break;
    }
  }


// Revert cursor to normal one
void FXMDIChild::revertCursor(){
  setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
  setDragCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
  }


// Draw rubberband box
void FXMDIChild::drawRubberBox(FXint x,FXint y,FXint w,FXint h){
  if(BORDERWIDTH*2<w && BORDERWIDTH*2<h){
    FXDCWindow dc(getParent());
    FXint xx,yy;
    dc.clipChildren(FALSE);
    dc.setFunction(BLT_SRC_XOR_DST);
    dc.setForeground(getParent()->getBackColor());
    translateCoordinatesTo(xx,yy,getParent(),x,y);
   // dc.drawHashBox(xx,yy,w,h,BORDERWIDTH);
    dc.setLineWidth(BORDERWIDTH);
    dc.drawRectangle(xx+BORDERWIDTH/2,yy+BORDERWIDTH/2,w-BORDERWIDTH,h-BORDERWIDTH);
    }
  }


// Draw animation morphing from old to new rectangle
void FXMDIChild::animateRectangles(FXint ox,FXint oy,FXint ow,FXint oh,FXint nx,FXint ny,FXint nw,FXint nh){
  if(xid && getApp()->getAnimSpeed()){
    FXDCWindow dc(getParent());
    FXint bx,by,bw,bh,s,t;
    dc.clipChildren(FALSE);
    dc.setFunction(BLT_SRC_XOR_DST);
    dc.setForeground(getParent()->getBackColor());
    for(s=0,t=100; s<=100; s+=5,t-=5){
      bx=(nx*s+ox*t)/100;
      by=(ny*s+oy*t)/100;
      bw=(nw*s+ow*t)/100;
      bh=(nh*s+oh*t)/100;
      if(BORDERWIDTH*2<bw && BORDERWIDTH*2<bh){
        dc.drawHashBox(bx,by,bw,bh,BORDERWIDTH);
        getApp()->flush(TRUE);
        fxsleep(getApp()->getAnimSpeed()*1000);
        dc.drawHashBox(bx,by,bw,bh,BORDERWIDTH);
        getApp()->flush(TRUE);
        }
      }
    }
  }


// Handle repaint
long FXMDIChild::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXint xx,yy,tw,th,titlespace,letters,dots,dotspace;
  FXint fh,mh,bh,bw,mw;
  FXDCWindow dc(this,ev);

  // Draw MDIChild background
  dc.setForeground(baseColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);

  // Only draw stuff when not maximized
  if(!(options&MDI_MAXIMIZED)){
    fh=font->getFontHeight();
    mw=windowbtn->getDefaultWidth();
    mh=windowbtn->getDefaultHeight();
    bw=deletebtn->getDefaultWidth();
    bh=deletebtn->getDefaultHeight();
    th=FXMAX3(fh,mh,bh)+4;


    // Draw border
    dc.setForeground(baseColor);
    dc.drawLine(0,0,width-2,0);
    dc.drawLine(0,0,0,height-2);
    dc.setForeground(hiliteColor);
    dc.drawLine(1,0+1,width-3,1);
    dc.drawLine(1,0+1,1,height-3);
    dc.setForeground(shadowColor);
    dc.drawLine(1,height-2,width-2,height-2);
    dc.drawLine(width-2,height-2,width-2,01);
    dc.setForeground(borderColor);
    dc.drawLine(0,height-1,width-1,height-1);
    dc.drawLine(width-1,0,width-1,height-1);

    // Draw title background
    dc.setForeground(isActive() ? (hasFocus() ? titleBackColor : shadowColor) : backColor);
    dc.fillRectangle(BORDERWIDTH,BORDERWIDTH,width-BORDERWIDTH*2,th);

    // Draw title
    if(!title.empty()){
      xx=BORDERWIDTH+mw+2+4;
      yy=BORDERWIDTH+font->getFontAscent()+(th-fh)/2;

      // Compute space for title
      titlespace=width-mw-3*bw-(BORDERWIDTH<<1)-2-4-4-6-2;

      dots=0;
      letters=title.length();

      // Title too large for space
      if(font->getTextWidth(title.text(),letters)>titlespace){
        dotspace=titlespace-font->getTextWidth("...",3);
        while(letters>0 && (tw=font->getTextWidth(title.text(),letters))>dotspace) letters--;
        dots=3;
        if(letters==0){
          letters=1;
          dots=0;
          }
        }

      // Draw as much of the title as possible
      dc.setForeground(isActive() ? titleColor : borderColor);
      dc.setTextFont(font);
      dc.drawText(xx,yy,title.text(),letters);
      dc.drawText(xx+font->getTextWidth(title.text(),letters),yy,"...",dots);
      }
    }
  return 1;
  }


// Find out where window was grabbed
FXuchar FXMDIChild::where(FXint x,FXint y){
  FXuchar code=DRAGNONE;
  FXint fh,mh,bh,th;
  fh=font->getFontHeight();
  mh=windowbtn->getDefaultHeight();
  bh=deletebtn->getDefaultHeight();
  th=FXMAX3(fh,mh,bh)+4;
  if(!isMinimized() && x<HANDLESIZE) code|=DRAGLEFT;
  if(!isMinimized() && width-HANDLESIZE<=x) code|=DRAGRIGHT;
  if(!isMinimized() && y<HANDLESIZE) code|=DRAGTOP;
  if(!isMinimized() && height-HANDLESIZE<=y) code|=DRAGBOTTOM;
  if(BORDERWIDTH<=x && x<=width-BORDERWIDTH && BORDERWIDTH<=y && y<BORDERWIDTH+th) code=DRAGTITLE;
  return code;
  }


// Focus on widget itself
long FXMDIChild::onFocusSelf(FXObject* sender,FXSelector sel,void* ptr){    // FIXME look at this carefully
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


// Gained focus
long FXMDIChild::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXint fh,mh,bh,th;
  FXComposite::onFocusIn(sender,sel,ptr);
  fh=font->getFontHeight();
  mh=windowbtn->getDefaultHeight();
  bh=deletebtn->getDefaultHeight();
  th=FXMAX3(fh,mh,bh)+4;
  windowbtn->setBackColor(isActive() ? titleBackColor : backColor);
  getParent()->handle(this,MKUINT(0,SEL_CHANGED),(void*)this);
  update(BORDERWIDTH,BORDERWIDTH,width-(BORDERWIDTH<<1),th);
  return 1;
  }


// Lost focus
long FXMDIChild::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXint fh,mh,bh,th;
  FXComposite::onFocusOut(sender,sel,ptr);
  fh=font->getFontHeight();
  mh=windowbtn->getDefaultHeight();
  bh=deletebtn->getDefaultHeight();
  th=FXMAX3(fh,mh,bh)+4;
  windowbtn->setBackColor(isActive() ? shadowColor : backColor);
  update(BORDERWIDTH,BORDERWIDTH,width-(BORDERWIDTH<<1),th);
  return 1;
  }


// Pressed LEFT button
long FXMDIChild::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(event->click_count==1){
      getApp()->forceRefresh();
      getApp()->flush();
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
          break;
        case DRAGBOTTOM:
          yoff=event->win_y-oldh;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGLEFT:
          xoff=event->win_x;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGRIGHT:
          xoff=event->win_x-oldw;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGTOPLEFT:
          xoff=event->win_x;
          yoff=event->win_y;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGTOPRIGHT:
          xoff=event->win_x-oldw;
          yoff=event->win_y;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGBOTTOMLEFT:
          xoff=event->win_x;
          yoff=event->win_y-oldh;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGBOTTOMRIGHT:
          xoff=event->win_x-oldw;
          yoff=event->win_y-oldh;
          drawRubberBox(0,0,oldw,oldh);
          action|=DRAGINVERTED;
          break;
        case DRAGTITLE:
          xoff=event->win_x;
          yoff=event->win_y;
          break;
        }
      flags|=FLAG_PRESSED;
      }
    return 1;
    }
  return 0;
  }


// Released LEFT button
long FXMDIChild::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(event->click_count==1){
      if(action!=DRAGNONE){
        FXint xx,yy;
        translateCoordinatesTo(xx,yy,getParent(),oldx,oldy);
        if(action&DRAGINVERTED) drawRubberBox(oldx,oldy,oldw,oldh);
        position(xx,yy,oldw,oldh);
        action=DRAGNONE;
        recalc();
        }
      }
    else if(event->click_count==2){
      if(options&MDI_MINIMIZED){
        animateRectangles(xpos,ypos,width,height,normalPosX,normalPosY,normalWidth,normalHeight);
        restore(TRUE);
        }
      else if(options&MDI_MAXIMIZED){
        animateRectangles(xpos,ypos,width,height,normalPosX,normalPosY,normalWidth,normalHeight);
        restore(TRUE);
        }
      else{
        animateRectangles(xpos,ypos,width,height,0,0,getParent()->getWidth(),getParent()->getHeight());
        maximize(TRUE);
        }
      }
    return 1;
    }
  return 0;
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
        setDragCursor(getApp()->getDefaultCursor(DEF_MOVE_CURSOR));
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


// Pressed MIDDLE button
long FXMDIChild::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(isEnabled()){
    handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// Released MIDDLE button
long FXMDIChild::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// Pressed RIGHT button
long FXMDIChild::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    lower();
    return 1;
    }
  return 0;
  }


// Released RIGHT button
long FXMDIChild::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// Update value from a message
long FXMDIChild::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdSetStringValue: NULL pointer.\n",getClassName()); }
  setTitle(*((FXString*)ptr));
  return 1;
  }


// Window was selected
long FXMDIChild::onSelected(FXObject*,FXSelector,void* ptr){    // FIXME
  if(!(flags&FLAG_ACTIVE)){
    if(target) target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
    windowbtn->setBackColor(hasFocus() ? titleBackColor : shadowColor);
    flags|=FLAG_ACTIVE;
    recalc();
    update();
    }
  return 1;
  }


// Window was deselected
long FXMDIChild::onDeselected(FXObject*,FXSelector,void* ptr){    // FIXME
  if(flags&FLAG_ACTIVE){
    if(target) target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
    windowbtn->setBackColor(backColor);
    flags&=~FLAG_ACTIVE;
    recalc();
    update();
    }
  return 1;
  }


// Window was minimized
long FXMDIChild::onMinimize(FXObject*,FXSelector,void*){
  return target && target->handle(this,MKUINT(message,SEL_MINIMIZE),NULL);
  }


// Window was restored
long FXMDIChild::onRestore(FXObject*,FXSelector,void*){
  return target && target->handle(this,MKUINT(message,SEL_RESTORE),NULL);
  }


// Window was maximized
long FXMDIChild::onMaximize(FXObject*,FXSelector,void*){
  return target && target->handle(this,MKUINT(message,SEL_MAXIMIZE),NULL);
  }


// Returns 1 if its OK to close the window
long FXMDIChild::onClose(FXObject*,FXSelector,void*){
  return !target || target->handle(this,MKUINT(message,SEL_CLOSE),NULL);
  }


// Returns 1 if its OK to close all windows
long FXMDIChild::onCloseAll(FXObject*,FXSelector,void*){
  return !target || target->handle(this,MKUINT(message,SEL_CLOSEALL),NULL);
  }


// Delete window
long FXMDIChild::onDelete(FXObject*,FXSelector,void*){
  FXMDIChild *alternative;

  // Try find another window to activate
  alternative=mdinext?mdinext:mdiprev;

  // First make sure we're inactive
  getParent()->handle(this,MKUINT(0,SEL_CHANGED),alternative);

  // Tell target we're history
  if(target) target->handle(this,MKUINT(message,SEL_DELETE),NULL);

  // Self destruct
  delete this;

  return 1;
  }


/*******************************************************************************/


// Restore window command
long FXMDIChild::onCmdRestore(FXObject*,FXSelector,void*){
  animateRectangles(xpos,ypos,width,height,normalPosX,normalPosY,normalWidth,normalHeight);
  restore(TRUE);
  return 1;
  }


// Maximize window command
long FXMDIChild::onCmdMaximize(FXObject*,FXSelector,void*){
  animateRectangles(xpos,ypos,width,height,0,0,getParent()->getWidth(),getParent()->getHeight());
  maximize(TRUE);
  return 1;
  }


// Minimize window command
long FXMDIChild::onCmdMinimize(FXObject*,FXSelector,void*){
  animateRectangles(xpos,ypos,width,height,iconPosX,iconPosY,getDefaultWidth(),getDefaultHeight());
  minimize(TRUE);
  return 1;
  }


// Close window after asking target if its allowed
long FXMDIChild::onCmdClose(FXObject*,FXSelector,void*){
  if(handle(this,MKUINT(0,SEL_CLOSE),NULL)){
    handle(this,MKUINT(ID_DELETE,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Delete window command
long FXMDIChild::onCmdDelete(FXObject*,FXSelector,void*){
  animateRectangles(xpos,ypos,width,height,xpos+width/2,ypos+height/2,0,0);
  handle(this,MKUINT(0,SEL_DELETE),NULL);   // FIXME
  return 1;
  }


/*******************************************************************************/


// Update restore command
long FXMDIChild::onUpdRestore(FXObject* sender,FXSelector,void*){
  if(isMinimized() || isMaximized()){
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Update minimized command
long FXMDIChild::onUpdMinimize(FXObject* sender,FXSelector,void*){
  if(!isMinimized()){
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Update maximized command
long FXMDIChild::onUpdMaximize(FXObject* sender,FXSelector,void*){
  if(!isMaximized()){
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Update close command
long FXMDIChild::onUpdClose(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update window menu button
long FXMDIChild::onUpdWindow(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  return 1;
  }


/*******************************************************************************/


// Update MDI close button on menu bar
long FXMDIChild::onUpdMenuClose(FXObject* sender,FXSelector,void*){
  if(isMaximized()){
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Update MDI restore button on menu bar
long FXMDIChild::onUpdMenuRestore(FXObject* sender,FXSelector,void*){
  if(isMaximized()){
    sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Update MDI minimized button on menu bar
long FXMDIChild::onUpdMenuMinimize(FXObject* sender,FXSelector,void*){
  if(isMaximized()){
    sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Update MDI window menu button on menu bar
long FXMDIChild::onUpdMenuWindow(FXObject* sender,FXSelector,void*){
  if(isMaximized()){
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
    ((FXMDIWindowButton*)sender)->setMenu(getWindowMenu());///// Should be a message!!!!
    ((FXMDIWindowButton*)sender)->setIcon(getWindowIcon());
    }
  else{
    sender->handle(this,MKUINT(ID_HIDE,SEL_COMMAND),NULL);
    }
  return 1;
  }


/*******************************************************************************/


// Set base color
void FXMDIChild::setBaseColor(FXColor clr){
  if(baseColor!=clr){
    baseColor=clr;
    update();
    }
  }


// Set highlight color
void FXMDIChild::setHiliteColor(FXColor clr){
  if(hiliteColor!=clr){
    hiliteColor=clr;
    update();
    }
  }


// Set shadow color
void FXMDIChild::setShadowColor(FXColor clr){
  if(shadowColor!=clr){
    shadowColor=clr;
    update();
    }
  }


// Set border color
void FXMDIChild::setBorderColor(FXColor clr){
  if(borderColor!=clr){
    borderColor=clr;
    update();
    }
  }


// Set title color
void FXMDIChild::setTitleColor(FXColor clr){
  if(titleColor!=clr){
    titleColor=clr;
    update();
    }
  }


// Set title color
void FXMDIChild::setTitleBackColor(FXColor clr){
  if(titleBackColor!=clr){
    titleBackColor=clr;
    update();
    }
  }


// Set new window title
void FXMDIChild::setTitle(const FXString& name){
  if(title!=name){
    title=name;
    update();
    }
  }


// Delegate all other messages to child window
long FXMDIChild::onDefault(FXObject* sender,FXSelector key,void* data){
  return contentWindow() && contentWindow()->handle(sender,key,data);
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
FXPopup* FXMDIChild::getWindowMenu() const {
  return windowbtn->getMenu();
  }


// Change window menu
void FXMDIChild::setWindowMenu(FXPopup* menu){
  windowbtn->setMenu(menu);
  }


// Change the font
void FXMDIChild::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Save object to stream
void FXMDIChild::save(FXStream& store) const {
  FXComposite::save(store);
  store << title;
  store << windowbtn;
  store << minimizebtn;
  store << restorebtn;
  store << maximizebtn;
  store << deletebtn;
  store << font;
  store << baseColor;
  store << hiliteColor;
  store << shadowColor;
  store << borderColor;
  store << titleColor;
  store << titleBackColor;
  store << iconPosX;
  store << iconPosY;
  store << iconWidth;
  store << iconHeight;
  store << normalPosX;
  store << normalPosY;
  store << normalWidth;
  store << normalHeight;
  }


// Load object from stream
void FXMDIChild::load(FXStream& store){
  FXComposite::load(store);
  store >> title;
  store >> windowbtn;
  store >> minimizebtn;
  store >> restorebtn;
  store >> maximizebtn;
  store >> deletebtn;
  store >> font;
  store >> baseColor;
  store >> hiliteColor;
  store >> shadowColor;
  store >> borderColor;
  store >> titleColor;
  store >> titleBackColor;
  store >> iconPosX;
  store >> iconPosY;
  store >> iconWidth;
  store >> iconHeight;
  store >> normalPosX;
  store >> normalPosY;
  store >> normalWidth;
  store >> normalHeight;
  }


// Destruct thrashes the pointers
FXMDIChild::~FXMDIChild(){
  if(mdiprev) mdiprev->mdinext=mdinext; else ((FXMDIClient*)getParent())->mdifirst=mdinext;
  if(mdinext) mdinext->mdiprev=mdiprev; else ((FXMDIClient*)getParent())->mdilast=mdiprev;
  if(((FXMDIClient*)getParent())->active==this) ((FXMDIClient*)getParent())->active=NULL;
  windowbtn=(FXMenuButton*)-1;
  minimizebtn=(FXButton*)-1;
  restorebtn=(FXButton*)-1;
  maximizebtn=(FXButton*)-1;
  deletebtn=(FXButton*)-1;
  font=(FXFont*)-1;
  }
