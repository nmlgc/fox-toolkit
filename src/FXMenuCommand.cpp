/********************************************************************************
*                                                                               *
*                         M e n u   C o m m a n d    W i d g e t                *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXMenuCommand.cpp,v 1.39 2002/01/18 22:43:01 jeroen Exp $                *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXMenuCommand.h"

/*
  Notes:
  - Accelerators.
  - Help text from constructor is third part; second part should be
    accelerator key combination.
  - When menu label changes, hotkey might have to be adjusted.
  - Fix it so menu stays up when after Alt-F, you press Alt-E.
  - MenuItems should be derived from FXLabel.
  - FXMenuCascade should send ID_POST/IDUNPOST to self.
  - Look into SEL_FOCUS_SELF some more...
  - We handle left, middle, right mouse buttons exactly the same;
    this permits popup menus posted by any mouse button.
*/


#define LEADSPACE   22
#define TRAILSPACE  16


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuCommand) FXMenuCommandMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuCommand::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXMenuCommand::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuCommand::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuCommand::onButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuCommand::onButtonRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuCommand::onButtonPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuCommand::onButtonRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuCommand::onButtonPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuCommand::onButtonRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXMenuCommand::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXMenuCommand::onKeyRelease),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXMenuCommand::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXMenuCommand::onHotKeyRelease),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CHECK,FXMenuCommand::onCheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNCHECK,FXMenuCommand::onUncheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXMenuCommand::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXMenuCommand::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXMenuCommand::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_ACCEL,FXMenuCommand::onCmdAccel),
  };


// Object implementation
FXIMPLEMENT(FXMenuCommand,FXMenuCaption,FXMenuCommandMap,ARRAYNUMBER(FXMenuCommandMap))


// Command menu item
FXMenuCommand::FXMenuCommand(){
  flags|=FLAG_ENABLED;
  state=MENUSTATE_NORMAL;
  acckey=0;
  }


// Command menu item
FXMenuCommand::FXMenuCommand(FXComposite* p,const FXString& text,FXIcon* ic,FXObject* tgt,FXSelector sel,FXuint opts):
  FXMenuCaption(p,text,ic,opts){
  FXAccelTable *table;
  FXWindow *owner;
  flags|=FLAG_ENABLED;
  defaultCursor=getApp()->getDefaultCursor(DEF_RARROW_CURSOR);
  target=tgt;
  message=sel;
  state=MENUSTATE_NORMAL;
  accel=text.extract(1,'\t');
  acckey=fxparseaccel(accel.text());
  if(acckey){
    owner=getShell()->getOwner();
    if(owner){
      table=owner->getAccelTable();
      if(table){
        table->addAccel(acckey,this,MKUINT(ID_ACCEL,SEL_COMMAND));
        }
      }
    }
  }


// Get default width
FXint FXMenuCommand::getDefaultWidth(){
  FXint tw,aw,iw;
  tw=aw=iw=0;
  if(!label.empty()) tw=font->getTextWidth(label.text(),label.length());
  if(!accel.empty()) aw=font->getTextWidth(accel.text(),accel.length());
  if(aw && tw) aw+=5;
  if(icon) iw=icon->getWidth()+5;
  return FXMAX(iw,LEADSPACE)+tw+aw+TRAILSPACE;
  }


// Get default height
FXint FXMenuCommand::getDefaultHeight(){
  FXint th,ih,h;
  th=ih=h=0;
  if(!label.empty() || !accel.empty()) th=font->getFontHeight()+5;
  if(icon) ih=icon->getHeight()+5;
  return FXMAX(th,ih);
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


// Update value from a message
long FXMenuCommand::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  if(ptr) check(); else uncheck();
  return 1;
  }


// Update value from a message
long FXMenuCommand::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  if(*((FXint*)ptr)) check(); else uncheck();
  return 1;
  }


// Obtain value from text field
long FXMenuCommand::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  *((FXint*)ptr)=isChecked();
  return 1;
  }


// Enter
long FXMenuCommand::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuCaption::onEnter(sender,sel,ptr);
  if(isEnabled() && canFocus()) setFocus();
  return 1;
  }


// Leave
long FXMenuCommand::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuCaption::onLeave(sender,sel,ptr);
  if(isEnabled() && canFocus()) killFocus();
  return 1;
  }


// Pressed button
long FXMenuCommand::onButtonPress(FXObject*,FXSelector,void*){
  if(!isEnabled()) return 0;
  return 1;
  }


// Released button
long FXMenuCommand::onButtonRelease(FXObject*,FXSelector,void*){
  FXbool active=isActive();
  if(!isEnabled()) return 0;
  getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  if(active && target){ target->handle(this,MKUINT(message,SEL_COMMAND),(void*)1); }
  return 1;
  }


// Keyboard press
long FXMenuCommand::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  FXTRACE((200,"%s::onKeyPress %p keysym=0x%04x state=%04x\n",getClassName(),this,event->code,event->state));
  //if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
  switch(event->code){
    case KEY_KP_Enter:
    case KEY_Return:
    case KEY_space:
    case KEY_KP_Space:
      return 1;
    }
  return 0;
  }


// Keyboard release
long FXMenuCommand::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  FXTRACE((200,"%s::onKeyRelease %p keysym=0x%04x state=%04x\n",getClassName(),this,event->code,event->state));
  //if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
  switch(event->code){
    case KEY_KP_Enter:
    case KEY_Return:
    case KEY_space:
    case KEY_KP_Space:
      getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)1);
      return 1;
    }
  return 0;
  }


// Hot key combination pressed
long FXMenuCommand::onHotKeyPress(FXObject*,FXSelector,void* ptr){
  FXTRACE((200,"%s::onHotKeyPress %p\n",getClassName(),this));
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  return 1;
  }


// Hot key combination released
long FXMenuCommand::onHotKeyRelease(FXObject*,FXSelector,void*){
  FXTRACE((200,"%s::onHotKeyRelease %p\n",getClassName(),this));
  if(isEnabled()){
    getParent()->handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)1);
    }
  return 1;
  }


// Accelerator activated
long FXMenuCommand::onCmdAccel(FXObject*,FXSelector,void*){
  if(isEnabled()){
    if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)1);
    return 1;
    }
  return 0;
  }


// Into focus chain
void FXMenuCommand::setFocus(){
  FXMenuCaption::setFocus();
  flags|=FLAG_ACTIVE;
  flags&=~FLAG_UPDATE;
  update();
  }


// Out of focus chain
void FXMenuCommand::killFocus(){
  FXMenuCaption::killFocus();
  flags&=~FLAG_ACTIVE;
  flags|=FLAG_UPDATE;
  update();
  }


// Show checked
void FXMenuCommand::check(){
  if(state!=MENUSTATE_CHECKED){
    state=MENUSTATE_CHECKED;
    update();
    }
  }


// Show unchecked
void FXMenuCommand::uncheck(){
  if(state!=MENUSTATE_NORMAL){
    state=MENUSTATE_NORMAL;
    update();
    }
  }


// Check if checked
FXint FXMenuCommand::isChecked() const {
  return (state==MENUSTATE_CHECKED);
  }


// Show radio checked
void FXMenuCommand::checkRadio(){
  if(state!=MENUSTATE_RCHECKED){
    state=MENUSTATE_RCHECKED;
    update();
    }
  }


// Show radio unchecked
void FXMenuCommand::uncheckRadio(){
  if(state!=MENUSTATE_NORMAL){
    state=MENUSTATE_NORMAL;
    update();
    }
  }


// See if radio checked
FXint FXMenuCommand::isRadioChecked() const {
  return (state==MENUSTATE_RCHECKED);
  }


// Handle repaint
long FXMenuCommand::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint xx,yy;

  xx=LEADSPACE;

  // Grayed out
  if(!isEnabled()){
    dc.setForeground(backColor);
    dc.fillRectangle(0,0,width,height);
    if(icon){
      dc.drawIconSunken(icon,3,(height-icon->getHeight())/2);
      if(icon->getWidth()+5>xx) xx=icon->getWidth()+5;
      }
    if(!label.empty()){
      yy=font->getFontAscent()+(height-font->getFontHeight())/2;
      dc.setTextFont(font);
      dc.setForeground(hiliteColor);
      dc.drawText(xx+1,yy+1,label.text(),label.length());
      dc.setForeground(shadowColor);
      dc.drawText(xx,yy,label.text(),label.length());
      if(!accel.empty()) dc.drawText(width-TRAILSPACE-font->getTextWidth(accel.text(),accel.length()),yy,accel.text(),accel.length());
      if(0<=hotoff){
        dc.fillRectangle(xx+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
        }
      }
    if(state==MENUSTATE_CHECKED){
      dc.setForeground(hiliteColor);
      drawCheck(dc,6,1+(height-8)/2);
      dc.setForeground(shadowColor);
      drawCheck(dc,5,(height-8)/2);
      }
    if(state==MENUSTATE_RCHECKED){
      dc.setForeground(hiliteColor);
      drawBullit(dc,8,1+(height-5)/2);
      dc.setForeground(shadowColor);
      drawBullit(dc,7,(height-5)/2);
      }
    }

  // Active
  else if(isActive()){
    dc.setForeground(selbackColor);
    dc.fillRectangle(0,0,width,height);
    if(icon){
      dc.drawIcon(icon,3,(height-icon->getHeight())/2);
      if(icon->getWidth()+5>xx) xx=icon->getWidth()+5;
      }
    if(!label.empty()){
      yy=font->getFontAscent()+(height-font->getFontHeight())/2;
      dc.setTextFont(font);
      dc.setForeground(isEnabled() ? seltextColor : shadowColor);
      dc.drawText(xx,yy,label.text(),label.length());
      if(!accel.empty()) dc.drawText(width-TRAILSPACE-font->getTextWidth(accel.text(),accel.length()),yy,accel.text(),accel.length());
      if(0<=hotoff){
        dc.fillRectangle(xx+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
        }
      }
    if(state==MENUSTATE_CHECKED){
      dc.setForeground(seltextColor);
      drawCheck(dc,5,(height-8)/2);
      }
    if(state==MENUSTATE_RCHECKED){
      dc.setForeground(seltextColor);
      drawBullit(dc,7,(height-5)/2);
      }
    }

  // Normal
  else{
    dc.setForeground(backColor);
    dc.fillRectangle(0,0,width,height);
    if(icon){
      dc.drawIcon(icon,3,(height-icon->getHeight())/2);
      if(icon->getWidth()+5>xx) xx=icon->getWidth()+5;
      }
    if(!label.empty()){
      yy=font->getFontAscent()+(height-font->getFontHeight())/2;
      dc.setTextFont(font);
      dc.setForeground(textColor);
      dc.drawText(xx,yy,label.text(),label.length());
      if(!accel.empty()) dc.drawText(width-TRAILSPACE-font->getTextWidth(accel.text(),accel.length()),yy,accel.text(),accel.length());
      if(0<=hotoff){
        dc.fillRectangle(xx+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
        }
      }
    if(state==MENUSTATE_CHECKED){
      dc.setForeground(textColor);
      drawCheck(dc,5,(height-8)/2);
      }
    if(state==MENUSTATE_RCHECKED){
      dc.setForeground(textColor);
      drawBullit(dc,7,(height-5)/2);
      }
    }
  return 1;
  }


// Draw check mark
void FXMenuCommand::drawCheck(FXDCWindow& dc,FXint x,FXint y){
  FXSegment seg[6];
  seg[0].x1=1+x; seg[0].y1=3+y; seg[0].x2=3+x; seg[0].y2=5+y;
  seg[1].x1=1+x; seg[1].y1=4+y; seg[1].x2=3+x; seg[1].y2=6+y;
  seg[2].x1=1+x; seg[2].y1=5+y; seg[2].x2=3+x; seg[2].y2=7+y;
  seg[3].x1=3+x; seg[3].y1=5+y; seg[3].x2=7+x; seg[3].y2=1+y;
  seg[4].x1=3+x; seg[4].y1=6+y; seg[4].x2=7+x; seg[4].y2=2+y;
  seg[5].x1=3+x; seg[5].y1=7+y; seg[5].x2=7+x; seg[5].y2=3+y;
  dc.drawLineSegments(seg,6);
  }


// Draw bullit
void FXMenuCommand::drawBullit(FXDCWindow& dc,FXint x,FXint y){
  FXSegment seg[5];
  seg[0].x1= 1+x; seg[0].y1 = 0+y; seg[0].x2 = 3+x; seg[0].y2 = 0+y;
  seg[1].x1= 0+x; seg[1].y1 = 1+y; seg[1].x2 = 4+x; seg[1].y2 = 1+y;
  seg[2].x1= 0+x; seg[2].y1 = 2+y; seg[2].x2 = 4+x; seg[2].y2 = 2+y;
  seg[3].x1= 0+x; seg[3].y1 = 3+y; seg[3].x2 = 4+x; seg[3].y2 = 3+y;
  seg[4].x1= 1+x; seg[4].y1 = 4+y; seg[4].x2 = 3+x; seg[4].y2 = 4+y;
  dc.drawLineSegments(seg,5);
  }


// Change accelerator text; note this just changes the text!
// This is because the accelerator's target may be different from
// the MenuCommands and we don't want to blow it away.
void FXMenuCommand::setAccelText(const FXString& text){
  if(accel!=text){
    accel=text;
    recalc();
    update();
    }
  }


// Save object to stream
void FXMenuCommand::save(FXStream& store) const {
  FXMenuCaption::save(store);
  store << state;
  store << accel;
  store << acckey;
  }


// Load object from stream
void FXMenuCommand::load(FXStream& store){
  FXMenuCaption::load(store);
  store >> state;
  store >> accel;
  store >> acckey;
  }


// Need to uninstall accelerator
FXMenuCommand::~FXMenuCommand(){
  FXAccelTable *table;
  FXWindow *owner;
  if(acckey){
    owner=getShell()->getOwner();
    if(owner){
      table=owner->getAccelTable();
      if(table){
        table->removeAccel(acckey);
        }
      }
    }
  }
