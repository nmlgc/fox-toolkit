/********************************************************************************
*                                                                               *
*                            W i n d o w   O b j e c t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* Major Contributions for Windows NT by Lyle Johnson                            *
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
* $Id: FXWindow.cpp,v 1.192.4.4 2003/06/09 14:07:32 fox Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXVisual.h"
#include "FXDCWindow.h"
#include "FXCursor.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXMainWindow.h"



/*
 Notes:
  - When window is disabled, it should loose focus
  - When no subclass handles SEL_SELECTION_REQUEST, send back None property here
  - Update should only happen if widget is not in some sort of transaction.
  - Not every widget needs help and tip data; move this down to buttons etc.
  - Default constructors [for serialization] should initialize dynamic member
    variables same as regular constructor.  Dynamic variables are those that
    are not saved during serialization.
  - If FLAG_DIRTY gets reset at the END of layout(), it will be safe to have
    show() and hide() to call recalc(), in case they get called from the
    application code.
  - Use INCR mechanism if it gets large.
  - Perhaps should post CLIPBOARD type list in advance also, just like windows insists.
    We will need to keep this list some place [perhaps same place as XDND] and clean it
    after we're done [This would imply the CLIPBOARD type list would be the same as the
    XDND type list, which is probably OK].
  - Add URL jump text also.
  - Can we call parent->recalc() in the constructor?
  - Need other focus models:
      o click to focus (like we have now)
      o focus only by tabs and arrows (buttons should not move focus when clicking)
      o point to focus (useful for canvas type widgets)
    Mouse wheel should probably not cause focus changes...
*/

#ifndef WIN32

// Basic events
#define BASIC_EVENT_MASK   (StructureNotifyMask|ExposureMask|PropertyChangeMask|EnterWindowMask|LeaveWindowMask|KeyPressMask|KeyReleaseMask)

// Additional events for shell widget events
#define SHELL_EVENT_MASK   (FocusChangeMask|StructureNotifyMask)

// Additional events for enabled widgets
#define ENABLED_EVENT_MASK (ButtonPressMask|ButtonReleaseMask|PointerMotionMask)

// These events are grabbed for mouse grabs
#define GRAB_EVENT_MASK    (ButtonPressMask|ButtonReleaseMask|PointerMotionMask|EnterWindowMask|LeaveWindowMask)

// Do not propagate mask
#define NOT_PROPAGATE_MASK (KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ButtonMotionMask)

#endif


// Side layout modes
#define LAYOUT_SIDE_MASK (LAYOUT_SIDE_LEFT|LAYOUT_SIDE_RIGHT|LAYOUT_SIDE_TOP|LAYOUT_SIDE_BOTTOM)


// Layout modes
#define LAYOUT_MASK (LAYOUT_SIDE_MASK|LAYOUT_RIGHT|LAYOUT_CENTER_X|LAYOUT_BOTTOM|LAYOUT_CENTER_Y|LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y)


#define DISPLAY(app) ((Display*)((app)->display))

/*******************************************************************************/

// Map
FXDEFMAP(FXWindow) FXWindowMap[]={
  FXMAPFUNC(SEL_UPDATE,0,FXWindow::onUpdate),
  FXMAPFUNC(SEL_PAINT,0,FXWindow::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXWindow::onMotion),
  FXMAPFUNC(SEL_CONFIGURE,0,FXWindow::onConfigure),
  FXMAPFUNC(SEL_MOUSEWHEEL,0,FXWindow::onMouseWheel),
  FXMAPFUNC(SEL_MAP,0,FXWindow::onMap),
  FXMAPFUNC(SEL_UNMAP,0,FXWindow::onUnmap),
  FXMAPFUNC(SEL_DRAGGED,0,FXWindow::onDragged),
  FXMAPFUNC(SEL_ENTER,0,FXWindow::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXWindow::onLeave),
  FXMAPFUNC(SEL_DESTROY,0,FXWindow::onDestroy),
  FXMAPFUNC(SEL_FOCUSIN,0,FXWindow::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXWindow::onFocusOut),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXWindow::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXWindow::onSelectionGained),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXWindow::onSelectionRequest),
  FXMAPFUNC(SEL_CLIPBOARD_LOST,0,FXWindow::onClipboardLost),
  FXMAPFUNC(SEL_CLIPBOARD_GAINED,0,FXWindow::onClipboardGained),
  FXMAPFUNC(SEL_CLIPBOARD_REQUEST,0,FXWindow::onClipboardRequest),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXWindow::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXWindow::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXWindow::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXWindow::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXWindow::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXWindow::onRightBtnRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXWindow::onUngrabbed),
  FXMAPFUNC(SEL_KEYPRESS,0,FXWindow::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXWindow::onKeyRelease),
  FXMAPFUNC(SEL_DND_ENTER,0,FXWindow::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXWindow::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXWindow::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXWindow::onDNDMotion),
  FXMAPFUNC(SEL_DND_REQUEST,0,FXWindow::onDNDRequest),
  FXMAPFUNC(SEL_FOCUS_SELF,0,FXWindow::onFocusSelf),
  FXMAPFUNC(SEL_BEGINDRAG,0,FXWindow::onBeginDrag),
  FXMAPFUNC(SEL_ENDDRAG,0,FXWindow::onEndDrag),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_TOGGLESHOWN,FXWindow::onUpdToggleShown),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_DELETE,FXWindow::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SHOW,FXWindow::onCmdShow),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_HIDE,FXWindow::onCmdHide),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_TOGGLESHOWN,FXWindow::onCmdToggleShown),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_RAISE,FXWindow::onCmdRaise),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_LOWER,FXWindow::onCmdLower),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_ENABLE,FXWindow::onCmdEnable),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_DISABLE,FXWindow::onCmdDisable),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UPDATE,FXWindow::onCmdUpdate),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_DELETE,FXWindow::onCmdDelete),
  };


// Object implementation
FXIMPLEMENT(FXWindow,FXDrawable,FXWindowMap,ARRAYNUMBER(FXWindowMap))


/*******************************************************************************/


// Drag type names
const FXchar FXWindow::deleteTypeName[]="DELETE";
const FXchar FXWindow::textTypeName[]="text/plain";
const FXchar FXWindow::colorTypeName[]="application/x-color";
const FXchar FXWindow::urilistTypeName[]="text/uri-list";


// Drag type atoms; first widget to need it should register the type
FXDragType FXWindow::deleteType=0;
FXDragType FXWindow::textType=0;
FXDragType FXWindow::colorType=0;
FXDragType FXWindow::urilistType=0;

// The string type is predefined or hardwired
#ifndef WIN32
FXDragType FXWindow::stringType=XA_STRING;
#else
FXDragType FXWindow::stringType=CF_TEXT;
#endif


/*******************************************************************************/


// For deserialization
FXWindow::FXWindow(){
  FXTRACE((100,"FXWindow::FXWindow %p\n",this));
  parent=(FXWindow*)-1;
  owner=(FXWindow*)-1;
  first=(FXWindow*)-1;
  last=(FXWindow*)-1;
  next=(FXWindow*)-1;
  prev=(FXWindow*)-1;
  focus=NULL;
  defaultCursor=(FXCursor*)-1;
  dragCursor=(FXCursor*)-1;
  accelTable=(FXAccelTable*)-1;
  target=NULL;
  message=0;
  xpos=0;
  ypos=0;
  backColor=0;
  flags=FLAG_DIRTY|FLAG_UPDATE|FLAG_RECALC;
  options=0;
  wk=0;
  }


// Only used for the root window
FXWindow::FXWindow(FXApp* a,FXVisual *vis):FXDrawable(a,1,1){
  FXTRACE((100,"FXWindow::FXWindow %p\n",this));
  visual=vis;
  parent=NULL;
  owner=NULL;
  first=last=NULL;
  next=prev=NULL;
  focus=NULL;
  wk=1;
  defaultCursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
  dragCursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
  accelTable=NULL;
  target=NULL;
  message=0;
  xpos=0;
  ypos=0;
  backColor=0;
  flags=FLAG_DIRTY|FLAG_SHOWN|FLAG_UPDATE|FLAG_RECALC;
  options=LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT;
  }


// This constructor is used for shell windows
FXWindow::FXWindow(FXApp* a,FXWindow* own,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXDrawable(a,w,h){
  FXTRACE((100,"FXWindow::FXWindow %p\n",this));
  parent=a->root;
  owner=own;
  visual=getApp()->getDefaultVisual();
  first=last=NULL;
  prev=parent->last;
  next=NULL;
  parent->last=this;
  if(prev){
    wk=prev->wk+1;
    prev->next=this;
    }
  else{
    wk=1;
    parent->first=this;
    }
  focus=NULL;
  defaultCursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
  dragCursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
  accelTable=NULL;
  target=NULL;
  message=0;
  xpos=x;
  ypos=y;
  backColor=getApp()->getBaseColor();
  flags=FLAG_DIRTY|FLAG_UPDATE|FLAG_RECALC|FLAG_SHELL;
  options=opts;
  }


// This constructor is used for all child windows
FXWindow::FXWindow(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXDrawable(p->getApp(),w,h){
  FXTRACE((100,"FXWindow::FXWindow %p\n",this));
  parent=p;
  owner=parent;
  visual=parent->getVisual();
  first=last=NULL;
  prev=parent->last;
  next=NULL;
  parent->last=this;
  if(prev){
    wk=prev->wk+1;
    prev->next=this;
    }
  else{
    wk=1;
    parent->first=this;
    }
  focus=NULL;
  defaultCursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
  dragCursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
  accelTable=NULL;
  target=NULL;
  message=0;
  xpos=x;
  ypos=y;
  backColor=getApp()->getBaseColor();
  flags=FLAG_DIRTY|FLAG_UPDATE|FLAG_RECALC;
  options=opts;
  }


/*******************************************************************************/


// Save data
void FXWindow::save(FXStream& store) const {
  FXDrawable::save(store);
  store << parent;
  store << owner;
  store << first;
  store << last;
  store << next;
  store << prev;
  store << focus;
  store << defaultCursor;
  store << dragCursor;
  store << accelTable;
  store << target;
  store << message;
  store << xpos;
  store << ypos;
  store << backColor;
  store << options;
  store << wk;
  }


// Load data
void FXWindow::load(FXStream& store){
  FXDrawable::load(store);
  store >> parent;
  store >> owner;
  store >> first;
  store >> last;
  store >> next;
  store >> prev;
  store >> focus;
  store >> defaultCursor;
  store >> dragCursor;
  store >> accelTable;
  store >> target;
  store >> message;
  store >> xpos;
  store >> ypos;
  store >> backColor;
  store >> options;
  store >> wk;
  }


/*******************************************************************************/


// Return a pointer to the shell window
FXWindow* FXWindow::getShell() const {
  register FXWindow *win=(FXWindow*)this;
  register FXWindow *p;
  while((p=win->parent) && p->parent) win=p;
  return win;
  }


// Return a pointer to the root window
FXWindow* FXWindow::getRoot() const {
  register FXWindow *win=(FXWindow*)this;
  while(win->parent) win=win->parent;
  return win;
  }


// Move window in chain before a sibling
void FXWindow::linkBefore(FXWindow* sibling){
  if(sibling!=this){
    if(prev) prev->next=next; else parent->first=next;
    if(next) next->prev=prev; else parent->last=prev;
    next=sibling;
    prev=sibling?sibling->prev:parent->last;
    if(prev) prev->next=this; else parent->first=this;
    if(next) next->prev=this; else parent->last=this;
    recalc();
    }
  }


// Move window in chain after a sibling
void FXWindow::linkAfter(FXWindow* sibling){
  if(sibling!=this){
    if(prev) prev->next=next; else parent->first=next;
    if(next) next->prev=prev; else parent->last=prev;
    next=sibling?sibling->next:parent->first;
    prev=sibling;
    if(prev) prev->next=this; else parent->first=this;
    if(next) next->prev=this; else parent->last=this;
    recalc();
    }
  }


// Test if logically inside
FXbool FXWindow::contains(FXint parentx,FXint parenty) const {
  return xpos<=parentx && parentx<xpos+width && ypos<=parenty && parenty<ypos+height;
  }


// Return true if specified window is a child of this window
FXbool FXWindow::containsChild(const FXWindow* child) const {
  while(child){
    if(child==this) return TRUE;
    child=child->parent;
    }
  return FALSE;
  }


// Return true if specified window is this window's parent
FXbool FXWindow::isChildOf(const FXWindow* window) const {
  if(window!=this){
    const FXWindow* w=this;
    while(w){
      if(w==window) return TRUE;
      w=w->parent;
      }
    }
  return FALSE;
  }


// Get child at x,y
FXWindow* FXWindow::getChildAt(FXint x,FXint y) const {
  register FXWindow *child;
  if(0<=x && 0<=y && x<width && y<height){
    for(child=last; child; child=child->prev){
      if(child->shown() && child->xpos<=x && child->ypos<=y && x<child->xpos+child->width && y<child->ypos+child->height) return child;
      }
    }
  return NULL;
  }


// Count number of children
FXint FXWindow::numChildren() const {
  register const FXWindow *child=first;
  register FXint num=0;
  while(child){
    child=child->next;
    num++;
    }
  return num;
  }


// Get index of child window
FXint FXWindow::indexOfChild(const FXWindow *window) const {
  register FXint index=0;
  if(!window || window->parent!=this) return -1;
  while(window->prev){
    window=window->prev;
    index++;
    }
  return index;
  }


// Get child window at index
FXWindow* FXWindow::childAtIndex(FXint index) const {
  register FXWindow* child=first;
  if(index<0) return NULL;
  while(index && child){
    child=child->next;
    index--;
    }
  return child;
  }


// Find common ancestor between window a and b
FXWindow* FXWindow::commonAncestor(FXWindow* a,FXWindow* b){
  register FXWindow *p1,*p2;
  if(a || b){
    if(!a) return b->getRoot();
    if(!b) return a->getRoot();
    p1=a;
    while(p1){
      p2=b;
      while(p2){
        if(p2==p1) return p1;
        p2=p2->parent;
        }
      p1=p1->parent;
      }
    }
  return NULL;
  }


// Return true if window is a shell window
FXbool FXWindow::isShell() const {
  return (flags&FLAG_SHELL)!=0;
  }


/*******************************************************************************/


// Handle repaint
long FXWindow::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  dc.setForeground(backColor);
  dc.fillRectangle(0,0,width,height);
  return 1;
  }


// Window was mapped to screen
long FXWindow::onMap(FXObject*,FXSelector,void* ptr){
  FXTRACE((250,"%s::onMap %p\n",getClassName(),this));
  return target && target->handle(this,MKUINT(message,SEL_MAP),ptr);
  }


// Window was unmapped; the grab is lost
long FXWindow::onUnmap(FXObject*,FXSelector,void* ptr){
  FXTRACE((250,"%s::onUnmap %p\n",getClassName(),this));
  if(getApp()->mouseGrabWindow==this) getApp()->mouseGrabWindow=NULL;
  if(getApp()->keyboardGrabWindow==this) getApp()->keyboardGrabWindow=NULL;
  return target && target->handle(this,MKUINT(message,SEL_UNMAP),ptr);
  }


// Handle configure notify
long FXWindow::onConfigure(FXObject*,FXSelector,void* ptr){
  FXTRACE((250,"%s::onConfigure %p\n",getClassName(),this));
  return target && target->handle(this,MKUINT(message,SEL_CONFIGURE),ptr);
  }


// The window was destroyed; the grab is lost
long FXWindow::onDestroy(FXObject*,FXSelector,void*){
  FXTRACE((250,"%s::onDestroy %p\n",getClassName(),this));
#ifndef WIN32
  XDeleteContext(DISPLAY(getApp()),xid,getApp()->wcontext);
#else
  SetWindowLong((HWND)xid,0,0);
#endif
  if(getApp()->mouseGrabWindow==this) getApp()->mouseGrabWindow=NULL;
  if(getApp()->keyboardGrabWindow==this) getApp()->keyboardGrabWindow=NULL;
  flags&=~(FLAG_SHOWN|FLAG_FOCUSED);
  xid=0;
  return 1;
  }


// Left button pressed
long FXWindow::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    }
  return 0;
  }


// Left button released
long FXWindow::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    }
  return 0;
  }


// Middle button released
long FXWindow::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    }
  return 0;
  }


// Middle button pressed
long FXWindow::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    }
  return 0;
  }


// Right button pressed
long FXWindow::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    }
  return 0;
  }


// Right button released
long FXWindow::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    }
  return 0;
  }


// Mouse motion
long FXWindow::onMotion(FXObject*,FXSelector,void* ptr){
  return isEnabled() && target && target->handle(this,MKUINT(message,SEL_MOTION),ptr);
  }


// Mouse wheel
long FXWindow::onMouseWheel(FXObject*,FXSelector,void* ptr){
  return isEnabled() && target && target->handle(this,MKUINT(message,SEL_MOUSEWHEEL),ptr);
  }


// Keyboard press
long FXWindow::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXTRACE((200,"%s::onKeyPress %p keysym=0x%04x state=%04x\n",getClassName(),this,((FXEvent*)ptr)->code,((FXEvent*)ptr)->state));
  return isEnabled() && target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr);
  }


// Keyboard release
long FXWindow::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXTRACE((200,"%s::onKeyRelease %p keysym=0x%04x state=%04x\n",getClassName(),this,((FXEvent*)ptr)->code,((FXEvent*)ptr)->state));
  return isEnabled() && target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr);
  }


// Start a drag operation
long FXWindow::onBeginDrag(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_BEGINDRAG),ptr);
  }


// End drag operation
long FXWindow::onEndDrag(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_ENDDRAG),ptr);
  }


// Dragged stuff around
long FXWindow::onDragged(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DRAGGED),ptr);
  }


// Entering window
long FXWindow::onEnter(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTRACE((150,"%s::onEnter %p (%s)\n",getClassName(),this, (event->code==CROSSINGNORMAL) ? "CROSSINGNORMAL" : (event->code==CROSSINGGRAB) ? "CROSSINGGRAB" : (event->code==CROSSINGUNGRAB)? "CROSSINGUNGRAB" : "?"));
  if(event->code!=CROSSINGGRAB){
    getApp()->cursorWindow=this;
    if(!(event->state&(SHIFTMASK|CONTROLMASK|LEFTBUTTONMASK|MIDDLEBUTTONMASK|RIGHTBUTTONMASK))) flags|=FLAG_TIP;
    flags|=FLAG_HELP;
    }
  if(isEnabled()){
    if(target) target->handle(this,MKUINT(message,SEL_ENTER),ptr);
    }
  return 1;
  }


// Leaving window
long FXWindow::onLeave(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTRACE((150,"%s::onLeave %p (%s)\n",getClassName(),this, (event->code==CROSSINGNORMAL) ? "CROSSINGNORMAL" : (event->code==CROSSINGGRAB) ? "CROSSINGGRAB" : (event->code==CROSSINGUNGRAB)? "CROSSINGUNGRAB" : "?"));
  if(event->code!=CROSSINGUNGRAB){
    getApp()->cursorWindow=parent;
    flags&=~(FLAG_TIP|FLAG_HELP);
    }
  if(isEnabled()){
    if(target) target->handle(this,MKUINT(message,SEL_LEAVE),ptr);
    }
  return 1;
  }


// True if window under the cursor
FXbool FXWindow::underCursor() const {
  return (getApp()->cursorWindow==this);
  }


// Gained focus
long FXWindow::onFocusIn(FXObject*,FXSelector,void* ptr){
  FXTRACE((250,"%s::onFocusIn %p\n",getClassName(),this));
  flags|=FLAG_FOCUSED;
  if(focus) focus->handle(focus,MKUINT(0,SEL_FOCUSIN),NULL);
  if(target) target->handle(this,MKUINT(message,SEL_FOCUSIN),ptr);
  return 1;
  }


// Lost focus
long FXWindow::onFocusOut(FXObject*,FXSelector,void* ptr){
  FXTRACE((250,"%s::onFocusOut %p\n",getClassName(),this));
  flags&=~FLAG_FOCUSED;
  if(focus) focus->handle(focus,MKUINT(0,SEL_FOCUSOUT),NULL);
  if(target) target->handle(this,MKUINT(message,SEL_FOCUSOUT),ptr);
  return 1;
  }


// Focus on widget itself, if its enabled
long FXWindow::onFocusSelf(FXObject*,FXSelector,void*){
  FXTRACE((250,"%s::onFocusSelf %p\n",getClassName(),this));
  if(isEnabled() && canFocus()){ setFocus(); return 1; }	// Patch from: Petri Hodju <phodju@cc.hut.fi>
  return 0;
  }


// Handle drag-and-drop enter
long FXWindow::onDNDEnter(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onDNDEnter %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_DND_ENTER),ptr)) return 1;
  return 0;
  }


// Handle drag-and-drop leave
long FXWindow::onDNDLeave(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onDNDLeave %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_DND_LEAVE),ptr)) return 1;
  return 0;
  }


// Handle drag-and-drop motion
long FXWindow::onDNDMotion(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onDNDMotion %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_DND_MOTION),ptr)) return 1;
  return 0;
  }


// Handle drag-and-drop drop
long FXWindow::onDNDDrop(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onDNDDrop %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_DND_DROP),ptr)) return 1;
  return 0;
  }


// Request for DND data
long FXWindow::onDNDRequest(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onDNDRequest %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_DND_REQUEST),ptr)) return 1;
  return 0;
  }


// Show window
long FXWindow::onCmdShow(FXObject*,FXSelector,void*){
  if(!shown()){ show(); recalc(); }
  return 1;
  }


// Hide window
long FXWindow::onCmdHide(FXObject*,FXSelector,void*){
  if(shown()){ hide(); recalc(); }
  return 1;
  }

// Hide or show window
long FXWindow::onCmdToggleShown(FXObject*,FXSelector,void*){
  shown() ? hide() : show();
  recalc();
  return 1;
  }


// Update hide or show window
long FXWindow::onUpdToggleShown(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_SETVALUE,SEL_COMMAND),(void*)(long)shown());
  return 1;
  }


// Raise window
long FXWindow::onCmdRaise(FXObject*,FXSelector,void*){
  raise();
  return 1;
  }


// Lower window
long FXWindow::onCmdLower(FXObject*,FXSelector,void*){
  lower();
  return 1;
  }


// Delete window
long FXWindow::onCmdDelete(FXObject*,FXSelector,void*){
  delete this;
  return 1;
  }


// Enable window
long FXWindow::onCmdEnable(FXObject*,FXSelector,void*){
  enable();
  return 1;
  }


// Disable window
long FXWindow::onCmdDisable(FXObject*,FXSelector,void*){
  disable();
  return 1;
  }


// In combination with autograying/autohiding widgets,
// this could be used to ungray or show when you don't want
// to write a whole handler routine just to do this.
long FXWindow::onUpdYes(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  return 1;
  }


// Update (repaint) window
long FXWindow::onCmdUpdate(FXObject*,FXSelector,void*){
  update();
  return 1;
  }


/*******************************************************************************/


// If window can have focus
FXbool FXWindow::canFocus() const {
  return FALSE;
  }


// Has window the focus
FXbool FXWindow::hasFocus() const {
  return (flags&FLAG_FOCUSED)!=0;
  }


// Set focus to this widget.
// The chain of focus from shell down to a control is changed.
// Widgets now in the chain may or may not gain real focus,
// depending on whether parent window already had a real focus!
// Setting the focus to a composite will cause descendants to loose it.
void FXWindow::setFocus(){
  if(parent && parent->focus!=this){
    if(parent->focus) parent->focus->killFocus(); else parent->setFocus();
    parent->focus=this;
    if(parent->hasFocus()) handle(this,MKUINT(0,SEL_FOCUSIN),NULL);
    }
  flags|=FLAG_HELP;
  }


// Kill focus to this widget.
void FXWindow::killFocus(){
  if(parent && parent->focus==this){
    if(focus) focus->killFocus();
    if(hasFocus()) handle(this,MKUINT(0,SEL_FOCUSOUT),NULL);
    parent->focus=NULL;
    }
  flags&=~FLAG_HELP;
  }


// Search widget tree for default window
FXWindow* FXWindow::findDefault(FXWindow* window){
  register FXWindow *win,*def;
  if(window->flags&FLAG_DEFAULT) return window;
  for(win=window->first; win; win=win->next){
    if((def=findDefault(win))!=NULL) return def;
    }
  return NULL;
  }


// Search widget tree for initial window
FXWindow* FXWindow::findInitial(FXWindow* window){
  register FXWindow *win,*ini;
  if(window->flags&FLAG_INITIAL) return window;
  for(win=window->first; win; win=win->next){
    if((ini=findInitial(win))!=NULL) return ini;
    }
  return NULL;
  }


// Make widget drawn as default
void FXWindow::setDefault(FXbool enable){
  register FXWindow *win;
  switch(enable){
    case FALSE:
      flags&=~FLAG_DEFAULT;
      break;
    case TRUE:
      if(!(flags&FLAG_DEFAULT)){
        win=findDefault(getShell());
        if(win) win->setDefault(FALSE);
        flags|=FLAG_DEFAULT;
        }
      break;
    case MAYBE:
      if(flags&FLAG_DEFAULT){
        flags&=~FLAG_DEFAULT;
        win=findInitial(getShell());
        if(win) win->setDefault(TRUE);
        }
      break;
    }
  }


// Return true if widget is drawn as default
FXbool FXWindow::isDefault() const {
  return (flags&FLAG_DEFAULT)!=0;
  }


// Make this window the initial default window
void FXWindow::setInitial(FXbool enable){
  register FXWindow *win;
  if((flags&FLAG_INITIAL) && !enable){
    flags&=~FLAG_INITIAL;
    }
  if(!(flags&FLAG_INITIAL) && enable){
    win=findInitial(getShell());
    if(win) win->setInitial(FALSE);
    flags|=FLAG_INITIAL;
    }
  }


// Return true if this is the initial default window
FXbool FXWindow::isInitial() const {
  return (flags&FLAG_INITIAL)!=0;
  }


/*******************************************************************************/


#ifndef WIN32

// Add this window to the list of colormap windows
void FXWindow::addColormapWindows(){
  Window windows[2],*windowsReturn,*windowList;
  int countReturn,i;

  // Check to see if there is already a property
  Status status=XGetWMColormapWindows(DISPLAY(getApp()),getShell()->id(),&windowsReturn,&countReturn);

  // If no property, just create one
  if(!status){
    windows[0]=id();
    windows[1]=getShell()->id();
    XSetWMColormapWindows(DISPLAY(getApp()),getShell()->id(),windows,2);
    }

  // There was a property, add myself to the beginning
  else{
    windowList=(Window*)malloc((sizeof(Window))*(countReturn+1));
    windowList[0]=id();
    for(i=0; i<countReturn; i++) windowList[i+1]=windowsReturn[i];
    XSetWMColormapWindows(DISPLAY(getApp()),getShell()->id(),windowList,countReturn+1);
    XFree((char*)windowsReturn);
    free(windowList);
    }
  }


// Remove it from colormap windows
void FXWindow::remColormapWindows(){
  Window *windowsReturn;
  int countReturn,i;
  Status status=XGetWMColormapWindows(DISPLAY(getApp()),getShell()->id(),&windowsReturn,&countReturn);
  if(status){
    for(i=0; i<countReturn; i++){
      if(windowsReturn[i]==id()){
        for(i++; i<countReturn; i++) windowsReturn[i-1]=windowsReturn[i];
        XSetWMColormapWindows(DISPLAY(getApp()),getShell()->id(),windowsReturn,countReturn-1);
        break;
        }
      }
    XFree((char*)windowsReturn);
    }
  }

#endif


/*******************************************************************************/


// Create X window
void FXWindow::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));

#ifndef WIN32

      XSetWindowAttributes wattr;
      XClassHint hint;
      unsigned long mask;

      // Gotta have a parent already created!
      if(!parent->id()){ fxerror("%s::create: trying to create window before creating parent window.\n",getClassName()); }

      // If window has owner, owner should have been created already
      if(owner && !owner->id()){ fxerror("%s::create: trying to create window before creating owner window.\n",getClassName()); }

      // Got to have a visual
      if(!visual){ fxerror("%s::create: trying to create window without a visual.\n",getClassName()); }

      // Initialize visual
      visual->create();

      // Create default cursor
      if(defaultCursor) defaultCursor->create();

      // Create drag cursor
      if(dragCursor) dragCursor->create();

      // Fill in the attributes
      mask=CWBackPixmap|CWWinGravity|CWBitGravity|CWBorderPixel|CWEventMask|CWDontPropagate|CWCursor|CWOverrideRedirect|CWSaveUnder|CWColormap;
      //mask=CWBackPixel|CWWinGravity|CWBitGravity|CWBorderPixel|CWEventMask|CWDontPropagate|CWCursor|CWOverrideRedirect|CWSaveUnder|CWColormap;

      // Events for normal windows
      wattr.event_mask=BASIC_EVENT_MASK;

      // Events for shell windows
      if(flags&FLAG_SHELL) wattr.event_mask|=SHELL_EVENT_MASK;

      // If enabled, turn on some more events
      if(flags&FLAG_ENABLED) wattr.event_mask|=ENABLED_EVENT_MASK;

      // FOX will not propagate events to ancestor windows
      wattr.do_not_propagate_mask=NOT_PROPAGATE_MASK;

      // Obtain colormap
      wattr.colormap=visual->colormap;

      // This is needed for OpenGL
      wattr.border_pixel=0;

      // Background
      wattr.background_pixmap=None;
      //wattr.background_pixel=visual->getPixel(backColor);

      // Preserving content during resize will be faster:- not turned
      // on yet as we will have to recode all widgets to decide when to
      // repaint or not to repaint the display when resized...
      wattr.bit_gravity=NorthWestGravity;
      wattr.bit_gravity=ForgetGravity;

      // The window gravity is NorthWestGravity, which means
      // if a child keeps same position relative to top/left
      // of its parent window, nothing extra work is incurred.
      wattr.win_gravity=NorthWestGravity;

      // Determine override redirect
      wattr.override_redirect=doesOverrideRedirect();

      // Determine save-unders
      wattr.save_under=doesSaveUnder();

      // Set cursor
      wattr.cursor=defaultCursor->id();

      // Finally, create the window
      xid=XCreateWindow(DISPLAY(getApp()),parent->id(),xpos,ypos,FXMAX(width,1),FXMAX(height,1),0,visual->depth,InputOutput,(Visual*)visual->visual,mask,&wattr);

      // Uh-oh, we failed
      if(!xid){ fxerror("%s::create: unable to create window.\n",getClassName()); }

      // Store for xid to C++ object mapping
      XSaveContext(DISPLAY(getApp()),xid,getApp()->wcontext,(XPointer)this);

      // Set resource and class name for toplevel windows.
      // In a perfect world this would be set in FXTopWindow, but for some strange reasons
      // some window-managers (e.g. fvwm) this will be too late and they will not recognize them.
      // Patch from axel.kohlmeyer@chemie.uni-ulm.de
      if(flags&FLAG_SHELL){
        hint.res_name="FoxApp";
        hint.res_class="FoxWindow";
        XSetClassHint(DISPLAY(getApp()),xid,&hint);
        }

      // We put the XdndAware property on all toplevel windows, so that
      // when dragging, we need to search no further than the toplevel window.
      if(flags&FLAG_SHELL){
        Atom propdata=(Atom)XDND_PROTOCOL_VERSION;
        XChangeProperty(DISPLAY(getApp()),xid,getApp()->xdndAware,XA_ATOM,32,PropModeReplace,(unsigned char*)&propdata,1);
        }

      // If window is a shell and it has an owner, make it stay on top of the owner
      if((flags&FLAG_SHELL) && owner){
        XSetTransientForHint(DISPLAY(getApp()),xid,owner->getShell()->id());
        }

      // If colormap different, set WM_COLORMAP_WINDOWS property properly
      if(visual->colormap!=DefaultColormap(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())))){
        FXTRACE((150,"%s::create: %p: adding to WM_COLORMAP_WINDOWS\n",getClassName(),this));
        addColormapWindows();
        }

      // Show if it was supposed to be
      if((flags&FLAG_SHOWN) && 0<width && 0<height) XMapWindow(DISPLAY(getApp()),xid);

#else

      // Gotta have a parent already created!
      if(!parent->id()){ fxerror("%s::create: trying to create window before creating parent window.\n",getClassName()); }

      // If window has owner, owner should have been created already
      if(owner && !owner->id()){ fxerror("%s::create: trying to create window before creating owner window.\n",getClassName()); }

      // Got to have a visual
      if(!visual){ fxerror("%s::create: trying to create window without a visual.\n",getClassName()); }

      // Initialize visual
      visual->create();

      // Create default cursor
      if(defaultCursor) defaultCursor->create();

      // Create drag cursor
      if(dragCursor) dragCursor->create();

      // Most windows use these style bits
      DWORD dwStyle=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
      DWORD dwExStyle=0;
      dwExStyle|=WS_EX_NOPARENTNOTIFY;
      HWND hParent=(HWND)parent->id();
      if(flags&FLAG_SHELL){
        if(isMemberOf(FXMETACLASS(FXTopWindow))){
          dwStyle=WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
          }
        else if(doesOverrideRedirect()){
          // To control window placement or control decoration, a window manager
          // often needs to redirect map or configure requests. Popup windows, however,
          // often need to be mapped without a window manager getting in the way.
          dwStyle=WS_POPUP|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
          dwExStyle=WS_EX_TOOLWINDOW;
          }
        else{
          // Other top-level shell windows (like dialogs)
          dwStyle=WS_POPUP|WS_CAPTION|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
          dwExStyle=WS_EX_DLGMODALFRAME|WS_EX_TOOLWINDOW;
          // if(hParent==GetDesktopWindow() && getApp()->getMainWindow()!=0){
          // if(getApp()->getMainWindow()->id()) hParent=(HWND)getApp()->getMainWindow()->id();
          }
        if(owner) hParent=(HWND)owner->id();
        }

      // Create this window
      xid=CreateWindowEx(dwExStyle,GetClass(),NULL,dwStyle,xpos,ypos,FXMAX(width,1),FXMAX(height,1),hParent,NULL,(HINSTANCE)FXApp::hInstance,this);

      // We put the XdndAware property on all toplevel windows, so that
      // when dragging, we need to search no further than the toplevel window.
      if(flags&FLAG_SHELL){
        HANDLE propdata=(HANDLE)XDND_PROTOCOL_VERSION;
        SetProp((HWND)xid,(LPCTSTR)MAKELONG(getApp()->xdndAware,0),propdata);
        }

      // Uh-oh, we failed
      if(!xid){ fxerror("%s::create: unable to create window.\n",getClassName()); }

      // Show if it was supposed to be.  Apparently, initial state
      // is neither shown nor hidden, so an explicit hide is needed.
      // Patch thanks to "Glenn Shen" <shen@hks.com>
      if(flags&FLAG_SHOWN)
        ShowWindow((HWND)xid,SW_SHOWNOACTIVATE);
      else
        ShowWindow((HWND)xid,SW_HIDE);
#endif
      }
    }
  }


/*
// Attach to existing window; possibly, the window belongs to another application.
void FXWindow::attach(FXID wnd){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::attach %p\n",getClassName(),this));
      // FIXME do more than this... but what?
#ifndef WIN32

      // Reparent under what WE think the parent is
      XReparentWindow(DISPLAY(getApp()),wnd,parent->id(),0,0);

      // Store for xid to C++ object mapping
      XSaveContext(DISPLAY(getApp()),wnd,getApp()->wcontext,(XPointer)this);
#else
      // Reparent under what WE think the parent is
      SetParent((HWND)wnd,(HWND)parent->id());

      // Store for xid to C++ object mapping
      SetWindowLong((HWND)wnd,0,(LONG)this);
#endif
      xid=x;
      }
    }
  }
*/


// Detach window
void FXWindow::detach(){

  // Detach visual
  visual->detach();

  // Detach default cursor
  if(defaultCursor) defaultCursor->detach();

  // Detach drag cursor
  if(dragCursor) dragCursor->detach();

  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::detach %p\n",getClassName(),this));

#ifndef WIN32

      // Remove from xid to C++ object mapping
      XDeleteContext(DISPLAY(getApp()),xid,getApp()->wcontext);
#else

      // Break binding between HWND and C++
      SetWindowLong((HWND)xid,0,0);
#endif
      }

    // No longer grabbed
    if(getApp()->mouseGrabWindow==this) getApp()->mouseGrabWindow=NULL;
    if(getApp()->keyboardGrabWindow==this) getApp()->keyboardGrabWindow=NULL;

    xid=0;
    }
  }


// Destroy
void FXWindow::destroy(){
  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::destroy %p\n",getClassName(),this));

#ifndef WIN32

      // If colormap different, set WM_COLORMAP_WINDOWS property properly
      if(visual->colormap!=DefaultColormap(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())))){
        FXTRACE((150,"%s::destroy: %p: removing from WM_COLORMAP_WINDOWS\n",getClassName(),this));
        remColormapWindows();
        }

      // Delete the XdndAware property
      if(flags&FLAG_SHELL){
        XDeleteProperty(DISPLAY(getApp()),xid,getApp()->xdndAware);
        }

      // Delete the window
      XDestroyWindow(DISPLAY(getApp()),xid);

      // Remove from xid to C++ object mapping
      XDeleteContext(DISPLAY(getApp()),xid,getApp()->wcontext);

#else

      // Delete the XdndAware property
      if(flags&FLAG_SHELL){
        RemoveProp((HWND)xid,(LPCTSTR)MAKELONG(getApp()->xdndAware,0));
        }

      // Break binding between HWND and C++
      SetWindowLong((HWND)xid,0,0);

      // Zap the window
      DestroyWindow((HWND)xid);

#endif
      }

    // No longer grabbed
    if(getApp()->mouseGrabWindow==this) getApp()->mouseGrabWindow=NULL;
    if(getApp()->keyboardGrabWindow==this) getApp()->keyboardGrabWindow=NULL;

    xid=0;
    }
  }


#ifdef WIN32

// Get this window's device context
FXID FXWindow::GetDC() const {
  return ::GetDC((HWND)xid);
  }

// Release it
int FXWindow::ReleaseDC(FXID hdc) const {
  return ::ReleaseDC((HWND)xid,(HDC)hdc);
  }

// Window class
const char* FXWindow::GetClass() const { return "FXWindow"; }


#endif


/*******************************************************************************/


// Test if active
FXbool FXWindow::isActive() const {
  return (flags&FLAG_ACTIVE)!=0;
  }


// Get default width
FXint FXWindow::getDefaultWidth(){
  return 1;
  }


// Get default height
FXint FXWindow::getDefaultHeight(){
  return 1;
  }


// Return width for given height
FXint FXWindow::getWidthForHeight(FXint){
  return getDefaultWidth();
  }


// Return height for given width
FXint FXWindow::getHeightForWidth(FXint){
  return getDefaultHeight();
  }


// Set X position
void FXWindow::setX(FXint x){
  xpos=x;
  recalc();
  }


// Set Y position
void FXWindow::setY(FXint y){
  ypos=y;
  recalc();
  }


// Set width
void FXWindow::setWidth(FXint w){
  if(w<0) w=0;
  width=w;
  recalc();
  }


// Set height
void FXWindow::setHeight(FXint h){
  if(h<0) h=0;
  height=h;
  recalc();
  }


// Change layout
void FXWindow::setLayoutHints(FXuint lout){
  FXuint opts=(options&~LAYOUT_MASK) | (lout&LAYOUT_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Get layout hints
FXuint FXWindow::getLayoutHints() const {
  return (options&LAYOUT_MASK);
  }


// Is widget a composite
FXbool FXWindow::isComposite() const {
  return 0;
  }


// Window does override-redirect
FXbool FXWindow::doesOverrideRedirect() const {
  return FALSE;
  }


// Window does save-unders
FXbool FXWindow::doesSaveUnder() const {
  return FALSE;
  }


// Add hot key to closest ancestor's accelerator table
void FXWindow::addHotKey(FXHotKey code){
  FXAccelTable *accel; FXWindow *win=this;
  while(win && (accel=win->getAccelTable())==NULL) win=win->parent;
  if(accel) accel->addAccel(code,this,MKUINT(ID_HOTKEY,SEL_KEYPRESS),MKUINT(ID_HOTKEY,SEL_KEYRELEASE));
  }


// Remove hot key from closest ancestor's accelerator table
void FXWindow::remHotKey(FXHotKey code){
  FXAccelTable *accel; FXWindow *win=this;
  while(win && (accel=win->getAccelTable())==NULL) win=win->parent;
  if(accel) accel->removeAccel(code);
  }


/*******************************************************************************/


// Set cursor
void FXWindow::setDefaultCursor(FXCursor* cur){
  if(defaultCursor!=cur){
    if(cur==NULL){ fxerror("%s::setDefaultCursor: NULL cursor argument.\n",getClassName()); }
    if(xid){
      if(cur->id()==0){ fxerror("%s::setDefaultCursor: Cursor has not been created yet.\n",getClassName()); }
#ifndef WIN32
      XDefineCursor(DISPLAY(getApp()),xid,cur->id());
#else
      if(!grabbed()) SetCursor((HCURSOR)cur->id());
#endif
      }
    defaultCursor=cur;
    }
  }


// Set drag cursor
void FXWindow::setDragCursor(FXCursor* cur){
  if(dragCursor!=cur){
    if(cur==NULL){ fxerror("%s::setDragCursor: NULL cursor argument.\n",getClassName()); }
    if(xid){
      if(cur->id()==0){ fxerror("%s::setDragCursor: Cursor has not been created yet.\n",getClassName()); }
#ifndef WIN32
      if(grabbed()){ XChangeActivePointerGrab(DISPLAY(getApp()),GRAB_EVENT_MASK,cur->id(),CurrentTime); }
#else
      if(grabbed()) SetCursor((HCURSOR)cur->id());
#endif
      }
    dragCursor=cur;
    }
  }


// Set window background
void FXWindow::setBackColor(FXColor clr){
  if(clr!=backColor){
    backColor=clr;
//     if(xid){
// #ifndef WIN32
//       XSetWindowBackground(DISPLAY(getApp()),xid,visual->getPixel(backColor));
// #else
//       FXASSERT(0);
// #endif
//       }
    update();
    }
  }


/*******************************************************************************/


// Lost the selection
long FXWindow::onSelectionLost(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onSelectionLost %p\n",getClassName(),this));
  if(target) target->handle(this,MKUINT(message,SEL_SELECTION_LOST),ptr);
  return 1;
  }


// Gained the selection
long FXWindow::onSelectionGained(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onSelectionGained %p\n",getClassName(),this));
  if(target) target->handle(this,MKUINT(message,SEL_SELECTION_GAINED),ptr);
  return 1;
  }


// Somebody wants our the selection
long FXWindow::onSelectionRequest(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onSelectionRequest %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_SELECTION_REQUEST),ptr)) return 1;
  return 0;
  }


// Has this window the selection
FXbool FXWindow::hasSelection() const {
  return (getApp()->selectionWindow==this);
  }


// Acquire the selection.
// We always generate SEL_SELECTION_LOST and SEL_SELECTION_GAINED
// because we assume the selection types may have changed, and want
// to give target opportunity to allocate the new data for these types.
FXbool FXWindow::acquireSelection(const FXDragType *types,FXuint numtypes){
  if(xid){
    if(!types || !numtypes){ fxerror("%s::acquireSelection: should have at least one type to select.\n",getClassName()); }
    if(getApp()->selectionWindow){
      getApp()->selectionWindow->handle(getApp(),MKUINT(0,SEL_SELECTION_LOST),&getApp()->event);
      getApp()->selectionWindow=NULL;
      FXFREE(&getApp()->xselTypeList);
      getApp()->xselNumTypes=0;
      }
#ifndef WIN32
    XSetSelectionOwner(DISPLAY(getApp()),XA_PRIMARY,xid,getApp()->event.time);
    if(XGetSelectionOwner(DISPLAY(getApp()),XA_PRIMARY)!=xid) return FALSE;
#endif
    if(!getApp()->selectionWindow){
      FXMEMDUP(&getApp()->xselTypeList,FXDragType,types,numtypes);
      getApp()->xselNumTypes=numtypes;
      getApp()->selectionWindow=this;
      getApp()->selectionWindow->handle(this,MKUINT(0,SEL_SELECTION_GAINED),&getApp()->event);
      }
    return TRUE;
    }
  return FALSE;
  }


// Release the selection
FXbool FXWindow::releaseSelection(){
  if(xid){
    if(getApp()->selectionWindow==this){
      getApp()->selectionWindow->handle(this,MKUINT(0,SEL_SELECTION_LOST),&getApp()->event);
#ifndef WIN32
      XSetSelectionOwner(DISPLAY(getApp()),XA_PRIMARY,None,getApp()->event.time);
#endif
      FXFREE(&getApp()->xselTypeList);
      getApp()->xselNumTypes=0;
      getApp()->selectionWindow=NULL;
      return TRUE;
      }
    }
  return FALSE;
  }


/*******************************************************************************/


// Lost the selection
long FXWindow::onClipboardLost(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onClipboardLost %p\n",getClassName(),this));
  if(target) target->handle(this,MKUINT(message,SEL_CLIPBOARD_LOST),ptr);
  return 1;
  }


// Gained the selection
long FXWindow::onClipboardGained(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onClipboardGained %p\n",getClassName(),this));
  if(target) target->handle(this,MKUINT(message,SEL_CLIPBOARD_GAINED),ptr);
  return 1;
  }


// Somebody wants our the selection
long FXWindow::onClipboardRequest(FXObject*,FXSelector,void* ptr){
  FXTRACE((100,"%s::onClipboardRequest %p\n",getClassName(),this));
  if(target && target->handle(this,MKUINT(message,SEL_CLIPBOARD_REQUEST),ptr)) return 1;
  return 0;
  }


// Has this window the selection
FXbool FXWindow::hasClipboard() const {
  return (getApp()->clipboardWindow==this);
  }


// Acquire the clipboard
// We always generate SEL_CLIPBOARD_LOST and SEL_CLIPBOARD_GAINED
// because we assume the clipboard types may have changed, and want
// to give target opportunity to allocate the new data for these types.
FXbool FXWindow::acquireClipboard(const FXDragType *types,FXuint numtypes){
  if(xid){
    if(!types || !numtypes){ fxerror("%s::acquireClipboard: should have at least one type to select.\n",getClassName()); }
#ifndef WIN32
    if(getApp()->clipboardWindow){
      getApp()->clipboardWindow->handle(getApp(),MKUINT(0,SEL_CLIPBOARD_LOST),&getApp()->event);
      getApp()->clipboardWindow=NULL;
      FXFREE(&getApp()->xcbTypeList);
      getApp()->xcbNumTypes=0;
      }
    XSetSelectionOwner(DISPLAY(getApp()),getApp()->xcbSelection,xid,getApp()->event.time);
    if(XGetSelectionOwner(DISPLAY(getApp()),getApp()->xcbSelection)!=xid) return FALSE;
    if(!getApp()->clipboardWindow){
      FXMEMDUP(&getApp()->xcbTypeList,FXDragType,types,numtypes);
      getApp()->xcbNumTypes=numtypes;
      getApp()->clipboardWindow=this;
      getApp()->clipboardWindow->handle(this,MKUINT(0,SEL_CLIPBOARD_GAINED),&getApp()->event);
      }
    return TRUE;
#else
    if(!OpenClipboard((HWND)xid)) return FALSE;
    register FXuint i;
    EmptyClipboard();                   // Will cause SEL_CLIPBOARD_LOST to be sent to owner
    for(i=0; i<numtypes; i++){ SetClipboardData(types[i],NULL); }
    CloseClipboard();
    if(GetClipboardOwner()!=xid) return FALSE;
    getApp()->clipboardWindow=this;
    handle(this,MKUINT(0,SEL_CLIPBOARD_GAINED),&getApp()->event);
    return TRUE;
#endif
    }
  return FALSE;
  }


// Release the clipboard
FXbool FXWindow::releaseClipboard(){
  if(xid){
    if(getApp()->clipboardWindow==this){
#ifndef WIN32
      handle(this,MKUINT(0,SEL_CLIPBOARD_LOST),&getApp()->event);
      XSetSelectionOwner(DISPLAY(getApp()),getApp()->xcbSelection,None,getApp()->event.time);
      FXFREE(&getApp()->xcbTypeList);
      getApp()->xcbNumTypes=0;
#else
      if(OpenClipboard((HWND)xid)){
        EmptyClipboard();               // Will cause SEL_CLIPBOARD_LOST to be sent to this window
        CloseClipboard();
        }
#endif
      getApp()->clipboardWindow=NULL;
      return TRUE;
      }
    }
  return FALSE;
  }


/*******************************************************************************/


// Get pointer location (in window coordinates)
FXint FXWindow::getCursorPosition(FXint& x,FXint& y,FXuint& buttons) const {
  if(xid){
#ifndef WIN32
    Window dum; int rx,ry;
    return XQueryPointer(DISPLAY(getApp()),xid,&dum,&dum,&rx,&ry,&x,&y,&buttons);
#else
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient((HWND)xid,&pt);
    x=pt.x; y=pt.y;
    buttons=fxmodifierkeys();
    return TRUE;
#endif
    }
  return FALSE;
  }


// Set pointer location (in window coordinates)
// Contributed by David Heath <dave@hipgraphics.com>
FXint FXWindow::setCursorPosition(FXint x,FXint y){
  if(xid){
#ifndef WIN32
    XWarpPointer(DISPLAY(getApp()),None,xid,0,0,0,0,x,y);
    return TRUE;
#else
    POINT pt;
    pt.x=x;
    pt.y=y;
    ClientToScreen((HWND)xid,&pt);
    SetCursorPos(pt.x,pt.y);
    return TRUE;
#endif
    }
  return FALSE;
  }


// Update this widget if allowed
long FXWindow::onUpdate(FXObject*,FXSelector,void*){

  FXTRACE((1000,"%s::onUpdate %p\n",getClassName(),this));

  // Do layout
  if(flags&FLAG_DIRTY) layout();

  // Do GUI update
  if(flags&FLAG_UPDATE){

    // No target, so we're done
    if(!target) return 0;

    // Check to see if the target object has been deleted
    if(*((void**)target) == (void*)-1L){fxerror("%s::onUpdate: %p references a deleted target object at %p.\n",getClassName(),this,target);}

    // Ask the target object to update the state of this widget
    return target->handle(this,MKUINT(message,SEL_UPDATE),NULL);
    }
  return 1;
  }


// Propagate window size change
void FXWindow::layout(){
  flags&=~FLAG_DIRTY;
  }


// Propagate layout changes upward
void FXWindow::recalc(){
  if(parent) parent->recalc();
  flags|=FLAG_DIRTY;
  }


// Update dirty rectangle
void FXWindow::update(FXint x,FXint y,FXint w,FXint h){
  if(xid){

    // We toss out rectangles outside the visible area
    if(x>=width || y>=height || x+w<=0 || y+h<=0) return;

    // Intersect with the window
    if(x<0){w+=x;x=0;}
    if(y<0){h+=y;y=0;}
    if(x+w>width){w=width-x;}
    if(y+h>height){h=height-y;}

    // Append the rectangle; it is a synthetic expose event!!
    if(w>0 && h>0){
#ifndef WIN32
      getApp()->addRepaint(xid,x,y,w,h,1);
#else
      RECT r;
      r.left=x;
      r.top=y;
      r.right=x+w;
      r.bottom=y+h;
      InvalidateRect((HWND)xid,&r,TRUE);
#endif
      }
    }
  }


// Update dirty window
void FXWindow::update(){
  update(0,0,width,height);
  }


// If marked but not yet painted, paint the given area
void FXWindow::repaint(FXint x,FXint y,FXint w,FXint h){
  if(xid){

    // We toss out rectangles outside the visible area
    if(x>=width || y>=height || x+w<=0 || y+h<=0) return;

    // Intersect with the window
    if(x<0){w+=x;x=0;}
    if(y<0){h+=y;y=0;}
    if(x+w>width){w=width-x;}
    if(y+h>height){h=height-y;}

    if(w>0 && h>0){
#ifndef WIN32
      getApp()->removeRepaints(xid,x,y,w,h);
#else
      RECT r;
      r.left=x;
      r.top=y;
      r.right=x+w;
      r.bottom=y+h;
      RedrawWindow((HWND)xid,&r,NULL,RDW_UPDATENOW);
#endif
      }
    }
  }


// If marked but not yet painted, paint the entire window
void FXWindow::repaint(){
  repaint(0,0,width,height);
  }


// Move window
void FXWindow::move(FXint x,FXint y){
  FXTRACE((200,"%s::move: x=%d y=%d\n",getClassName(),x,y));
  if((flags&FLAG_DIRTY)||(x!=xpos)||(y!=ypos)){
    xpos=x;
    ypos=y;
    if(xid){

      // Similar as for position(), we have to generate protocol
      // here so as to make the display reflect reality...
#ifndef WIN32
      XMoveWindow(DISPLAY(getApp()),xid,x,y);
#else
      SetWindowPos((HWND)xid,NULL,x,y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif
      if(flags&FLAG_DIRTY) layout();
      }
    }
  }


// Move and resize
void FXWindow::position(FXint x,FXint y,FXint w,FXint h){
  register FXint ow=width;
  register FXint oh=height;
  FXTRACE((200,"%s::position: x=%d y=%d w=%d h=%d\n",getClassName(),x,y,w,h));
  if(w<0) w=0;
  if(h<0) h=0;
  if((flags&FLAG_DIRTY)||(x!=xpos)||(y!=ypos)||(w!=ow)||(h!=oh)){
    xpos=x;
    ypos=y;
    width=w;
    height=h;
    if(xid){

      // Alas, we have to generate some protocol here even if the placement
      // as recorded in the widget hasn't actually changed.  This is because
      // there are ways to change the placement w/o going through position()!
#ifndef WIN32
      if(0<w && 0<h){
        if((flags&FLAG_SHOWN) && (ow<=0 || oh<=0)){
          XMapWindow(DISPLAY(getApp()),xid);
          }
        XMoveResizeWindow(DISPLAY(getApp()),xid,x,y,w,h);
        }
      else if(0<ow && 0<oh){
        XUnmapWindow(DISPLAY(getApp()),xid);
        }
#else
      SetWindowPos((HWND)xid,NULL,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif

      // We don't have to layout the interior of this widget unless
      // the size has changed or it was marked as dirty:- this is
      // a very good optimization as it's applied recursively!
      if((flags&FLAG_DIRTY)||(w!=ow)||(h!=oh)) layout();
      }
    }
  }


// Resize
void FXWindow::resize(FXint w,FXint h){
  register FXint ow=width;
  register FXint oh=height;
  FXTRACE((200,"%s::resize: w=%d h=%d\n",getClassName(),w,h));
  if(w<0) w=0;
  if(h<0) h=0;
  if((flags&FLAG_DIRTY)||(w!=ow)||(h!=oh)){
    width=w;
    height=h;
    if(xid){

      // Similar as for position(), we have to generate protocol here..
#ifndef WIN32
      if(0<w && 0<h){
        if((flags&FLAG_SHOWN) && (ow<=0 || oh<=0)){
          XMapWindow(DISPLAY(getApp()),xid);
          }
        XResizeWindow(DISPLAY(getApp()),xid,w,h);
        }
      else if(0<ow && 0<oh){
        XUnmapWindow(DISPLAY(getApp()),xid);
        }
#else
      SetWindowPos((HWND)xid,NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif

      // And of course the size has changed so layout is needed
      layout();
      }
    }
  }


#ifndef WIN32


// Scroll rectangle x,y,w,h by a shift of dx,dy
void FXWindow::scroll(FXint x,FXint y,FXint w,FXint h,FXint dx,FXint dy){
  if(xid && 0<w && 0<h && (dx || dy)){

    // No overlap:- repaint the whole thing
    if((w<=FXABS(dx)) || (h<=FXABS(dy))){
      getApp()->addRepaint(xid,x,y,w,h,1);
      }

    // Has overlap, so blit contents and repaint the exposed parts
    else{
      FXint  tx,ty,fx,fy,ex,ey,ew,eh;
      XEvent event;

      // Force server to catch up
      XSync(DISPLAY(getApp()),False);

      // Pull any outstanding repaint events into our own repaint rectangle list
      while(XCheckWindowEvent(DISPLAY(getApp()),xid,ExposureMask,&event)){
        if(event.xany.type==NoExpose) continue;
        getApp()->addRepaint(xid,event.xexpose.x,event.xexpose.y,event.xexpose.width,event.xexpose.height,0);
        if(event.xgraphicsexpose.count==0) break;
        }

      // Scroll all repaint rectangles of this window by the dx,dy
      getApp()->scrollRepaints(xid,dx,dy);

      // Compute blitted area
      if(dx>0){             // Content shifted right
        fx=x;
        tx=x+dx;
        ex=x;
        ew=dx;
        }
      else{                 // Content shifted left
        fx=x-dx;
        tx=x;
        ex=x+w+dx;
        ew=-dx;
        }
      if(dy>0){             // Content shifted down
        fy=y;
        ty=y+dy;
        ey=y;
        eh=dy;
        }
      else{                 // Content shifted up
        fy=y-dy;
        ty=y;
        ey=y+h+dy;
        eh=-dy;
        }

      // BLIT the contents
      XCopyArea(DISPLAY(getApp()),xid,xid,(GC)visual->scrollgc,fx,fy,w-ew,h-eh,tx,ty);

      // Post additional rectangles for the uncovered areas
      if(dy){
        getApp()->addRepaint(xid,x,ey,w,eh,1);
        }
      if(dx){
        getApp()->addRepaint(xid,ex,y,ew,h,1);
        }
      }
    }
  }


#else


// Scroll rectangle x,y,w,h by a shift of dx,dy
void FXWindow::scroll(FXint x,FXint y,FXint w,FXint h,FXint dx,FXint dy){
  if(xid && 0<w && 0<h && (dx || dy)){
    RECT rect;
    DWORD flags=SW_INVALIDATE;
    rect.left=x;
    rect.top=y;
    rect.right=x+w;
    rect.bottom=y+h;
    ScrollWindowEx((HWND)xid,dx,dy,&rect,&rect,NULL,NULL,SW_INVALIDATE);
    }
  }

#endif

// Force GUI refresh of this window and all of its children
void FXWindow::forceRefresh(){
  register FXWindow *child;
  handle(this,MKUINT(0,SEL_UPDATE),NULL);
  for(child=first; child; child=child->next){
    child->forceRefresh();
    }
  }


// Show window
void FXWindow::show(){
  if(!(flags&FLAG_SHOWN)){
    flags|=FLAG_SHOWN;
    if(xid){
#ifndef WIN32
      if(0<width && 0<height) XMapWindow(DISPLAY(getApp()),xid);
#else
      ShowWindow((HWND)xid,SW_SHOWNOACTIVATE);
#endif
      }
    }
  }


// Hide window
void FXWindow::hide(){
  if(flags&FLAG_SHOWN){
    killFocus();
    flags&=~FLAG_SHOWN;
    if(xid){
#ifndef WIN32
      if(getApp()->mouseGrabWindow==this){
        XUngrabPointer(DISPLAY(getApp()),CurrentTime);
        XFlush(DISPLAY(getApp()));
        handle(this,MKUINT(0,SEL_UNGRABBED),&getApp()->event);
        getApp()->mouseGrabWindow=NULL;
        }
      if(getApp()->keyboardGrabWindow==this){
        XUngrabKeyboard(DISPLAY(getApp()),getApp()->event.time);
        XFlush(DISPLAY(getApp()));
        getApp()->keyboardGrabWindow=NULL;
        }
      XUnmapWindow(DISPLAY(getApp()),xid);
#else
      if(getApp()->mouseGrabWindow==this){
        ReleaseCapture();
        SetCursor((HCURSOR)defaultCursor->id());
        handle(this,MKUINT(0,SEL_UNGRABBED),&getApp()->event);
        getApp()->mouseGrabWindow=NULL;
        }
      if(getApp()->keyboardGrabWindow==this){
        getApp()->keyboardGrabWindow=NULL;
        }
      ShowWindow((HWND)xid,SW_HIDE);
#endif
      }
    }
  }


// Check if logically shown
FXbool FXWindow::shown() const {
  return (flags&FLAG_SHOWN)!=0;
  }


// Reparent window under a new parent
void FXWindow::reparent(FXWindow* newparent){
  if(newparent==NULL){ fxerror("%s::reparent: NULL parent specified.\n",getClassName()); }
  if(parent==NULL){ fxerror("%s::reparent: cannot reparent root window.\n",getClassName()); }
  if(parent==getRoot() || newparent==getRoot()){ fxerror("%s::reparent: cannot reparent toplevel window.\n",getClassName()); }
  if(newparent!=parent){

    // Check for funny cases
    if(containsChild(newparent)){ fxerror("%s::reparent: new parent is child of window.\n",getClassName()); }

    // Both windows created or both non-created
    if(xid && !newparent->id()){ fxerror("%s::reparent: new parent not created yet.\n",getClassName()); }
    if(!xid && newparent->id()){ fxerror("%s::reparent: window not created yet.\n",getClassName()); }

    // Kill focus chain through this window
    killFocus();

    // Flag old parent as to be recalculated
    parent->recalc();

    // Unlink from old parent
    if(prev) prev->next=next; else parent->first=next;
    if(next) next->prev=prev; else parent->last=prev;

    // Link to new parent
    parent=newparent;
    prev=parent->last;
    next=NULL;
    parent->last=this;
    if(prev) prev->next=this; else parent->first=this;

    // New owner is the new parent
    owner=parent;

    // Hook up to new window in server too
    if(xid && parent->id()){
#ifndef WIN32
      XReparentWindow(DISPLAY(getApp()),xid,parent->id(),0,0);
#else
      SetParent((HWND)xid,(HWND)parent->id());
#endif
      }

    // Flag as to be recalculated
    recalc();
    }
  }


// Enable the window
void FXWindow::enable(){
  if(!(flags&FLAG_ENABLED)){
    flags|=FLAG_ENABLED;
    if(xid){
#ifndef WIN32
      FXuint events=BASIC_EVENT_MASK|ENABLED_EVENT_MASK;
      if(flags&FLAG_SHELL) events|=SHELL_EVENT_MASK;
      XSelectInput(DISPLAY(getApp()),xid,events);
#else
      EnableWindow((HWND)xid,TRUE);
#endif
      }
    }
  }


// Disable the window
void FXWindow::disable(){
  killFocus();
  if(flags&FLAG_ENABLED){
    flags&=~FLAG_ENABLED;
    if(xid){
#ifndef WIN32
      FXuint events=BASIC_EVENT_MASK;
      if(flags&FLAG_SHELL) events|=SHELL_EVENT_MASK;
      XSelectInput(DISPLAY(getApp()),xid,events);
      if(getApp()->mouseGrabWindow==this){
        XUngrabPointer(DISPLAY(getApp()),CurrentTime);
        XFlush(DISPLAY(getApp()));
        handle(this,MKUINT(0,SEL_UNGRABBED),&getApp()->event);
        getApp()->mouseGrabWindow=NULL;
        }
      if(getApp()->keyboardGrabWindow==this){
        XUngrabKeyboard(DISPLAY(getApp()),getApp()->event.time);
        XFlush(DISPLAY(getApp()));
        getApp()->keyboardGrabWindow=NULL;
        }
#else
      EnableWindow((HWND)xid,FALSE);
      if(getApp()->mouseGrabWindow==this){
        ReleaseCapture();
        SetCursor((HCURSOR)defaultCursor->id());
        handle(this,MKUINT(0,SEL_UNGRABBED),&getApp()->event);
        getApp()->mouseGrabWindow=NULL;
        }
      if(getApp()->keyboardGrabWindow==this){
        getApp()->keyboardGrabWindow=NULL;
        }
#endif
      }
    }
  }


// Is window enabled
FXbool FXWindow::isEnabled() const {
  return (flags&FLAG_ENABLED)!=0;
  }


// Raise (but do not activate!)
void FXWindow::raise(){
  if(xid){
#ifndef WIN32
    XRaiseWindow(DISPLAY(getApp()),xid);
#else
    SetWindowPos((HWND)xid,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
#endif
    }
  }


// Lower
void FXWindow::lower(){
  if(xid){
#ifndef WIN32
    XLowerWindow(DISPLAY(getApp()),xid);
#else
    SetWindowPos((HWND)xid,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
#endif
    }
  }


// Get coordinates from another window (for symmetry)
void FXWindow::translateCoordinatesFrom(FXint& tox,FXint& toy,const FXWindow* fromwindow,FXint fromx,FXint fromy) const {
  if(fromwindow==NULL){ fxerror("%s::translateCoordinatesFrom: from-window is NULL.\n",getClassName()); }
  if(xid && fromwindow->id()){
#ifndef WIN32
    Window tmp;
    XTranslateCoordinates(DISPLAY(getApp()),fromwindow->id(),xid,fromx,fromy,&tox,&toy,&tmp);
#else
    POINT pt;
    pt.x=fromx;
    pt.y=fromy;
    ClientToScreen((HWND)fromwindow->id(),&pt);
    ScreenToClient((HWND)xid,&pt);
    tox=pt.x;
    toy=pt.y;
#endif
    }
  }


// Get coordinates to another window (for symmetry)
void FXWindow::translateCoordinatesTo(FXint& tox,FXint& toy,const FXWindow* towindow,FXint fromx,FXint fromy) const {
  if(towindow==NULL){ fxerror("%s::translateCoordinatesTo: to-window is NULL.\n",getClassName()); }
  if(xid && towindow->id()){
#ifndef WIN32
    Window tmp;
    XTranslateCoordinates(DISPLAY(getApp()),xid,towindow->id(),fromx,fromy,&tox,&toy,&tmp);
#else
    POINT pt;
    pt.x=fromx;
    pt.y=fromy;
    ClientToScreen((HWND)xid,&pt);
    ScreenToClient((HWND)towindow->id(),&pt);
    tox=pt.x;
    toy=pt.y;
#endif
    }
  }


/*******************************************************************************/


// Acquire grab
// Grabs also switch to the drag cursor
void FXWindow::grab(){
  if(xid){
    FXTRACE((150,"%s::grab %p\n",getClassName(),this));
    if(dragCursor->id()==0){ fxerror("%s::grab: Cursor has not been created yet.\n",getClassName()); }
    if(!(flags&FLAG_SHOWN)){ fxwarning("%s::grab: Window is not visible.\n",getClassName()); }
#ifndef WIN32
    if(GrabSuccess!=XGrabPointer(DISPLAY(getApp()),xid,FALSE,GRAB_EVENT_MASK,GrabModeAsync,GrabModeAsync,None,dragCursor->id(),getApp()->event.time)){
      fxwarning("%s::grab: grab failed!\n",getClassName());
      }
#else
    SetCapture((HWND)xid);
    if(GetCapture()!=(HWND)xid){
      fxwarning("%s::grab: grab failed!\n",getClassName());
      }
    SetCursor((HCURSOR)dragCursor->id());
#endif
    getApp()->mouseGrabWindow=this;
    }
  }


// Release grab
// Ungrabs also switch back to the normal cursor
void FXWindow::ungrab(){
  if(xid){
    FXTRACE((150,"%s::ungrab %p\n",getClassName(),this));
    getApp()->mouseGrabWindow=NULL;
#ifndef WIN32
    XUngrabPointer(DISPLAY(getApp()),getApp()->event.time);
    XFlush(DISPLAY(getApp()));
#else
    ReleaseCapture();
    SetCursor((HCURSOR)defaultCursor->id());
#endif
    }
  }


// Return true if active grab is in effect
FXbool FXWindow::grabbed() const {
  return getApp()->mouseGrabWindow==this;
  }


// Grab keyboard device
void FXWindow::grabKeyboard(){
  if(xid){
    FXTRACE((150,"%s::grabKeyboard %p\n",getClassName(),this));
    if(!(flags&FLAG_SHOWN)){ fxwarning("%s::ungrabKeyboard: Window is not visible.\n",getClassName()); }
#ifndef WIN32
    XGrabKeyboard(DISPLAY(getApp()),xid,FALSE,GrabModeAsync,GrabModeAsync,getApp()->event.time);
#else
    FXASSERT(0); SetActiveWindow((HWND)xid); // FIXME Check this
#endif
    getApp()->keyboardGrabWindow=this;
    }
  }


// Ungrab keyboard device
void FXWindow::ungrabKeyboard(){
  if(xid){
    FXTRACE((150,"%s::ungrabKeyboard %p\n",getClassName(),this));
    getApp()->keyboardGrabWindow=NULL;
#ifndef WIN32
    XUngrabKeyboard(DISPLAY(getApp()),getApp()->event.time);
#else
#endif
    }
  }


// Return true if active grab is in effect
FXbool FXWindow::grabbedKeyboard() const {
  return getApp()->keyboardGrabWindow==this;
  }


// The widget lost the grab for some reason [Windows].
// Subclasses should try to clean up the mess...
long FXWindow::onUngrabbed(FXObject*,FXSelector,void* ptr){
  FXTRACE((150,"%s::onUngrabbed\n",getClassName()));
  if(target) target->handle(this,MKUINT(message,SEL_UNGRABBED),ptr);
//#ifdef WIN32
//  SetCursor((HCURSOR)defaultCursor->id());    // FIXME Maybe should be done with WM_SETCURSOR?
//#endif
  return 1;
  }


/*******************************************************************************/

// Enable widget as drop target
void FXWindow::dropEnable(){
  flags|=FLAG_DROPTARGET;
  }


// Disable widget as drop target
void FXWindow::dropDisable(){
  flags&=~FLAG_DROPTARGET;
  }


// Is window drop enabled
FXbool FXWindow::isDropEnabled() const {
  return (flags&FLAG_DROPTARGET)!=0;
  }


// Set drag rectangle to block events while inside
void FXWindow::setDragRectangle(FXint x,FXint y,FXint w,FXint h,FXbool wantupdates){
  if(xid==0){ fxerror("%s::setDragRectangle: window has not yet been created.\n",getClassName()); }
#ifndef WIN32
  Window tmp;
  int tox,toy;
  XTranslateCoordinates(DISPLAY(getApp()),xid,XDefaultRootWindow(DISPLAY(getApp())),x,y,&tox,&toy,&tmp);
  getApp()->xdndRect.x=tox;
  getApp()->xdndRect.y=toy;
  getApp()->xdndWantUpdates=wantupdates;
#else
  POINT pt;
  pt.x=x;
  pt.y=y;
  ClientToScreen((HWND)parent->id(),&pt);
  getApp()->xdndRect.x=(short)pt.x;
  getApp()->xdndRect.y=(short)pt.y;
#endif
  getApp()->xdndRect.w=w;
  getApp()->xdndRect.h=h;
  }


// Clear drag rectangle
void FXWindow::clearDragRectangle(){
  if(xid==0){ fxerror("%s::clearDragRectangle: window has not yet been created.\n",getClassName()); }
  getApp()->xdndRect.x=0;
  getApp()->xdndRect.y=0;
  getApp()->xdndRect.w=0;
  getApp()->xdndRect.h=0;
#ifndef WIN32
  getApp()->xdndWantUpdates=TRUE; // Isn't this bullshit?
#endif
  }


// Get dropped data; called in response to DND enter or DND drop
FXbool FXWindow::getDNDData(FXDNDOrigin origin,FXDragType targettype,FXuchar*& data,FXuint& size){
  if(xid==0){ fxerror("%s::getDNDData: window has not yet been created.\n",getClassName()); }
  switch(origin){
    case FROM_DRAGNDROP:
      getApp()->dragdropGetData(this,targettype,data,size);
      break;
    case FROM_CLIPBOARD:
      getApp()->clipboardGetData(this,targettype,data,size);
      break;
    case FROM_SELECTION:
      getApp()->selectionGetData(this,targettype,data,size);
      break;
    }
  return data!=NULL;
  }



// Set drop data; data array will be deleted by the system automatically!
FXbool FXWindow::setDNDData(FXDNDOrigin origin,FXDragType targettype,FXuchar* data,FXuint size){
  if(xid==0){ fxerror("%s::setDNDData: window has not yet been created.\n",getClassName()); }
  switch(origin){
    case FROM_DRAGNDROP:
      getApp()->dragdropSetData(this,targettype,data,size);   // FIXME need to know 8, 16, 32 format
      break;
    case FROM_CLIPBOARD:
      getApp()->clipboardSetData(this,targettype,data,size);
      break;
    case FROM_SELECTION:
      getApp()->selectionSetData(this,targettype,data,size);
      break;
    }
  return TRUE;
  }


// Inquire about types being dragged or available on the clipboard or selection
FXbool FXWindow::inquireDNDTypes(FXDNDOrigin origin,FXDragType*& types,FXuint& numtypes){
  if(xid==0){ fxerror("%s::inquireDNDTypes: window has not yet been created.\n",getClassName()); }
  switch(origin){
    case FROM_DRAGNDROP:
      getApp()->dragdropGetTypes(this,types,numtypes);
      break;
    case FROM_CLIPBOARD:
      getApp()->clipboardGetTypes(this,types,numtypes);
      break;
    case FROM_SELECTION:
      getApp()->selectionGetTypes(this,types,numtypes);
      break;
    }
  return types!=NULL;
  }


// Is a certain type being offered via drag and drop, clipboard, or selection?
FXbool FXWindow::offeredDNDType(FXDNDOrigin origin,FXDragType type){
  if(xid==0){ fxerror("%s::offeredDNDType: window has not yet been created.\n",getClassName()); }
  FXbool offered=FALSE;
  FXDragType *types;
  FXuint i,ntypes;
  if(inquireDNDTypes(origin,types,ntypes)){
    for(i=0; i<ntypes; i++){
      if(type==types[i]){ offered=TRUE; break; }
      }
    FXFREE(&types);
    }
  return offered;
  }


// Target accepts or rejects drop, and may suggest different action
void FXWindow::acceptDrop(FXDragAction action){
  getApp()->ansAction=DRAG_REJECT;
  if(action!=DRAG_REJECT){
    getApp()->ansAction=getApp()->ddeAction;
    if(action!=DRAG_ACCEPT) getApp()->ansAction=action;
    }
  }


// Target wants to know drag action of source
FXDragAction FXWindow::inquireDNDAction() const {
  return getApp()->ddeAction;
  }


// Source wants to find out if target accepted
FXDragAction FXWindow::didAccept() const {
  return getApp()->ansAction;
  }


// True if we're in a drag operation
FXbool FXWindow::isDragging() const {
  return (getApp()->dragWindow==this);
  }


// True if this window is drop target
FXbool FXWindow::isDropTarget() const {
  return (getApp()->dropWindow==this);
  }



// Start a drag on the types mentioned
FXbool FXWindow::beginDrag(const FXDragType *types,FXuint numtypes){
  if(xid==0){ fxerror("%s::beginDrag: window has not yet been created.\n",getClassName()); }
  if(!isDragging()){
    if(types==NULL || numtypes<1){ fxerror("%s::beginDrag: should have at least one type to drag.\n",getClassName()); }
#ifndef WIN32
    XSetSelectionOwner(DISPLAY(getApp()),getApp()->xdndSelection,xid,getApp()->event.time);
    if(XGetSelectionOwner(DISPLAY(getApp()),getApp()->xdndSelection)!=xid){
      fxwarning("%s::beginDrag: failed to acquire DND selection.\n",getClassName());
      return FALSE;
      }
    FXMALLOC(&getApp()->xdndTypeList,FXDragType,numtypes);
    memcpy(getApp()->xdndTypeList,types,sizeof(FXDragType)*numtypes);
    getApp()->xdndNumTypes=numtypes;
    XChangeProperty(DISPLAY(getApp()),xid,getApp()->xdndTypes,XA_ATOM,32,PropModeReplace,(unsigned char*)getApp()->xdndTypeList,getApp()->xdndNumTypes);
    getApp()->xdndTarget=0;
    getApp()->xdndProxyTarget=0;
    getApp()->ansAction=DRAG_REJECT;
    getApp()->xdndStatusPending=FALSE;
    getApp()->xdndStatusReceived=FALSE;
    getApp()->xdndWantUpdates=TRUE;
    getApp()->xdndRect.x=0;
    getApp()->xdndRect.y=0;
    getApp()->xdndRect.w=0;
    getApp()->xdndRect.h=0;
#else
    getApp()->xdndTypes=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,(numtypes+1)*sizeof(FXDragType),"XdndTypeList");
    if(getApp()->xdndTypes){
      FXDragType *dragtypes=(FXDragType*)MapViewOfFile(getApp()->xdndTypes,FILE_MAP_WRITE,0,0,0);
      if(dragtypes){
        dragtypes[0]=numtypes;
        memcpy(&dragtypes[1],types,numtypes*sizeof(FXDragType));
        UnmapViewOfFile(dragtypes);
        }
      }
    getApp()->xdndTarget=0;
    getApp()->ansAction=DRAG_REJECT;
    getApp()->xdndStatusPending=FALSE;
    getApp()->xdndStatusReceived=FALSE;
    getApp()->xdndRect.x=0;
    getApp()->xdndRect.y=0;
    getApp()->xdndRect.w=0;
    getApp()->xdndRect.h=0;
#endif
    getApp()->dragWindow=this;
    return TRUE;
    }
  return FALSE;
  }


// Drag to new position
FXbool FXWindow::handleDrag(FXint x,FXint y,FXDragAction action){
  if(xid==0){ fxerror("%s::handleDrag: window has not yet been created.\n",getClassName()); }
  if(action<DRAG_COPY || DRAG_PRIVATE<action){ fxerror("%s::handleDrag: illegal drag action.\n",getClassName()); }
  if(isDragging()){

#ifndef WIN32

    Window proxywindow,window,child,win,proxywin,root;
    unsigned long ni,ba;
    Window *ptr1;
    Window *ptr2;
    Atom   *ptr3;
    Atom    typ;
    int     fmt;
    Atom    version;
    int     dropx;
    int     dropy;
    XEvent  se;
    FXbool  forcepos=FALSE;

    // Find XDND aware window at the indicated location
    root=XDefaultRootWindow(DISPLAY(getApp()));
    window=0;
    proxywindow=0;
    version=0;
    win=root;
    while(1){
      if(!XTranslateCoordinates(DISPLAY(getApp()),root,win,x,y,&dropx,&dropy,&child)) break;
      proxywin=win;
      if(XGetWindowProperty(DISPLAY(getApp()),win,getApp()->xdndProxy,0,1,False,AnyPropertyType,&typ,&fmt,&ni,&ba,(unsigned char**)&ptr1)==Success){
        if(typ==XA_WINDOW && fmt==32 && ni>0){
          if(XGetWindowProperty(DISPLAY(getApp()),ptr1[0],getApp()->xdndProxy,0,1,False,AnyPropertyType,&typ,&fmt,&ni,&ba,(unsigned char**)&ptr2)==Success){
            if(typ==XA_WINDOW && fmt==32 && ni>0 && ptr2[0] == ptr1[0]){
              proxywin=ptr1[0];
              }
            XFree(ptr2);
            }
          }
        XFree(ptr1);
        }
      if(XGetWindowProperty(DISPLAY(getApp()),proxywin,getApp()->xdndAware,0,1,False,AnyPropertyType,&typ,&fmt,&ni,&ba,(unsigned char**)&ptr3)==Success){
        if(typ==XA_ATOM && fmt==32 && ni>0 && ptr3[0]>=3){
          window=win;
          proxywindow=proxywin;
          version=FXMIN(ptr3[0],XDND_PROTOCOL_VERSION);
          if(window!=root){XFree(ptr3);break;}
          }
        XFree(ptr3);
        }
      if(child==None) break;
      win=child;
      }

    FXTRACE((100,"proxywindow=%d window=%d version=%d\n",proxywindow,window,version));

    // Changed windows
    if(window!=getApp()->xdndTarget){

      // Moving OUT of XDND aware window?
      if(getApp()->xdndTarget!=0){
        FXTRACE((100,"Sending DNDLeave to %d (proxy %d)\n",getApp()->xdndTarget,getApp()->xdndProxyTarget));
        se.xclient.type=ClientMessage;
        se.xclient.display=DISPLAY(getApp());
        se.xclient.message_type=getApp()->xdndLeave;
        se.xclient.format=32;
        se.xclient.window=getApp()->xdndTarget;
        se.xclient.data.l[0]=xid;
        se.xclient.data.l[1]=0;
        se.xclient.data.l[2]=0;
        se.xclient.data.l[3]=0;
        se.xclient.data.l[4]=0;
        XSendEvent(DISPLAY(getApp()),getApp()->xdndProxyTarget,True,NoEventMask,&se);
        }

      // Reset XDND variables
      getApp()->xdndTarget=window;
      getApp()->xdndProxyTarget=proxywindow;
      getApp()->ansAction=DRAG_REJECT;
      getApp()->xdndStatusPending=FALSE;
      getApp()->xdndStatusReceived=FALSE;
      getApp()->xdndWantUpdates=TRUE;
      getApp()->xdndRect.x=x;
      getApp()->xdndRect.y=y;
      getApp()->xdndRect.w=1;
      getApp()->xdndRect.h=1;

      // Moving INTO XDND aware window?
      if(getApp()->xdndTarget!=0){
        FXTRACE((100,"Sending DNDEnter to %d (proxy %d)\n",getApp()->xdndTarget,getApp()->xdndProxyTarget));
        se.xclient.type=ClientMessage;
        se.xclient.display=DISPLAY(getApp());
        se.xclient.message_type=getApp()->xdndEnter;
        se.xclient.format=32;
        se.xclient.window=getApp()->xdndTarget;
        se.xclient.data.l[0]=xid;
        se.xclient.data.l[1]=version<<24;
        se.xclient.data.l[2]=getApp()->xdndNumTypes>=1 ? getApp()->xdndTypeList[0] : None;
        se.xclient.data.l[3]=getApp()->xdndNumTypes>=2 ? getApp()->xdndTypeList[1] : None;
        se.xclient.data.l[4]=getApp()->xdndNumTypes>=3 ? getApp()->xdndTypeList[2] : None;
        if(getApp()->xdndNumTypes>3) se.xclient.data.l[1]|=1;
        XSendEvent(DISPLAY(getApp()),getApp()->xdndProxyTarget,True,NoEventMask,&se);
        forcepos=TRUE;
        }
      }

    // Now send a position message
    if(getApp()->xdndTarget!=0){

      // Send position if we're outside the mouse box or ignoring it
      if(forcepos || getApp()->xdndRect.w==0 || getApp()->xdndRect.h==0 || getApp()->xdndWantUpdates || x<getApp()->xdndRect.x || y<getApp()->xdndRect.y || getApp()->xdndRect.x+getApp()->xdndRect.w<=x || getApp()->xdndRect.y+getApp()->xdndRect.h<=y){

        // No outstanding status message, so send new pos right away
        if(!getApp()->xdndStatusPending){
          FXTRACE((100,"Sending DNDPosition to %d (proxy %d)\n",getApp()->xdndTarget,getApp()->xdndProxyTarget));
          se.xclient.type=ClientMessage;
          se.xclient.display=DISPLAY(getApp());
          se.xclient.message_type=getApp()->xdndPosition;
          se.xclient.format=32;
          se.xclient.window=getApp()->xdndTarget;
          se.xclient.data.l[0]=xid;
          se.xclient.data.l[1]=0;
          se.xclient.data.l[2]=MKUINT(y,x);                               // Coordinates
          se.xclient.data.l[3]=getApp()->event.time;                      // Time stamp
          if(action==DRAG_COPY) se.xclient.data.l[4]=getApp()->xdndActionCopy;
          else if(action==DRAG_MOVE) se.xclient.data.l[4]=getApp()->xdndActionMove;
          else if(action==DRAG_LINK) se.xclient.data.l[4]=getApp()->xdndActionLink;
          else if(action==DRAG_PRIVATE) se.xclient.data.l[4]=getApp()->xdndActionPrivate;
          XSendEvent(DISPLAY(getApp()),getApp()->xdndProxyTarget,True,NoEventMask,&se);
          getApp()->xdndStatusPending=TRUE;       // Waiting for the other app to respond
          }
        }
      }

#else

    FXuint version=0;
    FXbool forcepos=FALSE;
    POINT point;
    HWND window;

    // Get drop window
    point.x=x;
    point.y=y;
    window=WindowFromPoint(point);// FIXME this finds only enabled/visible windows
    while(window){
      version=(FXuint)GetProp(window,(LPCTSTR)MAKELONG(getApp()->xdndAware,0));
      if(version) break;
      window=GetParent(window);
      }

    FXTRACE((100,"window=%d version=%d\n",window,version));

    // Changed windows
    if(window!=getApp()->xdndTarget){

      // Moving OUT of XDND aware window?
      if(getApp()->xdndTarget!=0){
        FXTRACE((100,"Sending DNDLeave to %d\n",getApp()->xdndTarget));
        PostMessage((HWND)getApp()->xdndTarget,WM_DND_LEAVE,0,(LPARAM)xid);
        }

      // Reset XDND variables
      getApp()->xdndTarget=window;
      getApp()->ansAction=DRAG_REJECT;
      getApp()->xdndStatusPending=FALSE;
      getApp()->xdndStatusReceived=FALSE;
      getApp()->xdndRect.x=x;
      getApp()->xdndRect.y=y;
      getApp()->xdndRect.w=1;
      getApp()->xdndRect.h=1;

      // Moving INTO XDND aware window?
      if(getApp()->xdndTarget!=0){
        FXTRACE((100,"Sending DNDEnter to %d\n",getApp()->xdndTarget));
        PostMessage((HWND)getApp()->xdndTarget,WM_DND_ENTER,0,(LPARAM)xid);
        forcepos=TRUE;
        }
      }

    // Now send a position message
    if(getApp()->xdndTarget!=0){

      // Send position if we're outside the mouse box or ignoring it
     if(forcepos || x<getApp()->xdndRect.x || y<getApp()->xdndRect.y || getApp()->xdndRect.x+getApp()->xdndRect.w<=x || getApp()->xdndRect.y+getApp()->xdndRect.h<=y){

        // No outstanding status message, so send new pos right away
        if(!getApp()->xdndStatusPending){
          FXTRACE((100,"Sending DNDPosition to %d\n",getApp()->xdndTarget));
          PostMessage((HWND)getApp()->xdndTarget,WM_DND_POSITION_REJECT+action,MAKELONG(x,y),(LPARAM)xid);
          getApp()->xdndStatusPending=TRUE;       // Waiting for the other app to respond
          }
        }
      }

#endif

    return TRUE;
    }
  return FALSE;
  }


#ifndef WIN32

struct XDNDMatch {
  Atom xdndStatus;
  Atom xdndPosition;
  Atom xdndFinished;
  Atom xdndDrop;
  Atom xdndEnter;
  Atom xdndLeave;
  };


// Scan event queue for XDND messages or selection requests
static Bool matchxdnd(Display*,XEvent* event,XPointer ptr){
  XDNDMatch *m=(XDNDMatch*)ptr;
  return event->type==SelectionRequest || (event->type==ClientMessage && (event->xclient.message_type==m->xdndStatus || event->xclient.message_type==m->xdndPosition || event->xclient.message_type==m->xdndFinished || event->xclient.message_type==m->xdndDrop || event->xclient.message_type==m->xdndEnter || event->xclient.message_type==m->xdndLeave));
  }

#endif


// Terminate the drag; if drop flag is false, don't drop even if accepted.
FXbool FXWindow::endDrag(FXbool drop){
  FXbool dropped=FALSE;
  if(xid==0){ fxerror("%s::endDrag: window has not yet been created.\n",getClassName()); }
  if(isDragging()){

#ifndef WIN32

    FXbool nodrop=TRUE;
    XEvent se;
    FXuint loops;
    XDNDMatch match;

    // Fill up match struct
    match.xdndStatus=getApp()->xdndStatus;
    match.xdndPosition=getApp()->xdndPosition;
    match.xdndFinished=getApp()->xdndFinished;
    match.xdndDrop=getApp()->xdndDrop;
    match.xdndEnter=getApp()->xdndEnter;
    match.xdndLeave=getApp()->xdndLeave;

    // Ever received a status
    if(getApp()->xdndStatusReceived && drop){

      // If a status message is still pending, wait for it
      if(getApp()->xdndStatusPending){
        loops=1000;
        do{
          FXTRACE((100,"Waiting for pending XdndStatus\n"));
          if(XCheckIfEvent(DISPLAY(getApp()),&se,matchxdnd,(char*)&match)){
            getApp()->dispatchEvent(se);

            // We got the status update, finally
            if(se.xclient.type==ClientMessage && se.xclient.message_type==getApp()->xdndStatus){
              FXTRACE((100,"Got XdndStatus\n"));
              getApp()->xdndStatusPending=FALSE;
              break;
              }

            // We got a selection request message, so there is still hope that
            // the target is functioning; lets give it a bit more time...
            if(se.xselection.type==SelectionRequest && se.xselectionrequest.selection==getApp()->xdndSelection){
              FXTRACE((100,"Got SelectionRequest\n"));
              loops=1000;
              }
            }
          fxsleep(10000);
          }
        while(--loops);
        }

      // Got our status message
      if(!getApp()->xdndStatusPending && getApp()->ansAction!=DRAG_REJECT){

        FXTRACE((100,"Sending XdndDrop\n"));
        se.xclient.type=ClientMessage;
        se.xclient.display=DISPLAY(getApp());
        se.xclient.message_type=getApp()->xdndDrop;
        se.xclient.format=32;
        se.xclient.window=getApp()->xdndTarget;
        se.xclient.data.l[0]=xid;
        se.xclient.data.l[1]=0;
        se.xclient.data.l[2]=getApp()->event.time;
        se.xclient.data.l[3]=0;
        se.xclient.data.l[4]=0;
        XSendEvent(DISPLAY(getApp()),getApp()->xdndProxyTarget,True,NoEventMask,&se);

        // Wait until the target has processed the drop; since the
        // target may be us, we keep processing all XDND related messages
        // while we're waiting here.  After some time, we assume the target
        // may have core dumped and we fall out of the loop.....
        loops=1000;
        do{
          FXTRACE((100,"Waiting for XdndFinish\n"));
          if(XCheckIfEvent(DISPLAY(getApp()),&se,matchxdnd,(char*)&match)){
            getApp()->dispatchEvent(se);

            // Got the finish message; we now know the drop has been completed and processed
            if(se.xclient.type==ClientMessage && se.xclient.message_type==getApp()->xdndFinished){
              FXTRACE((100,"Got XdndFinish\n"));
              dropped=TRUE;
              break;
              }

            // We got a selection request message, so there is still hope that
            // the target is functioning; lets give it a bit more time...
            if(se.xselection.type==SelectionRequest && se.xselectionrequest.selection==getApp()->xdndSelection){
              FXTRACE((100,"Got SelectionRequest\n"));
              loops=1000;
              }
            }
          fxsleep(10000);
          }
        while(--loops);

        // We tried a drop
        nodrop=FALSE;
        }
      }

    // Didn't drop, or didn't get any response, so just send a leave
    if(nodrop){
      FXTRACE((100,"Sending XdndLeave\n"));
      se.xclient.type=ClientMessage;
      se.xclient.display=DISPLAY(getApp());
      se.xclient.message_type=getApp()->xdndLeave;
      se.xclient.format=32;
      se.xclient.window=getApp()->xdndTarget;
      se.xclient.data.l[0]=xid;
      se.xclient.data.l[1]=0;
      se.xclient.data.l[2]=0;
      se.xclient.data.l[3]=0;
      se.xclient.data.l[4]=0;
      XSendEvent(DISPLAY(getApp()),getApp()->xdndProxyTarget,True,NoEventMask,&se);
      }

    // Clean up
    XSetSelectionOwner(DISPLAY(getApp()),getApp()->xdndSelection,None,getApp()->event.time);
    XDeleteProperty(DISPLAY(getApp()),xid,getApp()->xdndTypes);
    FXFREE(&getApp()->xdndTypeList);
    getApp()->xdndNumTypes=0;
    getApp()->xdndTarget=0;
    getApp()->xdndProxyTarget=0;
    getApp()->ansAction=DRAG_REJECT;
    getApp()->xdndStatusPending=FALSE;
    getApp()->xdndStatusReceived=FALSE;
    getApp()->xdndWantUpdates=TRUE;
    getApp()->xdndRect.x=0;
    getApp()->xdndRect.y=0;
    getApp()->xdndRect.w=0;
    getApp()->xdndRect.h=0;
    getApp()->dragWindow=NULL;

#else

    FXbool nodrop=TRUE;
    FXuint loops;
    MSG msg;

    // Ever received a status
    if(getApp()->xdndStatusReceived && drop){

      // If a status message is still pending, wait for it
      if(getApp()->xdndStatusPending){
        loops=1000;
        do{
          FXTRACE((100,"Waiting for XdndStatus\n"));
          if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            getApp()->dispatchEvent(msg);

            // We got the status update, finally
            if(WM_DND_STATUS_REJECT<=msg.message && msg.message<=WM_DND_STATUS_PRIVATE){
              FXTRACE((100,"Got XdndStatus\n"));
              getApp()->xdndStatusPending=FALSE;
              break;
              }

            // We got a selection request message, so there is still hope that
            // the target is functioning; lets give it a bit more time...
            if(msg.message==WM_DND_REQUEST){
              FXTRACE((100,"Got SelectionRequest\n"));
              loops=1000;
              }
            }
          fxsleep(10000);
          }
        while(--loops);
        FXTRACE((100,"Waiting for pending XdndStatus\n"));
        getApp()->xdndStatusPending=FALSE;
        }

      // Got our status message
      if(!getApp()->xdndStatusPending && getApp()->ansAction!=DRAG_REJECT){

        FXTRACE((100,"Sending XdndDrop\n"));
        PostMessage((HWND)getApp()->xdndTarget,WM_DND_DROP,0,(LPARAM)xid);

        // Wait until the target has processed the drop; since the
        // target may be us, we keep processing all XDND related messages
        // while we're waiting here.  After some time, we assume the target
        // may have core dumped and we fall out of the loop.....
        loops=1000;
        do{
          FXTRACE((100,"Waiting for XdndFinish\n"));
          if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            getApp()->dispatchEvent(msg);

            // Got the finish message; we now know the drop has been completed and processed
            if(msg.message==WM_DND_FINISH){
              FXTRACE((100,"Got XdndFinish\n"));
              dropped=TRUE;
              break;
              }

            // We got a selection request message, so there is still hope that
            // the target is functioning; lets give it a bit more time...
            if(msg.message==WM_DND_REQUEST){
              FXTRACE((100,"Got SelectionRequest\n"));
              loops=1000;
              }
            }
          fxsleep(10000);
          }
        while(--loops);

        // We tried a drop
        nodrop=FALSE;
        }
      }

    // Didn't drop, or didn't get any response, so just send a leave
    if(nodrop){
      FXTRACE((100,"Sending XdndLeave\n"));
      PostMessage((HWND)getApp()->xdndTarget,WM_DND_LEAVE,0,(LPARAM)xid);
      }


    // Get rid of the shared memory block containing the types list
    if(getApp()->xdndTypes) CloseHandle(getApp()->xdndTypes);
    getApp()->xdndTypes=NULL;

    // Clean up
    getApp()->xdndTarget=0;
    getApp()->ansAction=DRAG_REJECT;
    getApp()->xdndStatusPending=FALSE;
    getApp()->xdndStatusReceived=FALSE;
    getApp()->xdndRect.x=0;
    getApp()->xdndRect.y=0;
    getApp()->xdndRect.w=0;
    getApp()->xdndRect.h=0;
    getApp()->dragWindow=NULL;

#endif
    }
  return dropped;
  }


/*******************************************************************************/


// Delete window
FXWindow::~FXWindow(){
  FXTRACE((100,"FXWindow::~FXWindow %p\n",this));
  delete accelTable;
  if(prev) prev->next=next; else if(parent) parent->first=next;
  if(next) next->prev=prev; else if(parent) parent->last=prev;
  if(parent && parent->focus==this) parent->focus=NULL;
  if(getApp()->focusWindow==this) getApp()->focusWindow=NULL;
  if(getApp()->cursorWindow==this) getApp()->cursorWindow=parent;
  if(getApp()->mouseGrabWindow==this) getApp()->mouseGrabWindow=NULL;
  if(getApp()->keyboardGrabWindow==this) getApp()->keyboardGrabWindow=NULL;
  if(getApp()->keyWindow==this) getApp()->keyWindow=NULL;
  if(getApp()->selectionWindow==this) getApp()->selectionWindow=NULL;
  if(getApp()->clipboardWindow==this) getApp()->clipboardWindow=NULL;
  if(getApp()->dragWindow==this) getApp()->dragWindow=NULL;
  if(getApp()->dropWindow==this) getApp()->dropWindow=NULL;
  if(getApp()->refresher==this) getApp()->refresher=parent;
  if(parent) parent->recalc();
  destroy();
  parent=(FXWindow*)-1;
  owner=(FXWindow*)-1;
  first=last=(FXWindow*)-1;
  next=prev=(FXWindow*)-1;
  focus=(FXWindow*)-1;
  defaultCursor=(FXCursor*)-1;
  dragCursor=(FXCursor*)-1;
  accelTable=(FXAccelTable*)-1;
  target=(FXObject*)-1;
  }

