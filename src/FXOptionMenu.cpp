/********************************************************************************
*                                                                               *
*                             O p t i o n   M e n u                             *
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
* $Id: FXOptionMenu.cpp,v 1.18 1998/10/19 21:38:23 jvz Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXButton.h"
#include "FXMenuButton.h"
#include "FXTooltip.h"
#include "FXOptionMenu.h"

/*
  Notes:
  - FXOptionMenu should just pop the pane; FXOption should send the message.
  - Need API to inquire whether an FXOption is selected or not.
*/


#define MENUGLYPH_WIDTH  10
#define MENUGLYPH_HEIGHT 5

/*******************************************************************************/

// Map
FXDEFMAP(FXOption) FXOptionMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXOption::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXOption::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXOption::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXOption::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXOption::onLeftBtnRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXOption::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXOption::onDeactivate),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXWindow::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXWindow::onHotKeyRelease),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXOption::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXOption,FXLabel,FXOptionMap,ARRAYNUMBER(FXOptionMap))

  
// Make option menu entry
FXOption::FXOption(FXComposite* p,const char* text,FXIcon* ic,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXLabel(p,text,ic,opts,x,y,w,h,pl,pr,pt,pb),
  tip(text,'\t',1),
  help(text,'\t',2){
  target=tgt;
  message=sel;
  flags|=FLAG_ENABLED;
  defaultCursor=getApp()->rarrowCursor;
  }


// If window can have focus
FXbool FXOption::canFocus() const { return 1; }


// Get default width
FXint FXOption::getDefaultWidth(){
  FXint tw=0,iw=MENUGLYPH_WIDTH,s=0,w;
  if(label.text()){ tw=labelWidth(); s=4; }
  if(icon){ iw=icon->getWidth(); }
  if(!(options&(ICON_AFTER_TEXT|ICON_BEFORE_TEXT))) w=FXMAX(tw,iw); else w=tw+iw+s;
  return padleft+padright+(border<<1)+w;
  }


// Get default height
FXint FXOption::getDefaultHeight(){
  FXint th=0,ih=MENUGLYPH_HEIGHT,h;
  if(label.text()){ th=labelHeight(); }
  if(icon){ ih=icon->getHeight(); }
  if(!(options&(ICON_ABOVE_TEXT|ICON_BELOW_TEXT))) h=FXMAX(th,ih); else h=th+ih;
  return padtop+padbottom+(border<<1)+h;
  }



// Handle repaint 
long FXOption::onPaint(FXObject*,FXSelector,void*){
  FXint tw=0,th=0,iw=MENUGLYPH_WIDTH,ih=MENUGLYPH_HEIGHT,tx,ty,ix,iy;
  FXASSERT(xid!=0);
  drawFrame(0,0,width,height);
  if(label.text()){
    tw=labelWidth();
    th=labelHeight();
    }
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }
  just_x(tx,ix,tw,iw);
  just_y(ty,iy,th,ih);
  if(isActive()){
    setForeground(hiliteColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    drawLine(border,border,width-border-1,border);
    }
  else{
    setForeground(backColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    }
  if(icon){
    drawIcon(icon,ix,iy);
    }
  else if(isActive()){
    drawDoubleRaisedRectangle(ix,iy,MENUGLYPH_WIDTH,MENUGLYPH_HEIGHT);
    }
  if(label.text()){
    setTextFont(font);
    if(isEnabled()){
      setForeground(textColor);
      drawLabel(tx,ty,tw,th);
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


// Enter
long FXOption::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onEnter(sender,sel,ptr);
//fprintf(stderr,"%s::onEnter %08x\n",getClassName(),this);
  if(isEnabled() && canFocus()) setFocus();
  return 1;
  }


// Leave
long FXOption::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onLeave(sender,sel,ptr);
//fprintf(stderr,"%s::onLeave %08x\n",getClassName(),this);
  if(isEnabled() && canFocus()) killFocus();
  return 1;
  }


// Pressed left button; always unposts menu
long FXOption::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),this);
    if(target) target->handle(this,MKUINT(message,SEL_COMMAND),ptr);
    return 1;
    }
  return 0;
  }


// Released left button; unpost menu if cursor has moved
long FXOption::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(ev->moved){ 
      getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),this);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),ptr);
      }
    return 1;
    }
  return 0;
  }


// Activate; does nothing
long FXOption::onActivate(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onActivate %08x\n",getClassName(),this);
  return 1;
  }


// Deactivate; pops down the pane
long FXOption::onDeactivate(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onDeactivate %08x\n",getClassName(),this);
  getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),this);
  if(target) target->handle(this,MKUINT(message,SEL_COMMAND),ptr);
  return 1;
  }


// Into focus chain
void FXOption::setFocus(){
//fprintf(stderr,"%s::setFocus %08x\n",getClassName(),this);
  FXLabel::setFocus();
  flags|=FLAG_ACTIVE;
  flags&=~FLAG_UPDATE;
  update(0,0,width,height);
  }


// We were asked about status text
long FXOption::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// Out of focus chain
void FXOption::killFocus(){
//fprintf(stderr,"%s::killFocus %08x\n",getClassName(),this);
  FXLabel::killFocus();
  flags&=~FLAG_ACTIVE;
  flags|=FLAG_UPDATE;
  update(0,0,width,height);
  }


// Delete
FXOption::~FXOption(){
  }


/*******************************************************************************/


// Map
FXDEFMAP(FXOptionMenu) FXOptionMenuMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXOptionMenu::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXOptionMenu::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXOptionMenu::onLeftBtnRelease),
  FXMAPFUNC(SEL_FOCUSIN,0,FXOptionMenu::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXOptionMenu::onFocusOut),
  FXMAPFUNC(SEL_MOTION,0,FXOptionMenu::onMotion),
  FXMAPFUNC(SEL_KEYPRESS,0,FXOptionMenu::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXOptionMenu::onKeyRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXOptionMenu::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXOptionMenu::onDeactivate),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_POST,FXOptionMenu::onCmdPost),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXOptionMenu::onCmdUnpost),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXOptionMenu::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXOptionMenu::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXOptionMenu,FXLabel,FXOptionMenuMap,ARRAYNUMBER(FXOptionMenuMap))



// Make a option menu button
FXOptionMenu::FXOptionMenu(FXComposite* p,FXPopup* pup,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXLabel(p,NULL,NULL,opts,x,y,w,h,pl,pr,pt,pb){
  flags|=FLAG_ENABLED;
  dragCursor=getApp()->rarrowCursor;
  pane=pup;
  current=NULL;
  if(pane){
    current=(FXOption*)pane->getFirst();
    if(current){
      label=current->getText();
      icon=current->getIcon();
      }
    }
  }


// Create X window
void FXOptionMenu::create(){
  FXLabel::create();
  if(pane) pane->create();
  }


// Destroy X window
void FXOptionMenu::destroy(){
  FXLabel::destroy();
  }


// Get default width
FXint FXOptionMenu::getDefaultWidth(){
  FXint w=0;
  if(pane){ w=pane->getDefaultWidth(); }
  return (border<<1)+w;
  }


// Get default height
FXint FXOptionMenu::getDefaultHeight(){
  FXint h=0;
  if(pane && pane->getFirst()){
    h=pane->getFirst()->getDefaultHeight();
    }
  return (border<<1)+h;
  }


// Gained focus
long FXOptionMenu::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusIn(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }

  
// Lost focus
long FXOptionMenu::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusOut(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }


// Handle repaint 
long FXOptionMenu::onPaint(FXObject*,FXSelector,void*){
  FXint tw=0,th=0,iw=MENUGLYPH_WIDTH,ih=MENUGLYPH_HEIGHT,tx,ty,ix,iy;
  FXASSERT(xid!=0);
  
  drawFrame(0,0,width,height);
  
  // Draw background
  setForeground(backColor);
  fillRectangle(border,border,width-border*2,height-border*2);
  
  // Position text & icon
  if(label.text()){
    tw=labelWidth();
    th=labelHeight();
    }
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }
  
  // Keep some room for the arrow!
  just_x(tx,ix,tw,iw);
  just_y(ty,iy,th,ih);

  // Draw icon
  if(icon){
    drawIcon(icon,ix,iy);
    }
  
  // Or draw rectangle
  else{
    drawDoubleRaisedRectangle(ix,iy,MENUGLYPH_WIDTH,MENUGLYPH_HEIGHT);
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



// Keyboard press; forward to menu pane
long FXOptionMenu::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  flags&=~FLAG_TIP;
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXLabel::onKeyPress(sender,sel,ptr);
  }


// Keyboard release; forward to menu pane
long FXOptionMenu::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXLabel::onKeyRelease(sender,sel,ptr);
  }


// Pressed left button
long FXOptionMenu::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    if(pane){
      if(pane->shown()){
        handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
        }
      }
    return 1;
    }
  return 0;
  }


// Released left button
long FXOptionMenu::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(pane){
      if(ev->moved){ handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL); }
      }
    return 1;
    }
  return 0;
  }


// Key pressed
long FXOptionMenu::onActivate(FXObject*,FXSelector,void*){
  return 1;
  }
  

// Key released
long FXOptionMenu::onDeactivate(FXObject*,FXSelector,void* ptr){
  if(pane){
    if(pane->shown()){
      handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
      }
    else{
      handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
      }
    }
  return 1;
  }



// If we moved over the pane, we'll ungrab again, or re-grab
// when outside of the plane
long FXOptionMenu::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(pane && pane->shown()){
    flags&=~FLAG_TIP;
    if(pane->contains(ev->root_x,ev->root_y)){
      if(grabbed()) ungrab(); 
      }
    else{
      if(!grabbed()) grab();
      }
    return 1;
    }
  return 0;
  }



// Post the menu
long FXOptionMenu::onCmdPost(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    FXint x,y;
    if(!current) current=(FXOption*)pane->getFirst();
    if(!current) return 1;
    translateCoordinatesTo(x,y,getRoot(),0,0);
    pane->position(x,y,width,pane->getDefaultHeight());
    y+=2-current->getY();
    pane->popup(this,x,y,width,pane->getDefaultHeight());
    current->setFocus();
    flags&=~FLAG_UPDATE;
    if(!grabbed()) grab();
    }
  return 1;
  }


// Unpost the menu
// Sender was the original option that sent the message
long FXOptionMenu::onCmdUnpost(FXObject* sender,FXSelector,void* ptr){
  if(pane && pane->shown()){
    pane->popdown();
    if(grabbed()) ungrab();
    if(ptr) setCurrent((FXOption*)ptr);
    }
  flags|=FLAG_UPDATE;
  return 1;
  }


// Layout
void FXOptionMenu::layout(){
  FXLabel::layout();
  if(!current && pane && pane->getFirst()){
    setCurrent((FXOption*)pane->getFirst());
    }
  flags&=~FLAG_DIRTY;
  }


// Logically inside pane
FXbool FXOptionMenu::contains(FXint parentx,FXint parenty) const {
  if(pane && pane->shown() && pane->contains(parentx,parenty)) return 1;
  return 0;
  }


// Out of focus chain
void FXOptionMenu::killFocus(){
  FXLabel::killFocus();
  handle(current,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  }


// If window can have focus
FXbool FXOptionMenu::canFocus() const { return 1; }


// Set current selection
void FXOptionMenu::setCurrent(FXOption *win){
  if(win==NULL){ fxerror("%s::setCurrent: NULL window passed.\n",getClassName()); }
  if(win->getParent()!=pane){ fxerror("%s::setCurrent: expected parent of window to be the pane.\n",getClassName()); }
  if(current!=win){
    current=win;
    setText(current->getText());
    setIcon(current->getIcon());
    }
  }


// Set current option
void FXOptionMenu::setCurrentNo(FXint no){
  register FXint i=0;
  if(pane){
    FXOption *win=(FXOption*)pane->getFirst();
    while(win && i!=no){
      win=(FXOption*)win->getNext();
      i++;
      }
    if(win) setCurrent(win);
    }
  }


// Get current option
FXint FXOptionMenu::getCurrentNo() const {
  register FXint i=0;
  if(pane){
    FXOption *win=(FXOption*)pane->getFirst();
    while(win && win!=current){
      win=(FXOption*)win->getNext();
      i++;
      }
    }
  return i;
  }


// Change popup
void FXOptionMenu::setPopup(FXPopup *pup){
  FXOption *win=NULL;
  pane=pup;
  if(pane) win=(FXOption*)pane->getFirst();
  setCurrent(win);
  }


// We were asked about status text
long FXOptionMenu::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if((flags&FLAG_HELP) && current && current->getHelpText()){
    /////sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);////FIX FIX
    ((FXStatusline*)sender)->setText(current->getHelpText());
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXOptionMenu::onQueryTip(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
  if((flags&FLAG_TIP) && current && current->getTipText()){
    ((FXTooltip*)sender)->setText(current->getTipText());///// FIX ALSO
    return 1;
    }
  return 0;
  }


// True if popped up
FXbool FXOptionMenu::isPopped() const {
  return pane && pane->shown();
  }


// Delete it
FXOptionMenu::~FXOptionMenu(){
  pane=(FXPopup*)-1;
  current=(FXOption*)-1;
  }

