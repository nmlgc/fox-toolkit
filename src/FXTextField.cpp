/********************************************************************************
*                                                                               *
*                         T e x t   F i e l d   O b j e c t                     *
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
* $Id: FXTextField.cpp,v 1.79.4.3 2003/02/25 23:10:54 fox Exp $                  *
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
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXCursor.h"
#include "FXLabel.h"
#include "FXTextField.h"


/*
  Notes:

  - TextField passes string ptr in the SEL_COMMAND callback.

  - GUI updatable as long as its in the focus chain.

  - Double-click should select word, triple click all of text field.

  - TextField should return 0 for all unhandled keys!

  - Pressing mouse button will set the focus w/o claiming selection!

  - Change of cursor only implies makePositionVisible() if done by user.

  - Input verify and input verify callback operation:

    1) The input is tested to see if it qualifies as an integer or
       real number.
    2) The target is allowed to raise an objection: if a target does NOT
       handle the message, or handles the message and returns 0, then the
       new input is accepted.
    3) If none of the above applies the input is simply accepted;
       this is the default mode for generic text type-in.

    Note that the target callback is called AFTER already having verified that
    the entry is a number, so a target can simply assume that this has been checked
    already and just perform additional checks [e.g. numeric range].

    Also note that verify callbacks should allow for partially complete inputs,
    and that these inputs could be built up a character at a time, and in no
    particular order.

  - Option to grow/shrink textfield to fit text.

  - Perhaps need selstartpos,selendpos member variables to keep track of selection.

  - Maybe also send SEL_SELECTED, SEL_DESELECTED?

*/


#define JUSTIFY_MASK    (JUSTIFY_HZ_APART|JUSTIFY_VT_APART)
#define TEXTFIELD_MASK  (TEXTFIELD_PASSWD|TEXTFIELD_INTEGER|TEXTFIELD_REAL|TEXTFIELD_READONLY|TEXTFIELD_ENTER_ONLY|TEXTFIELD_LIMITED|TEXTFIELD_OVERSTRIKE)


/*******************************************************************************/

// Map
FXDEFMAP(FXTextField) FXTextFieldMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTextField::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXTextField::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXTextField::ID_BLINK,FXTextField::onBlink),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXTextField::onAutoScroll),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTextField::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTextField::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXTextField::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXTextField::onMiddleBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXTextField::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXTextField::onKeyRelease),
  FXMAPFUNC(SEL_VERIFY,0,FXTextField::onVerify),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXTextField::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXTextField::onSelectionGained),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXTextField::onSelectionRequest),
  FXMAPFUNC(SEL_CLIPBOARD_LOST,0,FXTextField::onClipboardLost),
  FXMAPFUNC(SEL_CLIPBOARD_GAINED,0,FXTextField::onClipboardGained),
  FXMAPFUNC(SEL_CLIPBOARD_REQUEST,0,FXTextField::onClipboardRequest),
  FXMAPFUNC(SEL_FOCUSIN,0,FXTextField::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXTextField::onFocusOut),
  FXMAPFUNC(SEL_FOCUS_SELF,0,FXTextField::onFocusSelf),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXTextField::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXTextField::onQueryHelp),
  FXMAPFUNC(SEL_UPDATE,FXTextField::ID_TOGGLE_EDITABLE,FXTextField::onUpdToggleEditable),
  FXMAPFUNC(SEL_UPDATE,FXTextField::ID_TOGGLE_OVERSTRIKE,FXTextField::onUpdToggleOverstrike),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXTextField::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXTextField::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALVALUE,FXTextField::onCmdSetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXTextField::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXTextField::onCmdGetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALVALUE,FXTextField::onCmdGetRealValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXTextField::onCmdGetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_CURSOR_HOME,FXTextField::onCmdCursorHome),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_CURSOR_END,FXTextField::onCmdCursorEnd),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_CURSOR_RIGHT,FXTextField::onCmdCursorRight),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_CURSOR_LEFT,FXTextField::onCmdCursorLeft),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_MARK,FXTextField::onCmdMark),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_EXTEND,FXTextField::onCmdExtend),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_SELECT_ALL,FXTextField::onCmdSelectAll),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_DESELECT_ALL,FXTextField::onCmdDeselectAll),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_CUT_SEL,FXTextField::onCmdCutSel),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_COPY_SEL,FXTextField::onCmdCopySel),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_PASTE_SEL,FXTextField::onCmdPasteSel),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_DELETE_SEL,FXTextField::onCmdDeleteSel),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_OVERST_STRING,FXTextField::onCmdOverstString),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_INSERT_STRING,FXTextField::onCmdInsertString),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_BACKSPACE,FXTextField::onCmdBackspace),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_DELETE,FXTextField::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_TOGGLE_EDITABLE,FXTextField::onCmdToggleEditable),
  FXMAPFUNC(SEL_COMMAND,FXTextField::ID_TOGGLE_OVERSTRIKE,FXTextField::onCmdToggleOverstrike),
  };


// Object implementation
FXIMPLEMENT(FXTextField,FXFrame,FXTextFieldMap,ARRAYNUMBER(FXTextFieldMap))



/*******************************************************************************/


// For serialization
FXTextField::FXTextField(){
  flags|=FLAG_ENABLED;
  font=(FXFont*)-1;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  cursor=0;
  anchor=0;
  blinker=NULL;
  columns=0;
  shift=0;
  }


// Construct and init
FXTextField::FXTextField(FXComposite* p,FXint ncols,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  if(ncols<0) ncols=0;
  contents.fill('\0',ncols+1);
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  defaultCursor=getApp()->getDefaultCursor(DEF_TEXT_CURSOR);
  dragCursor=getApp()->getDefaultCursor(DEF_TEXT_CURSOR);
  font=getApp()->getNormalFont();
  backColor=getApp()->getBackColor();
  textColor=getApp()->getForeColor();
  selbackColor=getApp()->getSelbackColor();
  seltextColor=getApp()->getSelforeColor();
  cursor=0;
  anchor=0;
  blinker=NULL;
  columns=ncols;
  shift=0;
  }


// Create X window
void FXTextField::create(){
  FXFrame::create();
  font->create();
  }


// Change the font
void FXTextField::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Enable the window
void FXTextField::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXFrame::enable();
    update();
    }
  }


// Disable the window
void FXTextField::disable(){
  if(flags&FLAG_ENABLED){
    FXFrame::disable();
    update();
    }
  }


// Get default size
FXint FXTextField::getDefaultWidth(){
  return padleft+padright+(border<<1)+columns*font->getTextWidth("8",1);
  }


FXint FXTextField::getDefaultHeight(){
  return padtop+padbottom+(border<<1)+font->getFontHeight();
  }


// We now really do have the selection; repaint the text field
long FXTextField::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onSelectionGained(sender,sel,ptr);
  update();
  return 1;
  }


// We lost the selection somehow; repaint the text field
long FXTextField::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onSelectionLost(sender,sel,ptr);
  update();
  return 1;
  }


// Somebody wants our selection; the text field will furnish it if the target doesn't
long FXTextField::onSelectionRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXuchar *data;
  FXuint start,len;

  // Perhaps the target wants to supply its own data for the selection
  if(FXFrame::onSelectionRequest(sender,sel,ptr)) return 1;

  // Return text of the selection
  if(event->target==stringType){
    if(anchor<cursor){start=anchor;len=cursor-anchor;}else{start=cursor;len=anchor-cursor;}
    FXMALLOC(&data,FXuchar,len);
    if(options&TEXTFIELD_PASSWD){
      memset((FXchar*)data,'*',len);      // We shall not reveal the password!
      }
    else{
      memcpy(data,&contents[start],len);
      }
    setDNDData(FROM_SELECTION,stringType,data,len);
    return 1;
    }

  return 0;
  }


// We now really do have the clipboard, keep clipped text
long FXTextField::onClipboardGained(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onClipboardGained(sender,sel,ptr);
  return 1;
  }


// We lost the clipboard, free clipped text
long FXTextField::onClipboardLost(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onClipboardLost(sender,sel,ptr);
  clipped.clear();
  return 1;
  }


// Somebody wants our clipped text
long FXTextField::onClipboardRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXuchar *data;
  FXuint len;

  // Perhaps the target wants to supply its own data for the clipboard
  if(FXFrame::onClipboardRequest(sender,sel,ptr)) return 1;

  // Return clipped text
  if(event->target==stringType){
    len=clipped.length();
    FXCALLOC(&data,FXuchar,len+1);
    if(options&TEXTFIELD_PASSWD){
      memset((FXchar*)data,'*',len);      // We shall not reveal the password!
      }
    else{
      memcpy(data,clipped.text(),len);
      }
#ifndef WIN32
    setDNDData(FROM_CLIPBOARD,stringType,data,len);
#else
    setDNDData(FROM_CLIPBOARD,stringType,data,len+1);
#endif
    return 1;
    }

  return 0;
  }


// Gained focus
long FXTextField::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onFocusIn(sender,sel,ptr);
  if(isEditable()){
    if(!blinker) blinker=getApp()->addTimeout(getApp()->getBlinkSpeed(),this,ID_BLINK);
    drawCursor(FLAG_CARET);
    }
  if(hasSelection()){
    update(border,border,width-(border<<1),height-(border<<1));
    }
  return 1;
  }


// Lost focus
long FXTextField::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onFocusOut(sender,sel,ptr);
  if(blinker){getApp()->removeTimeout(blinker);blinker=NULL;}
  drawCursor(0);
  if(hasSelection()){
    update(border,border,width-(border<<1),height-(border<<1));
    }
  return 1;
  }


// Focus on widget itself
long FXTextField::onFocusSelf(FXObject* sender,FXSelector sel,void* ptr){
  if(FXFrame::onFocusSelf(sender,sel,ptr)){
    FXEvent *event=(FXEvent*)ptr;
    if(event->type==SEL_KEYPRESS || event->type==SEL_KEYRELEASE){
      handle(this,MKUINT(ID_SELECT_ALL,SEL_COMMAND),NULL);
      }
    return 1;
    }
  return 0;
  }


// If window can have focus
FXbool FXTextField::canFocus() const { return 1; }


// Into focus chain
void FXTextField::setFocus(){
  FXFrame::setFocus();
  setDefault(TRUE);
  flags&=~FLAG_UPDATE;                    // Thanks to Derek Ney <derek@hipgraphics.com>
  }


// Out of focus chain
void FXTextField::killFocus(){
  FXFrame::killFocus();
  setDefault(MAYBE);
  flags|=FLAG_UPDATE;
  if(flags&FLAG_CHANGED){
    flags&=~FLAG_CHANGED;
    if(!(options&TEXTFIELD_ENTER_ONLY) && target){
      target->handle(this,MKUINT(message,SEL_COMMAND),(void*)contents.text());
      }
    }
  }


// We were asked about status text
long FXTextField::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(!help.empty() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXTextField::onQueryTip(FXObject* sender,FXSelector,void*){
  if(!tip.empty() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&tip);
    return 1;
    }
  return 0;
  }


// Update value from a message
long FXTextField::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setText((const FXchar*)ptr);
  return 1;
  }


// Update value from a message
long FXTextField::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  setText(FXStringVal(*((FXint*)ptr)));
  return 1;
  }


// Update value from a message
long FXTextField::onCmdSetRealValue(FXObject*,FXSelector,void* ptr){
  setText(FXStringVal(*((FXdouble*)ptr)));
  return 1;
  }


// Update value from a message
long FXTextField::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  setText(*((FXString*)ptr));
  return 1;
  }


// Obtain value from text field
long FXTextField::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  *((FXint*)ptr)=FXIntVal(contents);
  return 1;
  }


// Obtain value from text field
long FXTextField::onCmdGetRealValue(FXObject*,FXSelector,void* ptr){
  *((FXdouble*)ptr)=FXDoubleVal(contents);
  return 1;
  }


// Obtain value from text field
long FXTextField::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  *((FXString*)ptr)=getText();
  return 1;
  }


// Pressed left button
long FXTextField::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(ev->click_count==1){
      pos=index(ev->win_x);
      setCursorPos(pos);
      if(ev->state&SHIFTMASK){
        extendSelection(pos);
        }
      else{
        killSelection();
        setAnchorPos(pos);
        }
      makePositionVisible(pos);
      flags|=FLAG_PRESSED;
      flags&=~FLAG_UPDATE;
      }
    else{
      setAnchorPos(0);
      setCursorPos(contents.length());
      extendSelection(contents.length());
      makePositionVisible(cursor);
      }
    return 1;
    }
  return 0;
  }


// Released left button
long FXTextField::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// Pressed middle button to paste
long FXTextField::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    pos=index(ev->win_x);
    setCursorPos(pos);
    setAnchorPos(pos);
    makePositionVisible(pos);
    update(border,border,width-(border<<1),height-(border<<1));
    flags&=~FLAG_UPDATE;
    return 1;
    }
  return 0;
  }


// Released middle button causes paste of selection
long FXTextField::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuchar *data; FXuint len;
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;

    // Paste text from selection (X-Windows only)
    if(isEditable()){
      if(getDNDData(FROM_SELECTION,stringType,data,len)){
        FXRESIZE(&data,FXchar,len+1); data[len]='\0';
        handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)data);
        FXFREE(&data);
        return 1;
        }
      }
    else{
      getApp()->beep();
      }
    }
  return 0;
  }


// Moved
long FXTextField::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint t;
  if(flags&FLAG_PRESSED){
    if(ev->win_x<(border+padleft) || (width-border-padright)<ev->win_x){
      getApp()->addTimeout(getApp()->getScrollSpeed(),this,FXWindow::ID_AUTOSCROLL);
      }
    else{
      t=index(ev->win_x);
      if(t!=cursor){
        drawCursor(0);
        cursor=t;
        extendSelection(cursor);
        }
      }
    return 1;
    }
  return 0;
  }


// Automatic scroll
long FXTextField::onAutoScroll(FXObject*,FXSelector,void*){
  FXuint buttons; FXint x,y,t,ll,rr,ww,tw,lim;
  if(flags&FLAG_PRESSED){
    t=cursor;
    getCursorPosition(x,y,buttons);
    ll=border+padleft;
    rr=width-border-padright;
    ww=rr-ll;

    if(options&TEXTFIELD_PASSWD)
      tw=font->getTextWidth("*",1)*contents.length();
    else
      tw=font->getTextWidth(contents.text(),contents.length());

    if(options&JUSTIFY_RIGHT){
      lim=tw-ww;

      // Scroll left
      if(x<ll){
        if(lim>0){
          shift+=ll-x;
          if(shift>=lim)
            shift=lim;
          else
            getApp()->addTimeout(getApp()->getScrollSpeed(),this,FXWindow::ID_AUTOSCROLL);
          }
        t=index(ll);
        }

      // Scroll right
      if(rr<x){
        if(lim>0){
          shift+=rr-x;
          if(shift<=0)
            shift=0;
          else
            getApp()->addTimeout(getApp()->getScrollSpeed(),this,FXWindow::ID_AUTOSCROLL);
          }
        t=index(rr);
        }
      FXASSERT(shift>=0);
      }
    else{
      lim=ww-tw;

      // Scroll left
      if(x<ll){
        if(lim<0){
          shift+=ll-x;
          if(shift>=0)
            shift=0;
          else
            getApp()->addTimeout(getApp()->getScrollSpeed(),this,FXWindow::ID_AUTOSCROLL);
          }
        t=index(ll);
        }

      // Scroll right
      if(rr<x){
        if(lim<0){
          shift+=rr-x;
          if(shift<=lim)
            shift=lim;
          else
            getApp()->addTimeout(getApp()->getScrollSpeed(),this,FXWindow::ID_AUTOSCROLL);
          }
        t=index(rr);
        }
      FXASSERT(shift<=0);
      }

    // Extend the selection
    if(t!=cursor){
      drawCursor(0);
      cursor=t;
      extendSelection(cursor);
      }
    }
  return 1;
  }


// Blink the cursor
long FXTextField::onBlink(FXObject*,FXSelector,void*){
  drawCursor(flags^FLAG_CARET);
  blinker=getApp()->addTimeout(getApp()->getBlinkSpeed(),this,ID_BLINK);
  return 0;
  }


// Overstrike toggle
long FXTextField::onCmdToggleOverstrike(FXObject*,FXSelector,void*){
  options^=TEXTFIELD_OVERSTRIKE;
  return 1;
  }


// Update overstrike toggle
long FXTextField::onUpdToggleOverstrike(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&TEXTFIELD_OVERSTRIKE) ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  return 1;
  }



// Editable toggle
long FXTextField::onCmdToggleEditable(FXObject*,FXSelector,void*){
  options^=TEXTFIELD_READONLY;
  return 1;
  }


// Update editable toggle
long FXTextField::onUpdToggleEditable(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&TEXTFIELD_READONLY) ? ID_UNCHECK : ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Move the cursor
void FXTextField::setCursorPos(FXint pos){
  FXint len=contents.length();
  if(pos>len) pos=len; else if(pos<0) pos=0;
  if(cursor!=pos){
    drawCursor(0);
    cursor=pos;
    if(isEditable() && hasFocus()) drawCursor(FLAG_CARET);
    }
  }


// Set anchor position
void FXTextField::setAnchorPos(FXint pos){
  FXint len=contents.length();
  if(pos>len) pos=len; else if(pos<0) pos=0;
  anchor=pos;
  }


// Draw the cursor
void FXTextField::drawCursor(FXuint state){
  FXint cl,ch,xx,len;
  if(!xid) return;
  if((state^flags)&FLAG_CARET){
    FXDCWindow dc(this);
    len=contents.length();
    FXASSERT(0<=cursor && cursor<=len);
    FXASSERT(0<=anchor && anchor<=len);
    xx=coord(cursor)-1;
    dc.setClipRectangle(border,border,width-(border<<1),height-(border<<1));
    if(flags&FLAG_CARET){
      dc.setForeground(backColor);
      dc.fillRectangle(xx,padtop+border,1,height-padbottom-padtop-(border<<1));
      dc.fillRectangle(xx-2,padtop+border,5,1);
      dc.fillRectangle(xx-2,height-border-padbottom-1,5,1);
      cl=cursor-1;
      ch=cursor+1;
      drawTextRange(dc,FXMAX(cl,0),FXMIN(ch,len));     // Gotta redraw these letters...
      flags&=~FLAG_CARET;
      }
    else{
      dc.setForeground(textColor);
      dc.fillRectangle(xx,padtop+border,1,height-padbottom-padtop-(border<<1));
      dc.fillRectangle(xx-2,padtop+border,5,1);
      dc.fillRectangle(xx-2,height-border-padbottom-1,5,1);
      flags|=FLAG_CARET;
      }
    }
  }


// Fix scroll amount after text changes or widget resize
void FXTextField::layout(){
  register FXint ll,rr,ww,tw,len;
  if(!xid) return;
  len=contents.length();
  ll=border+padleft;
  rr=width-border-padright;
  ww=rr-ll;
  if(ww<=0){
    shift=0;
    }
  else{
    if(options&TEXTFIELD_PASSWD)
      tw=font->getTextWidth("*",1)*len;
    else
      tw=font->getTextWidth(contents.text(),len);
    if(options&JUSTIFY_RIGHT){
      if(shift<0) shift=0;
      if(ww>=tw) shift=0;
      else if(shift+ww>tw) shift=tw-ww;
      FXASSERT(shift>=0);
      }
    else{
      if(shift>0) shift=0;
      if(ww>=tw) shift=0;
      else if(shift+tw<ww) shift=ww-tw;
      }
    }
  update();
  flags&=~FLAG_DIRTY;
  }


// Force position to become fully visible; we assume layout is correct
void FXTextField::makePositionVisible(FXint pos){
  FXint cw,ll,rr,ww,oldshift;
  FXint len;
  if(!xid) return;
  oldshift=shift;
  len=contents.length();
  ll=border+padleft;
  rr=width-border-padright;
  ww=rr-ll;
  if(pos>len) pos=len; else if(pos<0) pos=0;
  if(options&JUSTIFY_RIGHT){
    FXASSERT(shift>=0);
    if(options&TEXTFIELD_PASSWD)
      cw=font->getTextWidth("*",1)*(len-pos);
    else
      cw=font->getTextWidth(&contents[pos],len-pos);
    if(shift-cw>0) shift=cw;
    else if(shift-cw<-ww) shift=cw-ww;
    FXASSERT(shift>=0);
    }
  else{
    FXASSERT(shift<=0);
    if(options&TEXTFIELD_PASSWD)
      cw=font->getTextWidth("*",1)*pos;
    else
      cw=font->getTextWidth(contents.text(),pos);
    if(shift+cw<0) shift=-cw;
    else if(shift+cw>=ww) shift=ww-cw;
    FXASSERT(shift<=0);
    }
  if(shift!=oldshift){
    update(border,border,width-(border<<1),height-(border<<1));
    }
  }


// Find index from coord
FXint FXTextField::index(FXint x) const {
  FXint ci,len;
  FXint cx,cw;
  len=contents.length();
  if(options&JUSTIFY_RIGHT){
    FXASSERT(shift>=0);
    x=x-(shift+width-border-padright);
    if(x>0) return len;
    if(options&TEXTFIELD_PASSWD){
      cw=font->getTextWidth("*",1);
      ci=len+(x-(cw>>1))/cw;
      if(ci<0) ci=0;
      FXASSERT(0<=ci && ci<=len);
      }
    else{
      cx=0;
      ci=len;
      while(0<ci){
        FXASSERT(0<ci);
        cw=font->getTextWidth(&contents[ci-1],1);
        if(x>(cx-(cw>>1))) break;
        cx-=cw;
        ci-=1;
        }
      FXASSERT(0<=ci && ci<=len);
      }
    }
  else{
    FXASSERT(shift<=0);
    x=x-shift-border-padleft;
    if(x<0) return 0;
    if(options&TEXTFIELD_PASSWD){
      cw=font->getTextWidth("*",1);
      ci=(x+(cw>>1))/cw;
      if(ci>len) ci=len;
      FXASSERT(0<=ci && ci<=len);
      }
    else{
      cx=0;
      ci=0;
      while(ci<len){
        FXASSERT(ci<=len);
        cw=font->getTextWidth(&contents[ci],1);
        if(x<(cx+(cw>>1))) break;
        cx+=cw;
        ci+=1;
        }
      FXASSERT(0<=ci && ci<=len);
      }
    }
  return ci;
  }


// Find coordinate from index
FXint FXTextField::coord(FXint i) const {
  FXint tx;
  FXASSERT(0<=i && i<=contents.length());
  if(options&JUSTIFY_RIGHT){
    if(options&TEXTFIELD_PASSWD){
      tx=shift+width-border-padright-font->getTextWidth("*",1)*(contents.length()-i);
      }
    else{
      tx=shift+width-border-padright-font->getTextWidth(&contents[i],contents.length()-i);
      }
    }
  else{
    if(options&TEXTFIELD_PASSWD){
      tx=shift+border+padleft+font->getTextWidth("*",1)*i;
      }
    else{
      tx=shift+border+padleft+font->getTextWidth(contents.text(),i);
      }
    }
  return tx;
  }


// Return TRUE if position is visible
FXbool FXTextField::isPosVisible(FXint pos) const {
  FXint len=contents.length();
  if(0<=pos && pos<=len){
    FXint x=coord(pos);
    return border+padleft<=x && x<=width-border-padright;
    }
  return FALSE;
  }


// Return TRUE if position pos is selected
FXbool FXTextField::isPosSelected(FXint pos) const {
  return hasSelection() && FXMIN(anchor,cursor)<=pos && pos<=FXMAX(anchor,cursor);
  }


// Draw text fragment
void FXTextField::drawTextFragment(FXDCWindow& dc,FXint x,FXint y,FXint fm,FXint to){
  x+=font->getTextWidth(contents.text(),fm);
  y+=font->getFontAscent();
  dc.drawText(x,y,&contents[fm],to-fm);
  }


// Draw text fragment in password mode
void FXTextField::drawPWDTextFragment(FXDCWindow& dc,FXint x,FXint y,FXint fm,FXint to){
  register FXint cw=font->getTextWidth("*",1);
  register FXint i;
  y+=font->getFontAscent();
  for(i=fm; i<to; i++){dc.drawText(x+cw*i,y,"*",1);}
  }


// Draw range of text
void FXTextField::drawTextRange(FXDCWindow& dc,FXint fm,FXint to){
  register FXint sx,ex,xx,yy,cw,hh,ww,si,ei,lx,rx;
  if(to<=fm) return;
  dc.setTextFont(font);

  // Text color
  dc.setForeground(textColor);

  // Height
  hh=font->getFontHeight();

  // Text centered in y
  if((options&JUSTIFY_TOP) && (options&JUSTIFY_BOTTOM)){
    yy=border+padtop+(height-padbottom-padtop-(border<<1)-hh)/2;
    }

  // Text sticks to top of field
  else if(options&JUSTIFY_TOP){
    yy=padtop+border;
    }

  // Text sticks to bottom of field
  else if(options&JUSTIFY_BOTTOM){
    yy=height-padbottom-border-hh;
    }

  // Text centered in y
  else{
    yy=border+padtop+(height-padbottom-padtop-(border<<1)-hh)/2;
    }

  if(anchor<cursor){si=anchor;ei=cursor;}else{si=cursor;ei=anchor;}

  // Password mode
  if(options&TEXTFIELD_PASSWD){
    cw=font->getTextWidth("*",1);
    ww=cw*contents.length();

    // Text sticks to right of field
    if(options&JUSTIFY_RIGHT){
      xx=shift+width-border-padright-ww;
      }

    // Text on left is the default
    else{
      xx=shift+border+padleft;
      }

    // Reduce to avoid drawing excessive amounts of text
    lx=xx+cw*fm;
    rx=xx+cw*to;
    while(fm<to){
      if(lx+cw>=0) break;
      lx+=cw;
      fm++;
      }
    while(fm<to){
      if(rx-cw<width) break;
      rx-=cw;
      to--;
      }
      
    // Adjust selected range
    if(si<fm) si=fm;
    if(ei>to) ei=to;
      
    // Nothing selected
    if(!hasSelection() || to<=si || ei<=fm){
      drawPWDTextFragment(dc,xx,yy,fm,to);
      }

    // Stuff selected
    else{
      if(fm<si){
        drawPWDTextFragment(dc,xx,yy,fm,si);
        }
      else{
        si=fm;
        }
      if(ei<to){
        drawPWDTextFragment(dc,xx,yy,ei,to);
        }
      else{
        ei=to;
        }
      if(si<ei){
        sx=xx+cw*si;
        ex=xx+cw*ei;
        if(hasFocus()){
          dc.setForeground(selbackColor);
          dc.fillRectangle(sx,padtop+border,ex-sx,height-padtop-padbottom-(border<<1));
          dc.setForeground(seltextColor);
          drawPWDTextFragment(dc,xx,yy,si,ei);
          }
        else{
          dc.setForeground(baseColor);
          dc.fillRectangle(sx,padtop+border,ex-sx,height-padtop-padbottom-(border<<1));
          dc.setForeground(textColor);
          drawPWDTextFragment(dc,xx,yy,si,ei);
          }
        }
      }
    }

  // Normal mode
  else{
    ww=font->getTextWidth(contents.text(),contents.length());

    // Text sticks to right of field
    if(options&JUSTIFY_RIGHT){
      xx=shift+width-border-padright-ww;
      }

    // Text on left is the default
    else{
      xx=shift+border+padleft;
      }

    // Reduce to avoid drawing excessive amounts of text
    lx=xx+font->getTextWidth(&contents[0],fm);
    rx=lx+font->getTextWidth(&contents[fm],to-fm);
    while(fm<to){
      cw=font->getTextWidth(&contents[fm],1);
      if(lx+cw>=0) break;
      lx+=cw;
      fm++;
      }
    while(fm<to){
      cw=font->getTextWidth(&contents[to-1],1);
      if(rx-cw<width) break;
      rx-=cw;
      to--;
      }
      
    // Adjust selected range
    if(si<fm) si=fm;
    if(ei>to) ei=to;
      
    // Nothing selected
    if(!hasSelection() || to<=si || ei<=fm){
      drawTextFragment(dc,xx,yy,fm,to);
      }

    // Stuff selected
    else{
      if(fm<si){
        drawTextFragment(dc,xx,yy,fm,si);
        }
      else{
        si=fm;
        }
      if(ei<to){
        drawTextFragment(dc,xx,yy,ei,to);
        }
      else{
        ei=to;
        }
      if(si<ei){
        sx=xx+font->getTextWidth(contents.text(),si);
        ex=xx+font->getTextWidth(contents.text(),ei);
        if(hasFocus()){
          dc.setForeground(selbackColor);
          dc.fillRectangle(sx,padtop+border,ex-sx,height-padtop-padbottom-(border<<1));
          dc.setForeground(seltextColor);
          drawTextFragment(dc,xx,yy,si,ei);
          }
        else{
          dc.setForeground(baseColor);
          dc.fillRectangle(sx,padtop+border,ex-sx,height-padtop-padbottom-(border<<1));
          dc.setForeground(textColor);
          drawTextFragment(dc,xx,yy,si,ei);
          }
        }
      }
    }
  }


// Handle repaint
long FXTextField::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  // Draw frame
  drawFrame(dc,0,0,width,height);

  // Gray background if disabled
  if(isEnabled())
    dc.setForeground(backColor);
  else
    dc.setForeground(baseColor);

  // Draw background
  dc.fillRectangle(border,border,width-(border<<1),height-(border<<1));

  // Draw text, clipped against frame interior
  dc.setClipRectangle(border,border,width-(border<<1),height-(border<<1));
  drawTextRange(dc,0,contents.length());

  // Draw caret
  if(flags&FLAG_CARET){
    int xx=coord(cursor)-1;
    dc.setForeground(textColor);
    dc.fillRectangle(xx,padtop+border,1,height-padbottom-padtop-(border<<1));
    dc.fillRectangle(xx-2,padtop+border,5,1);
    dc.fillRectangle(xx-2,height-border-padbottom-1,5,1);
    }
  return 1;
  }


// Move cursor to begin of line
long FXTextField::onCmdCursorHome(FXObject*,FXSelector,void*){
  setCursorPos(0);
  makePositionVisible(0);
  return 1;
  }


// Move cursor to end of line
long FXTextField::onCmdCursorEnd(FXObject*,FXSelector,void*){
  setCursorPos(contents.length());
  makePositionVisible(cursor);
  return 1;
  }


// Move cursor right
long FXTextField::onCmdCursorRight(FXObject*,FXSelector,void*){
  setCursorPos(cursor+1);
  makePositionVisible(cursor);
  return 1;
  }


// Move cursor left
long FXTextField::onCmdCursorLeft(FXObject*,FXSelector,void*){
  setCursorPos(cursor-1);
  makePositionVisible(cursor);
  return 1;
  }


// Mark
long FXTextField::onCmdMark(FXObject*,FXSelector,void*){
  setAnchorPos(cursor);
  return 1;
  }


// Extend
long FXTextField::onCmdExtend(FXObject*,FXSelector,void*){
  extendSelection(cursor);
  return 1;
  }


// Select All
long FXTextField::onCmdSelectAll(FXObject*,FXSelector,void*){
  selectAll();
  makePositionVisible(cursor);
  return 1;
  }


// Deselect All
long FXTextField::onCmdDeselectAll(FXObject*,FXSelector,void*){
  killSelection();
  return 1;
  }


// Cut
long FXTextField::onCmdCutSel(FXObject*,FXSelector,void*){
  FXDragType types[1];
  if(hasSelection()){
    if(isEditable()){
      types[0]=stringType;
      if(acquireClipboard(types,1)){
        if(anchor<cursor)
          clipped=contents.mid(anchor,cursor-anchor);
        else
          clipped=contents.mid(cursor,anchor-cursor);
        handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
        }
      }
    else{
      getApp()->beep();
      }
    }
  return 1;
  }


// Copy onto cliboard
long FXTextField::onCmdCopySel(FXObject*,FXSelector,void*){
  FXDragType types[1];
  if(hasSelection()){
    types[0]=stringType;
    if(acquireClipboard(types,1)){
      if(anchor<cursor)
        clipped=contents.mid(anchor,cursor-anchor);
      else
        clipped=contents.mid(cursor,anchor-cursor);
      }
    }
  return 1;
  }


// Paste
long FXTextField::onCmdPasteSel(FXObject*,FXSelector,void*){
  FXuchar *data; FXuint len;
  if(isEditable()){
    if(hasSelection()){
      handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
      }
    if(getDNDData(FROM_CLIPBOARD,stringType,data,len)){
      FXRESIZE(&data,FXchar,len+1); data[len]='\0';
      handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)data);
      FXFREE(&data);
      return 1;
      }
    }
  else{
    getApp()->beep();
    }
  return 1;
  }


// Delete selection
long FXTextField::onCmdDeleteSel(FXObject*,FXSelector,void*){
  int st,en;
  if(!hasSelection()) return 1;
  st=FXMIN(anchor,cursor);
  en=FXMAX(anchor,cursor);
  setCursorPos(st);
  setAnchorPos(cursor);
  contents.remove(st,en-st);
  layout();
  makePositionVisible(st);
  killSelection();
  flags|=FLAG_CHANGED;
  if(target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());}
  return 1;
  }


// Overstrike string
long FXTextField::onCmdOverstString(FXObject*,FXSelector,void* ptr){
  FXString tentative=contents;
  FXint len=strlen((FXchar*)ptr);
  tentative.replace(cursor,len,(FXchar*)ptr,len);
  if(handle(this,MKUINT(0,SEL_VERIFY),(void*)tentative.text())){ getApp()->beep(); return 1; }
  contents=tentative;
  layout();
  setCursorPos(cursor+1);
  setAnchorPos(cursor);
  makePositionVisible(cursor);
  update(border,border,width-(border<<1),height-(border<<1));
  flags|=FLAG_CHANGED;
  if(target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());}
  return 1;
  }


// Insert a string
long FXTextField::onCmdInsertString(FXObject*,FXSelector,void* ptr){
  FXString tentative=contents;
  FXint len=strlen((FXchar*)ptr);
  tentative.insert(cursor,(FXchar*)ptr,len);
  if(handle(this,MKUINT(0,SEL_VERIFY),(void*)tentative.text())){ getApp()->beep(); return 1; }
  contents=tentative;
  layout();
  setCursorPos(cursor+len);
  setAnchorPos(cursor);
  makePositionVisible(cursor);
  update(border,border,width-(border<<1),height-(border<<1));
  flags|=FLAG_CHANGED;
  if(target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());}
  return 1;
  }


// Backspace character
long FXTextField::onCmdBackspace(FXObject*,FXSelector,void*){
  if(cursor<1){ getApp()->beep(); return 1; }
  setCursorPos(cursor-1);
  setAnchorPos(cursor);
  contents.remove(cursor,1);
  layout();
  makePositionVisible(cursor);
  update(border,border,width-(border<<1),height-(border<<1));
  flags|=FLAG_CHANGED;
  if(target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());}
  return 1;
  }


// Delete character
long FXTextField::onCmdDelete(FXObject*,FXSelector,void*){
  if(cursor>=contents.length()){ getApp()->beep(); return 1; }
  contents.remove(cursor,1);
  layout();
  setCursorPos(cursor);
  setAnchorPos(cursor);
  makePositionVisible(cursor);
  update(border,border,width-(border<<1),height-(border<<1));
  flags|=FLAG_CHANGED;
  if(target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)contents.text());}
  return 1;
  }


// Verify tentative input.
long FXTextField::onVerify(FXObject*,FXSelector,void* ptr){
  register FXchar *p=(FXchar*)ptr;

  // Limit number of columns
  if(options&TEXTFIELD_LIMITED){
    if(strlen(p)>columns) return 1;
    }

  // Integer input
  if(options&TEXTFIELD_INTEGER){
    while(isspace((FXuchar)*p)) p++;
    if(*p=='-' || *p=='+') p++;
    while(isdigit((FXuchar)*p)) p++;
    while(isspace((FXuchar)*p)) p++;
    if(*p!='\0') return 1;    // Objection!
    }

  // Real input
  if(options&TEXTFIELD_REAL){
    while(isspace((FXuchar)*p)) p++;
    if(*p=='-' || *p=='+') p++;
    while(isdigit((FXuchar)*p)) p++;
    if(*p=='.') p++;
    while(isdigit((FXuchar)*p)) p++;
    if(*p=='E' || *p=='e'){
      p++;
      if(*p=='-' || *p=='+') p++;
      while(isdigit((FXuchar)*p)) p++;
      }
    while(isspace((FXuchar)*p)) p++;
    if(*p!='\0') return 1;    // Objection!
    }

  // Target has chance to object to the proposed change
  if(target && target->handle(this,MKUINT(message,SEL_VERIFY),ptr)) return 1;

  // No objections have been raised!
  return 0;
  }


// Pressed a key
long FXTextField::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    FXTRACE((200,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state));
    if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
    flags&=~FLAG_UPDATE;
    switch(event->code){
      case KEY_Right:
      case KEY_KP_Right:
        if(!(event->state&SHIFTMASK)){
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_CURSOR_RIGHT,SEL_COMMAND),NULL);
        if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
          }
        return 1;
      case KEY_Left:
      case KEY_KP_Left:
        if(!(event->state&SHIFTMASK)){
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_CURSOR_LEFT,SEL_COMMAND),NULL);
        if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
          }
        return 1;
      case KEY_Home:
      case KEY_KP_Home:
        if(!(event->state&SHIFTMASK)){
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_CURSOR_HOME,SEL_COMMAND),NULL);
        if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
          }
        return 1;
      case KEY_End:
      case KEY_KP_End:
        if(!(event->state&SHIFTMASK)){
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_CURSOR_END,SEL_COMMAND),NULL);
        if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
          }
        return 1;
      case KEY_Insert:
      case KEY_KP_Insert:
        if(event->state&CONTROLMASK){
          handle(this,MKUINT(ID_COPY_SEL,SEL_COMMAND),NULL);
          return 1;
          }
        else if(event->state&SHIFTMASK){
          if(isEditable()){
            handle(this,MKUINT(ID_PASTE_SEL,SEL_COMMAND),NULL);
            }
          else{
            getApp()->beep();
            }
          return 1;
          }
        else{
          handle(this,MKUINT(ID_TOGGLE_OVERSTRIKE,SEL_COMMAND),NULL);
          }
        return 1;
      case KEY_Delete:
      case KEY_KP_Delete:
        if(isEditable()){
          if(hasSelection())
            handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          else
            handle(this,MKUINT(ID_DELETE,SEL_COMMAND),NULL);
          }
        else{
          getApp()->beep();
          }
        return 1;
      case KEY_BackSpace:
        if(isEditable()){
          if(hasSelection()){
            handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
            }
          else{
            handle(this,MKUINT(ID_BACKSPACE,SEL_COMMAND),NULL);
            }
          }
        else{
          getApp()->beep();
          }
        return 1;
      case KEY_Return:
      case KEY_KP_Enter:
        if(isEditable()){
          flags|=FLAG_UPDATE;
          flags&=~FLAG_CHANGED;
          if(target){
            target->handle(this,MKUINT(message,SEL_COMMAND),(void*)contents.text());
            }
          }
        else{
          getApp()->beep();
          }
        return 1;
      case KEY_a:
        if(!(event->state&CONTROLMASK)) goto ins;
        handle(this,MKUINT(ID_SELECT_ALL,SEL_COMMAND),NULL);
        return 1;
      case KEY_x:
        if(!(event->state&CONTROLMASK)) goto ins;
        if(isEditable()){
          handle(this,MKUINT(ID_CUT_SEL,SEL_COMMAND),NULL);
          }
        else{
          getApp()->beep();
          }
        return 1;
      case KEY_c:
        if(!(event->state&CONTROLMASK)) goto ins;
        handle(this,MKUINT(ID_COPY_SEL,SEL_COMMAND),NULL);
        return 1;
      case KEY_v:
        if(!(event->state&CONTROLMASK)) goto ins;
        if(isEditable()){
          handle(this,MKUINT(ID_PASTE_SEL,SEL_COMMAND),NULL);
          }
        else{
          getApp()->beep();
          }
        return 1;
      default:
ins:    if((event->state&(CONTROLMASK|ALTMASK)) || ((FXuchar)event->text[0]<32)) return 0;
        if(isEditable()){
          if(hasSelection()){
            handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
            }
          if(options&TEXTFIELD_OVERSTRIKE){
            handle(this,MKUINT(ID_OVERST_STRING,SEL_COMMAND),(void*)event->text.text());
            }
          else{
            handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)event->text.text());
            }
          }
        else{
          getApp()->beep();
          }
        return 1;
      }
    }
  return 0;
  }


// Key Release
long FXTextField::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    FXTRACE((200,"%s::onKeyRelease keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state));
    if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
    }
  return 0;
  }


// Kill the selection
FXbool FXTextField::killSelection(){
  if(hasSelection()){
    releaseSelection();
    update(border,border,width-(border<<1),height-(border<<1));
    return TRUE;
    }
  return FALSE;
  }


// Select all text
FXbool FXTextField::selectAll(){
  setAnchorPos(0);
  setCursorPos(contents.length());
  extendSelection(cursor);
  return TRUE;
  }


// Set selection
FXbool FXTextField::setSelection(FXint pos,FXint len){
  setAnchorPos(pos);
  setCursorPos(pos+len);
  extendSelection(cursor);
  return TRUE;
  }


// Extend selection
FXbool FXTextField::extendSelection(FXint pos){
  FXDragType types[1];
  FXint l=contents.length();

  // Validate position
  if(pos<0) pos=0;
  else if(pos>l) pos=l;

  // Got a selection at all?
  if(anchor!=pos){
    types[0]=stringType;
    if(!hasSelection()){
      acquireSelection(types,1);
      }
    }
  else{
    if(hasSelection()){
      releaseSelection();
      }
    }

  update(border,border,width-(border<<1),height-(border<<1));
  return TRUE;
  }


// Set text being displayed
void FXTextField::setText(const FXString& text){
  if(contents!=text){
    FXint len=text.length();
    contents=text;
    if(anchor>len) anchor=len;
    if(cursor>len) cursor=len;
    if(anchor==cursor) releaseSelection();
    if(xid) layout();
    makePositionVisible(cursor);
    update(border,border,width-(border<<1),height-(border<<1));
    }
  }


// Set text color
void FXTextField::setTextColor(FXColor clr){
  if(textColor!=clr){
    textColor=clr;
    update();
    }
  }


// Set select background color
void FXTextField::setSelBackColor(FXColor clr){
  if(selbackColor!=clr){
    selbackColor=clr;
    update();
    }
  }


// Set selected text color
void FXTextField::setSelTextColor(FXColor clr){
  if(seltextColor!=clr){
    seltextColor=clr;
    update();
    }
  }


// Change number of columns
void FXTextField::setNumColumns(FXint ncols){
  if(ncols<0) ncols=0;
  if(columns!=ncols){
    shift=0;
    columns=ncols;
    layout();   // This may not be necessary!
    recalc();
    update();
    }
  }


// Change help text
void FXTextField::setHelpText(const FXString& text){
  help=text;
  }


// Change tip text
void FXTextField::setTipText(const FXString& text){
  tip=text;
  }


// Return true if editable
FXbool FXTextField::isEditable() const {
  return (options&TEXTFIELD_READONLY)==0;
  }


// Set widget is editable or not
void FXTextField::setEditable(FXbool edit){
  if(edit) options&=~TEXTFIELD_READONLY; else options|=TEXTFIELD_READONLY;
  }


// Change text style
void FXTextField::setTextStyle(FXuint style){
  FXuint opts=(options&~TEXTFIELD_MASK) | (style&TEXTFIELD_MASK);
  if(options!=opts){
    shift=0;
    options=opts;
    recalc();
    update();
    }
  }


// Get text style
FXuint FXTextField::getTextStyle() const {
  return (options&TEXTFIELD_MASK);
  }


// Set text justify style
void FXTextField::setJustify(FXuint style){
  FXuint opts=(options&~JUSTIFY_MASK) | (style&JUSTIFY_MASK);
  if(options!=opts){
    shift=0;
    options=opts;
    recalc();
    update();
    }
  }


// Get text justify style
FXuint FXTextField::getJustify() const {
  return (options&JUSTIFY_MASK);
  }


// Save object to stream
void FXTextField::save(FXStream& store) const {
  FXFrame::save(store);
  store << contents;
  store << font;
  store << textColor;
  store << selbackColor;
  store << seltextColor;
  store << columns;
  store << help;
  store << tip;
  }


// Load object from stream
void FXTextField::load(FXStream& store){
  FXFrame::load(store);
  store >> contents;
  store >> font;
  store >> textColor;
  store >> selbackColor;
  store >> seltextColor;
  store >> columns;
  store >> help;
  store >> tip;
  }


// Clean up
FXTextField::~FXTextField(){
  if(blinker) getApp()->removeTimeout(blinker);
  font=(FXFont*)-1;
  blinker=(FXTimer*)-1;
  }
