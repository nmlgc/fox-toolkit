/********************************************************************************
*                                                                               *
*                       M e n u    B u t t o n    O b j e c t                   *
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
* $Id: FXMenuButton.cpp,v 1.6 1998/10/30 05:29:50 jeroen Exp $                  *
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
#include "FXTooltip.h"

/*
  To do:
  - Should FXMenuButtons, etc. own the popup menus:- NO!
  - Need sideways options and so on.
*/

#define MENUBUTTONARROW_WIDTH   11
#define MENUBUTTONARROW_HEIGHT   5

#define MENUBUTTONWINDOW_WIDTH  16
#define MENUBUTTONWINDOW_HEIGHT 14


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuButton) FXMenuButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuButton::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXMenuButton::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuButton::onLeave),
  FXMAPFUNC(SEL_MOTION,0,FXMenuButton::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuButton::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuButton::onLeftBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXMenuButton::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXMenuButton::onKeyRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXMenuButton::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXMenuButton::onDeactivate),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_POST,FXMenuButton::onCmdPost),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXMenuButton::onCmdUnpost),
  };


// Object implementation
FXIMPLEMENT(FXMenuButton,FXButton,FXMenuButtonMap,ARRAYNUMBER(FXMenuButtonMap))


// Deserialization
FXMenuButton::FXMenuButton(){
  pane=(FXMenuPane*)-1;
  }


// Make a check button
FXMenuButton::FXMenuButton(FXComposite* p,const char* text,FXIcon* ic,FXMenuPane* pup,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXButton(p,text,ic,NULL,0,opts,x,y,w,h,pl,pr,pt,pb){
  pane=pup;
  }


// Create X window; makes sure pane is created
void FXMenuButton::create(){
  FXButton::create();
  if(pane) pane->create();
  }


// Get default width
FXint FXMenuButton::getDefaultWidth(){
  FXint tw=0,iw,s=0,w;
  if(label.text()){ tw=labelWidth(); s=4; }
  iw=MENUBUTTONARROW_WIDTH;
  if(options&MENUBUTTON_WINDOW) iw=MENUBUTTONWINDOW_WIDTH; 
  if(icon){ iw=icon->getWidth(); }
  if(!(options&(ICON_AFTER_TEXT|ICON_BEFORE_TEXT))) w=FXMAX(tw,iw); else w=tw+iw+s;
  return padleft+padright+(border<<1)+w;
  }


// Get default height
FXint FXMenuButton::getDefaultHeight(){
  FXint th=0,ih,h;
  if(label.text()){ th=labelHeight(); }
  ih=MENUBUTTONARROW_HEIGHT;
  if(options&MENUBUTTON_WINDOW) ih=MENUBUTTONWINDOW_HEIGHT; 
  if(icon){ ih=icon->getHeight(); }
  if(!(options&(ICON_ABOVE_TEXT|ICON_BELOW_TEXT))) h=FXMAX(th,ih); else h=th+ih;
  return padtop+padbottom+(border<<1)+h;
  }


// Handle repaint 
long FXMenuButton::onPaint(FXObject*,FXSelector,void*){
  FXint tw=0,th=0,iw,ih,tx,ty,ix,iy;
  FXPoint points[3];
  FXASSERT(xid!=0);
  
  // Draw frame
  if(options&(FRAME_RAISED|FRAME_SUNKEN)){
    if(isDefault()){
      drawBorderRectangle(0,0,width,height);
      if(!isEnabled() || (state==STATE_UP)){
        if(options&FRAME_THICK) drawDoubleRaisedRectangle(1,1,width-1,height-1);
        else drawRaisedRectangle(1,1,width-1,height-1);
        }
      else{
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width-1,height-1);
        else drawSunkenRectangle(0,0,width-1,height-1);
        }
      }
    else{
      if(!isEnabled() || (state==STATE_UP)){
        if(options&FRAME_THICK) drawDoubleRaisedRectangle(0,0,width,height);
        else drawRaisedRectangle(0,0,width,height);
        }
      else{
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width,height);
        else drawSunkenRectangle(0,0,width,height);
        }
      }
    }

  // Draw background
  if(options&MENUBUTTON_WINDOW){
    setForeground(backColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    }
  else if(state==STATE_UP){
    setForeground(backColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    }
  else{
    setForeground(hiliteColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    drawLine(border,border,width-border-1,border);
    }
  
  // Position text & icon
  if(label.text()){
    tw=labelWidth();
    th=labelHeight();
    }
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }
  else if(options&MENUBUTTON_WINDOW){
    iw=MENUBUTTONWINDOW_WIDTH;
    ih=MENUBUTTONWINDOW_HEIGHT; 
    }
  else{
    iw=MENUBUTTONARROW_WIDTH;  
    ih=MENUBUTTONARROW_HEIGHT; 
    }
  
  // Keep some room for the arrow!
  just_x(tx,ix,tw,iw);
  just_y(ty,iy,th,ih);

  // Move a bit when pressed
  if(state){ ++tx; ++ty; ++ix; ++iy; }

  // Draw icon
  if(icon){
    drawIcon(icon,ix,iy);
    }
  
  // Window menu
  else if(options&MENUBUTTON_WINDOW){
    setForeground(shadowColor);
    drawLine(ix,iy,MENUBUTTONWINDOW_WIDTH-1,iy);
    drawLine(ix,iy,ix,MENUBUTTONWINDOW_HEIGHT-1);
    drawLine(ix,iy+MENUBUTTONWINDOW_HEIGHT-2,ix+MENUBUTTONWINDOW_WIDTH-2,iy+MENUBUTTONWINDOW_HEIGHT-2);
    drawLine(ix+1,iy+4,ix+MENUBUTTONWINDOW_WIDTH-2,iy+4);
    drawLine(ix+MENUBUTTONWINDOW_WIDTH-2,iy,ix+MENUBUTTONWINDOW_WIDTH-2,iy+MENUBUTTONWINDOW_HEIGHT-2);
    setForeground(baseColor);
    drawLine(ix+1,iy+1,MENUBUTTONWINDOW_WIDTH-3,iy+1);
    drawLine(ix+1,iy+1,ix+1,MENUBUTTONWINDOW_HEIGHT-3);
    setForeground(textColor);
    drawLine(ix,iy+MENUBUTTONWINDOW_HEIGHT-1,ix+MENUBUTTONWINDOW_WIDTH,iy+MENUBUTTONWINDOW_HEIGHT-1);
    drawLine(ix+MENUBUTTONWINDOW_WIDTH-1,iy,ix+MENUBUTTONWINDOW_WIDTH-1,iy+MENUBUTTONWINDOW_HEIGHT-1);
    drawLine(ix+2,iy+2,ix+MENUBUTTONWINDOW_WIDTH-2,iy+2);
    drawLine(ix+2,iy+3,ix+MENUBUTTONWINDOW_WIDTH-2,iy+3);
    setForeground(hiliteColor);
    fillRectangle(ix+2,iy+5,MENUBUTTONWINDOW_WIDTH-4,MENUBUTTONWINDOW_HEIGHT-7);
    drawPoint(ix+MENUBUTTONWINDOW_WIDTH-4,iy+3);
    drawPoint(ix+MENUBUTTONWINDOW_WIDTH-6,iy+3);
    drawPoint(ix+MENUBUTTONWINDOW_WIDTH-8,iy+3);
    }
    
  // Up arrow
  else if(options&MENUBUTTON_UP){
    if(isEnabled()) setForeground(textColor); else setForeground(shadowColor);
    points[0].x=ix+(MENUBUTTONARROW_WIDTH>>1);
    points[0].y=iy-1;
    points[1].x=ix;
    points[1].y=iy+MENUBUTTONARROW_HEIGHT;
    points[2].x=ix+MENUBUTTONARROW_WIDTH;
    points[2].y=iy+MENUBUTTONARROW_HEIGHT;
    fillPolygon(points,3);
    }
  
  // Down arrow
  else{
    if(isEnabled()) setForeground(textColor); else setForeground(shadowColor);
    points[0].x=ix+1;
    points[0].y=iy;
    points[2].x=ix+MENUBUTTONARROW_WIDTH-1;
    points[2].y=iy;
    points[1].x=ix+(MENUBUTTONARROW_WIDTH>>1);
    points[1].y=iy+MENUBUTTONARROW_HEIGHT;
    fillPolygon(points,3);
    }

  // Draw text
  if(label.text()){
    setTextFont(font);
    if(isEnabled()){
      setForeground(textColor);
      drawLabel(tx,ty,tw,th);
      if(hasFocus()){ drawFocusRectangle(border+2,border+2,width-2*border-4,height-2*border-4); }
      }
    else{
      setForeground(hiliteColor);
      drawLabel(tx+1,ty+1,tw,th);
      setForeground(shadowColor);
      drawLabel(tx,ty,tw,th);
      }
    }
  return 1;
  }


// Pressed left button
long FXMenuButton::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    return handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    }
  return 0;
  }


// Released left button
long FXMenuButton::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    return handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    }
  return 0;
  }


// Keyboard press; forward to menu pane, or handle it here
long FXMenuButton::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXButton::onKeyPress(sender,sel,ptr);
  }
    

// Keyboard release; forward to menu pane, or handle here
long FXMenuButton::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXButton::onKeyRelease(sender,sel,ptr);
  }


// Button being pressed
long FXMenuButton::onActivate(FXObject*,FXSelector,void* ptr){
  if(state){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    }
  else{
    handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
    }
  return 1;
  }
  

// Button being released
long FXMenuButton::onDeactivate(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(ev->moved){ handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL); }
  return 1;
  }


// Inside the button
long FXMenuButton::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onEnter(sender,sel,ptr);
  return 1;
  }


// Outside the button
long FXMenuButton::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onLeave(sender,sel,ptr);
  return 1;
  }


// If we moved over the pane, we'll ungrab again, or re-grab
// when outside of the plane
long FXMenuButton::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(state!=STATE_UP){
    if(pane){
      if(pane->contains(ev->root_x,ev->root_y)){
        if(grabbed()) ungrab(); 
        }
      else{
        if(!grabbed()) grab();
        }
      return 1;
      }
    }
  return 0;
  }


// Post the menu
long FXMenuButton::onCmdPost(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    FXint x,y;
    translateCoordinatesTo(x,y,getRoot(),0,0);
    if(options&MENUBUTTON_UP)
      y=y-pane->getHeight();
    else
      y=y+height;
    pane->popup(this,x,y);
    if(!grabbed()) grab();
    }
  setState(STATE_DOWN);
  return 1;
  }


// Unpost the menu
long FXMenuButton::onCmdUnpost(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    pane->popdown();
    if(grabbed()) ungrab();
    }
  setState(STATE_UP);
  return 1;
  }


// Out of focus chain
void FXMenuButton::killFocus(){
  FXButton::killFocus();
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  }


// Logically inside pane
FXbool FXMenuButton::contains(FXint parentx,FXint parenty) const {
  if(pane && pane->shown() && pane->contains(parentx,parenty)) return 1;
  return 0;
  }



// Set menu button popup style
void FXMenuButton::setPopupStyle(FXuint style){
  FXuint opts=(options&~MENUBUTTON_MASK) | (style&MENUBUTTON_MASK);
  if(options!=opts){
    options=opts;
    update(0,0,width,height);
    }
  }


// Get menu button popup style
FXuint FXMenuButton::getPopupStyle() const {
  return (options&MENUBUTTON_MASK);
  }


// Delete it
FXMenuButton::~FXMenuButton(){
  pane=(FXMenuPane*)-1;
  }
