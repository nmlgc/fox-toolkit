/********************************************************************************
*                                                                               *
*                           B u t t o n    O b j e c t s                        *
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
* $Id: FXButton.cpp,v 1.55 1998/11/02 02:08:29 jeroen Exp $                     *
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
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenu.h"
#include "FXTooltip.h"

/*
  To do:
  - Use flags for button instead of a whole integer
  - Add ``flat'' toolbar style also
  - Need check-style also (stay in when pressed, pop out when unpressed).
  - Who owns the icon(s)?
  - Arrow buttons should auto-repeat with a timer of some kind
  - "&Label\tTooltip\tHelptext\thttp://server/application/helponitem.html"
  - CheckButton should send SEL_COMMAND.
  - Default button mode:- should somehow get focus.
  - Add button multiple-click translations elsewhere
  - Button should be able to behave like a check (radio) button.
  - Need to draw ``around'' the icon etc. So it doesn't flash to background.
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXButton) FXButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXButton::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXButton::onUpdate),
  FXMAPFUNC(SEL_ACTIVATE,0,FXButton::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXButton::onDeactivate),
  FXMAPFUNC(SEL_ENTER,0,FXButton::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXButton::onLeave),
  FXMAPFUNC(SEL_FOCUSIN,0,FXButton::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXButton::onFocusOut),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXWindow::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXWindow::onHotKeyRelease),
  FXMAPFUNC(SEL_CLICKED,0,FXButton::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXButton::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXButton::onTripleClicked),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXButton::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXButton::onQueryHelp),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_CHECK,FXButton::onCheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNCHECK,FXButton::onUncheck),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXButton::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXButton::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXButton::onCmdGetIntValue),
  };


// Object implementation
FXIMPLEMENT(FXButton,FXLabel,FXButtonMap,ARRAYNUMBER(FXButtonMap))

  
// Deserialization
FXButton::FXButton(){
  state=STATE_UP;
  }


// Construct and init
FXButton::FXButton(FXComposite* p,const char* text,FXIcon* ic,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXLabel(p,text,ic,opts,x,y,w,h,pl,pr,pt,pb),
  tip(text,'\t',1),
  help(text,'\t',2){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  dragCursor=getApp()->rarrowCursor;
  state=STATE_UP;
  }


// Create X window
void FXButton::create(){
  FXLabel::create();
  }
 

// Enable the window
void FXButton::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXLabel::enable();
    update(0,0,width,height);
    }
  }


// Disable the window
void FXButton::disable(){
  if(flags&FLAG_ENABLED){
    FXLabel::disable();
    update(0,0,width,height);
    }
  }


// Set button state
void FXButton::setState(FXuint s){
  if(state!=s){
    state=s;
    update(0,0,width,height);
    }
  }


// If window can have focus
FXbool FXButton::canFocus() const { return 1; }


// Update value from a message
long FXButton::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setState((FXint)ptr);
  return 1;
  }


// Update value from a message
long FXButton::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setState(*((FXint*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXButton::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXint*)ptr)=getState(); return 1; }
  return 0;
  }


// Check the menu button
long FXButton::onCheck(FXObject*,FXSelector,void*){ 
  setState(STATE_CHECKED); 
  return 1; 
  }


// Check the menu button
long FXButton::onUncheck(FXObject*,FXSelector,void*){ 
  setState(STATE_UNCHECKED); 
  return 1; 
  }


// Implement auto-hide or auto-gray modes
long FXButton::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  //FXLabel::onUpdate(sender,sel,ptr);
  if(!FXLabel::onUpdate(sender,sel,ptr)){
    if(options&BUTTON_AUTOHIDE){if(shown()){hide();recalc();}}
    if(options&BUTTON_AUTOGRAY){disable();}
    }
  else{
    if(options&BUTTON_AUTOHIDE){if(!shown()){show();recalc();}}
    if(options&BUTTON_AUTOGRAY){enable();}
    }
  return 1;
  }


// Gained focus
long FXButton::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusIn(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }

  
// Lost focus
long FXButton::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onFocusOut(sender,sel,ptr);
  update(border,border,width-(border<<1),height-(border<<1));
  return 1;
  }


// Button being activated
long FXButton::onActivate(FXObject*,FXSelector,void*){
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  if(state!=STATE_ENGAGED) setState(STATE_DOWN);
  return 0;
  }
  

// Button being deactivated
long FXButton::onDeactivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXuint click=(state==STATE_DOWN);
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  if(state!=STATE_ENGAGED) setState(STATE_UP);
  if(event->click_count==1) handle(this,MKUINT(0,SEL_CLICKED),(void*)click);
  else if(event->click_count==2) handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)click);
  else if(event->click_count==3) handle(this,MKUINT(0,SEL_TRIPLECLICKED),(void*)click);
  return 1;
  }


// Clicked on button
long FXButton::onClicked(FXObject*,FXSelector,void* ptr){
  if(target){
    if(target->handle(this,MKUINT(message,SEL_CLICKED),ptr)) return 1;
    if(ptr && target->handle(this,MKUINT(message,SEL_COMMAND),ptr)) return 1;
    }
  return 0;
  }
  

// Double clicked on button
long FXButton::onDoubleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr);
  }
  
  
// Triple clicked
long FXButton::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }


// We were asked about status text
long FXButton::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXButton::onQueryTip(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
  if(tip.text() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&tip);
    return 1;
    }
  return 0;
  }


// Entered button
long FXButton::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onEnter(sender,sel,ptr);
  update(0,0,width,height);
  if(isEnabled()){
    if((flags&FLAG_PRESSED) && (state!=STATE_ENGAGED)) setState(STATE_DOWN);
    return 1;
    }
  return 0;
  }


// Left button
long FXButton::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXLabel::onLeave(sender,sel,ptr);
  update(0,0,width,height);
  if(isEnabled()){
    if((flags&FLAG_PRESSED) && (state!=STATE_ENGAGED)) setState(STATE_UP);
    return 1;
    }
  return 0;
  }


// Handle repaint 
long FXButton::onPaint(FXObject*,FXSelector,void*){
  FXint tw=0,th=0,iw=0,ih=0,tx,ty,ix,iy;
  FXASSERT(xid!=0);
  
  // Got a border at all?
  if(options&(FRAME_RAISED|FRAME_SUNKEN)){

    // Toolbar style
    if(options&BUTTON_TOOLBAR){
      
      // Enabled and cursor inside, and up
      if(isEnabled() && underCursor() && (state==STATE_UP)){
        setForeground(backColor);
        fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleRaisedRectangle(0,0,width,height);
        else drawRaisedRectangle(0,0,width,height);
        }
      
      // Enabled and cursor inside and down
      else if(isEnabled() && underCursor() && (state==STATE_DOWN)){
        setForeground(backColor);
        fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width,height);
        else drawSunkenRectangle(0,0,width,height);
        }
      
      // Enabled and checked
      else if(isEnabled() && (state==STATE_ENGAGED)){
        setForeground(hiliteColor);
        fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width,height);
        else drawSunkenRectangle(0,0,width,height);
        }
        
      // Disabled or unchecked or not under cursor
      else{
        setForeground(backColor);
        fillRectangle(0,0,width,height);
        }
      }
 
    // Normal style
    else{
      
      // Default
      if(isDefault()){
        
        // Black default border
        drawBorderRectangle(0,0,width,height);
        
        // Draw in up state if disabled or up
        if(!isEnabled() || (state==STATE_UP)){
          setForeground(backColor);
          fillRectangle(border+1,border+1,width-border*2-2,height-border*2-2);
          if(options&FRAME_THICK) drawDoubleRaisedRectangle(1,1,width-1,height-1);
          else drawRaisedRectangle(1,1,width-1,height-1);
          }
        
        // Draw sunken if enabled and either checked or pressed
        else{
          if(state==STATE_ENGAGED) setForeground(hiliteColor); else setForeground(backColor);
          fillRectangle(border,border,width-border*2-1,height-border*2-1);
          if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width-1,height-1);
          else drawSunkenRectangle(0,0,width-1,height-1);
          }
        }
      
      // Non-Default
      else{
        
        // Draw in up state if disabled or up
        if(!isEnabled() || (state==STATE_UP)){
          setForeground(backColor);
          fillRectangle(border,border,width-border*2,height-border*2);
          if(options&FRAME_THICK) drawDoubleRaisedRectangle(0,0,width,height);
          else drawRaisedRectangle(0,0,width,height);
          }
        
        // Draw sunken if enabled and either checked or pressed
        else{
          if(state==STATE_ENGAGED) setForeground(hiliteColor); else setForeground(backColor);
          fillRectangle(border,border,width-border*2,height-border*2);
          if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width,height);
          else drawSunkenRectangle(0,0,width,height);
          }
        }
      }
    }
  
  // No borders
  else{
    if(isEnabled() && (state==STATE_ENGAGED)){
      setForeground(hiliteColor);
      fillRectangle(0,0,width,height);
      }
    else{
      setForeground(backColor);
      fillRectangle(0,0,width,height);
      }
    }
  
  // Place text & icon
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
  
  // Shift a bit when pressed
  if(state){ ++tx; ++ty; ++ix; ++iy; }

  // Draw the icon
  if(icon){
    if(isEnabled())
      drawIcon(icon,ix,iy);
    else
      drawIconShaded(icon,ix,iy);
    }
  
  // Draw the text
  if(label.text()){
    setTextFont(font);
    if(isEnabled()){
      setForeground(textColor);
      drawLabel(tx,ty,tw,th);
      if(hasFocus()){
        drawFocusRectangle(border+2,border+2,width-2*border-4,height-2*border-4);
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


// Change help text
void FXButton::setHelpText(const FXchar* text){
  help=text;
  }


// Change tip text
void FXButton::setTipText(const FXchar* text){
  tip=text;
  }


// Save object to stream
void FXButton::save(FXStream& store) const {
  FXLabel::save(store);
  store << tip;
  store << help;
  }

      

// Load object from stream
void FXButton::load(FXStream& store){
  FXLabel::load(store);
  store >> tip;
  store >> help;
  }  

