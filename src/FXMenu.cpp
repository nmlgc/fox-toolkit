/********************************************************************************
*                                                                               *
*                             M e n u   O b j e c t s                           *
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
* $Id: FXMenu.cpp,v 1.56 1998/10/30 04:49:08 jeroen Exp $                       *
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
  - FXMenuCascade should send ID_POST/IDUNPOST to self.
*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXMenuItem,FXFrame,NULL,0)


// Generic menu item
FXMenuItem::FXMenuItem(FXComposite* p,FXuint opts):
  FXFrame(p,opts, 0,0, 0,0){ 
  }


void FXMenuItem::create(){
  FXFrame::create();
  show();
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuSeparator) FXMenuSeparatorMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuSeparator::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXMenuSeparator,FXMenuItem,FXMenuSeparatorMap,ARRAYNUMBER(FXMenuSeparatorMap))


// Separator item
FXMenuSeparator::FXMenuSeparator(FXComposite* p,FXuint opts):
  FXMenuItem(p,opts){ 
  defaultCursor=getApp()->rarrowCursor;
  }


// Handle repaint 
long FXMenuSeparator::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onPaint(sender,sel,ptr);
  setForeground(shadowColor);
  drawLine(1,0,width-1,0);
  setForeground(hiliteColor);
  drawLine(1,1,width-1,1);
  return 1;
  }


// Get default size
FXint FXMenuSeparator::getDefaultHeight(){ return 2; }


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuEntry) FXMenuEntryMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuEntry::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXMenuEntry::onUpdate),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXMenuEntry::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXMenuEntry,FXMenuItem,FXMenuEntryMap,ARRAYNUMBER(FXMenuEntryMap))



// Text menu item
FXMenuEntry::FXMenuEntry(FXComposite* p,const char* text,FXuint opts):
  FXMenuItem(p,opts),
  label(text,'\t','&',0),
  accel(text,'\t',1),
  help(text,'\t',2){
  font=getApp()->normalFont;
  hotkey=fxparsehotkey(text);
  hotoff=fxfindhotkeyoffset(text);
  addHotKey(hotkey);
  textColor=0;
  seltextColor=0;
  selbackColor=0;
  }


// Create X Window
void FXMenuEntry::create(){
  FXMenuItem::create();
  textColor=acquireColor(getApp()->foreColor);
  seltextColor=acquireColor(getApp()->selforeColor);
  selbackColor=acquireColor(getApp()->selbackColor);
  font->create();
  }

  
// Change text
void FXMenuEntry::setText(const FXchar* text){
  if(label!=text){
    label=FXString(text,'\t','&',0);
    remHotKey(hotkey);
    hotkey=fxparsehotkey(text);
    hotoff=fxfindhotkeyoffset(text);
    addHotKey(hotkey);
    recalc();
    update(0,0,width,height);
    }
  }


// Change font
void FXMenuEntry::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Set text color
void FXMenuEntry::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Set select background color
void FXMenuEntry::setSelBackColor(FXPixel clr){
  selbackColor=clr;
  update(0,0,width,height);
  }


// Set selected text color
void FXMenuEntry::setSelTextColor(FXPixel clr){
  seltextColor=clr;
  update(0,0,width,height);
  }


// Handle repaint 
long FXMenuEntry::onPaint(FXObject*,FXSelector,void*){
  FXASSERT(xid);
  if(isActive()){
    setForeground(selbackColor);
    fillRectangle(1,1,width-2,height-2);
    if(label.text()){
      setTextFont(font);
      setForeground(isEnabled() ? seltextColor : shadowColor);
      drawText(16,1+font->getFontAscent(),label.text(),label.length());
      if(accel.text()) drawText(width-14-font->getTextWidth(accel.text(),accel.length()),1+font->getFontAscent(),accel.text(),accel.length());
      if(options&MENU_DEFAULT){
        drawText(17,1+font->getFontAscent(),label.text(),label.length());
        if(accel.text()) drawText(width-15-font->getTextWidth(accel.text(),accel.length()),1+font->getFontAscent(),accel.text(),accel.length());
        }
      if(0<=hotoff){
        drawLine(17+font->getTextWidth(label.text(),hotoff),2+font->getFontAscent(),17+font->getTextWidth(label.text(),hotoff+1)-1,2+font->getFontAscent());
        }
      }
    }
  else{
    setForeground(backColor);
    fillRectangle(0,0,width,height);
    if(label.text()){
      setTextFont(font);
      if(isEnabled()){
        setForeground(textColor);
        drawText(16,1+font->getFontAscent(),label.text(),label.length());
        if(accel.text()) drawText(width-14-font->getTextWidth(accel.text(),accel.length()),1+font->getFontAscent(),accel.text(),accel.length());
        if(options&MENU_DEFAULT){
          drawText(17,1+font->getFontAscent(),label.text(),label.length());
          if(accel.text()) drawText(width-15-font->getTextWidth(accel.text(),accel.length()),1+font->getFontAscent(),accel.text(),accel.length());
          }
        if(0<=hotoff){
          drawLine(17+font->getTextWidth(label.text(),hotoff),2+font->getFontAscent(),17+font->getTextWidth(label.text(),hotoff+1)-1,2+font->getFontAscent());
          }
        }
      else{
        setForeground(hiliteColor);
        drawText(17,2+font->getFontAscent(),label.text(),label.length());
        setForeground(shadowColor);
        drawText(16,1+font->getFontAscent(),label.text(),label.length());
        if(accel.text()) drawText(width-14-font->getTextWidth(accel.text(),accel.length()),1+font->getFontAscent(),accel.text(),accel.length());
        if(0<=hotoff){
          drawLine(17+font->getTextWidth(label.text(),hotoff),2+font->getFontAscent(),17+font->getTextWidth(label.text(),hotoff+1)-1,2+font->getFontAscent());
          }
        }
      }
    }
  return 1;
  }


// We were asked about status text
long FXMenuEntry::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// Implement auto-hide or auto-gray modes
long FXMenuEntry::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXMenuItem::onUpdate(sender,sel,ptr)){
    if(options&MENU_AUTOHIDE){if(shown()){hide();recalc();}}
    if(options&MENU_AUTOGRAY){disable();}
    }
  else{
    if(options&MENU_AUTOHIDE){if(!shown()){show();recalc();}}
    if(options&MENU_AUTOGRAY){enable();}
    }
  return 1;
  }


// Get default width
FXint FXMenuEntry::getDefaultWidth(){
  FXint w=0;
  if(label.text()) w+=font->getTextWidth(label.text(),label.length());
  if(accel.text()) w+=font->getTextWidth(accel.text(),accel.length());
  if(label.text() && accel.text()) w+=5;
  return 30+w;
  }


// Get default height
FXint FXMenuEntry::getDefaultHeight(){
  FXint h=0;
  if(label.text() || accel.text()) h=font->getFontHeight();
  return 5+h;
  }


// Change accelerator text
void FXMenuEntry::setAccelText(const FXchar* text){
  accel=text;
  }


// Change help text
void FXMenuEntry::setHelpText(const FXchar* text){
  help=text;
  }


// Zap it
FXMenuEntry::~FXMenuEntry(){
  remHotKey(hotkey);
  font=(FXFont*)-1;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuCommand) FXMenuCommandMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuCommand::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXMenuCommand::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuCommand::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuCommand::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuCommand::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuCommand::onDefault),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuCommand::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuCommand::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuCommand::onDefault),
  FXMAPFUNC(SEL_ACTIVATE,0,FXMenuCommand::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXMenuCommand::onDeactivate),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CHECK,FXMenuCommand::onCheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNCHECK,FXMenuCommand::onUncheck),
  };


// Object implementation
FXIMPLEMENT(FXMenuCommand,FXMenuEntry,FXMenuCommandMap,ARRAYNUMBER(FXMenuCommandMap))


// Command menu item
FXMenuCommand::FXMenuCommand(FXComposite* p,const char* text,FXObject* tgt,FXSelector sel,FXuint opts):
  FXMenuEntry(p,text,opts){
  flags|=FLAG_ENABLED;
  defaultCursor=getApp()->rarrowCursor;
  target=tgt;
  message=sel;
  }


// Create X window
void FXMenuCommand::create(){
  FXMenuEntry::create();
  }


// If window can have focus
FXbool FXMenuCommand::canFocus() const { 
  return 1; 
  }


// Check the menu button
long FXMenuCommand::onCheck(FXObject*,FXSelector,void*){ 
  check(); 
  return 1; 
  }


// Check the menu button
long FXMenuCommand::onUncheck(FXObject*,FXSelector,void*){ 
  uncheck(); 
  return 1; 
  }


// Pressed left button
long FXMenuCommand::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    return handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    }
  return 0;
  }


// Released left button
long FXMenuCommand::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    return handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    }
  return 0;
  }


// Activate
long FXMenuCommand::onActivate(FXObject*,FXSelector,void*){
  return 1;
  }


// Deactivate
long FXMenuCommand::onDeactivate(FXObject* sender,FXSelector,void* ptr){
  FXbool doit=isActive();
  getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);     // Forces popdown!
  if(isEnabled()){
    return doit && target && target->handle(this,MKUINT(message,SEL_COMMAND),ptr);
    }
  return 1;
  }


// Into focus chain
void FXMenuCommand::setFocus(){
  FXMenuEntry::setFocus();
  flags|=FLAG_ACTIVE;
  flags&=~FLAG_UPDATE;
  update(0,0,width,height);
  }


// Out of focus chain
void FXMenuCommand::killFocus(){
  FXMenuEntry::killFocus();
  flags&=~FLAG_ACTIVE;
  flags|=FLAG_UPDATE;
  update(0,0,width,height);
  }


// Enter
long FXMenuCommand::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onEnter(sender,sel,ptr);
  if(isEnabled() && canFocus()) setFocus();
  return 1;
  }


// Leave
long FXMenuCommand::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onLeave(sender,sel,ptr);
  if(isEnabled() && canFocus()) killFocus();
  return 1;
  }


// Show as default
void FXMenuCommand::setDefault(){
  if(!(options&MENU_DEFAULT)){
    options|=MENU_DEFAULT;
    update(0,0,width,height);
    }
  }
    

// Show as other
void FXMenuCommand::setOther(){
  if(options&MENU_DEFAULT){
    options&=~MENU_DEFAULT;
    update(0,0,width,height);
    }
  }


// See if its default
FXint FXMenuCommand::isDefault() const {
  return (options&MENU_DEFAULT)!=0; 
  }


// Enable the menu entry
void FXMenuCommand::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXMenuEntry::enable();
    update(0,0,width,height);
    }
  }


// Disable the menu entry
void FXMenuCommand::disable(){
  if(flags&FLAG_ENABLED){
    FXMenuEntry::disable();
    update(0,0,width,height);
    }
  }


// Show checked
void FXMenuCommand::check(){
  if(!(flags&FLAG_CHECKED)){
    flags|=FLAG_CHECKED;
    update(0,0,width,height);
    }
  }
    

// Show unchecked
void FXMenuCommand::uncheck(){
  if(flags&FLAG_CHECKED){
    flags&=~FLAG_CHECKED;
    update(0,0,width,height);
    }
  }


// Check if checked
FXint FXMenuCommand::isChecked() const {
  return (flags&FLAG_CHECKED)!=0; 
  }


// Show radio checked
void FXMenuCommand::checkRadio(){
  if(!(flags&FLAG_RCHECKED)){
    flags|=FLAG_RCHECKED;
    update(0,0,width,height);
    }
  }


// Show radio unchecked
void FXMenuCommand::uncheckRadio(){
  if(flags&FLAG_RCHECKED){
    flags&=~FLAG_RCHECKED;
    update(0,0,width,height);
    }
  }


// See if radio checked
FXint FXMenuCommand::isRadioChecked() const {
  return (flags&FLAG_RCHECKED)!=0; 
  }


// Handle repaint 
long FXMenuCommand::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onPaint(sender,sel,ptr);
  if(flags&(FLAG_CHECKED|FLAG_RCHECKED)){
    setForeground(isActive() ? seltextColor : textColor);
    if(flags&FLAG_CHECKED) drawCheck(1,3,14,11);
    if(flags&FLAG_RCHECKED) drawBullit(1,3,14,11);
    }
  return 1;
  }


// Draw check mark
void FXMenuCommand::drawCheck(FXint l,FXint t,FXint  ,FXint b){
  FXSegment seg[6];
  FXASSERT(xid!=0);
  t = (t + b - 8) >> 1; ++t;
  seg[0].x1=1+l; seg[0].y1=3+t; seg[0].x2=3+l; seg[0].y2=5+t;
  seg[1].x1=1+l; seg[1].y1=4+t; seg[1].x2=3+l; seg[1].y2=6+t;
  seg[2].x1=1+l; seg[2].y1=5+t; seg[2].x2=3+l; seg[2].y2=7+t;
  seg[3].x1=3+l; seg[3].y1=5+t; seg[3].x2=7+l; seg[3].y2=1+t;
  seg[4].x1=3+l; seg[4].y1=6+t; seg[4].x2=7+l; seg[4].y2=2+t;
  seg[5].x1=3+l; seg[5].y1=7+t; seg[5].x2=7+l; seg[5].y2=3+t;
  drawLineSegments(seg,6);
  }


// Draw bullit
void FXMenuCommand::drawBullit(FXint l,FXint t,FXint r,FXint b){
  FXSegment seg[5];
  FXASSERT(xid!=0);
  t=(t+b-5)>>1; ++t;
  l=(l+r-5)>>1; ++l;
  seg[0].x1= 1+l; seg[0].y1 = 0+t; seg[0].x2 = 3+l; seg[0].y2 = 0+t;
  seg[1].x1= 0+l; seg[1].y1 = 1+t; seg[1].x2 = 4+l; seg[1].y2 = 1+t;
  seg[2].x1= 0+l; seg[2].y1 = 2+t; seg[2].x2 = 4+l; seg[2].y2 = 2+t;
  seg[3].x1= 0+l; seg[3].y1 = 3+t; seg[3].x2 = 4+l; seg[3].y2 = 3+t;
  seg[4].x1= 1+l; seg[4].y1 = 4+t; seg[4].x2 = 3+l; seg[4].y2 = 4+t;
  drawLineSegments(seg,5);
  } 



/*******************************************************************************/

// Map
FXDEFMAP(FXMenuCascade) FXMenuCascadeMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuCascade::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXMenuCascade::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuCascade::onLeave),
  FXMAPFUNC(SEL_TIMEOUT,1,FXMenuCascade::onTimeout),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXMenuCascade::onFocusRight),
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXMenuCascade::onFocusLeft),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuCascade::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuCascade::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuCascade::onDefault),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuCascade::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuCascade::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuCascade::onDefault),
  FXMAPFUNC(SEL_KEYPRESS,0,FXMenuCascade::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXMenuCascade::onKeyRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXMenuCascade::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXMenuCascade::onDeactivate),
  };


// Object implementation
FXIMPLEMENT(FXMenuCascade,FXMenuEntry,FXMenuCascadeMap,ARRAYNUMBER(FXMenuCascadeMap))


// Make cascade menu button
FXMenuCascade::FXMenuCascade(FXComposite* p,const char* text,FXMenuPane* pup,FXuint opts):
  FXMenuEntry(p,text,opts){
  defaultCursor=getApp()->rarrowCursor;
  flags|=FLAG_ENABLED;
  pane=pup;
  timer=NULL;
  }


// Create X window; make sure pane is created
void FXMenuCascade::create(){
  FXMenuEntry::create();
  if(pane) pane->create();
  }


// Destroy X window
void FXMenuCascade::destroy(){
  if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
  FXMenuEntry::destroy();
  }


// If window can have focus
FXbool FXMenuCascade::canFocus() const { 
  return 1; 
  }


// Focus moved to right:- this shows the submenu!
long FXMenuCascade::onFocusRight(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    FXint x,y;
    if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
    translateCoordinatesTo(x,y,getRoot(),width,0);
    pane->popup(((FXMenuPane*)getParent())->getOwner(),x,y);
    }
  return 1;
  }


// Focus moved to left:- this hides the submenu!
long FXMenuCascade::onFocusLeft(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
    pane->popdown();
    return 1;
    }
  return 0;
  }


// When pressed, perform ungrab, then process normally
long FXMenuCascade::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    ungrab();
    return handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    }
  return 0;
  }


// Released left button
long FXMenuCascade::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    return handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    }
  return 0;
  }


// Activate; toggles menu
long FXMenuCascade::onActivate(FXObject*,FXSelector,void*){
  if(isEnabled()){
    if(pane){
      if(!pane->shown()){
        FXint x,y;
        if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
        translateCoordinatesTo(x,y,getRoot(),width,0);
        pane->popup(((FXMenuPane*)getParent())->getOwner(),x,y);
        }
      else{
        pane->popdown();
        }
      }
    }
  return 1;
  }


// Deactivate
long FXMenuCascade::onDeactivate(FXObject*,FXSelector,void*){
  return 1;
  }


// Keyboard press; forward to submenu pane
long FXMenuCascade::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXMenuEntry::onKeyPress(sender,sel,ptr);
  }


// Keyboard release; forward to submenu pane
long FXMenuCascade::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXMenuEntry::onKeyRelease(sender,sel,ptr);
  }


// Into focus chain
void FXMenuCascade::setFocus(){
  FXMenuEntry::setFocus();
  flags|=FLAG_ACTIVE;
  flags&=~FLAG_UPDATE;
  update(0,0,width,height);
  }


// Out of focus chain; hide submenu if it was up
void FXMenuCascade::killFocus(){
  FXMenuEntry::killFocus();
  if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
  if(pane && pane->shown()) pane->popdown();
  flags&=~FLAG_ACTIVE;
  flags|=FLAG_UPDATE;
  update(0,0,width,height);
  }


// Enter; set timer for delayed popup
long FXMenuCascade::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onEnter(sender,sel,ptr);
  if(isEnabled() && canFocus()){
    if(!timer){ timer=getApp()->addTimeout(getApp()->menuPause,this,1); }
    setFocus();
    }
  return 1;
  }


// Leave
long FXMenuCascade::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onLeave(sender,sel,ptr);
  return 1;
  }


// Timeout; pop up the submenu
long FXMenuCascade::onTimeout(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    FXint x,y;
    translateCoordinatesTo(x,y,getRoot(),width,0);
    pane->popup(((FXMenuPane*)getParent())->getOwner(),x,y);
    }
  timer=NULL;
  return 1;
  }


// Handle repaint 
long FXMenuCascade::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onPaint(sender,sel,ptr);
  setForeground(isActive() ? seltextColor : textColor);
  drawTriangle(width-10,3,width-6,11);
  return 1;
  }


// Draw triangle
void FXMenuCascade::drawTriangle(FXint l,FXint t,FXint r,FXint b){
  FXPoint points[3];
  int m=(t+b)/2;
  FXASSERT(xid!=0);
  points[0].x=l;
  points[0].y=t;
  points[1].x=l;
  points[1].y=b;
  points[2].x=r;
  points[2].y=m;
  fillPolygon(points,3);
  }


// Test if logically inside
FXbool FXMenuCascade::contains(FXint parentx,FXint parenty) const {
  FXint x,y;
  if(FXMenuEntry::contains(parentx,parenty)) return 1;
  if(getPopup() && getPopup()->shown()){
    getParent()->translateCoordinatesTo(x,y,getRoot(),parentx,parenty);
    if(getPopup()->contains(x,y)) return 1;
    }
  return 0;
  }



// Delete it
FXMenuCascade::~FXMenuCascade(){
  pane=(FXMenuPane*)-1;
  timer=(FXTimer*)-1;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuTitle) FXMenuTitleMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuTitle::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXMenuTitle::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuTitle::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuTitle::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuTitle::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_KEYPRESS,0,FXMenuTitle::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXMenuTitle::onKeyRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXMenuTitle::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXMenuTitle::onDeactivate),
  FXMAPFUNC(SEL_FOCUS_UP,0,FXMenuTitle::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXMenuTitle::onFocusDown),
  FXMAPFUNC(SEL_FOCUSIN,0,FXMenuTitle::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXMenuTitle::onFocusOut),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_POST,FXMenuTitle::onCmdPost),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXMenuTitle::onCmdUnpost),
  };

  
// Object implementation
FXIMPLEMENT(FXMenuTitle,FXMenuEntry,FXMenuTitleMap,ARRAYNUMBER(FXMenuTitleMap))


// Make a menu title button
FXMenuTitle::FXMenuTitle(FXComposite* p,const char* text,FXMenuPane* pup,FXuint opts):
  FXMenuEntry(p,text,opts){
  flags|=FLAG_ENABLED;
  options&=~LAYOUT_FILL_X;
  pane=pup;
  }


// Create X window; make sure pane is created
void FXMenuTitle::create(){
  FXMenuEntry::create();
  if(pane) pane->create();
  }


// Get default width
FXint FXMenuTitle::getDefaultWidth(){
  return 12+font->getTextWidth(label.text(),label.length());
  }


// Get default height
FXint FXMenuTitle::getDefaultHeight(){
  return 5+font->getFontHeight();
  }


// If window can have focus
FXbool FXMenuTitle::canFocus() const { 
  return 1; 
  }


// When pressed, perform ungrab, then process normally
long FXMenuTitle::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
//    ungrab();
    return handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    }
  return 0;
  }


// Released left button
long FXMenuTitle::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
//    ungrab();
    return handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    }
  return 0;
  }


// Activate
long FXMenuTitle::onActivate(FXObject*,FXSelector,void* ptr){
  if(flags&FLAG_ACTIVE){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    }
  else{
    handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Deactivate
long FXMenuTitle::onDeactivate(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(ev->moved){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
    }
  return 1;
  }


// Post the menu
long FXMenuTitle::onCmdPost(FXObject*,FXSelector,void*){
  FXint x,y;
  if(pane && !pane->shown()){
    translateCoordinatesTo(x,y,getRoot(),0,0);
    pane->popup(getParent(),x-1,y+height);
    if(!getParent()->grabbed()) getParent()->grab();
    }
  flags|=FLAG_ACTIVE;
  update(0,0,width,height);
  return 1;
  }


// Unpost the menu
long FXMenuTitle::onCmdUnpost(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    pane->popdown();
    if(getParent()->grabbed()) getParent()->ungrab();
    }
  flags&=~FLAG_ACTIVE;
  update(0,0,width,height);
  return 1;
  }


// Focus moved down
long FXMenuTitle::onFocusDown(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Focus moved up
long FXMenuTitle::onFocusUp(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Keyboard press; forward to menu pane
long FXMenuTitle::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXMenuEntry::onKeyPress(sender,sel,ptr);
  }


// Keyboard release; forward to menu pane
long FXMenuTitle::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
  return FXMenuEntry::onKeyRelease(sender,sel,ptr);
  }


// Into focus chain
void FXMenuTitle::setFocus(){
  FXWindow *menuitem=getParent()->getFocus();
  FXbool active=menuitem && menuitem->isActive();
  FXMenuEntry::setFocus();
  if(active) handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
  }
    


// Out of focus chain
void FXMenuTitle::killFocus(){
  FXMenuEntry::killFocus();
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  }


// Enter
long FXMenuTitle::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onEnter(sender,sel,ptr);
  if(isEnabled() && canFocus() && getParent()->getFocus()) setFocus();
  return 1;
  }


// Leave
long FXMenuTitle::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onLeave(sender,sel,ptr);
  return 1;
  }


// Handle repaint 
long FXMenuTitle::onPaint(FXObject*,FXSelector,void*){
  if(isActive()){
    setForeground(selbackColor);
    fillRectangle(0,0,width,height);
    if(label.text()){
      setTextFont(font);
      setForeground(seltextColor);
      drawText(6,3+font->getFontAscent(),label.text(),label.length());
      if(0<=hotoff) drawLine(6+font->getTextWidth(label.text(),hotoff),4+font->getFontAscent(),6+font->getTextWidth(label.text(),hotoff+1)-1,4+font->getFontAscent());
      }
    }
  else{
    setForeground(backColor);
    fillRectangle(0,0,width,height);
    if(label.text()){
      setTextFont(font);
      if(isEnabled()){
        setForeground(textColor);
        drawText(6,3+font->getFontAscent(),label.text(),label.length());
        if(0<=hotoff) drawLine(6+font->getTextWidth(label.text(),hotoff),4+font->getFontAscent(),6+font->getTextWidth(label.text(),hotoff+1)-1,4+font->getFontAscent());
        }
      else{
        setForeground(hiliteColor);
        drawText(7,4+font->getFontAscent(),label.text(),label.length());
        setForeground(shadowColor);
        drawText(6,3+font->getFontAscent(),label.text(),label.length());
        if(0<=hotoff) drawLine(6+font->getTextWidth(label.text(),hotoff),4+font->getFontAscent(),6+font->getTextWidth(label.text(),hotoff+1)-1,4+font->getFontAscent());
        }
      }
    }
  return 1;
  }


// Gained focus
long FXMenuTitle::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onFocusIn(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }

  
// Lost focus
long FXMenuTitle::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuEntry::onFocusOut(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }


// Test if logically inside
FXbool FXMenuTitle::contains(FXint parentx,FXint parenty) const {
  FXint x,y;
  if(FXMenuEntry::contains(parentx,parenty)) return 1;
  if(getPopup() && getPopup()->shown()){
    getParent()->translateCoordinatesTo(x,y,getRoot(),parentx,parenty);
    if(getPopup()->contains(x,y)) return 1;
    }
  return 0;
  }


// Delete it
FXMenuTitle::~FXMenuTitle(){
  pane=(FXMenuPane*)-1;
  }


/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXMenuPane,FXPopup,NULL,0)



// Build empty one
FXMenuPane::FXMenuPane(FXApp* a,FXuint opts):
  FXPopup(a,NULL,opts|FRAME_RAISED|FRAME_THICK){
  accelTable=new FXAccelTable;
  }


// Cursor is considered inside when it's in this window, or in any subwindow 
// that's open; we'll find the latter through the cascade menu, by asking it for 
// it's popup window.
FXbool FXMenuPane::contains(FXint parentx,FXint parenty) const {
  FXint x,y;
  if(FXPopup::contains(parentx,parenty)) return 1;
  if(getFocus()){
    getParent()->translateCoordinatesTo(x,y,this,parentx,parenty);
    if(getFocus()->contains(x,y)) return 1;
    }
  return 0;
  }

