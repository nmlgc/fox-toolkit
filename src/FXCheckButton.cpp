/********************************************************************************
*                                                                               *
*                    C h e c k   B u t t o n    O b j e c t                     *
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
* $Id: FXCheckButton.cpp,v 1.6 1998/10/30 04:49:07 jeroen Exp $                 *
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
#include "FXCheckButton.h"

/*
  Notes:
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXCheckButton) FXCheckButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXCheckButton::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXCheckButton::onUpdate),
  FXMAPFUNC(SEL_ENTER,0,FXCheckButton::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXCheckButton::onLeave),
  FXMAPFUNC(SEL_FOCUSIN,0,FXCheckButton::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXCheckButton::onFocusOut),
  FXMAPFUNC(SEL_ACTIVATE,0,FXCheckButton::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXCheckButton::onDeactivate),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXCheckButton::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXCheckButton::onQueryHelp),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CHECK,FXCheckButton::onCheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNCHECK,FXCheckButton::onUncheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNKNOWN,FXCheckButton::onUnknown),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXCheckButton::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXCheckButton::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXCheckButton::onCmdGetIntValue),
  };


// Object implementation
FXIMPLEMENT(FXCheckButton,FXLabel,FXCheckButtonMap,ARRAYNUMBER(FXCheckButtonMap))

  
  
// Deserialization
FXCheckButton::FXCheckButton(){
  check=FALSE;
  oldcheck=FALSE;
  boxColor=0;
  }


// Make a check button
FXCheckButton::FXCheckButton(FXComposite* p,const char* text,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXLabel(p,text,NULL,opts,x,y,w,h,pl,pr,pt,pb),
  tip(text,'\t',1),
  help(text,'\t',2){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  check=FALSE;
  oldcheck=FALSE;
  boxColor=0;
  }


// Make X window
void FXCheckButton::create(){
  FXLabel::create();
  boxColor=acquireColor(FXRGB(255,255,255));
  }


// If window can have focus
FXbool FXCheckButton::canFocus() const { return 1; }


// Enable the window
void FXCheckButton::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXLabel::enable();
    update(0,0,width,height);
    }
  }


// Disable the window
void FXCheckButton::disable(){
  if(flags&FLAG_ENABLED){
    FXLabel::disable();
    update(0,0,width,height);
    }
  }


// Get default width
FXint FXCheckButton::getDefaultWidth(){
  FXint tw=0,s=0,w;
  if(label.text()){ 
    tw=labelWidth();
    s=4;
    }
  if(!(options&(ICON_AFTER_TEXT|ICON_BEFORE_TEXT))) w=FXMAX(tw,13); else w=tw+13+s;
  return padleft+padright+w+(border<<1);
  }


// Get default height
FXint FXCheckButton::getDefaultHeight(){
  FXint th=0,h;
  if(label.text()){ 
    th=labelHeight();
    }
  if(!(options&(ICON_ABOVE_TEXT|ICON_BELOW_TEXT))) h=FXMAX(th,13); else h=th+13;
  return padtop+padbottom+h+(border<<1);
  }


// Check button
void FXCheckButton::setCheck(FXuint s){
  if(check!=s){
    check=s;
    update(0,0,width,height);
    }
  }


// Change state to checked
long FXCheckButton::onCheck(FXObject*,FXSelector,void*){ 
  setCheck(TRUE); 
  return 1; 
  }


// Change state to unchecked
long FXCheckButton::onUncheck(FXObject*,FXSelector,void*){ 
  setCheck(FALSE); 
  return 1; 
  }


// Change state to indeterminate
long FXCheckButton::onUnknown(FXObject*,FXSelector,void*){ 
  setCheck(MAYBE); 
  return 1; 
  }


// Update value from a message
long FXCheckButton::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setCheck((FXint)ptr);
  return 1;
  }


// Update value from a message
long FXCheckButton::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setCheck(*((FXint*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXCheckButton::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXint*)ptr)=getCheck(); return 1; }
  return 0;
  }


// Gained focus
long FXCheckButton::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusIn(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }

  
// Lost focus
long FXCheckButton::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusOut(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }


// Implement auto-hide or auto-gray modes
long FXCheckButton::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXLabel::onUpdate(sender,sel,ptr)){
    if(options&CHECKBUTTON_AUTOHIDE){if(shown()){hide();recalc();}}
    if(options&CHECKBUTTON_AUTOGRAY){disable();}
    }
  else{
    if(options&CHECKBUTTON_AUTOHIDE){if(!shown()){show();recalc();}}
    if(options&CHECKBUTTON_AUTOGRAY){enable();}
    }
  return 1;
  }



// Button being pressed
long FXCheckButton::onActivate(FXObject*,FXSelector,void*){
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  oldcheck=check;
  setCheck(!oldcheck);
  return 0;
  }
  

// Button being released
long FXCheckButton::onDeactivate(FXObject*,FXSelector,void*){
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  if(check!=oldcheck){
    if(target && target->handle(this,MKUINT(message,SEL_COMMAND),(void*)check)) return 1;
    }
  return 0;
  }


// Entered button
long FXCheckButton::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onEnter(sender,sel,ptr);
  if(isEnabled()){
    if(flags&FLAG_PRESSED) setCheck(!oldcheck);
    return 1;
    }
  return 0;
  }


// Left button
long FXCheckButton::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onLeave(sender,sel,ptr);
  if(isEnabled()){
    if(flags&FLAG_PRESSED) setCheck(oldcheck);
    return 1;
    }
  return 0;
  }


// We were asked about status text
long FXCheckButton::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXCheckButton::onQueryTip(FXObject* sender,FXSelector,void*){
  if(tip.text() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&tip);
    return 1;
    }
  return 0;
  }


// Handle repaint 
long FXCheckButton::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint tw=0,th=0,tx,ty,ix,iy;
  FXASSERT(xid!=0);
  
  FXFrame::onPaint(sender,sel,ptr);
  
  if(label.text()){
    tw=font->getTextWidth(label.text(),label.length());
    th=font->getFontHeight();
    }

  just_x(tx,ix,tw,13);
  just_y(ty,iy,th,13);

  setForeground(shadowColor);
  drawLine(ix,iy,ix+11,iy);
  drawLine(ix,iy,ix,iy+11);

  setForeground(borderColor);
  drawLine(ix+1,iy+1,ix+10,iy+1);
  drawLine(ix+1,iy+1,ix+1,iy+10);

  setForeground(hiliteColor);
  drawLine(ix,iy+12,ix+12,iy+12);
  drawLine(ix+12,iy+12,ix+12,iy);

  setForeground(backColor);
  drawLine(ix+2,iy+11,ix+11,iy+11);
  drawLine(ix+11,iy+2,ix+11,iy+11);

  if(check==MAYBE)
    setForeground(baseColor);
  else
    setForeground(boxColor);
  fillRectangle(ix+2,iy+2,9,9);

  if(check!=FALSE){
    FXSegment seg[6];
    seg[0].x1=3+ix; seg[0].y1=5+iy; seg[0].x2=5+ix; seg[0].y2=7+iy;
    seg[1].x1=3+ix; seg[1].y1=6+iy; seg[1].x2=5+ix; seg[1].y2=8+iy;
    seg[2].x1=3+ix; seg[2].y1=7+iy; seg[2].x2=5+ix; seg[2].y2=9+iy;
    seg[3].x1=5+ix; seg[3].y1=7+iy; seg[3].x2=9+ix; seg[3].y2=3+iy;
    seg[4].x1=5+ix; seg[4].y1=8+iy; seg[4].x2=9+ix; seg[4].y2=4+iy;
    seg[5].x1=5+ix; seg[5].y1=9+iy; seg[5].x2=9+ix; seg[5].y2=5+iy;
    if(isEnabled()){
      if(check==MAYBE)
        setForeground(shadowColor);
      else
        setForeground(textColor);
      }
    else{
      setForeground(shadowColor);
      }
    drawLineSegments(seg,6);
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


// Set box color
void FXCheckButton::setBoxColor(FXPixel clr){
  boxColor=clr;
  update(0,0,width,height);
  }


// Save object to stream
void FXCheckButton::save(FXStream& store) const {
  FXLabel::save(store);
  store << check;
  store << oldcheck;
  }


// Load object from stream
void FXCheckButton::load(FXStream& store){
  FXLabel::load(store);
  store >> check;
  store >> oldcheck;
  }  

