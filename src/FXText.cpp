/********************************************************************************
*                                                                               *
*                    M u l t i - L i ne   T e x t   O b j e c t                 *
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
* $Id: FXText.cpp,v 1.56 1998/11/02 02:13:51 jeroen Exp $                       *
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
#include "FXComposite.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXText.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXTooltip.h"



/*
  Notes:
  - Line start array is one longer than number of visible lines.
  - Keep '\0' at end of buffer!
  - Do we need all those self-messages??
  - Need send message after text delete.
  - Add serialization.
  - GUI updatable as long as its in the focus chain.
  - Handle tabs and other control codes properly (in particular, backspace? for ^H_ stuff?)
  - We want both tab translated to spaces as well as tab-stops from an array.
  - Some control characters in the buffer may be OK (e.g. ^L)
  - Need to add buffer read/write here also.
  - Cursor should repaint left/right side based on style
  - Update cursor etc ONLY when inside selection....
  - Mouse based selection callbacks not consistent
  - Ghost images cursor when select dragging..
  - Redraw when cutting text in middle
  - Add routine to determine widest line in a chunk of buffer
*/


#define MINSIZE   80                  // Minimum gap size
#define NVISLINES 20                  // Initial visible lines count

#define STYLE_UNDERLINE 1
#define STYLE_FILL      2
#define STYLE_SELECTED  4
#define STYLE_CONTROL   8
#define STYLE_TAB       16

#define MOUSE_NONE    0                // None in effect
#define MOUSE_CHARS   1                // Character drag select
#define MOUSE_WORDS   2                // Word drag select
#define MOUSE_LINES   3                // Line drag select
#define MOUSE_SCROLL  4                // Scrolling mode
#define MOUSE_DRAG    5                // DND-drag
#define MOUSE_PASTE   6                // Text paste


/*******************************************************************************/

  
// Map
FXDEFMAP(FXText) FXTextMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXText::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXText::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,1,FXText::onBlink),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXText::onAutoScroll),
  FXMAPFUNC(SEL_FOCUSIN,0,FXText::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXText::onFocusOut),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXText::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXText::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXText::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXText::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXText::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXText::onRightBtnRelease),
  FXMAPFUNC(SEL_CHANGED,0,FXText::onChanged),
  FXMAPFUNC(SEL_DND_ENTER,0,FXText::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXText::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXText::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXText::onDNDMotion),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXText::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXText::onSelectionGained),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXText::onSelectionRequest),
  FXMAPFUNC(SEL_KEYPRESS,0,FXText::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXText::onKeyRelease),
  FXMAPFUNC(SEL_SELECTED,0,FXText::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXText::onDeselected),
  FXMAPFUNC(SEL_INSERTED,0,FXText::onInserted),
  FXMAPFUNC(SEL_DELETED,0,FXText::onDeleted),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_TOGGLE_EDITABLE,FXText::onUpdToggleEditable),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_CURSOR_LINE,FXText::onUpdCursorLine),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_CURSOR_COLUMN,FXText::onUpdCursorColumn),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_CUT_SEL,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_COPY_SEL,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_DELETE_SEL,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_TOP,FXText::onCmdCursorTop),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_BOTTOM,FXText::onCmdCursorBottom),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_HOME,FXText::onCmdCursorHome),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_END,FXText::onCmdCursorEnd),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_RIGHT,FXText::onCmdCursorRight),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_LEFT,FXText::onCmdCursorLeft),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_UP,FXText::onCmdCursorUp),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_DOWN,FXText::onCmdCursorDown),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_WORD_LEFT,FXText::onCmdCursorWordLeft),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_WORD_RIGHT,FXText::onCmdCursorWordRight),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_PAGEDOWN,FXText::onCmdCursorPageDown),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_PAGEUP,FXText::onCmdCursorPageUp),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_SCRNTOP,FXText::onCmdCursorScreenTop),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_SCRNBTM,FXText::onCmdCursorScreenBottom),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_SCRNCTR,FXText::onCmdCursorScreenCenter),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_MARK,FXText::onCmdMark),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_EXTEND,FXText::onCmdExtend),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_INSERT_CHAR,FXText::onCmdInsertChar),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_INSERT_STRING,FXText::onCmdInsertString),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_INSERT_NEWLINE,FXText::onCmdInsertNewline),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CUT_SEL,FXText::onCmdCutSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_COPY_SEL,FXText::onCmdCopySel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_PASTE_SEL,FXText::onCmdPasteSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_SEL,FXText::onCmdDeleteSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_CHAR,FXText::onCmdSelectChar),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_WORD,FXText::onCmdSelectWord),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_LINE,FXText::onCmdSelectLine),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_ALL,FXText::onCmdSelectAll),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DESELECT_ALL,FXText::onCmdDeselectAll),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_BACKSPACE,FXText::onCmdBackspace),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_BACKSPACE_WORD,FXText::onCmdBackspaceWord),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_BACKSPACE_BOL,FXText::onCmdBackspaceBol),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE,FXText::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_WORD,FXText::onCmdDeleteWord),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_EOL,FXText::onCmdDeleteEol),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_LINE,FXText::onCmdDeleteLine),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_TOGGLE_EDITABLE,FXText::onCmdToggleEditable),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_LINE,FXText::onCmdCursorLine),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_COLUMN,FXText::onCmdCursorColumn),
  };


// Object implementation
FXIMPLEMENT(FXText,FXScrollArea,FXTextMap,ARRAYNUMBER(FXTextMap))

  
// Word delimiters
const FXchar FXText::delimiters[]="~.,/\\`'!@#$%^&*()-=+{}|[]\":;<>?";
  

// Drag and drop name for simple text
const FXchar FXText::textDragTypeName[]="text/plain";


// Request for this type will delete the selection
const FXchar FXText::textDeleteTypeName[]="DELETE";


// Text drag and drop type
FXDragType FXText::textDragType=0;
FXDragType FXText::textDeleteType=0;

/*******************************************************************************/


// Helper
static inline FXint fxabs(FXint a){ return a<0?-a:a; }

// Test if its a word delimiter
FXbool FXText::isdelim(FXchar ch){ return strchr(FXText::delimiters,ch)!=NULL; }


// Text widget
FXText::FXText(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  FXCALLOC(&buffer,FXchar,MINSIZE);
  FXCALLOC(&lines,FXint,NVISLINES+1);
  length=1;
  nlines=0;
  nvislines=NVISLINES;
  gapstart=0;
  gapend=MINSIZE-1;
  topline=0;
  topy=0;
  toplineno=1;
  selstartpos=0;
  selendpos=0;  
  anchorpos=0;
  cursorpos=0;
  cursorline=0;
  cursorlineno=1;
  cursorcol=0;
  cursorx=0;
  cursory=0;
  cursorprefx=-1;
  margintop=2;
  marginbottom=2;
  marginleft=3;
  marginright=3;
  //font=getApp()->normalFont;
  //font=new FXFont(getApp(),"-adobe-courier-bold-r-*-*-12-*-*-*-*-*-iso8859-1");
  font=new FXFont(getApp(),"9x15bold");
  defaultCursor=getApp()->textCursor;
  dragCursor=getApp()->dndCopyCursor;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  baseColor=0;
  cursorColor=0;
  blinker=NULL;
  textWidth=0;
  textHeight=0;
  tabdist=8;
  modified=FALSE;
  dragmode=MOUSE_NONE;
  grabx=0;
  graby=0;
  }



// Create X window
void FXText::create(){
  FXScrollArea::create();
  font->create();
  textColor=acquireColor(getApp()->foreColor);
  selbackColor=acquireColor(getApp()->selbackColor);
  seltextColor=acquireColor(getApp()->selforeColor);
  cursorColor=acquireColor(FXRGB(192,0,0));
  baseColor=acquireColor(getApp()->backColor);
  if(!textDragType){textDragType=getApp()->registerDragType(textDragTypeName);}
  if(!textDeleteType){textDeleteType=getApp()->registerDragType(textDeleteTypeName);}
  dropEnable();
  }


// If window can have focus
FXbool FXText::canFocus() const { return 1; }


// Propagate size change
void FXText::recalc(){ 
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


// Enable the window
void FXText::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXScrollArea::enable();
    update(0,0,viewport_w,viewport_h);
    }
  }


// Disable the window
void FXText::disable(){
  if(flags&FLAG_ENABLED){
    FXScrollArea::disable();
    update(0,0,viewport_w,viewport_h);
    }
  }


// Move content
void FXText::moveContents(FXint x,FXint y){
  register FXint ln,delta,i,dx,dy,tx,ty,fx,fy,ex,ey,lineh;
  
  // Get line height
  lineh=font->getFontHeight();
  
  // Turn cursor off
  drawCursor(0);
  
  // Find first partially visible line after the scroll
  delta=0;
  while(y+topy+margintop+lineh<=0){
    ln=forwardNLines(topline);
    if(ln>=length) break;
    topline=ln;
    toplineno++;
    topy+=lineh;
    delta++;
    }
  while(y+topy+margintop>0){
    if(topline<=0) break;
    topline=backwardNLines(topline);
    toplineno--;
    topy-=lineh;
    delta--;
    }
  
  // Did it move more than a line's worth?
  if(delta!=0){
  
    // Update line starts
    if(delta<0 && -delta<nvislines){
      for(i=nvislines; i>=-delta; i--) lines[i]=lines[delta+i];
      calcLines(0,-delta);
      }
    else if(delta>0 && delta<nvislines){
      for(i=0; i<=nvislines-delta; i++) lines[i]=lines[delta+i];
      calcLines(nvislines-delta,nvislines);
      }
    else{
      calcLines(0,nvislines);
      }
    }
  
  // Blit and/or repaint
  if(pos_x<x){
    dx=x-pos_x; 
    fx=marginleft; 
    tx=marginleft+dx; 
    ex=marginleft;
    }
  else{
    dx=pos_x-x; 
    fx=marginleft+dx; 
    tx=marginleft; 
    ex=viewport_w-marginright-dx;
    }
  if(pos_y<y){
    dy=y-pos_y; 
    fy=margintop; 
    ty=margintop+dy; 
    ey=margintop;
    }
  else{
    dy=pos_y-y; 
    fy=margintop+dy; 
    ty=margintop; 
    ey=viewport_h-marginbottom-dy;
    }
  pos_x=x;
  pos_y=y;
  if((flags&FLAG_OBSCURED) || (viewport_w<=dx) || (viewport_h<=dy)){
    update(0,0,viewport_w,viewport_h);
    }
  else{
    FXEvent ev;
    ev.type=SEL_PAINT;
    ev.window=xid;
    drawArea(this,fx,fy,viewport_w-marginleft-marginright-dx,viewport_h-margintop-marginbottom-dy,tx,ty);
    if(dy){
      ev.rect.x=0;
      ev.rect.y=ey;
      ev.rect.w=viewport_w;
      ev.rect.h=dy;
      handle(this,MKUINT(0,SEL_PAINT),&ev);
      }
    if(dx){
      ev.rect.x=ex;
      ev.rect.y=0;
      ev.rect.w=dx;
      ev.rect.h=viewport_h;
      handle(this,MKUINT(0,SEL_PAINT),&ev);
      }
    }
  }


// Recompute size of text 
void FXText::recompute(){
  register FXint beg,end,tw,th,nl,t;
  beg=nl=tw=th=0;
  while(beg<length){
    end=lineEnd(beg);
    if((t=lineWidth(beg,end-beg))>tw) tw=t;
    th+=lineHeight(beg,end-beg);
    beg=end+1;
    nl++;
    }
  textWidth=tw;
  textHeight=th;
  nlines=nl;
  flags&=~FLAG_RECALC;
  }


// Determine content width of scroll area
FXint FXText::getContentWidth(){ 
  if(flags&FLAG_RECALC) recompute();
  return marginleft+marginright+textWidth; 
  }


// Determine content height of scroll area
FXint FXText::getContentHeight(){ 
  if(flags&FLAG_RECALC) recompute();
  return margintop+marginbottom+textHeight; 
  }


// Recalculate layout
void FXText::layout(){
  
  // Scrollbars adjusted
  FXScrollArea::layout();
  
  // Number of visible lines
  nvislines=(getViewportHeight()+2*font->getFontHeight()-2)/font->getFontHeight();
  
  // Number of visible lines changed; lines is 1 longer than nvislines, 
  // so we can find the end of a line faster for every visible line
  FXRESIZE(&lines,FXint,nvislines+1);
  
  // As scroll position may have changed, need to adjust topline
  findTopLine();

  // Recompute line starts
  calcLines(0,nvislines);
  
  // Set line size based on font
  vertical->setLine(font->getFontHeight());
  horizontal->setLine(font->getTextWidth(" ",1));
  
  // Force repaint
  //update(0,0,width,height);
  
  // Done
  flags&=~FLAG_DIRTY;
  }


// Gained focus
long FXText::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onFocusIn\n",getClassName());
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(!blinker){blinker=getApp()->addTimeout(getApp()->blinkSpeed,this,1);}
  drawCursor(FLAG_CARET);
  return 1;
  }


// Lost focus
long FXText::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onFocusOut\n",getClassName());
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(blinker){blinker=getApp()->removeTimeout(blinker);}
  drawCursor(FLAG_CARET);
  flags|=FLAG_UPDATE;
  return 1;
  }


/*******************************************************************************/


// We were asked about status text
long FXText::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    ((FXStatusline*)sender)->setText(help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXText::onQueryTip(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
  if(tip.text() && (flags&FLAG_TIP)){
    ((FXTooltip*)sender)->setText(tip);
    return 1;
    }
  return 0;
  }


// Blink the cursor
long FXText::onBlink(FXObject*,FXSelector,void*){
  drawCursor(flags^FLAG_CARET);
  blinker=getApp()->addTimeout(getApp()->blinkSpeed,this,1);
  return 0;
  }
 

// Pressed left button
long FXText::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    flags|=FLAG_PRESSED;
    flags&=~FLAG_UPDATE;
    if(ev->click_count==1){
      pos=getPosAt(ev->win_x,ev->win_y);
      if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
//fprintf(stderr,"getPosAt(%d,%d) = %d\n",ev->win_x,ev->win_y,pos);
      if(ev->state&SHIFTMASK){
        extendSelection(pos,SELECT_CHARS);
        }
      else{
        killSelection();
        setAnchorPos(pos);
        }
      dragmode=MOUSE_CHARS;
      }
    
    // Select word by double click
    else if(ev->click_count==2){
      setAnchorPos(cursorpos);
      extendSelection(cursorpos,SELECT_WORDS);
      dragmode=MOUSE_WORDS;
      }
    
    // Select line by triple click
    else{
      setAnchorPos(cursorpos);
      extendSelection(cursorpos,SELECT_LINES);
      dragmode=MOUSE_LINES;
      }
    return 1;
    }
  return 0;
  }


// Released left button
long FXText::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){ 
    flags&=~FLAG_PRESSED;
    flags|=FLAG_UPDATE;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    dragmode=MOUSE_NONE;
    return 1;
    }
  return 0;
  }


// Pressed middle button
long FXText::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    pos=getPosAt(ev->win_x,ev->win_y);
    
    // Move over
    if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
    
    // Start text drag
    if(isPosSelected(pos)){
      grab();
      beginDrag(&textDragType,1);
      dragmode=MOUSE_DRAG;
      flags|=FLAG_PRESSED;
      flags&=~FLAG_UPDATE;
      }
    
    // Place cursor
    else if(isEditable()){
      dragmode=MOUSE_PASTE;
      flags|=FLAG_PRESSED;
      flags&=~FLAG_UPDATE;
      }
    
    // Beep
    else{
      getApp()->beep();
      }
    return 1;
    }
  return 0;
  }


// Released middle button
long FXText::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXuchar *data;
  FXuint  size;
  FXint what[2];
  if(isEnabled()){
    flags&=~FLAG_PRESSED;
    flags|=FLAG_UPDATE;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    
    // Drop text somewhere
    if(dragmode==MOUSE_DRAG){
      ungrab();
      endDrag((didAccept()!=DRAG_REJECT));
      }
    
    // Paste text from selection
    else if(dragmode==MOUSE_PASTE){
      if(getDNDData(XA_STRING,data,size)){
        what[0]=cursorpos;
        what[1]=size;
        insert(cursorpos,(FXchar*)data,size);
        handle(this,MKUINT(0,SEL_INSERTED),(void*)what);
        handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
        FXFREE(&data);
        }
      }
    dragmode=MOUSE_NONE;
    return 1;
    }
  return 0;
  }


// Pressed right button
long FXText::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
  if(canFocus()) setFocus();
  flags&=~FLAG_UPDATE;
  flags|=FLAG_PRESSED;
  setFocus();
  grabx=ev->win_x-pos_x;
  graby=ev->win_y-pos_y;
  dragmode=MOUSE_SCROLL;
  return 1;
  }


// Released right button
long FXText::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(!isEnabled()) return 0;
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
  dragmode=MOUSE_NONE;
  return 1;
  }


// Autoscroll timer fired 
long FXText::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  FXScrollArea::onAutoScroll(sender,sel,ptr);
  switch(dragmode){
    case MOUSE_CHARS:
      if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_CHARS);
        if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
        }
      break;
    case MOUSE_WORDS:
      if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_WORDS);
        if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
        }
      break;
    case MOUSE_LINES:
      if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_LINES);
        if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
        }
      break;
    }
  return 1;
  }


// Handle real or simulated mouse motion
long FXText::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXDragAction action;
  FXint pos;
  if(flags&FLAG_PRESSED){
    switch(dragmode){
      case MOUSE_CHARS:
        if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
        if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
          pos=getPosAt(ev->win_x,ev->win_y);
          extendSelection(pos,SELECT_CHARS);
          if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
          }
        break;
      case MOUSE_WORDS:
        if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
        if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
          pos=getPosAt(ev->win_x,ev->win_y);
          extendSelection(pos,SELECT_WORDS);
          if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
          }
        break;
      case MOUSE_LINES:
        if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
        if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
          pos=getPosAt(ev->win_x,ev->win_y);
          extendSelection(pos,SELECT_LINES);
          if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
          }
        break;
      case MOUSE_SCROLL:
        setPosition(ev->win_x-grabx,ev->win_y-graby);
        break;
      case MOUSE_DRAG:
        if(startAutoScroll(ev->win_x,ev->win_y,TRUE)) return 1;
        action=DRAG_COPY;
        if(isEditable()){
          if(isDropTarget()) action=DRAG_MOVE;
          if(ev->state&CONTROLMASK) action=DRAG_COPY;
          if(ev->state&SHIFTMASK) action=DRAG_MOVE;
          }
        handleDrag(ev->root_x,ev->root_y,action);
        if(didAccept()!=DRAG_REJECT){
          if(action==DRAG_MOVE)
            setDragCursor(getApp()->dndMoveCursor);
          else
            setDragCursor(getApp()->dndCopyCursor);
          }
        else{
          setDragCursor(getApp()->dontdropCursor);
          }
        break;
      case MOUSE_PASTE:
        if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
        if((fxabs(ev->win_x-ev->click_x)>getApp()->dragDelta)||(fxabs(ev->win_y-ev->click_y)>getApp()->dragDelta)){
          pos=getPosAt(ev->win_x,ev->win_y);
          if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
          }
        break;
      }
    return 1;
    }
  return 0;
  }


// Cursor location changed
long FXText::onChanged(FXObject*,FXSelector,void* ptr){
  makePositionVisible((FXint)ptr);
  setCursorPos((FXint)ptr);
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  return 1;
  }


// Selected text
long FXText::onSelected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
  }


// Deselected text
long FXText::onDeselected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
  }


// Inserted text
long FXText::onInserted(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_INSERTED),ptr);
  }


// Deleted text
long FXText::onDeleted(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DELETED),ptr);
  }


// We now really do have the selection
long FXText::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
  FXbool hadselection=hasSelection();
//fprintf(stderr,"%s::onSelectionGained %x\n",getClassName(),this);
  FXScrollArea::onSelectionGained(sender,sel,ptr);
  //if(!hadselection) updateRange(selstartpos,selendpos);
  return 1;
  }


// We lost the selection somehow 
long FXText::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
  FXbool hadselection=hasSelection();
//fprintf(stderr,"%s::onSelectionLost %x\n",getClassName(),this);
  FXScrollArea::onSelectionLost(sender,sel,ptr);
  if(hadselection) updateRange(selstartpos,selendpos);
  return 1;
  }


// Handle drag-and-drop enter
long FXText::onDNDEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onDNDEnter(sender,sel,ptr);
//fprintf(stderr,"%s::onDNDEnter win(%d)\n",getClassName(),xid);
  return 1;
  }


// Handle drag-and-drop leave
long FXText::onDNDLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onDNDLeave(sender,sel,ptr);
//fprintf(stderr,"%s::onDNDLeave win(%d)\n",getClassName(),xid);
  return 1;
  }


// Handle drag-and-drop motion
long FXText::onDNDMotion(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXDragAction action;
  FXint pos;
//fprintf(stderr,"%s::onDNDMotion\n",getClassName());
  
  // Handled elsewhere
  if(FXScrollArea::onDNDMotion(sender,sel,ptr)) return 1;
  
  // Correct drop type
  if(offeredDNDType(textDragType)){
    
    // Is target editable?
    if(isEditable()){
      action=inquireDNDAction();
 
      // Check for legal DND action
      if(action==DRAG_COPY || action==DRAG_MOVE){
 
        // Get the suggested drop position
        pos=getPosAt(ev->win_x,ev->win_y);
 
        // Move cursor to new position
        if(cursorpos!=pos) handle(this,MKUINT(0,SEL_CHANGED),(void*)pos);
 
        // We don't accept a drop on the selection
        if(!isPosSelected(pos)){
          acceptDrop(DRAG_ACCEPT);
          }
        }
      }
    return 1;
    }
  
  // Didn't handle it here
  return 0;
  }


// Handle drag-and-drop drop
long FXText::onDNDDrop(FXObject* sender,FXSelector sel,void* ptr){
  FXuchar *data; FXuint len;
  FXuchar *junk; FXuint dum;
  FXint what[2];
//fprintf(stderr,"%s::onDNDDrop win(%d)\n",getClassName(),xid);

  // Try handling it in base class first
  if(FXScrollArea::onDNDDrop(sender,sel,ptr)) return 1;
  
  // Should really not have gotten this if non-editable
  if(isEditable()){
    
    // Try handle here
    if(getDNDData(textDragType,data,len)){
 
      // Need to ask the source to delete his copy
      if(inquireDNDAction()==DRAG_MOVE){
        getDNDData(textDeleteType,junk,dum);
        if(junk){fxerror("%s::onDNDDrop: didn't expect returned data.\n",getClassName());}
        }

      what[0]=cursorpos;
      what[1]=len;
      insert(cursorpos,(FXchar*)data,len);
      handle(this,MKUINT(0,SEL_INSERTED),(void*)what);
      handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
      FXFREE(&data);
      }
    return 1;
    }
  return 0;
  }


// Somebody wants our selection
long FXText::onSelectionRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXuchar *data; FXint len;
  FXint what[2];
  
//fprintf(stderr,"%s::onSelectionRequest\n",getClassName());

  // Try handling it in base class first
  if(FXScrollArea::onSelectionRequest(sender,sel,ptr)) return 1;
  
  // Requestor wants the text of the selection
  if(getDNDType()==XA_STRING || getDNDType()==textDragType){
    FXASSERT(selstartpos<=selendpos);
    len=selendpos-selstartpos;
    FXCALLOC(&data,FXuchar,len+1);
    extract((FXchar*)data,selstartpos,len);
    setDNDData(XA_STRING,data,len);
    return 1;
    }

  // Zap the original
  if(getDNDType()==textDeleteType){
    if(isEditable()){
      what[0]=selstartpos;
      what[1]=selendpos-selstartpos;
      handle(this,MKUINT(0,SEL_DELETED),(void*)what);
      remove(what[0],what[1]);
      handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
      }
    return 1;
    }
  return 0;
  }


/*******************************************************************************/


// Keyboard press
long FXText::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXDragAction action;
  char entry;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  switch(event->code){
    case KEY_Shift_L:
    case KEY_Shift_R:
      if(dragmode==MOUSE_DRAG){
        action=DRAG_COPY;
        if(isEditable()) action=DRAG_MOVE;
        if(didAccept()!=DRAG_REJECT){
          if(action==DRAG_MOVE)
            setDragCursor(getApp()->dndMoveCursor);
          else
            setDragCursor(getApp()->dndCopyCursor);
          }
        else{
          setDragCursor(getApp()->dontdropCursor);
          }
        }
      break;
    case KEY_Control_L:
    case KEY_Control_R:
      if(dragmode==MOUSE_DRAG){
        action=DRAG_COPY;
        if(isEditable() && (event->state&SHIFTMASK)) action=DRAG_MOVE;
        if(didAccept()!=DRAG_REJECT){
          setDragCursor(getApp()->dndCopyCursor);
          }
        else{
          setDragCursor(getApp()->dontdropCursor);
          }
        }
      break;
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
      break;
    case KEY_Up:
    case KEY_KP_Up:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_CURSOR_UP,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Down:
    case KEY_KP_Down:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_CURSOR_DOWN,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Left:
    case KEY_KP_Left:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_WORD_LEFT,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_LEFT,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Right:
    case KEY_KP_Right:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_WORD_RIGHT,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_RIGHT,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Home:
    case KEY_KP_Home:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_TOP,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_HOME,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_End:
    case KEY_KP_End:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_BOTTOM,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_END,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Page_Up:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_CURSOR_PAGEUP,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Page_Down:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_CURSOR_PAGEDOWN,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      break;
    case KEY_Delete:
    case KEY_KP_Delete:
      if(isEditable()){
        if(hasSelection()){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else if(event->state&CONTROLMASK){
          handle(this,MKUINT(ID_DELETE_WORD,SEL_COMMAND),NULL);
          }
        else if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_DELETE_EOL,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_DELETE,SEL_COMMAND),NULL);
          }
        }
      else{
        getApp()->beep();
        }
      break;
    case KEY_BackSpace:
      if(isEditable()){
        if(hasSelection()){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else if(event->state&CONTROLMASK){
          handle(this,MKUINT(ID_BACKSPACE_WORD,SEL_COMMAND),NULL);
          }
        else if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_BACKSPACE_BOL,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_BACKSPACE,SEL_COMMAND),NULL);
          }
        }
      else{
        getApp()->beep();
        }
      break;
    case KEY_Return:
    case KEY_KP_Enter:
    case KEY_ISO_Enter:
      if(isEditable()){
        if(hasSelection()){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_INSERT_NEWLINE,SEL_COMMAND),NULL);
        }
      else{
        getApp()->beep();
        }
      break;
    default:
      entry=fxkeyval(event->code,event->state);
//fprintf(stderr,"code=%04x state=%04x entry=%c(%02x)\n",event->code,event->state,' '<=entry?entry:' ',entry);
      switch(entry){
        case 'A'-'@':
          handle(this,MKUINT(ID_SELECT_ALL,SEL_COMMAND),NULL);
          break;
        case 'X'-'@':
          if(isEditable()){
            handle(this,MKUINT(ID_CUT_SEL,SEL_COMMAND),NULL);
            }
          else{
            getApp()->beep();
            }
          break;
        case 'C'-'@':
          handle(this,MKUINT(ID_COPY_SEL,SEL_COMMAND),NULL);
          break;
        case 'V'-'@':
          if(isEditable()){
            handle(this,MKUINT(ID_PASTE_SEL,SEL_COMMAND),NULL);
            }
          else{
            getApp()->beep();
            }
          break;
        case 'K'-'@':
          if(isEditable()){
            handle(this,MKUINT(ID_DELETE_LINE,SEL_COMMAND),NULL);
            }
          else{
            getApp()->beep();
            }
          break;
        case 'H'-'@':
          handle(this,MKUINT(ID_CURSOR_SCRNTOP,SEL_COMMAND),NULL);
          break;
        case 'M'-'@':
          handle(this,MKUINT(ID_CURSOR_SCRNCTR,SEL_COMMAND),NULL);
          break;
        case 'L'-'@':
          handle(this,MKUINT(ID_CURSOR_SCRNBTM,SEL_COMMAND),NULL);
          break;
        default:
          if(isEditable()){
            if(hasSelection()){
              handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
              }
            if((' '<=entry && entry<='~') || entry=='\t'){
              handle(this,MKUINT(ID_INSERT_CHAR,SEL_COMMAND),(void*)entry);
              }
            }
          else{
            getApp()->beep();
            }
          break;
        }
      break;
    }
  return 1;
  }


// Keyboard release
long FXText::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXDragAction action;
  if(!isEnabled()) return 0;
  switch(event->code){
    case KEY_Shift_L:
    case KEY_Shift_R:
      if(dragmode==MOUSE_DRAG){
        action=DRAG_COPY;
        if(isEditable() && !(event->state&CONTROLMASK) && isDropTarget()) action=DRAG_MOVE;
        if(didAccept()!=DRAG_REJECT){
          if(action==DRAG_MOVE)
            setDragCursor(getApp()->dndMoveCursor);
          else
            setDragCursor(getApp()->dndCopyCursor);
          }
        else{
          setDragCursor(getApp()->dontdropCursor);
          }
        }
      break;
    case KEY_Control_L:
    case KEY_Control_R:
      if(dragmode==MOUSE_DRAG){
        action=DRAG_COPY;
        if(isEditable() && (isDropTarget() || (event->state&SHIFTMASK))) action=DRAG_MOVE;
        if(didAccept()!=DRAG_REJECT){
          if(action==DRAG_MOVE)
            setDragCursor(getApp()->dndMoveCursor);
          else
            setDragCursor(getApp()->dndCopyCursor);
          }
        else{
          setDragCursor(getApp()->dontdropCursor);
          }
        }
      break;
    }
  return 1;
  }


/*******************************************************************************/

// Move cursor to top of buffer
long FXText::onCmdCursorTop(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)0);
  }


// Move cursor to bottom of buffer
long FXText::onCmdCursorBottom(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)(length-1));
  }


// Move cursor to begin of line
long FXText::onCmdCursorHome(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)lineStart(cursorpos));
  }


// Move cursor to end of line
long FXText::onCmdCursorEnd(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)lineEnd(cursorpos));
  }


// Move cursor right
long FXText::onCmdCursorRight(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)(cursorpos+1));
  }


// Move cursor left
long FXText::onCmdCursorLeft(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)(cursorpos-1));
  }


// Move cursor to previous line
long FXText::onCmdCursorUp(FXObject*,FXSelector,void*){
  FXint newline,newpos,end;
  if(cursorpos<=0) return 1;
  newline=backwardNLines(cursorline);
  newpos=newline+cursorcol;
  end=lineEnd(newline);
  if(newpos>end) newpos=end;/// Need take into account preferred pos
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)newpos);
  }


// Move cursor to next line
long FXText::onCmdCursorDown(FXObject*,FXSelector,void*){
  FXint newline,newpos,end;
  if(cursorpos>=length) return 1;
  newline=forwardNLines(cursorline);
  newpos=newline+cursorcol;
  end=lineEnd(newline);
  if(newpos>end) newpos=end;/// Need take into account preferred pos
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)newpos);
  }


// Word Left
long FXText::onCmdCursorWordLeft(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)leftWord(cursorpos));
  }


// Word Right
long FXText::onCmdCursorWordRight(FXObject*,FXSelector,void*){
  return handle(this,MKUINT(0,SEL_CHANGED),(void*)rightWord(cursorpos));
  }


// Page down
long FXText::onCmdCursorPageDown(FXObject*,FXSelector,void*){
  setTopLine(forwardNLines(lines[0],viewport_h/font->getFontHeight()));
  return 1;
  }


// Page up
long FXText::onCmdCursorPageUp(FXObject*,FXSelector,void*){
  setTopLine(backwardNLines(lines[0],viewport_h/font->getFontHeight()));
  return 1;
  }


// Cursor pos to top of screen
long FXText::onCmdCursorScreenTop(FXObject*,FXSelector,void*){
  setTopLine(cursorpos);
  return 1;
  }


// Cursor pos to bottom of screen
long FXText::onCmdCursorScreenBottom(FXObject*,FXSelector,void*){
  setBottomLine(cursorpos);
  return 1;
  }


// Cursor pos to center of screen
long FXText::onCmdCursorScreenCenter(FXObject*,FXSelector,void*){
  setCenterLine(cursorpos);
  return 1;
  }


// Mark
long FXText::onCmdMark(FXObject*,FXSelector,void*){
  setAnchorPos(cursorpos);
  return 1;
  }


// Extend
long FXText::onCmdExtend(FXObject*,FXSelector,void*){
  extendSelection(cursorpos);
  return 1;
  }


// Insert a character
long FXText::onCmdInsertChar(FXObject*,FXSelector sel,void* ptr){
  FXint what[2];
  FXchar ch=(FXchar)ptr;
  what[0]=cursorpos;
  what[1]=1;
  insert(what[0],&ch,1);
  handle(this,MKUINT(0,SEL_INSERTED),(void*)what);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Insert a string
long FXText::onCmdInsertString(FXObject*,FXSelector sel,void* ptr){
  FXint what[2];
  FXchar* string=(FXchar*)ptr;
  if(string){
    what[0]=cursorpos;
    what[1]=strlen(string);
    insert(what[0],string,what[1]);
    handle(this,MKUINT(0,SEL_INSERTED),(void*)what);
    handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
    flags|=FLAG_CHANGED;
    modified=TRUE;
    }
  return 1;
  }


// Insert a character
long FXText::onCmdInsertNewline(FXObject*,FXSelector sel,void*){
  FXint what[2];
  what[0]=cursorpos;
  what[1]=1;
  insert(what[0],"\n",1);
  handle(this,MKUINT(0,SEL_INSERTED),(void*)what);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Cut
long FXText::onCmdCutSel(FXObject*,FXSelector,void*){
  fxwarning("%x::onCmdCutSel: unimplemented\n",getClassName());
  modified=TRUE;
  return 1;
  }


// Copy
long FXText::onCmdCopySel(FXObject*,FXSelector,void*){
  fxwarning("%x::onCmdCopySel: unimplemented\n",getClassName());
  return 1;
  }


// Paste
long FXText::onCmdPasteSel(FXObject*,FXSelector,void*){
  fxwarning("%x::onCmdPasteSel: unimplemented\n",getClassName());
  modified=TRUE;
  return 1;
  }


// Delete selection
long FXText::onCmdDeleteSel(FXObject*,FXSelector,void*){
  FXint what[2];
  if(!hasSelection()) return 1;
  what[0]=selstartpos;
  what[1]=selendpos-selstartpos;
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  killSelection();
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Select character
long FXText::onCmdSelectChar(FXObject*,FXSelector,void*){
  FXint what[2];
  setAnchorPos(cursorpos);
  extendSelection(cursorpos+1,SELECT_CHARS);
  what[0]=selstartpos;
  what[1]=1;
  handle(this,MKUINT(0,SEL_SELECTED),(void*)what);
  return 1;
  }


// Select Word
long FXText::onCmdSelectWord(FXObject*,FXSelector,void*){
  FXint what[2];
  setAnchorPos(cursorpos);
  extendSelection(cursorpos,SELECT_WORDS);
  what[0]=selstartpos;
  what[1]=selendpos-what[0];
  handle(this,MKUINT(0,SEL_SELECTED),(void*)what);
  return 1;
  }


// Select Line
long FXText::onCmdSelectLine(FXObject*,FXSelector,void*){
  FXint what[2];
  setAnchorPos(cursorpos);
  extendSelection(cursorpos,SELECT_LINES);
  what[0]=selstartpos;
  what[1]=selendpos-what[0];
  handle(this,MKUINT(0,SEL_SELECTED),(void*)what);
  return 1;
  }


// Select All
long FXText::onCmdSelectAll(FXObject*,FXSelector,void*){
  FXint what[2];
  setAnchorPos(0);
  extendSelection(length-1,SELECT_CHARS);
  what[0]=selstartpos;
  what[1]=selendpos-what[0];
  handle(this,MKUINT(0,SEL_SELECTED),(void*)what);
  return 1;
  }


// Deselect All
long FXText::onCmdDeselectAll(FXObject*,FXSelector,void*){
  FXint what[2];
  what[0]=selstartpos;
  what[1]=selendpos-what[0];
  handle(this,MKUINT(0,SEL_DESELECTED),(void*)what);
  killSelection();
  return 1;
  }


// Backspace character
long FXText::onCmdBackspace(FXObject*,FXSelector,void*){
  FXint what[2];
  if(cursorpos<1){ getApp()->beep(); return 1; }
  what[0]=cursorpos-1;
  what[1]=1;
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Backspace word
long FXText::onCmdBackspaceWord(FXObject*,FXSelector,void*){
  FXint what[2];
  what[0]=leftWord(cursorpos);
  what[1]=cursorpos-what[0];
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Backspace bol
long FXText::onCmdBackspaceBol(FXObject*,FXSelector,void*){
  FXint what[2];
  what[0]=lineStart(cursorpos);
  what[1]=cursorpos-what[0];
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete character
long FXText::onCmdDelete(FXObject*,FXSelector,void*){
  FXint what[2];
  if(cursorpos==length-1){ getApp()->beep(); return 1; }
  what[0]=cursorpos;
  what[1]=1;
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete word
long FXText::onCmdDeleteWord(FXObject*,FXSelector,void*){
  FXint what[2];
  what[0]=cursorpos;
  what[1]=rightWord(cursorpos)-what[0];
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete end of line
long FXText::onCmdDeleteEol(FXObject*,FXSelector,void*){
  FXint what[2];
  what[0]=cursorpos;
  what[1]=lineEnd(cursorpos)-what[0];
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete line
long FXText::onCmdDeleteLine(FXObject*,FXSelector,void*){
  FXint what[2];
  what[0]=lineStart(cursorpos);
  what[1]=forwardNLines(cursorpos,1)-what[0];
  handle(this,MKUINT(0,SEL_DELETED),(void*)what);
  remove(what[0],what[1]);
  handle(this,MKUINT(0,SEL_CHANGED),(void*)cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }

/*******************************************************************************/

// Editable toggle
long FXText::onCmdToggleEditable(FXObject*,FXSelector,void*){
  options^=TEXT_READONLY;
  return 1;
  }

// Update editable toggle
long FXText::onUpdToggleEditable(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&TEXT_READONLY) ? ID_UNCHECK : ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Move cursor to indicated line
long FXText::onCmdCursorLine(FXObject* sender,FXSelector,void* ptr){
  FXint line,delta,newline,newpos,end;
  if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&line)){
    if(line<1) line=1;
    if(line>nlines) line=nlines;
    newline=cursorline;
    if(line>cursorlineno){
      delta=line-cursorlineno;
      newline=forwardNLines(cursorline,delta);
      }
    else if(line<cursorlineno){
      delta=cursorlineno-line;
      newline=backwardNLines(cursorline,delta);
      }
    if(newline!=cursorline){
      newpos=newline+cursorcol;
      end=lineEnd(newline);
      if(newpos>end) newpos=end;/// Need take into account preferred pos
      handle(this,MKUINT(0,SEL_CHANGED),(void*)newpos);
      }
    }
  return 1;
  }


// Being asked about current line number
long FXText::onUpdCursorLine(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&cursorlineno);
  return 1;
  }


// Move cursor to indicated column
long FXText::onCmdCursorColumn(FXObject* sender,FXSelector,void* ptr){
  FXint col,pos,end;
  if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&col)){
    pos=cursorline+col-1;
    end=lineEnd(cursorpos);
    if(pos<cursorline) pos=cursorline;
    if(pos>end) pos=end;
    setCursorPos(pos);
    }
  return 1;
  }


// Being asked about current column
long FXText::onUpdCursorColumn(FXObject* sender,FXSelector,void*){
  FXint col=cursorcol+1;
  sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&col);
  return 1;
  }


// Update somebody who works on the selection
long FXText::onUpdHaveSelection(FXObject* sender,FXSelector sel,void* ptr){
  FXuint msg=hasSelection() ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

/*******************************************************************************/

// Character width
FXint FXText::charWidth(FXchar ch,FXint indent) const {
  register FXint tw;
  FXASSERT(indent>=0);
  if(ch=='\t'){ tw=tabdist; if(!(options&TEXT_TABPIXELS)) tw*=font->getTextWidth(" ",1); return (tw-indent%tw); }
  if(ch<' '){ ch|=0x40; return font->getTextWidth("^",1)+font->getTextWidth(&ch,1); }
  return font->getTextWidth(&ch,1);
  }


// Calculate line width
FXint FXText::lineWidth(FXint pos,FXint n) const {
  FXint w=0;
  while(pos<gapstart && 0<n){w+=charWidth(buffer[pos],w);pos++;n--;}
  pos+=gapend-gapstart;
  while(0<n){w+=charWidth(buffer[pos],w);pos++;n--;}
  return w;
  }


// Calculate line height
FXint FXText::lineHeight(FXint pos,FXint n) const {
  return font->getFontHeight();
  }


// Obtain text style at position pos
FXuint FXText::getStyleAt(FXint linepos,FXint linelen,FXint column) const {
  register FXuint style=0;
  register FXint pos;
  register FXchar ch;
  
  // Line pos is legal?
  FXASSERT(0<=linepos && linepos<=length);
  
  // Empty line can not be selected; empty lines happen only at end of buffer
  if(linelen<=0) return STYLE_FILL;
  
  // Get legal position
  pos=linepos+FXMIN(column,linelen-1);
  
  // Position is legal
  FXASSERT(0<=pos && pos<=length);
  
  // Blank part of line
  if(column>=linelen-1) style|=STYLE_FILL;
  
  // Selected part of text
  if(hasSelection() && selstartpos<=pos && pos<selendpos) style|=STYLE_SELECTED;
  
  // Special style for control characters
  ch=buffer[pos<gapstart ? pos : pos-gapstart+gapend];
  
  // Get special style
  if(ch=='\t') style|=STYLE_FILL; else if(ch<' ') style|=STYLE_CONTROL; 

  return style;
  }


// Draw chunk from buffer
void FXText::drawBufString(FXint x,FXint y,FXint w,FXint h,FXint pos,FXint n,FXuint style){
  FXchar ch;
  
  // Filled space 
  if(style&STYLE_FILL){
    if(style&STYLE_SELECTED){
      setForeground(selbackColor);
      }
    else{
      setForeground(backColor);
      }
    fillRectangle(x,y,w,h);
    }
  
  // String of control characters
  else if(style&STYLE_CONTROL){
    FXASSERT(0<=pos && pos+n<length);
    y+=font->getFontAscent();
    if(style&STYLE_SELECTED){
      setForeground(seltextColor);
      setBackground(selbackColor);
      }
    else{
      setForeground(textColor);
      setBackground(backColor);
      }
    while(pos<gapstart && 0<n){
      ch=buffer[pos]|0x40;
      drawImageText(x,y,"^",1);
      x+=font->getTextWidth("^",1);
      drawImageText(x,y,&ch,1);
      x+=font->getTextWidth(&ch,1);
      pos++;
      n--;
      }
    while(0<n){
      ch=buffer[pos-gapstart+gapend]|0x40;
      drawImageText(x,y,"^",1);
      x+=font->getTextWidth("^",1);
      drawImageText(x,y,&ch,1);
      x+=font->getTextWidth(&ch,1);
      pos++;
      n--;
      }
    }
  
  // Regular text
  else{
    FXASSERT(0<=pos && pos+n<length);
    y+=font->getFontAscent();
    if(style&STYLE_SELECTED){
      setForeground(seltextColor);
      setBackground(selbackColor);
      }
    else{
      setForeground(textColor);
      setBackground(backColor);
      }
    if(pos+n<=gapstart){
      drawImageText(x,y,&buffer[pos],n);
      }
    else if(pos>=gapstart){
      drawImageText(x,y,&buffer[pos-gapstart+gapend],n);
      }
    else{
      drawImageText(x,y,&buffer[pos],gapstart-pos);
      x+=font->getTextWidth(&buffer[pos],gapstart-pos);
      drawImageText(x,y,&buffer[gapend],pos+n-gapstart);
      }
    if(style&STYLE_UNDERLINE) drawLine(x,y+1,x+w,y+1);
    }
  }


// Draw partial text line with correct style
void FXText::drawTextLine(FXint line,FXint lclip,FXint rclip,FXint fc,FXint tc){
  register FXint x,y,w,h,pos,len,sc,ec,cw;
  register FXuint style,newstyle;
  register FXchar c;
  FXASSERT(0<=fc);
  FXASSERT(lclip<rclip);
  if(line<0 || line>=nvislines) return;
//fprintf(stderr,"drawTextLine line=%d lclip=%d rclip=%d fc=%d tc=%d pos_x=%d pos_y=%d \n",line,lclip,rclip,fc,tc,pos_x,pos_y);
  pos=lines[line];
  len=lines[line+1]-pos;              // Includes newline or nul
  w=0;
  h=font->getFontHeight();
  x=0;
  y=topy+line*h;
  for(sc=0; ; sc++){
    if(sc>=len-1){
      cw=font->getTextWidth(" ",1);   // For selections outside of text 
      }
    else{
      c=getChar(pos+sc);
      cw=charWidth(c,x);
      }
    if(sc>=fc && x+pos_x+marginleft+cw>=lclip) break;
    x+=cw;
    }
  FXASSERT(fc<=sc);
  style=newstyle=getStyleAt(pos,len,sc);
  for(ec=sc; ec<=tc; ec++){
    if(ec<len) newstyle=getStyleAt(pos,len,ec);
    if(x+pos_x+marginleft+w>=rclip){
      drawBufString(pos_x+marginleft+x,pos_y+margintop+y,w,h,pos+sc,ec-sc,style);
      break;
      }
    if(newstyle!=style){
      drawBufString(pos_x+marginleft+x,pos_y+margintop+y,w,h,pos+sc,ec-sc,style);
      style=newstyle;
      x+=w;
      w=0;
      sc=ec;
      }
    if(ec>=len-1){
      cw=font->getTextWidth(" ",1);   // For selections outside of text 
      }
    else{
      c=getChar(pos+ec);
      cw=charWidth(c,x+w);
      }
    w+=cw;
    }
  }


// Draw the cursor
void FXText::drawCursor(FXuint state){
  register FXint xx,yt,yb,cl;
  if((state^flags)&FLAG_CARET){
    xx=pos_x+marginleft+cursorx;
    yt=pos_y+margintop+cursory;
    yb=yt+font->getFontHeight()-1;
    setClipRectangle(marginleft,margintop,viewport_w-marginleft-marginright,viewport_h-margintop-marginbottom);
    if(flags&FLAG_CARET){
      if(lines[0]<=cursorpos && cursorpos<=lines[nvislines]){
        
        // Cursor may be in the selection
        if(hasSelection() && selstartpos<=cursorpos && cursorpos<selendpos){
          setForeground(selbackColor);
          }
        else{
          setForeground(backColor);
          }
        
        // Repaint cursor in background to erase it
        drawLine(xx,yt,xx,yb);
        drawLine(xx+1,yt,xx+1,yb);
        drawLine(xx-2,yt,xx+3,yt);
        drawLine(xx-2,yb,xx+3,yb);
        
        // Restore text
        setTextFont(font);
        cl=posToLine(cursorpos);
        drawTextLine(cl,xx-3,xx+3,0,10000);
        }
      flags&=~FLAG_CARET;
      }
    else{
      if(lines[0]<=cursorpos && cursorpos<=lines[nvislines]){
        setForeground(cursorColor);
        drawLine(xx,yt,xx,yb);
        drawLine(xx+1,yt,xx+1,yb);
        drawLine(xx-2,yt,xx+3,yt);
        drawLine(xx-2,yb,xx+3,yb);
        flags|=FLAG_CARET;
        }
      }
    clearClipRectangle();
    }
  }


// Repaint rectangle of text
void FXText::drawTextRectangle(FXint x,FXint y,FXint w,FXint h){
  register FXint ff,ll,yy,xx,ln;
  xx=x-pos_x-marginleft;
  yy=pos_y+margintop+topy;
  ff=(y-yy)/font->getFontHeight();
  ll=(y+h-yy)/font->getFontHeight();
//fprintf(stderr,"ff=%d ll=%d\n",ff,ll);
  if(ff<0) ff=0;
  if(ll>nvislines) ll=nvislines;
  if(ff>ll) return; 
//fprintf(stderr,"repainting lines %d .. %d\n",ff,ll);
  for(ln=ff; ln<=ll; ln++){
    drawTextLine(ln,x,x+w,0,10000);
    }
  }


// Repaint text range
void FXText::updateRange(FXint beg,FXint end){
  register FXint tl,bl,fc,lc,ty,by,lx,rx,t;
  if(beg>end){t=beg;beg=end;end=t;}
  if(end<=lines[0] || lines[nvislines]<=beg) return;
//fprintf(stderr,"lines[0]=%d lines[%d]=%d beg=%d end=%d\n",lines[0],nvislines,lines[nvislines],beg,end);
  if(beg<=lines[0]){
    tl=0;
    fc=0;
    }
  else{
    tl=posToLine(beg);
    fc=beg-lines[tl];
    }
  if(lines[nvislines-1]<=end){
    bl=nvislines-1;
    lc=lines[nvislines]-lines[bl];
    }
  else{
    bl=posToLine(end);
    lc=end-lines[bl];
    }
  if(tl==bl){
    ty=pos_y+margintop+topy+tl*font->getFontHeight();
    by=ty+font->getFontHeight();
    lx=pos_x+marginleft+lineWidth(lines[tl],fc);
    if(end<(lines[tl+1]-1)) rx=pos_x+marginleft+lineWidth(lines[tl],lc); else rx=width;
    }
  else{
    ty=pos_y+margintop+topy+tl*font->getFontHeight();
    by=pos_y+margintop+topy+(bl+1)*font->getFontHeight();
    lx=0;
    rx=width;
    }
  update(lx,ty,rx-lx,by-ty);
  }


// // Draw range of text
// void FXText::drawTextRange(FXint beg,FXint end){
//   FXint fl,ll,fi,li,ln,t;
//   if(beg>end){t=beg;beg=end;end=t;}
//   if(end<=lines[0] || lines[nvislines]<=beg) return;
//   drawCursor(0);
//   if(beg<=lines[0]){
//     fl=0;
//     fi=0;
//     }
//   else{
//     fl=posToLine(beg);
//     fi=beg-lines[fl];
//     }
//   if(lines[nvislines-1]<=end){
//     ll=nvislines-1;
//     li=lines[nvislines]-lines[ll];
//     }
//   else{
//     ll=posToLine(end);
//     li=end-lines[ll];
//     }
//   
// fprintf(stderr,"fl=%d fi=%d beg=%d ll=%d li=%d end=%d\n",fl,fi,beg,ll,li,end);
//   if(fl==ll){
//     drawTextLine(fl,0,viewport_w,fi,li);
//     }
//   else{
//     drawTextLine(fl,0,viewport_w,fi,10000);
//     for(ln=fl+1; ln<ll; ln++) drawTextLine(ln,0,viewport_w,0,10000);
//     drawTextLine(ll,0,viewport_w,0,li);
//     }
//   }


// Draw item list
long FXText::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXASSERT(xid);
  FXASSERT(buffer);
  FXASSERT(gapstart<=length);
  FXASSERT(gapstart<gapend);
  FXASSERT(getChar(length-1)=='\0');
  setClipRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  setTextFont(font);
  drawTextRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  clearArea(0,0,viewport_w,margintop);
  clearArea(0,margintop,marginleft,viewport_h-margintop-marginbottom);
  clearArea(viewport_w-marginright,margintop,marginright,viewport_h-margintop-marginbottom);
  clearArea(0,viewport_h-marginbottom,viewport_w,marginbottom);
  clearClipRectangle();
  return 1;
  }


// Find line number from visible pos
FXint FXText::posToLine(FXint pos) const {
  register FXint ln;
  FXASSERT(lines[0]<=pos);
  FXASSERT(pos<=lines[nvislines]);
  for(ln=0; ln<nvislines-1 && lines[ln+1]<=pos && lines[ln]<lines[ln+1]; ln++);
  FXASSERT(0<=ln && ln<nvislines);
  FXASSERT(lines[ln]<=pos && pos<=lines[ln+1]);
  return ln;
  }


// See if pos is a visible position
FXbool FXText::posVisible(FXint pos) const {
  return lines[0]<=pos && pos<=lines[nvislines];
  }


// See if position is in the selection
FXbool FXText::isPosSelected(FXint pos) const {
  return hasSelection() && selstartpos<=pos && pos<selendpos;
  }


// Determine first visible line
void FXText::findTopLine(){
  register FXint ln,h;
  h=font->getFontHeight();
  while(pos_y+topy+margintop+h<=0){
    ln=forwardNLines(topline);
    if(ln>=length) break;
    topline=ln;
    toplineno++;
    topy+=h;
    }
  while(pos_y+topy+margintop>0){
    if(topline<=0) break;
    topline=backwardNLines(topline);
    toplineno--;
    topy-=h;
    }
  }


// Localize position at x,y
FXint FXText::getPosAt(FXint x,FXint y) const {
  register FXint ln,ls,le,cx,cw;
  register FXchar ch;
  x=x-pos_x-marginleft;
  y=y-pos_y-margintop;
  ln=(y-topy)/font->getFontHeight();
  if(ln<0) return lines[0];
  if(ln>=nvislines) return lines[nvislines];
  if(x<0) return lines[ln];
  ls=lines[ln];
  le=lines[ln+1];
  cx=0;
  while(ls<le){
    ch=buffer[ls<gapstart ? ls : ls-gapstart+gapend];
    if(ch=='\n' || ch=='\0') return ls;
    cw=charWidth(ch,cx);
    if(x<=(cx+(cw>>1))) return ls;
    cx+=cw;
    ls+=1;
    }
  return le;
  }


// Determine x,y from position pos
FXint FXText::getYOfPos(FXint pos) const {
  FXint ln,t,y,h;
  
  // Validate position
  if(pos<0) pos=0; else if(pos>=length) pos=length-1;
  
  h=font->getFontHeight();
  
  // Above visible part of buffer
  if(pos<lines[0]){
    ln=topline;
    y=topy;
    while(pos<ln){
      t=backwardNLines(ln);
      ln=t;
      y-=h;
      if(pos>=t) break;
      }
    }
  
  // Below visible part of buffer
  else if(pos>=lines[nvislines]){
    ln=topline;
    y=topy;
    while(ln<pos){
      t=forwardNLines(ln);
      if(pos<t) break;
      ln=t;
      y+=h;
      }
    }
  
  // In visible part of buffer
  else{
    FXASSERT(lines[0]<=pos);
    FXASSERT(pos<=lines[nvislines]);
    t=posToLine(pos);
    ln=lines[t];
    y=topy+h*t;
    }
  
  return y;
  }


// Calculate X position of pos
FXint FXText::getXOfPos(FXint pos) const {
  FXint base;
  if(pos<0) pos=0; else if(pos>=length) pos=length;
  base=lineStart(pos);
  return lineWidth(base,pos-base);
  }


// Get line number of position
FXint FXText::getLineOfPos(FXint pos) const {
  FXint ln,t,no;
  if(pos<0 || length<=pos){ fxerror("%s::getLineOfPos: argument out of range for buffer.\n",getClassName()); }
  
  // Above visible part of buffer
  if(pos<lines[0]){
    ln=topline;
    no=toplineno;
    while(pos<ln){
      t=backwardNLines(ln);
      ln=t;
      no--;
      if(pos>=t) break;
      }
    }
  
  // Below visible part of buffer
  else if(pos>=lines[nvislines]){
    ln=topline;
    no=toplineno;
    while(ln<pos){
      t=forwardNLines(ln);
      if(pos<t) break;
      ln=t;
      no++;
      }
    }
  
  // In visible part of buffer
  else{
    FXASSERT(lines[0]<=pos);
    FXASSERT(pos<=lines[nvislines]);
    no=toplineno+posToLine(pos);
    }
  
  return no;
  }


// Get column number of position
FXint FXText::getColumnOfPos(FXint pos) const {
  if(pos<0 || length<=pos){ fxerror("%s::getColumnOfPos: argument out of range for buffer.\n",getClassName()); }
  return pos-lineStart(pos);
  }


// Force position to become fully visible
void FXText::makePositionVisible(FXint pos){
  FXint x,y,nx,ny;
  
  // Get coordinates of position
  x=getXOfPos(pos);
  y=getYOfPos(pos);
//fprintf(stderr,"makePositionVisible pos=%d x=%d y=%d lineh=%d\n",pos,x,y,font->getFontHeight());
  
  // Check vertical visibility
  ny=pos_y;
  if(pos_y+margintop+y<0) 
    ny=-y;
  else if(pos_y+margintop+y+font->getFontHeight()>viewport_h) 
    ny=viewport_h-font->getFontHeight()-marginbottom-margintop-y;
  
  // Check Horizontal visibility
  nx=pos_x;
//   if(pos_x+marginleft+x<0)
//     nx=-x;
//   else if(pos_x+marginleft+x>viewport_w)
//     nx=viewport_w-marginright-marginleft-x;
  
  // If needed, scroll
  if(nx!=pos_x || ny!=pos_y) setPosition(nx,ny);
  }


// Make line containing pos the top visible line
void FXText::setTopLine(FXint pos){
  setPosition(pos_x,-getYOfPos(pos));
  }


// Make line containing pos the bottom visible line
void FXText::setBottomLine(FXint pos){
  setPosition(pos_x,viewport_h-font->getFontHeight()-marginbottom-margintop-getYOfPos(pos));
  }


// Center line of pos in the middle of the screen
void FXText::setCenterLine(FXint pos){
  setPosition(pos_x,viewport_h/2+font->getFontHeight()/2-getYOfPos(pos));
  }


// Get top line
FXint FXText::getTopLine() const { 
  return lines[0]; 
  }


// Get bottom line
FXint FXText::getBottomLine() const { 
  return lines[nvislines-1]; 
  }


/*******************************************************************************/


// Move the gap
void FXText::movegap(FXint pos){
  register FXint gaplen=gapend-gapstart;
  FXASSERT(0<=pos && pos<=length);
  FXASSERT(1<=gaplen);
  if(pos>gapstart){ 
    memmove(&buffer[gapstart],&buffer[gapend],pos-gapstart);
    gapend=pos+gaplen;
    gapstart=pos;
    }
  else if(pos<gapstart){
    memmove(&buffer[pos+gaplen],&buffer[pos],gapstart-pos);
    gapend=pos+gaplen;
    gapstart=pos;
    }
  }


// Size gap
void FXText::sizegap(FXint sz){
  register FXint gaplen=gapend-gapstart;
  if(sz>=gaplen){
    sz+=MINSIZE;
    if(!FXRESIZE(&buffer,FXchar,length+sz)){fxerror("%s::sizegap: out of memory.\n",getClassName());}
    memmove(&buffer[gapstart+sz],&buffer[gapend],length-gapstart);
    gapend=gapstart+sz;
    }
  }


// Recalculate line starts
void FXText::calcLines(FXint s,FXint e){
  register FXint l,pos,gaplen;
  FXASSERT(nvislines>0);
  gaplen=gapend-gapstart;
  if(s<0) s=0; else if(s>nvislines) s=nvislines;
  if(e<0) e=0; else if(e>nvislines) e=nvislines;
  if(s<=e){
    FXASSERT(0<=topline && topline<length);
    if(s==0){
      lines[0]=topline;
      s=1;
      }
    pos=lines[s-1];
    l=s;
    while(l<=e){
      while(pos<gapstart && pos<length){
        if(buffer[pos++]=='\n') goto xx;
        }
      while(pos<length){
        if(buffer[gaplen+pos++]=='\n') goto xx;
        }
xx:   lines[l++]=pos;
      }
    while(l<=e){
      lines[l++]=length;
      }
// fprintf(stderr,"length=%d\nline starts:\n",length);
// for(l=0; l<=nvislines; l++){fprintf(stderr,"line[%2d][%2d]=%d\n",toplineno+l,l,lines[l]);}
// fprintf(stderr,"\n");
    }
  }


// Update line starts after a buffer change
void FXText::updateChanged(FXint pos,FXint ncinserted,FXint ncdeleted,FXint nlinserted,FXint nldeleted){
  register FXint ncdelta=ncinserted-ncdeleted;
  register FXint nldelta=nlinserted-nldeleted;
  register FXint i,line,nbefore,x,y,h;
  
//fprintf(stderr,"BEFORE: pos=%d ncins=%d ncdel=%d nlins=%d nldel=%d topline=%d toplineno=%d topy=%d nlines=%d \n",pos,ncinserted,ncdeleted,nlinserted,nldeleted,topline,toplineno,topy,nlines);
// 
// fprintf(stderr,"line starts BEFORE:\n");
// for(i=0; i<=nvislines; i++){fprintf(stderr,"line[%2d][%2d]=%d \n",toplineno+i,i,lines[i]);}
// fprintf(stderr,"\n");
//   

  // Change was completely before visible part
  if(pos+ncdeleted<topline){
//fprintf(stderr,"change before topline\n");
    toplineno+=nldelta;
    topline+=ncdelta;
    topy+=nldelta*font->getFontHeight();
    for(i=0; i<=nvislines; i++) lines[i]+=ncdelta;
    pos_y-=font->getFontHeight()*nldelta;
    vertical->setPosition(-pos_y);
    /// Scan for widest line...
    textHeight+=nldelta*font->getFontHeight();
    FXScrollArea::layout();
    }
    
  // Tail of change overlaps visible part
  else if(pos<topline){
    
    // Last part of visible text unchanged
    if(pos+ncdeleted<=lines[nvislines-1]){
      FXASSERT(lines[0]<=pos+ncdeleted);
//fprintf(stderr,"last part unchanged\n");
      line=1+posToLine(pos+ncdeleted);

      // Lines before the first unaffected visible line left after change
      nbefore=toplineno+line+nldelta-1;
      
      // Try not to scroll if we can; we will repaint the top part
      if(nbefore>line){
//fprintf(stderr,"unscrolled\n");
        toplineno=toplineno+nldelta;
        FXASSERT(toplineno>=1);
        topline=backwardNLines(lines[line]+ncdelta,line);
        FXASSERT(topline>=0);
        topy=topy+font->getFontHeight()*nldelta;
        FXASSERT(topy>=0);
        pos_y-=font->getFontHeight()*nldelta;
        vertical->setPosition(-pos_y);
        /// Scan for widest line...
        textHeight+=nldelta*font->getFontHeight();
        FXScrollArea::layout();
        update(0,0,width,pos_y+margintop+topy+line*font->getFontHeight());
        }
      
      // Too many lines are gone, we have to scroll&repaint the whole thing
      else{
//fprintf(stderr,"scrolled\n");
        toplineno=1;
        topline=0;
        topy=0;
        vertical->setPosition(0);
        pos_y=0;
        /// Scan for widest line...
        textHeight+=nldelta*font->getFontHeight();
        FXScrollArea::layout();
        update(0,0,width,height);
        }
      }
    
    // All visible text changed
    else{

      // Can not get back to same top line number
      if(toplineno>nlines+nldelta){
//fprintf(stderr,"all visible changed reset top\n");
        toplineno=1;
        topline=0;
        topy=0;
        vertical->setPosition(0);
        pos_y=0;
        }
      
      // Get back to same top line number from start of buffer
      else{
//fprintf(stderr,"all visible changed move top\n");
        topline=forwardNLines(0,toplineno);
        }
      /// Scan for widest line...
      textHeight+=nldelta*font->getFontHeight();
      FXScrollArea::layout();
      update(0,0,width,height);
      }
    calcLines(0,nvislines);
    }
  
  // Head of change in visible part
  else if(pos<=lines[nvislines]){
    FXASSERT(topline<=pos);
    
    // Line where change started
    line=posToLine(pos);
    
    // More lines means paint the bottom half
    if(nldelta>0){
//fprintf(stderr,"head of change visible; more lines\n");
      for(i=nvislines; i>=line+nldelta; i--) lines[i]=lines[i-nldelta]+ncdelta;
      y=pos_y+margintop+topy+line*font->getFontHeight();
      textHeight+=nldelta*font->getFontHeight();
      FXScrollArea::layout();
      update(0,y,width,height-y);
      }
    
    // Less lines means paint bottom half also
    else if(nldelta<0){
//fprintf(stderr,"head of change visible; less lines\n");
      for(i=line+1; i<=nvislines+nldelta; i++) lines[i]=lines[i-nldelta]+ncdelta;
      y=pos_y+margintop+topy+line*font->getFontHeight();
      // We decrease the total text height, but keep it a little bit bigger
      // to prevent having to hop the text
      textHeight+=nldelta*font->getFontHeight();
      h=topy+font->getFontHeight()*(nvislines-2);
      if(textHeight<h) textHeight=h;
      FXScrollArea::layout();
      update(0,y,width,height-y);
      }
    
    // Same lines
    else{
//fprintf(stderr,"head of change visible; same lines\n");
      for(i=line+1; i<=nvislines; i++) lines[i]=lines[i]+ncdelta;
      if(nlinserted==0){
        x=pos_x+marginleft+lineWidth(lines[line],pos-lines[line]);
        y=pos_y+margintop+topy+line*font->getFontHeight();
        update(x,y,width-x,font->getFontHeight());
        }
      else{
        y=pos_y+margintop+topy+line*font->getFontHeight();
        update(0,y,width,nlinserted*font->getFontHeight());
        }
      }
    if(nlinserted>0) calcLines(line+1,line+nlinserted);
    if(nldelta<0) calcLines(nvislines+nldelta,nvislines);
    }
  
  // Change was completely after visible part
  else{
//fprintf(stderr,"past end & at tail\n");
    textHeight+=nldelta*font->getFontHeight();
    FXScrollArea::layout();
    }
  
  // Number of lines changed
  nlines+=nldelta;
  
//fprintf(stderr,"AFTER: pos=%d ncins=%d ncdel=%d nlins=%d nldel=%d topline=%d toplineno=%d topy=%d nlines=%d \n",pos,ncinserted,ncdeleted,nlinserted,nldeleted,topline,toplineno,topy,nlines);

// fprintf(stderr,"line starts AFTER:\n");
// for(i=0; i<=nvislines; i++){fprintf(stderr,"line[%2d][%2d]=%d \n",toplineno+i,i,lines[i]);}
// fprintf(stderr,"\n");
  
  // Fix selection 
  FXASSERT(selstartpos<=selendpos);
  if(pos+ncdeleted<=selstartpos){
    selstartpos+=ncdelta;
    selendpos+=ncdelta;
    }
  else if(pos<selendpos){
    selendpos=pos+ncinserted;
    if(pos<selstartpos) selstartpos=pos+ncinserted;
    }
  
  // Fix anchor position
  if(pos+ncdeleted<=anchorpos) anchorpos+=ncdelta;
  else if(pos<=anchorpos) anchorpos=pos+ncinserted;
  
  // Fix cursor position
//fprintf(stderr,"cursorpos was=%d\n",cursorpos);
  if(pos+ncdeleted<=cursorpos) cursorpos+=ncdelta;
  else if(pos<=cursorpos) cursorpos=pos+ncinserted;
  
  // Update position variables
  cursorlineno=getLineOfPos(cursorpos);
  cursorline=lineStart(cursorpos);
  cursorcol=cursorpos-cursorline;
  cursory=font->getFontHeight()*(cursorlineno-1);
  cursorx=lineWidth(cursorline,cursorcol)-1;
//fprintf(stderr,"cursorpos is=%d\n",cursorpos);
  
  // Verify some essential invariants
  FXASSERT(1<=toplineno);
  FXASSERT(0<=topline && topline<length);
  FXASSERT(0<=topy);
  }


// Insert some text
void FXText::insert(FXint pos,const FXchar *text,FXint n){
  register FXint nl=0,i;
  FXASSERT(text);
  FXASSERT(0<=pos && pos<length);
  FXASSERT(0<=n);
  drawCursor(0);
  for(i=0; i<n; i++) nl+=(text[i]=='\n');
  sizegap(n);
  movegap(pos);
  memcpy(&buffer[pos],text,n);
  gapstart+=n;
  length+=n;
  updateChanged(pos,n,0,nl,0);
  }
    

// Remove some text 
void FXText::remove(FXint pos,FXint n){
  register FXint nl=0,i,p;
  FXASSERT(0<=pos && pos+n<length);
  FXASSERT(0<=n);
  drawCursor(0);
  for(p=pos,i=0; i<n && p<gapstart; p++,i++) nl+=(buffer[p]=='\n');
  for(p+=gapend-gapstart; i<n; p++,i++) nl+=(buffer[p]=='\n');
  if(pos>gapstart) movegap(pos); else if(pos+n<gapstart) movegap(pos+n);
  gapend+=pos+n-gapstart;
  gapstart-=gapstart-pos;
  length-=n;
  updateChanged(pos,0,n,0,nl);
  }
  
  
// Grab range of text
void FXText::extract(FXchar *text,FXint pos,FXint n) const {
  FXASSERT(n>=0);
  FXASSERT(0<=pos && pos<length);
  if(pos+n<=gapstart){
    memcpy(text,&buffer[pos],n);
    }
  else if(pos>=gapstart){
    memcpy(text,&buffer[pos-gapstart+gapend],n);
    }
  else{
    memcpy(text,&buffer[pos],gapstart-pos);
    memcpy(&text[gapstart-pos],&buffer[gapend],pos+n-gapstart);
    }
  text[n]='\0';
  }


// Find the position of the first character of the line containing pos,
// or begin of the buffer.
FXint FXText::lineStart(FXint pos) const {
  register FXint gaplen=gapend-gapstart;
  FXASSERT(0<=gapstart && gapstart<=length);
  if(pos>=length) pos=length-1;
  pos--;
  while(pos>=gapstart){
    if(buffer[pos+gaplen]=='\n') return pos+1;
    pos--;
    }
  while(pos>=0){
    if(buffer[pos]=='\n') return pos+1;
    pos--;
    }
  return 0;
  }


// Find the position of the '\n' of the line containing pos,
// or the '\0' at the end of the buffer.
FXint FXText::lineEnd(FXint pos) const {
  register FXint gaplen=gapend-gapstart;
  register FXchar c;
  FXASSERT(0<=gapstart && gapstart<=length);
  if(pos<0) pos=0;
  while(pos<gapstart){
    if((c=buffer[pos])=='\n' || c=='\0') return pos;
    pos++;
    }
  while(pos<length){
    if((c=buffer[pos+gaplen])=='\n' || c=='\0') return pos;
    pos++;
    }
  return length;
  }


// Get character
FXchar FXText::getChar(FXint pos) const {
  FXASSERT(0<=pos && pos<length);
  return buffer[pos<gapstart ? pos : pos-gapstart+gapend];
  }


// Set character
void FXText::setChar(FXint pos,FXchar ch){
  FXASSERT(0<=pos && pos<length);
  buffer[pos<gapstart ? pos : pos-gapstart+gapend]=ch;
  }


// Search forward [pos...len> for character c, -1 if not found
FXint FXText::findf(FXchar c,FXint pos) const {
  register FXint gaplen=gapend-gapstart;
  FXASSERT(0<=pos && pos<length);
  while(pos<gapstart){
    if(buffer[pos]==c) return pos;
    pos++;
    }
  while(pos<length){
    if(buffer[pos+gaplen]==c) return pos;
    pos++;
    }
  return -1;
  }


// Search backward [0...pos> for character c, -1 if not found
FXint FXText::findb(FXchar c,FXint pos) const {
  register FXint gaplen=gapend-gapstart;
  FXASSERT(0<=pos && pos<length);
  pos--;
  while(pos>=gapstart){
    if(buffer[pos+gaplen]==c) return pos;
    pos--;
    }
  while(pos>=0){
    if(buffer[pos]==c) return pos;
    pos--;
    }
  return -1;
  }


// Move the cursor
void FXText::setCursorPos(FXint pos){
  if(pos<0) pos=0; else if(pos>=length) pos=length-1;
  if(cursorpos!=pos){
    drawCursor(0);
    cursorpos=pos;
    cursorline=lineStart(cursorpos);
    cursorlineno=getLineOfPos(cursorpos);
    cursorcol=pos-cursorline;
    cursory=font->getFontHeight()*(cursorlineno-1);
    cursorx=lineWidth(cursorline,cursorcol)-1;
    drawCursor(FLAG_CARET);
    }
  }


// Set anchor position
void FXText::setAnchorPos(FXint pos){
  if(pos<0) pos=0; else if(pos>=length) pos=length-1;
  anchorpos=pos;
  selstartpos=pos;
  selendpos=pos;
  }


// Kill the selection
FXbool FXText::killSelection(){
  if(hasSelection()){
    releaseSelection();
    updateRange(selstartpos,selendpos);
    selstartpos=anchorpos;
    selendpos=anchorpos;
    getApp()->flush();
    return TRUE;
    }
  return FALSE;
  }


// Extend selection
FXbool FXText::extendSelection(FXint pos,FXTextSelectionMode mode){
  FXbool changes=FALSE;
  FXint sp,ep;
  
  // Validate position
  if(pos<0) pos=0; else if(pos>=length) pos=length-1;
  
  // Did position change?
  switch(mode){
 
    // Selecting characters
    case SELECT_CHARS:
      if(pos<=anchorpos){
        sp=pos;
        ep=anchorpos;
        }
      else{
        sp=anchorpos;
        ep=pos;
        }
      break;
 
    // Selecting words
    case SELECT_WORDS:
      if(pos<=anchorpos){
        sp=wordStart(pos);
        ep=wordEnd(anchorpos);
        }
      else{
        sp=wordStart(anchorpos);
        ep=wordEnd(pos);
        }
      break;
 
    // Selecting lines
    case SELECT_LINES:
      if(pos<=anchorpos){
        sp=lineStart(pos);
        ep=forwardNLines(anchorpos);
        }
      else{
        sp=lineStart(anchorpos);
        ep=forwardNLines(pos);
        }
      break;
    }
  
  // Got a selection at all?
  if(sp!=ep){
    acquireSelection();
    }
  else{
    releaseSelection();
    }
  
  // Start of selection changed
  if(sp!=selstartpos){
    updateRange(sp,selstartpos);
    selstartpos=sp;
    changes=TRUE;
    }
  
  // End of selection changed
  if(ep!=selendpos){
    updateRange(selendpos,ep);
    selendpos=ep;
    changes=TRUE;
    }
  
  // Must be true
  FXASSERT(0<=selstartpos);
  FXASSERT(selendpos<length);
  FXASSERT(selstartpos<=selendpos);
  getApp()->flush();
  return changes;
  }


// Forward N lines, to beginning of nth line forward
FXint FXText::forwardNLines(FXint pos,FXint nlines) const {
  register FXint gaplen=gapend-gapstart;
  if(pos<0) pos=0; else if(pos>=length) pos=length-1;
  FXASSERT(0<=gapstart && gapstart<=length);
  if(nlines<=0) return pos;
  while(pos<gapstart && pos<length-1){
    if(buffer[pos]=='\n'){
      --nlines;
      if(nlines==0) return pos+1;
      }
    pos++;
    }
  while(pos<length-1){
    if(buffer[pos+gaplen]=='\n'){
      --nlines;
      if(nlines==0) return pos+1;
      }
    pos++;
    }
  return length-1;
  }


// Backward N lines, to beginning of nth line backward
FXint FXText::backwardNLines(FXint pos,FXint nlines) const {
  register FXint gaplen=gapend-gapstart;
  if(pos<0) pos=0; else if(pos>=length) pos=length-1;
  FXASSERT(0<=gapstart && gapstart<=length);
  if(nlines<=0) return pos;
  pos--;
  while(pos>=gapstart){
    if(buffer[pos+gaplen]=='\n'){
      if(nlines==0) return pos+1;
      --nlines;
      }
    pos--;
    }
  while(pos>=0){
    if(buffer[pos]=='\n'){
      if(nlines==0) return pos+1;
      --nlines;
      }
    pos--;
    }
  return 0;
  }


// Find end of previous word
FXint FXText::leftWord(FXint pos) const {
  FXchar ch;
  if(pos>=length) pos=length-1;
  if(0<pos){
    ch=getChar(pos-1);
    if(isdelim(ch)) return pos-1;
    }
  while(0<pos){
    ch=getChar(pos-1);
    if(isdelim(ch)) return pos;
    if(isspace(ch)) break;
    pos--;
    }
  while(0<pos){
    ch=getChar(pos-1);
    if(!isspace(ch)) return pos;
    pos--;
    }
  return 0;
  }


// Find begin of next word
FXint FXText::rightWord(FXint pos) const {
  FXchar ch;
  if(pos<0) pos=0;
  if(pos<length-1){
    ch=getChar(pos);
    if(isdelim(ch)) return pos+1;
    }
  while(pos<length-1){
    ch=getChar(pos);
    if(isdelim(ch)) return pos;
    if(isspace(ch)) break;
    pos++;
    }
  while(pos<length-1){
    ch=getChar(pos);
    if(!isspace(ch)) return pos;
    pos++;
    }
  return length-1;
  }


// Find begin of word
FXint FXText::wordStart(FXint pos) const {
  register FXchar ch;
  if(pos<0) return 0; 
  if(pos<length-1){
    ch=getChar(pos);
    if(isspace(ch) || isdelim(ch)) return pos;
    }
  else{
    pos=length-1;
    }
  while(0<pos){
    ch=getChar(pos-1);
    if(isspace(ch) || isdelim(ch)) return pos;
    pos--;
    }
  return 0;
  }


// FInd end of word
FXint FXText::wordEnd(FXint pos) const {
  register FXchar ch;
  if(pos>=length) return length-1;
  if(pos>0){
    ch=getChar(pos-1);
    if(isspace(ch) || isdelim(ch)) return pos;
    }
  else{
    pos=0;
    }
  while(pos<length-1){
    ch=getChar(pos);
    if(isspace(ch) || isdelim(ch)) return pos;
    pos++;
    }
  return length-1;
  }


// Make a valid position
FXint FXText::validPos(FXint pos) const {
  return pos<0 ? 0 : pos>=length ? length-1 : pos;
  }


/*******************************************************************************/


// Change top margin
void FXText::setMarginTop(FXint mt){
  if(margintop!=mt){
    margintop=mt;
    recalc();
    update(0,0,width,height);
    }
  }


// Change bottom margin
void FXText::setMarginBottom(FXint mb){
  if(marginbottom!=mb){
    marginbottom=mb;
    recalc();
    update(0,0,width,height);
    }
  }


// Change left margin
void FXText::setMarginLeft(FXint ml){
  if(marginleft!=ml){
    marginleft=ml;
    recalc();
    update(0,0,width,height);
    }
  }


// Change right margin
void FXText::setMarginRight(FXint mr){
  if(marginright!=mr){
    marginright=mr;
    recalc();
    update(0,0,width,height);
    }
  }


// Change the font
void FXText::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Set text color
void FXText::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Set select background color
void FXText::setSelBackColor(FXPixel clr){
  selbackColor=clr;
  update(0,0,width,height);
  }


// Set selected text color
void FXText::setSelTextColor(FXPixel clr){
  seltextColor=clr;
  update(0,0,width,height);
  }


// Set cursor color
void FXText::setCursorColor(FXPixel clr){
  cursorColor=clr;
  }


// Change help text
void FXText::setHelpText(const FXchar* text){
  help=text;
  }


// Change tip text
void FXText::setTipText(const FXchar* text){
  tip=text;
  }


// Change text style
void FXText::setTextStyle(FXuint style){
  FXuint opts=(options&~TEXT_MASK) | (style&TEXT_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    update(0,0,width,height);
    }
  }
  
  
// Get text style
FXuint FXText::getTextStyle() const {
  return (options&TEXT_MASK); 
  }


// Return true if editable
FXbool FXText::isEditable() const { 
  return (options&TEXT_READONLY)==0; 
  }


// Set widget is editable or not
void FXText::setEditable(FXbool edit){
  if(edit) options&=~TEXT_READONLY; else options|=TEXT_READONLY;
  }


// Change text in the buffer
void FXText::setText(const FXchar* text){
  FXint len=1;
  
  // Get length of text
  if(text) len=strlen(text)+1;
  
  // Resize the buffer
  if(!FXRESIZE(&buffer,FXchar,len+MINSIZE)){fxerror("%s::setText: out of memory.\n",getClassName());}
  gapstart=len;
  gapend=gapstart+MINSIZE;
  length=len;
  
  // Place mandatory nul in there
  buffer[0]='\0';
  
  // Fill with text
  if(text) memcpy(buffer,text,len);
  
  // Viewing top line
  topline=0;
  toplineno=1;
  topy=0;
  
  // Reset cursorpos, anchor, caret
  selstartpos=0;
  selendpos=0;
  anchorpos=0;
  cursorpos=0;
  cursorline=0;
  cursorlineno=1;
  cursorcol=0;
  cursorx=0;
  cursory=0;
  
  // Reset position
  pos_x=0;
  pos_y=0;
  
  // Reset scrollbars
  horizontal->setPosition(0);
  vertical->setPosition(0);
  
  // Mark to recompute stuff
  recalc();
  
  // Force layout
  layout();
  
  // No selection
  releaseSelection();
  
  // Repaint
  update(0,0,width,height);
  }


// We return a constant copy of the buffer
const FXchar *FXText::getText(){
  FXASSERT(0<length);
  FXASSERT(gapstart<=gapend);
  
  // Squeeze out the gap to move it to the end of the buffer
  memmove(&buffer[gapstart],&buffer[gapend],length-gapstart);
  gapend=length+gapend-gapstart;
  gapstart=length;
  
  // Maintain the invariant
  FXASSERT(buffer[length-1]=='\0');
  return buffer;
  }


// Save object to stream
void FXText::save(FXStream& store) const {
  FXScrollArea::save(store);
  // ....
  }


// Load object from stream
void FXText::load(FXStream& store){
  FXScrollArea::load(store);
  // ....
  }  


// Clean up
FXText::~FXText(){
  if(blinker) getApp()->removeTimeout(blinker);
  FXFREE(&buffer);
  FXFREE(&lines);
  buffer=(FXchar*)-1;
  lines=(FXint*)-1;
  font=(FXFont*)-1;
  blinker=(FXTimer*)-1;
  }
