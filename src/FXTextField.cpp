/********************************************************************************
*                                                                               *
*                           T e x t F i e l d   O b j e c t                     *
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
* $Id: FXTextField.cpp,v 1.44 1998/10/30 15:49:39 jvz Exp $                   *
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
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXTextField.h"
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXTooltip.h"

/*
  To do:
  - Observe justify options.
  - Add numeric (float/integer) input modes.
  - Add clipboard (^X/^V/^C) capability.
  - Add API's.
  - shift arrow/shift end should highlight between anchor and cursor.
  - Want to simplify string again.
  - ^A and ^E should go to begin, end, ^F, ^B forward and backward.
  - Focus into TextField should select all.
  - Need to generalize GUI Update scheme.
  - Double click should select word, not whole text.
  - TextField passes string as the ptr argument in the callback.
  - Should add some messages that allow it to update.
  - GUI Updatable as long as its in the focus chain
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXTextField) FXTextFieldMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTextField::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXTextField::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXTextField::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTextField::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTextField::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXTextField::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXTextField::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXTextField::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXTextField::onRightBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXTextField::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXTextField::onKeyRelease),
  FXMAPFUNC(SEL_MOTION,0,FXTextField::onMotion),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXTextField::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXTextField::onSelectionGained),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXTextField::onSelectionRequest),
  FXMAPFUNC(SEL_FOCUSIN,0,FXTextField::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXTextField::onFocusOut),
  FXMAPFUNC(SEL_TIMEOUT,1,FXTextField::onBlink),
  FXMAPFUNC(SEL_TIMEOUT,2,FXTextField::onAutoLeftScroll),
  FXMAPFUNC(SEL_TIMEOUT,3,FXTextField::onAutoRightScroll),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXTextField::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXTextField::onQueryHelp),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXTextField::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXTextField::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALVALUE,FXTextField::onCmdSetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXTextField::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXTextField::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALVALUE,FXTextField::onCmdGetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXTextField::onCmdGetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXTextField,FXCell,FXTextFieldMap,ARRAYNUMBER(FXTextFieldMap))


// Construct and init
FXTextField::FXTextField(FXComposite* p,FXuint cols,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXCell(p,opts,x,y,w,h,pl,pr,pt,pb),contents(cols+1){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  defaultCursor=getApp()->textCursor;
  dragCursor=getApp()->textCursor;
  font=getApp()->normalFont;
  selbackColor=0;
  seltextColor=0;
  cursor=0;
  anchor=0;
  blinker=NULL;
  columns=cols;
  scroll=0;
  }


// Create X window
void FXTextField::create(){
  FXCell::create();
  setBackColor(acquireColor(FXRGB(255,255,255)));
  textColor=acquireColor(getApp()->foreColor);
  selbackColor=acquireColor(getApp()->selbackColor);
  seltextColor=acquireColor(getApp()->selforeColor);
  font->create();
  }


// Change the font
void FXTextField::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Enable the window
void FXTextField::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXFrame::enable();
    update(0,0,width,height);
    }
  }


// Disable the window
void FXTextField::disable(){
  if(flags&FLAG_ENABLED){
    FXFrame::disable();
    update(0,0,width,height);
    }
  }


// Get default size
FXint FXTextField::getDefaultWidth(){
  return padleft+padright+(border<<1)+columns*font->getTextWidth("M",1);
  }


FXint FXTextField::getDefaultHeight(){
  return padtop+padbottom+(border<<1)+font->getFontHeight();
  }


// Find index from coord
FXint FXTextField::index(FXint x){
  FXint cx,cw; FXuint ci;
  x=x-scroll-padleft-border;
  if(x<0) return 0;
  cx=ci=0;
  while(ci<(FXint)contents.length()){
    cw=font->getTextWidth(&contents[ci],1);
    if(x<(cx+(cw>>1))) break;
    cx+=cw;
    ci+=1;
    }
  return ci;
  }


// Find coordinate from index
FXint FXTextField::coord(FXint i){
  return padleft+border+scroll+font->getTextWidth(contents.text(),i);
  }


// Enter textfield
long FXTextField::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXCell::onEnter(sender,sel,ptr);
  return 1;
  }


// Leave textfield
long FXTextField::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXCell::onLeave(sender,sel,ptr);
  return 1; 
  }


// We now really do have the selection; repaint the text field
long FXTextField::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionGained %x\n",getClassName(),this);
  FXCell::onSelectionGained(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }


// We lost the selection somehow; repaint the text field
long FXTextField::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionLost %x\n",getClassName(),this);
  FXCell::onSelectionLost(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }



// Somebody wants our selection; the text field will furnish it if the target doesn't
long FXTextField::onSelectionRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXuchar *data; 
  FXuint st,l;
//fprintf(stderr,"%s::onSelectionRequest %x\n",getClassName(),this);
  
  // First, perhaps the target wants to supply its own data for the request
  if(FXCell::onSelectionRequest(sender,sel,ptr)) return 1;
  
  // Otherwise (most likely) its the string from this text we return
  if(getDNDType()==XA_STRING){
    if(anchor<cursor){st=anchor;l=cursor-anchor;}else{st=cursor;l=anchor-cursor;}
    FXCALLOC(&data,FXuchar,l+1);
    strncpy((FXchar*)data,&contents[st],l);
    setDNDData(XA_STRING,data,l);
    }
  return 0;
  }


// Pressed middle button to paste
long FXTextField::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
  setFocus();
  setCursorPos(index(ev->win_x));
  setAnchorPos(getCursorPos());
  update(border,border,width-(border<<1),height-(border<<1));
  flags&=~FLAG_UPDATE;
  return 0;
  }


// Released middle button causes paste of selection
long FXTextField::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuchar *data;
  FXuint   size;
  if(!isEnabled()) return 0;
  flags|=FLAG_UPDATE;
  if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
  if(getDNDData(XA_STRING,data,size)){
    contents.insert(cursor,(FXchar*)data,size);
    setCursorPos(cursor+size);
    flags|=FLAG_CHANGED;
    FXFREE(&data);
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
    update(border,border,width-(border<<1),height-(border<<1));
    return 1;
    }
  return 0;
  }


// Pressed right button
long FXTextField::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
  setFocus();
  flags&=~FLAG_UPDATE;
  return 0;
  }


// Released right button
long FXTextField::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(!isEnabled()) return 0;
  flags|=FLAG_UPDATE;
  if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
  return 0;
  }


// Gained focus
long FXTextField::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onFocusIn\n",getClassName());
  FXFrame::onFocusIn(sender,sel,ptr);
  if(!blinker) blinker=getApp()->addTimeout(getApp()->blinkSpeed,this,1);
  drawCursor(FLAG_CARET);
  if(hasSelection()){
    update(border,border,width-(border<<1),height-(border<<1));
    }
  return 1;
  }

  
// Lost focus
long FXTextField::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onFocusOut\n",getClassName());
  FXFrame::onFocusOut(sender,sel,ptr);
  if(blinker) getApp()->removeTimeout(blinker); 
  blinker=NULL;
  drawCursor(0);
  if(flags&FLAG_CHANGED){
    flags&=~FLAG_CHANGED;
    if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)contents.text());
    }
  if(hasSelection()){
    update(border,border,width-(border<<1),height-(border<<1));
    }
  flags|=FLAG_UPDATE;
  return 1;
  }


// If window can have focus
FXbool FXTextField::canFocus() const { return 1; }


// We were asked about status text
long FXTextField::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){ 
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXTextField::onQueryTip(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
  if(flags&FLAG_TIP){
    if(tip.text()){
      sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&tip);
      }
    else if(contents.text()){   // Perhaps this should be an option...
      if(font->getTextWidth(contents.text(),contents.length()) > (width-padleft-padright-border-border)){
        sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&contents);
        }
      }
    return 1;
    }
  return 0;
  }


// Update value from a message
long FXTextField::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  if(ptr){
    setText((const FXchar*)ptr);
    }
  return 1;
  }


// Update value from a message
long FXTextField::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  FXchar buf[16];
  if(ptr){
    sprintf(buf,"%d",*((FXint*)ptr));
    setText(buf);
    }
  return 1;
  }


// Update value from a message
long FXTextField::onCmdSetRealValue(FXObject*,FXSelector,void* ptr){
  FXchar buf[128];
  if(ptr){
    sprintf(buf,"%.10g",*((FXdouble*)ptr));
    setText(buf);
    }
  return 1;
  }


// Update value from a message
long FXTextField::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){
    setText(*((FXString*)ptr));
    }
  return 1;
  }


// Obtain value from text field
long FXTextField::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  if(ptr){
    if(sscanf(contents.text(),"%d",(FXint*)ptr)) return 1;
    }
  return 0;
  }


// Obtain value from text field
long FXTextField::onCmdGetRealValue(FXObject*,FXSelector,void* ptr){
  if(ptr){
    if(sscanf(contents.text(),"%lf",(FXdouble*)ptr)) return 1;
    }
  return 0;
  }


// Obtain value from text field
long FXTextField::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){
    *((FXString*)ptr) = contents;
    return 1;
    }
  return 0;
  }


// Pressed left button
long FXTextField::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  if(ev->click_count==2){
    setAnchorPos(0);
    setCursorPos(contents.length());
    acquireSelection();
    }
  else{
    setFocus();
    releaseSelection();
    setAnchorPos(index(ev->win_x));
    setCursorPos(getAnchorPos());
    }
  return 1;
  }


// Released left button
long FXTextField::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
  flags&=~FLAG_PRESSED;
  return 1;
  }


// Moved
long FXTextField::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(isEnabled() && (flags&FLAG_PRESSED)){
    if((ev->win_x<border+padleft) && (scroll<0)){
      getApp()->addTimeout(getApp()->scrollSpeed,this,2);
      }
    else if((width-padright-border<ev->win_x) && (width-padleft-padright-(border<<1)-font->getTextWidth(contents.text(),contents.length())<scroll)){
      getApp()->addTimeout(getApp()->scrollSpeed,this,3);
      }
    else{
      FXint t=index(ev->win_x);
      if(t!=cursor){
        drawCursor(0);
        cursor=t;
        acquireSelection();
        update(border,border,width-(border<<1),height-(border<<1));
        }
      }
    return 1;
    }
  return 0;
  }


// Automatic scroll left
long FXTextField::onAutoLeftScroll(FXObject*,FXSelector,void*){
  FXuint buttons; FXint x,y;
  getCursorPosition(x,y,buttons);
  if((flags&FLAG_PRESSED) && (x<border+padleft) && (scroll<0)){
    scroll+=border+padleft-x;
    if(scroll>=0) scroll=0;
    getApp()->addTimeout(getApp()->scrollSpeed,this,2);
    setCursorPos(index(border+padleft));
    update(border,border,width-(border<<1),height-(border<<1));
    }
  return 1;
  }


// Automatic scroll right
long FXTextField::onAutoRightScroll(FXObject*,FXSelector,void*){
  FXuint buttons; FXint x,y;
  getCursorPosition(x,y,buttons);
  FXint fw,tw,s;
  fw=width-padleft-padright-(border<<1);
  tw=font->getTextWidth(contents.text(),contents.length());
  s=fw-tw;
  if((flags&FLAG_PRESSED) && (width-padright-border<x) && (s<scroll)){
    scroll+=width-padright-border-x;
    if(scroll<s) scroll=s;
    getApp()->addTimeout(getApp()->scrollSpeed,this,3);
    setCursorPos(index(width-padright-border));
    update(border,border,width-(border<<1),height-(border<<1));
    }
  return 1;
  }


// Zap selected stuff
void FXTextField::killSelection(){
  int st,en;
  st=FXMIN(anchor,cursor); 
  en=FXMAX(anchor,cursor);
  contents.remove(st,en-st);
  cursor=st;
  releaseSelection();
  }


// Blink the cursor
long FXTextField::onBlink(FXObject*,FXSelector,void*){
  drawCursor(flags^FLAG_CARET);
  blinker=getApp()->addTimeout(getApp()->blinkSpeed,this,1);
  return 0;
  }


// Draw the cursor
void FXTextField::drawCursor(FXuint state){
  FXint cl,ch,xx=coord(cursor)-1;
  if((state^flags)&FLAG_CARET){
    setClipRectangle(border,border,width-(border<<1),height-(border<<1));
    if(flags&FLAG_CARET){
      setForeground(backColor);
      drawLine(xx,padtop+border,xx,height-border-padbottom-1);
      drawLine(xx-2,padtop+border,xx+2,padtop+border);
      drawLine(xx-2,height-border-padbottom-1,xx+2,height-border-padbottom-1);
      FXASSERT(0<=cursor);
      FXASSERT(cursor<=contents.length());
      cl=cursor-1;
      ch=cursor+1;
      drawTextRange(FXMAX(cl,0),FXMIN(ch,contents.length()));     // Gotta redraw these letters...
      flags&=~FLAG_CARET;
      }
    else{
      setForeground(textColor);
      drawLine(xx,padtop+border,xx,height-border-padbottom-1);
      drawLine(xx-2,padtop+border,xx+2,padtop+border);
      drawLine(xx-2,height-border-padbottom-1,xx+2,height-border-padbottom-1);
      flags|=FLAG_CARET;
      }
    clearClipRectangle();
    }
  }


// Move the cursor
void FXTextField::setCursorPos(FXint pos){
  FXint xx;
  if(cursor!=pos){
    drawCursor(0);
    if(pos>(FXint)contents.length()) pos=contents.length(); else if(pos<0) pos=0;
    xx=scroll+font->getTextWidth(contents.text(),pos);
    if(xx<0){
      scroll=-font->getTextWidth(contents.text(),pos);
      update(0,0,width,height);
      }
    else if(xx>=(width-padright-padleft-(border<<1))){
      scroll=(width-padright-padleft-(border<<1))-font->getTextWidth(contents.text(),pos);
      update(0,0,width,height);
      }
    cursor=pos;
    if(hasFocus()) drawCursor(FLAG_CARET);
    }
  }


// Set anchor position
void FXTextField::setAnchorPos(FXint pos){
  if(pos>(FXint)contents.length()) pos=contents.length(); else if(pos<0) pos=0;
  anchor=pos;
  }


// Draw range of text
void FXTextField::drawTextRange(FXuint fm,FXuint to){
  FXint fx,tx,sx,ex; FXuint si,ei;
  if(to<=fm) return;
  setTextFont(font);
  fx=font->getTextWidth(contents.text(),fm);
  tx=font->getTextWidth(contents.text(),to);
//fprintf(stderr,"fm=%d to=%d\n",fm,to);
  if(!hasSelection()){
    setForeground(textColor);
    drawText(scroll+padleft+border+fx,padtop+border+font->getFontAscent(),&contents[fm],to-fm);
    }
  else{
    if(anchor<cursor){si=anchor;ei=cursor;}else{si=cursor;ei=anchor;}
    sx=font->getTextWidth(contents.text(),si);
    ex=font->getTextWidth(contents.text(),ei);
    setForeground(textColor);
    if(tx<=sx){
      drawText(scroll+padleft+border+fx,padtop+border+font->getFontAscent(),&contents[fm],to-fm);
      return;
      }
    if(ex<=fx){
      drawText(scroll+padleft+border+fx,padtop+border+font->getFontAscent(),&contents[fm],to-fm);
      return;
      }
    if(fx<sx){
      drawText(scroll+padleft+border+fx,padtop+border+font->getFontAscent(),&contents[fm],si-fm);
      }
    else{
      sx=fx;
      si=fm;
      }
    if(ex<tx){
      drawText(scroll+padleft+border+ex,padtop+border+font->getFontAscent(),&contents[ei],to-ei);
      }
    else{
      ex=tx;
      ei=to;
      }
    if(sx<ex){
      if(hasFocus()){
        setForeground(selbackColor);
        fillRectangle(scroll+padleft+border+sx,padtop+border,ex-sx,font->getFontHeight());
        setForeground(seltextColor);
        drawText(scroll+padleft+border+sx,padtop+border+font->getFontAscent(),&contents[si],ei-si);
        }
      else{
        setForeground(baseColor);
        fillRectangle(scroll+padleft+border+sx,padtop+border,ex-sx,font->getFontHeight());
        setForeground(textColor);
        drawText(scroll+padleft+border+sx,padtop+border+font->getFontAscent(),&contents[si],ei-si);
        }
      }
    }
  }


// Pressed a key
long FXTextField::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  char entry;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
//fprintf(stderr,"key code=%02x\n",ev->code);
  flags&=~FLAG_UPDATE;
  switch(ev->code){
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Caps_Lock:
    case KEY_Shift_Lock:
    case KEY_Meta_L:
    case KEY_Meta_R:
    case KEY_Alt_L:
    case KEY_Alt_R:
    case KEY_Super_L:
    case KEY_Super_R:
    case KEY_Hyper_L:
    case KEY_Hyper_R:
    case KEY_Scroll_Lock:
    case KEY_Sys_Req:
      return 1;
    case KEY_Right:
    case KEY_KP_Right:
      setCursorPos(cursor+1);
      releaseSelection();
      return 1;
    case KEY_Left:
    case KEY_KP_Left:
      setCursorPos(cursor-1);
      releaseSelection();
      return 1;
    case KEY_Home:
    case KEY_KP_Home:
      setCursorPos(0);
      releaseSelection();
      return 1;
    case KEY_End:
    case KEY_KP_End:
      setCursorPos(contents.length());
      releaseSelection();
      return 1;
    case KEY_Delete:
    case KEY_KP_Delete:
      if(hasSelection())
        killSelection();
      else
        contents.remove(cursor,1);
      flags|=FLAG_CHANGED;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());
      update(0,0,width,height);
      return 1;
    case KEY_BackSpace:
      if(hasSelection()){
        killSelection();
        }
      else if(cursor>0){
        setCursorPos(cursor-1);
        contents.remove(cursor,1);
        }
      flags|=FLAG_CHANGED;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());
      update(0,0,width,height);
      return 1;
    case KEY_Return:
    case KEY_KP_Enter:
      if(flags&FLAG_CHANGED){
        if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)contents.text());
        flags&=~FLAG_CHANGED;
        }
      flags|=FLAG_UPDATE;
      return 1;
    case KEY_Tab:                     // Pretend we didn't handle this so we can get out
    case KEY_Next:
    case KEY_Prior:
    case KEY_ISO_Left_Tab:
      return 0;
    default:
      //if(ev->state&ALTMASK) return 0; // Unhandled here also
      entry=fxkeyval(ev->code,ev->state);
//fprintf(stderr,"code=%04x state=%04x entry=%c(%02x)\n",ev->code,ev->state,' '<=entry?entry:' ',entry);
      if(entry<' ' || entry>'~') return 0;
      if(hasSelection()) killSelection();
      contents.insert(cursor,&entry,1);
      setCursorPos(cursor+1);
      if(cursor>(FXint)contents.length()) cursor=contents.length();
      flags|=FLAG_CHANGED;
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());
      update(0,0,width,height);
      return 1;
    }
  return 0;
  }


// Key Release 
long FXTextField::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
//fprintf(stderr,"%s::onKeyRelease keysym=0x%04x\n",getClassName(),event->code);
  switch(event->code){
    case KEY_Tab:                     // Pretend we didn't handle this so we can get out
    case KEY_Next:
    case KEY_Prior:
    case KEY_ISO_Left_Tab:
      return 0;
    default:
      return 1;
    }
  return 1;
  }



// Handle repaint 
long FXTextField::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onPaint(sender,sel,ptr);
  setClipRectangle(border,border,width-(border<<1),height-(border<<1));
  drawTextRange(0,contents.length());
  clearClipRectangle();
  return 1;
  }


// Set text being displayed
void FXTextField::setText(const FXchar* text){
  releaseSelection();
  if(contents!=text){
    contents.clear();
    contents.append(text);
    update(border,border,width-(border<<1),height-(border<<1));
    }
  setCursorPos(0);
  scroll=0;
  }


// Set text color
void FXTextField::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Set select background color
void FXTextField::setSelBackColor(FXPixel clr){
  selbackColor=clr;
  update(0,0,width,height);
  }


// Set selected text color
void FXTextField::setSelTextColor(FXPixel clr){
  seltextColor=clr;
  update(0,0,width,height);
  }


// Change number of columns
void FXTextField::setNumColumns(FXuint cols){
  if(columns!=cols){
    columns=cols;
    recalc();
    update(0,0,width,height);
    }
  }


// Change help text
void FXTextField::setHelpText(const FXchar* text){
  help=text;
  }


// Change tip text
void FXTextField::setTipText(const FXchar* text){
  tip=text;
  }


// Clean up
FXTextField::~FXTextField(){
  if(blinker) getApp()->removeTimeout(blinker);
  blinker=(FXTimer*)-1;
  }
