/********************************************************************************
*                                                                               *
*                  R a d i o   B u t t o n    O b j e c t                       *
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
* $Id: FXRadioButton.cpp,v 1.5 1998/10/30 04:49:08 jeroen Exp $                 *
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
#include "FXRadioButton.h"

/*
  To do:
  - Need check-style also (stay in when pressed, pop out when unpressed).
  - Who owns the icon(s)?
  - Arrow buttons should auto-repeat with a timer of some kind
  - "&Label\tTooltip\tHelptext\thttp://server/application/helponitem.html"
  - CheckButton should send SEL_COMMAND.
  - Default button mode:- should somehow get focus.
  - Add button multiple-click translations elsewhere
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXRadioButton) FXRadioButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXRadioButton::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXRadioButton::onUpdate),
  FXMAPFUNC(SEL_ENTER,0,FXRadioButton::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXRadioButton::onLeave),
  FXMAPFUNC(SEL_FOCUSIN,0,FXRadioButton::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXRadioButton::onFocusOut),
  FXMAPFUNC(SEL_ACTIVATE,0,FXRadioButton::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXRadioButton::onDeactivate),
  FXMAPFUNC(SEL_UNCHECK_RADIO,0,FXRadioButton::onUncheckRadio),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXRadioButton::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXRadioButton::onQueryHelp),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CHECK,FXRadioButton::onCheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNCHECK,FXRadioButton::onUncheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNKNOWN,FXRadioButton::onUnknown),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXRadioButton::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXRadioButton::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXRadioButton::onCmdGetIntValue),
  };


// Object implementation
FXIMPLEMENT(FXRadioButton,FXLabel,FXRadioButtonMap,ARRAYNUMBER(FXRadioButtonMap))


// Deserialization
FXRadioButton::FXRadioButton(){
  check=FALSE;
  oldcheck=FALSE;
  radioColor=0;
  }


// Make a check button
FXRadioButton::FXRadioButton(FXComposite* p,const char* text,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXLabel(p,text,NULL,opts,x,y,w,h,pl,pr,pt,pb),
  tip(text,'\t',1),
  help(text,'\t',2){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  check=FALSE;
  oldcheck=FALSE;
  radioColor=0;
  }


// Make X window
void FXRadioButton::create(){
  FXLabel::create();
  radioColor=acquireColor(FXRGB(255,255,255));
  }


// If window can have focus
FXbool FXRadioButton::canFocus() const { return 1; }


// Enable the window
void FXRadioButton::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXLabel::enable();
    update(0,0,width,height);
    }
  }


// Disable the window
void FXRadioButton::disable(){
  if(flags&FLAG_ENABLED){
    FXLabel::disable();
    update(0,0,width,height);
    }
  }


// Get default width
FXint FXRadioButton::getDefaultWidth(){
  FXint tw=0,s=0,w;
  if(label.text()){ 
    tw=labelWidth();
    s=4;
    }
  if(!(options&(ICON_AFTER_TEXT|ICON_BEFORE_TEXT))) w=FXMAX(tw,13); else w=tw+13+s;
  return padleft+padright+w+(border<<1);
  }


// Get default height
FXint FXRadioButton::getDefaultHeight(){
  FXint th=0,h;
  if(label.text()){ 
    th=labelHeight();
    }
  if(!(options&(ICON_ABOVE_TEXT|ICON_BELOW_TEXT))) h=FXMAX(th,13); else h=th+13;
  return padtop+padbottom+h+(border<<1);
  }


// Check button
void FXRadioButton::setCheck(FXuint s){
  if(check!=s){
    check=s;
    update(0,0,width,height);
    }
  }


// Change state to checked
long FXRadioButton::onCheck(FXObject*,FXSelector,void*){ 
  setCheck(TRUE); 
  return 1; 
  }


// Change state to unchecked
long FXRadioButton::onUncheck(FXObject*,FXSelector,void*){ 
  setCheck(FALSE); 
  return 1; 
  }


// Change state to indeterminate
long FXRadioButton::onUnknown(FXObject*,FXSelector,void*){ 
  setCheck(MAYBE); 
  return 1; 
  }


// Update value from a message
long FXRadioButton::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setCheck((FXint)ptr);
  return 1;
  }


// Update value from a message
long FXRadioButton::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setCheck(*((FXint*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXRadioButton::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXint*)ptr)=getCheck(); return 1; }
  return 0;
  }


// Uncheck radio button, sent from parent
long FXRadioButton::onUncheckRadio(FXObject*,FXSelector,void*){
  if(getCheck()){
    setCheck(FALSE);
    if(target && target->handle(this,MKUINT(message,SEL_COMMAND),(void*)check)) return 1;
    }
  return 0;
  }


// Gained focus
long FXRadioButton::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusIn(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }

  
// Lost focus
long FXRadioButton::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusOut(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }


// Handle repaint 
long FXRadioButton::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint tw=0,th=0,tx,ty,ix,iy;
  FXASSERT(xid!=0);
  
  FXFrame::onPaint(sender,sel,ptr);
  
  if(label.text()){
    tw=font->getTextWidth(label.text(),label.length());
    th=font->getFontHeight();
    }

  just_x(tx,ix,tw,13);
  just_y(ty,iy,th,13);

  setForeground(radioColor);
  fillArc(ix+1,iy+1,11,11,0,23040);

  setForeground(shadowColor);
  drawArc(ix,iy,13,13,45*64,180*64);

  setForeground(hiliteColor);
  drawArc(ix,iy,13,13,225*64,180*64);

  setForeground(borderColor);
  drawArc(ix+1,iy+1,11,11,45*64,180*64);

  setForeground(backColor);
  drawArc(ix+1,iy+1,11,11,225*64,180*64);

  if(check!=FALSE){
    if(isEnabled())
      setForeground(textColor);
    else
      setForeground(shadowColor);
    fillArc(ix+3,iy+3,7,7,0,23040);
    }

  if(label.text()){
    setTextFont(font);
    if(isEnabled()){
      setForeground(textColor);
      drawLabel(tx,ty,tw,th);
      if(hasFocus()){
        drawFocusRectangle(tx-1,ty-1,tw+2,th+2);
        }
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


// Implement auto-hide or auto-gray modes
long FXRadioButton::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXLabel::onUpdate(sender,sel,ptr)){
    if(options&RADIOBUTTON_AUTOHIDE){if(shown()){hide();recalc();}}
    if(options&RADIOBUTTON_AUTOGRAY){disable();}
    }
  else{
    if(options&RADIOBUTTON_AUTOHIDE){if(!shown()){show();recalc();}}
    if(options&RADIOBUTTON_AUTOGRAY){enable();}
    }
  return 1;
  }


// Button being pressed
long FXRadioButton::onActivate(FXObject*,FXSelector,void*){
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  oldcheck=check;
  setCheck(TRUE);
  return 0;
  }


// Button being released
long FXRadioButton::onDeactivate(FXObject*,FXSelector,void*){
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  if(oldcheck!=check){ 
    getParent()->handle(this,MKUINT(0,SEL_UNCHECK_OTHER),NULL); 
    if(target && target->handle(this,MKUINT(message,SEL_COMMAND),(void*)check)) return 1;
    }
  return 0;
  }


// Entered button
long FXRadioButton::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onEnter(sender,sel,ptr);
  if(isEnabled()){
    if(flags&FLAG_PRESSED) setCheck(TRUE);
    return 1;
    }
  return 0;
  }


// Left button
long FXRadioButton::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onLeave(sender,sel,ptr);
  if(isEnabled()){
    if(flags&FLAG_PRESSED) setCheck(oldcheck);
    return 1;
    }
  return 0;
  }


// We were asked about status text
long FXRadioButton::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXRadioButton::onQueryTip(FXObject* sender,FXSelector,void*){
  if(tip.text() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&tip);
    return 1;
    }
  return 0;
  }


// Set box color
void FXRadioButton::setRadioColor(FXPixel clr){
  radioColor=clr;
  update(0,0,width,height);
  }


// Save object to stream
void FXRadioButton::save(FXStream& store) const {
  FXLabel::save(store);
  store << check;
  store << oldcheck;
  }


// Load object from stream
void FXRadioButton::load(FXStream& store){
  FXLabel::load(store);
  store >> check;
  store >> oldcheck;
  }  

