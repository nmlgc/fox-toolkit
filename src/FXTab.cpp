/********************************************************************************
*                                                                               *
*                               T a b   O b j e c t                             *
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
* $Id: FXTab.cpp,v 1.15 1998/10/23 22:07:37 jvz Exp $                         *
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
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXTooltip.h"
#include "FXTab.h"

/*
  Notes:
  - Should focus go to tab items?
  - Should callbacks come from tab items?
  - FXTabItems should catch SEL_ACTIVATE etc.
  - Should redesign this stuff a little.
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXTabItem) FXTabItemMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTabItem::onPaint),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXWindow::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXWindow::onHotKeyRelease),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXTabItem::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXTabItem::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXTabItem,FXLabel,FXTabItemMap,ARRAYNUMBER(FXTabItemMap))


// Tab item
FXTabItem::FXTabItem(FXComposite* p,const char* text,FXIcon* ic,FXuint opts):
  FXLabel(p,text,ic,opts, 0,0, 0,0),
  tip(text,'\t',1),
  help(text,'\t',2){
  border=2;
  }


// Handle repaint 
long FXTabItem::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint tw=0,th=0,iw=0,ih=0,tx,ty,ix,iy;
  FXWindow::onPaint(sender,sel,ptr);
  switch(options&TAB_ORIENT_MASK){
    case TAB_LEFT:
      setForeground(hiliteColor);
      drawLine(2,0,width-1,0);
      drawLine(0,2,1,1);
      drawLine(0,height-2,0,2);
      setForeground(shadowColor);
      drawLine(2,height-2,width-1,height-2);
      setForeground(borderColor);
      drawLine(3,height-1,width-1,height-1);
      break;
    case TAB_RIGHT:
      setForeground(hiliteColor);
      drawLine(0,0,width-3,0);
      drawLine(width-3,0,width-1,3);
      setForeground(shadowColor);
      drawLine(width-2,2,width-2,height-2);
      drawLine(0,height-2,width-2,height-2);
      setForeground(borderColor);
      drawLine(0,height-1,width-3,height-1);
      drawLine(width-1,3,width-1,height-4);
      drawLine(width-3,height-1,width-1,height-4);
      break;
    case TAB_BOTTOM:
      setForeground(hiliteColor);
      drawLine(0,0,0,height-4);
      drawLine(0,height-4,1,height-2);
      setForeground(shadowColor);
      drawLine(2,height-2,width-3,height-2);
      drawLine(width-2,0,width-2,height-3);
      drawLine(width-2,0,width-1,0);
      setForeground(borderColor);
      drawLine(3,height-1,width-4,height-1);
      drawLine(width-4,height-1,width-1,height-4);
      drawLine(width-1,1,width-1,height-4);
      break;
    case TAB_TOP:
      setForeground(hiliteColor);
      drawLine(0,height-1,0,2);
      drawLine(0,2,2,0);
      drawLine(2,0,width-3,0);
      setForeground(shadowColor);
      drawLine(width-2,1,width-2,height-1);
      setForeground(borderColor);
      drawLine(width-2,1,width-1,2);
      drawLine(width-1,2,width-1,height-2);
      setForeground(hiliteColor);
      drawLine(width-1,height-1,width-1,height-1);
      break;
    }
  if(label.text()){
    tw=font->getTextWidth(label.text(),label.length());
    th=font->getFontHeight();
    }
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }
  just_x(tx,ix,tw,iw);
  just_y(ty,iy,th,ih);
  if(icon){
    drawIcon(icon,ix,iy);
    }
  if(label.text()){
    setTextFont(font);
    setForeground(textColor);
    //drawText(tx,ty+font->getFontAscent(),label.text(),label.length());
    drawLabel(tx,ty,tw,th);
    }
  return 1;
  }


// We were asked about status text
long FXTabItem::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXTabItem::onQueryTip(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
  if(tip.text() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&tip);
    return 1;
    }
  return 0;
  }


// Change help text
void FXTabItem::setHelpText(const FXchar* text){
  help=text;
  }


// Change tip text
void FXTabItem::setTipText(const FXchar* text){
  tip=text;
  }


/*******************************************************************************/


// Map
FXDEFMAP(FXSwitcher) FXSwitcherMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXSwitcher::onPaint),
  FXMAPFUNCS(SEL_COMMAND,FXSwitcher::ID_MAKETOP_FIRST,FXSwitcher::ID_MAKETOP_LAST,FXSwitcher::onCmdMakeTop),
  FXMAPFUNCS(SEL_UPDATE,FXSwitcher::ID_MAKETOP_FIRST,FXSwitcher::ID_MAKETOP_LAST,FXSwitcher::onUpdMakeTop),
  };


// Object implementation
FXIMPLEMENT(FXSwitcher,FXComposite,FXSwitcherMap,ARRAYNUMBER(FXSwitcherMap))


// Make a switcher window
FXSwitcher::FXSwitcher(FXComposite *p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h){
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  current=0;
  }


// Create X window & make sure right one's on top
void FXSwitcher::create(){
  FXComposite::create();
  baseColor=acquireColor(getApp()->backColor);
  hiliteColor=acquireColor(getApp()->hiliteColor);
  shadowColor=acquireColor(getApp()->shadowColor);
  borderColor=acquireColor(getApp()->borderColor);
  show();
  }


// Handle repaint 
long FXSwitcher::onPaint(FXObject*,FXSelector,void*){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(0,0,width-2,0);
  drawLine(0,0,0,height-2);
  setForeground(baseColor);
  drawLine(1,1,width-3,1);
  drawLine(1,1,1,height-3);
  setForeground(shadowColor);
  drawLine(1,height-2,width-2,height-2);
  drawLine(width-2,height-2,width-2,1);
  setForeground(borderColor);
  drawLine(0,height-1,width-1,height-1);
  drawLine(width-1,0,width-1,height-1);
  return 1;
  }


// Bring nth to the top
long FXSwitcher::onCmdMakeTop(FXObject*,FXSelector sel,void*){
  setCurrent(SELID(sel)-ID_MAKETOP_FIRST);
  return 1;
  }


// Update the nth button
long FXSwitcher::onUpdMakeTop(FXObject* sender,FXSelector sel,void* ptr){
  FXuint msg= ((SELID(sel)-ID_MAKETOP_FIRST) == current) ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Get maximum child width
FXint FXSwitcher::getDefaultWidth(){
  register FXWindow* child;
  register int t,m;
  for(m=0,child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      if(m<(t=child->getDefaultWidth())) m=t;
      }
    }
  return m+4;
  }


// Get maximum child height
FXint FXSwitcher::getDefaultHeight(){
  register FXWindow* child;
  register int t,m;
  for(m=0,child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      if(m<(t=child->getDefaultHeight())) m=t;
      }
    }
  return m+4;
  }


// Recalculate layout
void FXSwitcher::layout(){
  register FXWindow *c;
  register FXuint i;
//fprintf(stderr,"%s::layout w=%d h=%d\n",getClassName(),width,height);
  for(i=0,c=getFirst(); c; c=c->getNext(),i++){
    c->position(2,2,width-4,height-4);
    if(i==current) c->raise(); else c->lower();
    }
  flags&=~FLAG_DIRTY;
  }

  
// Set current subwindow
void FXSwitcher::setCurrent(FXuint f){
  register FXWindow *c;
  register FXuint i;
  if(current!=f){
    for(i=0,c=getFirst(); c; c=c->getNext(),i++){
      if(i==f) c->raise(); else c->lower();
      }
    current=f;
    }
  }


// Set base color
void FXSwitcher::setBaseColor(FXPixel clr){
  baseColor=clr;
  update(0,0,width,height);
  }


// Set highlight color
void FXSwitcher::setHiliteColor(FXPixel clr){
  hiliteColor=clr;
  update(0,0,width,height);
  }


// Set shadow color
void FXSwitcher::setShadowColor(FXPixel clr){
  shadowColor=clr;
  update(0,0,width,height);
  }


// Set border color
void FXSwitcher::setBorderColor(FXPixel clr){
  borderColor=clr;
  update(0,0,width,height);
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXTabBar) FXTabBarMap[]={
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTabBar::onButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTabBar::onButtonRelease),
  FXMAPFUNC(SEL_FOCUSIN,0,FXTabBar::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXTabBar::onFocusOut),
  };


// Object implementation
FXIMPLEMENT(FXTabBar,FXComposite,FXTabBarMap,ARRAYNUMBER(FXTabBarMap))


// Make a tab bar 
FXTabBar::FXTabBar(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  active=0;
  }



// Create X window
void FXTabBar::create(){
  FXComposite::create();
  show();
  }


// Get width
FXint FXTabBar::getDefaultWidth(){
  register FXWindow* child;
  register int t,s;
  if(options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS)){
    for(s=0,child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        if(child->getLayoutHints()&LAYOUT_FIX_WIDTH) t=child->getWidth(); else t=child->getDefaultWidth();
        if(s<t) s=t;
        }
      }
    s+=3;
    }
  else{
    for(s=0,child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        if(child->getLayoutHints()&LAYOUT_FIX_WIDTH) t=child->getWidth(); else t=child->getDefaultWidth();
        s+=t;
        }
      }
    s+=2;
    }
  return s;
  }


// Get height
FXint FXTabBar::getDefaultHeight(){
  register FXWindow* child;
  register int t,s;
  if(options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS)){
    for(s=0,child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        if(child->getLayoutHints()&LAYOUT_FIX_HEIGHT) t=child->getHeight(); else t=child->getDefaultHeight();
        s+=t;
        }
      }
    s+=3;
    }
  else{
    for(s=0,child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        if(child->getLayoutHints()&LAYOUT_FIX_HEIGHT) t=child->getHeight(); else t=child->getDefaultHeight();
        if(s<t) s=t;
        }
      }
    s+=2;
    }
  return s;
  }


// Recalculate layout
void FXTabBar::layout(){
  register int i,w,h,y,x,tabsize;
  register FXWindow *c;
//fprintf(stderr,"%s::layout w=%d h=%d\n",getClassName(),width,height);
  if(options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS)){
    x=0;
    tabsize=getDefaultWidth();
    if(options&TABBOOK_RIGHTTABS) x=width-tabsize-2;
    for(i=0,y=2,c=getFirst(); c; c=c->getNext(),i++){
      h=c->getDefaultHeight();
      if(active==i){
        c->position(x,y-2,tabsize+1,h+3);
        c->raise();
        }
      else {
        c->position(x+2,y,tabsize-1,h);
        c->lower();
        }
      y+=h;
      }
    }
  else{
    y=0;
    tabsize=getDefaultHeight();
    if(options&TABBOOK_BOTTOMTABS) y=height-tabsize-2;
    for(i=0,x=2,c=getFirst(); c; c=c->getNext(),i++){
      w=c->getDefaultWidth();
      if(active==i){
        c->position(x-2,y,w+3,tabsize+1);
        c->raise();
        }
      else {
        c->position(x,y+2,w,tabsize-1);
        c->lower();
        }
      x+=w;
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Set current subwindow
void FXTabBar::setActive(FXint t){
  if(0<=t && t!=active){
    active=t;
    layout();
    }
  }


// Button being pressed
long FXTabBar::onButtonPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXWindow* win=getChildAt(event->win_x,event->win_y);
  FXWindow *c;
  FXint i;
  setFocus();
  for(i=0,c=getFirst(); c; c=c->getNext(),i++){
    if(win==c && i!=active){
      setActive(i);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)i);
      }
    }
  return 1;
  }
  

// Button being released
long FXTabBar::onButtonRelease(FXObject*,FXSelector,void*){
  return 1;
  }


// Gained focus
long FXTabBar::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXComposite::onFocusIn(sender,sel,ptr);
  return 1;
  }

  
// Lost focus
long FXTabBar::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXComposite::onFocusOut(sender,sel,ptr);
  return 1;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXTabBook) FXTabBookMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTabBook::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTabBook::onButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTabBook::onButtonRelease),
  };


// Object implementation
FXIMPLEMENT(FXTabBook,FXTabBar,FXTabBookMap,ARRAYNUMBER(FXTabBookMap))


// Make a tab book
FXTabBook::FXTabBook(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXTabBar(p,tgt,sel,opts,x,y,w,h){
  switcher=new FXSwitcher(this);
  }


FXint FXTabBook::tabwidth() const {
  register FXint t,wcum,wmax;
  register FXWindow* child;
  wcum=wmax=0;
  for(child=getFirst()->getNext(); child; child=child->getNext()){
    if(child->shown()){
      if(child->getLayoutHints()&LAYOUT_FIX_WIDTH) t=child->getWidth(); else t=child->getDefaultWidth();
      if(wmax<t) wmax=t;
      wcum+=t;
      }
    }
  return options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS) ? wmax : wcum;
  }


FXint FXTabBook::tabheight() const {
  register FXint t,hcum,hmax;
  register FXWindow* child;
  hcum=hmax=0;
  for(child=getFirst()->getNext(); child; child=child->getNext()){
    if(child->shown()){
      if(child->getLayoutHints()&LAYOUT_FIX_HEIGHT) t=child->getHeight(); else t=child->getDefaultHeight();
      if(hmax<t) hmax=t;
      hcum+=t;
      }
    }
  return options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS) ? hcum : hmax;
  }


// Get width
FXint FXTabBook::getDefaultWidth(){
  FXint tw=tabwidth();
  if(options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS))
    return tw+switcher->getDefaultWidth();
  else
    return FXMAX(tw,switcher->getDefaultWidth());
  }


// Get height
FXint FXTabBook::getDefaultHeight(){
  FXint th=tabheight();
  if(options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS))
    return FXMAX(th,switcher->getDefaultHeight());
  else
    return th+switcher->getDefaultHeight();
  }


// Recalculate layout
void FXTabBook::layout(){
  register int i,w,h,y,x,tabsize;
  register FXWindow* child;
//fprintf(stderr,"%s::layout w=%d h=%d\n",getClassName(),width,height);
  if(options&(TABBOOK_LEFTTABS|TABBOOK_RIGHTTABS)){
    x=0;
    tabsize=tabwidth();
    if(options&TABBOOK_RIGHTTABS) x=width-tabsize-2;
    for(i=0,y=2,child=getFirst()->getNext(); child; child=child->getNext(),i++){
      h=child->getDefaultHeight();
      if(active==i){
        child->position(x,y-2,tabsize+1,h+3);
        child->raise();
        }
      else {
        child->position(x+2,y,tabsize-1,h);
        child->lower();
        }
      y+=h;
      }
    }
  else{
    y=0;
    tabsize=tabheight();
    if(options&TABBOOK_BOTTOMTABS) y=height-tabsize-2;
    for(i=0,x=2,child=getFirst()->getNext(); child; child=child->getNext(),i++){
      w=child->getDefaultWidth();
      if(active==i){
        child->position(x-2,y,w+3,tabsize+1);
        child->raise();
        }
      else{
        child->position(x,y+2,w,tabsize-1);
        child->lower();
        }
      x+=w;
      }
    }
  if(options&TABBOOK_LEFTTABS){
    switcher->position(tabsize,0,width-tabsize,height);
    }
  else if(options&TABBOOK_RIGHTTABS){
    switcher->position(0,0,width-tabsize,height);
    }
  else if(options&TABBOOK_BOTTOMTABS){
    switcher->position(0,0,width,height-tabsize);
    }
  else{ /*options&TABBOOK_TOPTABS*/
    switcher->position(0,tabsize,width,height-tabsize);
    }
  flags&=~FLAG_DIRTY;
  }


// Button being pressed
long FXTabBook::onButtonPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXWindow* win=getChildAt(event->win_x,event->win_y);
  FXWindow* c;
  FXint i;
  setFocus();
  for(i=0,c=getFirst()->getNext(); c; c=c->getNext(),i++){
    if(win==c && i!=active){
      setActive(i);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)i);
      }
    }
  return 1;
  }
  

// Button being released
long FXTabBook::onButtonRelease(FXObject*,FXSelector,void*){
  return 1;
  }


// Set current subwindow
void FXTabBook::setActive(FXint t){
  if(0<=t && t!=active){
    active=t;
    switcher->setCurrent(t);
    layout();
    }
  }


