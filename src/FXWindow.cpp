/********************************************************************************
*                                                                               *
*                            W i n d o w   O b j e c t                          *
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
* $Id: FXWindow.cpp,v 1.104 1998/10/30 15:49:39 jvz Exp $                    *
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
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"


/*
 To Do:
  - Forward various messages to target
  - Look at destroy(); cannot call overload from base class's destructor!
  - Add virtual overloads for colors (as colors depend on map!)
  - When window is disabled, it should loose focus
  - When no subclass handles SEL_SELECTION_REQUEST, send back None property here
  - The jury on a special FXColormap class is still out.
  - Add URL jump text also.
  - Update should only happen if widget is not in some sort of transaction.
  - Not every widget needs help and tip data; move this down to buttons etc.
  - Should sole modifier keys be ignored before attempting handling through acceltable?
  - Need clipboard support too.
  - Should we catch & translate Escape key & turn it into a SEL_CANCEL message?
  - Should window have a pointer to the main (shell-) window, e.g. for popups, transientfors...?
  - Default constructors [for serialization] should:
      - Initialize dynamic member variables same as regular constructor.
        Dynamic variables are those that are not saved during serialization.
      - Initialize non-dynamic variables with ``garbage.''  Those variables
        are saved during serialization, and should be loaded back also.
  - We initialize/deinitialize with ``garbage'' to bring latent software bugs
    in the application codes to light as early as possible; this is a permanent
    feature in FOX, not just for compiles in debug mode.
  - If FLAG_DIRTY gets reset at the END of layout(), it will be safe to have
    show() and hide() to call recalc(), in case they get called from the
    application code.
  - Need some sort of journal facility.
  - Use INCR mechanism if it gets large.
  - Consolidate FXWindow and FXApp DND stuff into one place, if possible.
  - getDNDType() may be phased out since we pass the ddeTarget type in the ptr part of
    the callbacks
  - There is some merit to choose a handle- or standin-object as the sender in a
    DDE selection request.  This is so we can send it messages.  This handler would
    encapsulate the data transfer protocols between two apps.
  - Change: passModalEvents() should not rely on FXPopups only; perhaps a flag?
  - When someone requests a selection with target ddeDelete, the selection should
    be deleted!
*/

// Basic events
#define BASIC_EVENT_MASK   (ExposureMask|StructureNotifyMask|PropertyChangeMask|EnterWindowMask|LeaveWindowMask|VisibilityChangeMask|KeyPressMask|KeyReleaseMask)

// Additional events for shell widget events
#define SHELL_EVENT_MASK   (FocusChangeMask|StructureNotifyMask)

// Additional events for enabled widgets        
#define ENABLED_EVENT_MASK (ButtonPressMask|ButtonReleaseMask|PointerMotionMask)

// These events are grabbed for mouse grabs
#define GRAB_EVENT_MASK    (ButtonPressMask|ButtonReleaseMask|PointerMotionMask|EnterWindowMask|LeaveWindowMask|VisibilityChangeMask)


/*******************************************************************************/

// Map
FXDEFMAP(FXWindow) FXWindowMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXWindow::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXWindow::onUpdate),
  FXMAPFUNC(SEL_ENTER,0,FXWindow::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXWindow::onLeave),
  FXMAPFUNC(SEL_DESTROY,0,FXWindow::onDestroy),
  FXMAPFUNC(SEL_VISIBILITY,0,FXWindow::onVisibility),
  FXMAPFUNC(SEL_CONFIGURE,0,FXWindow::onConfigure),
  FXMAPFUNC(SEL_MAP,0,FXWindow::onMap),
  FXMAPFUNC(SEL_UNMAP,0,FXWindow::onUnmap),
  FXMAPFUNC(SEL_FOCUSIN,0,FXWindow::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXWindow::onFocusOut),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXWindow::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXWindow::onSelectionGained),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXWindow::onSelectionRequest),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXWindow::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXWindow::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXWindow::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXWindow::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXWindow::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXWindow::onRightBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXWindow::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXWindow::onKeyRelease),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXWindow::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXWindow::onHotKeyRelease),
  FXMAPFUNC(SEL_DND_ENTER,0,FXWindow::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXWindow::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXWindow::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXWindow::onDNDMotion),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SHOW,FXWindow::onCmdShow),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_HIDE,FXWindow::onCmdHide),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_TOGGLESHOWN,FXWindow::onCmdToggleShown),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_TOGGLESHOWN,FXWindow::onUpdToggleShown),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_RAISE,FXWindow::onCmdRaise),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_LOWER,FXWindow::onCmdLower),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_ENABLE,FXWindow::onCmdEnable),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_DISABLE,FXWindow::onCmdDisable),
  };


// Object implementation
FXIMPLEMENT(FXWindow,FXDrawable,FXWindowMap,ARRAYNUMBER(FXWindowMap))

/*******************************************************************************/

  
// For deserialization
FXWindow::FXWindow(){
  parent=(FXWindow*)-1;
  shell=(FXWindow*)-1;
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
  }


// Only used for the Root Window
FXWindow::FXWindow(FXApp* a):FXDrawable(a,1,1,0){
  parent=NULL;
  shell=this;
  first=last=NULL;
  next=prev=NULL;
  focus=NULL;
  target=NULL;
  message=0;
  defaultCursor=getApp()->arrowCursor;
  dragCursor=getApp()->arrowCursor;
  accelTable=NULL;
  xpos=0;
  ypos=0;
  backColor=0;
  flags=FLAG_DIRTY|FLAG_SHOWN|FLAG_MAPPED|FLAG_UPDATE|FLAG_RECALC;
  options=LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT;
  }


// This constructor is used for Shell Windows
FXWindow::FXWindow(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXDrawable(a,w,h,0){
  parent=a->root;
  shell=this;
  first=last=NULL;
  prev=parent->last;
  next=NULL;
  parent->last=this;
  if(prev) prev->next=this; else parent->first=this;
  focus=NULL;
  target=NULL;
  message=0;
  defaultCursor=getApp()->arrowCursor;
  dragCursor=getApp()->arrowCursor;
  accelTable=NULL;
  xpos=x;
  ypos=y;
  backColor=0;
  flags=FLAG_DIRTY|FLAG_UPDATE|FLAG_RECALC;
  options=opts;
  }


// This constructor is used for all child-windows
FXWindow::FXWindow(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXDrawable(p->getApp(),w,h,0){
  parent=p;
  shell=p->getShell();
  first=last=NULL;
  prev=parent->last;
  next=NULL;
  parent->last=this;
  if(prev) prev->next=this; else parent->first=this;
  focus=NULL;
  target=NULL;
  message=0;
  defaultCursor=getApp()->arrowCursor;
  dragCursor=getApp()->arrowCursor;
  accelTable=NULL;
  xpos=x;
  ypos=y;
  backColor=0;
  flags=FLAG_DIRTY|FLAG_UPDATE|FLAG_RECALC;
  options=opts;
  }


/*******************************************************************************/


// Save data
void FXWindow::save(FXStream& store) const {
  FXDrawable::save(store);
  store << parent;
  store << shell;
  store << first;
  store << last;
  store << next;
  store << prev;
  store << focus;
  store << defaultCursor;
  store << dragCursor;
  store << accelTable;
  store << target;/////How to resolve?
  store << message;
  store << xpos;
  store << ypos;
  store << options;
  }


// Load data
void FXWindow::load(FXStream& store){ 
  FXDrawable::load(store);
  store >> parent;
  store >> shell;
  store >> first;
  store >> last;
  store >> next;
  store >> prev;
  store >> focus;
  store >> defaultCursor;
  store >> dragCursor;
  store >> accelTable;
  store >> target;/////How to resolve?
  store >> message;
  store >> xpos;
  store >> ypos;
  store >> options;
  }


/*******************************************************************************/


// Handle repaint 
long FXWindow::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  clearArea(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  return 1;
  }


// Keep track of visibility changes
long FXWindow::onVisibility(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  flags&=~(FLAG_OBSCURED|FLAG_INVISIBLE);
  if(ev->code==VISIBILITYPARTIAL) flags|=FLAG_OBSCURED;
  if(ev->code==VISIBILITYNONE) flags|=(FLAG_OBSCURED|FLAG_INVISIBLE);
  return 1;
  }


// Keep track of mapped state
long FXWindow::onMap(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onMap %lx\n",getClassName(),this);
  flags|=FLAG_MAPPED;
  return 0;
  }


// Keep track of mapped state
long FXWindow::onUnmap(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onUnmap %lx\n",getClassName(),this);
  if(getApp()->grabWindow==this) getApp()->grabWindow=NULL;
  flags&=~FLAG_MAPPED;
  return 0;
  }


// Handle configure notify
long FXWindow::onConfigure(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onConfigure %04x\n",getClassName(),this);
  return 0;
  }


// Handle destroy event
long FXWindow::onDestroy(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onDestroy %04x\n",getClassName(),this);
  XDeleteContext(getDisplay(),xid,getApp()->wcontext);
  if(getApp()->grabWindow==this) getApp()->grabWindow=NULL;
  flags&=~(FLAG_MAPPED|FLAG_SHOWN|FLAG_FOCUSED);
  xid=0;
  return 1;
  }


// Hot key combination pressed
long FXWindow::onHotKeyPress(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onHotKeyPress %08x\n",getClassName(),this);
  flags&=~FLAG_TIP;     // Kill tip
  if(isEnabled()){
    if(canFocus()) setFocus();
    return handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    }
  return 0;
  }


// Hot key combination released
long FXWindow::onHotKeyRelease(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onHotKeyRelease %08x\n",getClassName(),this);
  flags&=~FLAG_TIP;
  if(isEnabled()){
    return handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    }
  return 0;
  }


// Handle incoming button events; generally, we give the target a 
// first shot at it; if it doesn't handle it, we'll bounce it around
// as an activate/deactivate event to this widget, after we
// set the focus to this widget!
long FXWindow::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    return handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    }
  return 0;
  }

long FXWindow::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    return handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    }
  return 0;
  }

long FXWindow::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    return 1;
    }
  return 0;
  }

long FXWindow::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }

long FXWindow::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    return 1;
    }
  return 0;
  }

long FXWindow::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    return handle(this,MKUINT(ID_QUERY_MENU,SEL_COMMAND),ptr);
    }
  return 0;
  }


// Keyboard press; forward to focus child
long FXWindow::onKeyPress(FXObject*,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
//fprintf(stderr,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state);
  
  // Kill the tip
  flags&=~FLAG_TIP;
  
  // Try bounce to the target first
  if(!getFocus() && isEnabled() && target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
  
  // Next, try current focus widget
  if(getFocus() && getFocus()->handle(focus,sel,ptr)) return 1;
  
  XAutoRepeatOff(getDisplay());
  
  // Next, check the accelerators...
  if(getAccelTable() && getAccelTable()->handle(this,sel,ptr)) return 1;
  
  // Otherwise, perform the routine keyboard processing; ignore modifiers
  switch(event->code){
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
    case KEY_space:
    case KEY_KP_Enter:
    case KEY_Return:
      if(isEnabled() && handle(this,MKUINT(0,SEL_ACTIVATE),ptr)) return 1;
      break;
    case KEY_Tab:
      if(event->state&SHIFTMASK) goto prv;
    case KEY_Next:
      return handle(this,MKUINT(0,SEL_FOCUS_NEXT),ptr);
    case KEY_Prior: 
    case KEY_ISO_Left_Tab:
prv:  return handle(this,MKUINT(0,SEL_FOCUS_PREV),ptr);
    case KEY_Up:    
      return handle(this,MKUINT(0,SEL_FOCUS_UP),ptr);
    case KEY_Down:  
      return handle(this,MKUINT(0,SEL_FOCUS_DOWN),ptr);
    case KEY_Left:  
      return handle(this,MKUINT(0,SEL_FOCUS_LEFT),ptr);
    case KEY_Right: 
      return handle(this,MKUINT(0,SEL_FOCUS_RIGHT),ptr);
    case KEY_Home:
      return handle(this,MKUINT(0,SEL_FOCUS_HOME),ptr);
    case KEY_End:
      return handle(this,MKUINT(0,SEL_FOCUS_END),ptr);
    }
  return 0;
  }


// Keyboard release; sent to focus widget
long FXWindow::onKeyRelease(FXObject*,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
//fprintf(stderr,"%s::onKeyRelease keysym=0x%04x\n",getClassName(),event->code);
  
  // Try bounce to the target first
  if(!getFocus() && isEnabled() && target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;

  // Next bounce to focus widget
  if(getFocus() && getFocus()->handle(focus,sel,ptr)) return 1;
  
  XAutoRepeatOn(getDisplay());
  
  // Next, check the accelerators...
  if(getAccelTable() && getAccelTable()->handle(this,sel,ptr)) return 1;

  // Otherwise, perform the routine keyboard processing
  switch(event->code){
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
    case KEY_space:
    case KEY_KP_Enter:
    case KEY_Return:
      if(isEnabled() && handle(this,MKUINT(0,SEL_DEACTIVATE),ptr)) return 1;
      break;
    }
  return 0;
  }


// Entering window
long FXWindow::onEnter(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
//fprintf(stderr,"%s::onEnter %08x\n",getClassName(),this);
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_ENTER),ptr)) return 1;
    }
  getApp()->cursorWindow=this;
  flags|=(FLAG_CURSOR|FLAG_HELP);
  if(event->state==0) flags|=FLAG_TIP;
  return 1;
  }


// Leaving window
long FXWindow::onLeave(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
//fprintf(stderr,"%s::onLeave %08x\n",getClassName(),this);
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEAVE),ptr)) return 1;
    }
  getApp()->cursorWindow=NULL;
  flags&=~(FLAG_CURSOR|FLAG_TIP|FLAG_HELP);
  return 1;
  }


// True if window under the cursor
FXbool FXWindow::underCursor() const {
  return (flags&FLAG_CURSOR)!=0;
  }


// Gained focus
long FXWindow::onFocusIn(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onFocusIn %08x\n",getClassName(),this);
  if(focus) focus->handle(focus,MKUINT(0,SEL_FOCUSIN),NULL);
  if(target) target->handle(this,MKUINT(message,SEL_FOCUSIN),ptr);
  flags|=FLAG_FOCUSED;
  return 1;
  }


// Lost focus
long FXWindow::onFocusOut(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onFocusOut %08x\n",getClassName(),this);
  if(focus) focus->handle(focus,MKUINT(0,SEL_FOCUSOUT),NULL);
  if(target) target->handle(this,MKUINT(message,SEL_FOCUSOUT),ptr);
  flags&=~FLAG_FOCUSED;
  return 1;
  }


// Handle drag-and-drop enter
long FXWindow::onDNDEnter(FXObject*,FXSelector,void* ptr){
  if(isDropEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_DND_ENTER),ptr)) return 1;
    }
  return 0;
  }


// Handle drag-and-drop leave
long FXWindow::onDNDLeave(FXObject*,FXSelector,void* ptr){
  if(isDropEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_DND_LEAVE),ptr)) return 1;
    }
  return 0;
  }


// Handle drag-and-drop motion
long FXWindow::onDNDMotion(FXObject*,FXSelector,void* ptr){
  if(isDropEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_DND_MOTION),ptr)) return 1;
    }
  return 0;
  }


// Handle drag-and-drop drop
long FXWindow::onDNDDrop(FXObject*,FXSelector,void* ptr){
  if(isDropEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_DND_DROP),ptr)) return 1;
    }
  return 0;
  }


// Show window
long FXWindow::onCmdShow(FXObject*,FXSelector,void*){ 
  show(); recalc(); 
  return 1; 
  }


// Hide window
long FXWindow::onCmdHide(FXObject*,FXSelector,void*){ 
  hide(); 
  recalc(); 
  return 1; 
  }

// Hide or show window
long FXWindow::onCmdToggleShown(FXObject*,FXSelector,void*){ 
  shown() ? hide() : show(); 
  recalc(); 
  return 1;
  }


// Update hide or show window 
long FXWindow::onUpdToggleShown(FXObject* sender,FXSelector,void* ptr){ 
  FXuint msg=shown() ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
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


// Returns true; could be used for GUI updating in connection
// with auto-gray or auto-hide options.
long FXWindow::onUpdYes(FXObject*,FXSelector,void*){ 
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
  if(shell==this) return;
  if(parent->focus==this){
    if(focus) focus->killFocus();
    }
  else{
    if(parent->focus) 
      parent->focus->killFocus();
    else
      parent->setFocus();
    parent->focus=this;
    if(parent->hasFocus()) handle(this,MKUINT(0,SEL_FOCUSIN),NULL);
    }
  getApp()->focusWindow=this;
  flags|=FLAG_HELP;
  }


// Kill focus to this widget.
void FXWindow::killFocus(){
  if(shell==this) return;
  if(parent->focus!=this) return;
  if(focus) focus->killFocus();
  if(hasFocus()) handle(this,MKUINT(0,SEL_FOCUSOUT),NULL);
  parent->focus=NULL;
  getApp()->focusWindow=NULL;
  flags&=~FLAG_HELP;
  }


/*******************************************************************************/


// Create X window
void FXWindow::create(){ 
  if(!xid){
    XSetWindowAttributes wattr;
    unsigned long mask;
    Visual* visual;
    
    // Gotta have display open!
    if(!getDisplay()){ fxerror("%s::create: trying to create window before opening display.\n",getClassName()); }
  
    // Gotta have a parent already created!
    if(!parent->id()){ fxerror("%s::create: trying to create window before creating parent window.\n",getClassName()); }
      
    // Have created default cursor?
    if(!defaultCursor->id()){ fxerror("%s::create(): default cursor has not yet been created.\n",getClassName()); }
  
    // Have created drag cursor?
    if(!dragCursor->id()){ fxerror("%s::create(): drag cursor has not yet been created.\n",getClassName()); }
  
    // Fill in the attributes
    mask=CWBackPixel|CWWinGravity|CWBitGravity|CWBorderPixel|CWEventMask|CWDontPropagate|CWCursor|CWOverrideRedirect|CWSaveUnder|CWColormap;
    
    // Events for normal windows
    wattr.event_mask=BASIC_EVENT_MASK;
    
    // Events for shell windows
    if(shell==this) wattr.event_mask|=SHELL_EVENT_MASK;
    
    // If enabled, turn on some more events
    if(flags&FLAG_ENABLED) wattr.event_mask|=ENABLED_EVENT_MASK;
    
    // FOX will not propagate events to ancestor windows
    wattr.do_not_propagate_mask=0;////////////////// Add all events here /////////

    // Determine visual first
    visual=getDefaultVisual();
    
    // Next, determine depth
    depth=getDefaultDepth();
    
    // Next, determine colormap
    wattr.colormap=getDefaultColormap();
    
    // Next, the background color; it should be allocated from the new colormap
    backColor=acquireColor(getApp()->backColor);
    wattr.background_pixel=backColor;
    
    // This is needed for OpenGL
    wattr.border_pixel=0;
    
    // No background
    wattr.background_pixmap=None;
    
    // We don't seem to be able to do ForgetGravity for win_gravity
    // Its the same as UnmapGravity, which is useless...
    wattr.bit_gravity=ForgetGravity;
    wattr.win_gravity=NorthWestGravity;
    
    // Determine override redirect
    wattr.override_redirect=doesOverrideRedirect();
    
    // Determine save-unders
    wattr.save_under=doesSaveUnder();
  
    // Set cursor  
    wattr.cursor=defaultCursor->id();
    
    // Finally, create the window
    xid=XCreateWindow(getDisplay(),parent->id(),xpos,ypos,FXMAX(width,1),FXMAX(height,1),0,depth,InputOutput,visual,mask,&wattr);
    
    // Uh-oh, we failed
    if(!xid){ fxerror("%s::create: unable to create window.\n",getClassName()); }
  
    // Store for xid to C++ object mapping
    XSaveContext(getDisplay(),xid,getApp()->wcontext,(XPointer)this);
    }
  }


// Destroy
void FXWindow::destroy(){
  if(xid){
    
    // Delete the window
    XDestroyWindow(getDisplay(),xid);
    
    // Remove from xid to C++ object mapping
    XDeleteContext(getDisplay(),xid,getApp()->wcontext);
    
    // No longer grabbed
    if(getApp()->grabWindow==this) getApp()->grabWindow=NULL;
    
    // No more window either
    xid=0;
    }
  }


/*******************************************************************************/

// Get default width
FXint FXWindow::getDefaultWidth(){ 
  return 1; 
  }


// Get default height
FXint FXWindow::getDefaultHeight(){ 
  return 1; 
  }


// Window is the default target
FXbool FXWindow::isDefault() const { 
  return (flags&FLAG_DEFAULT)!=0; 
  }


// Test if active
FXbool FXWindow::isActive() const {
  return (flags&FLAG_ACTIVE)!=0;
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
  width=FXMAX(w,1); 
  recalc();
  }


// Set height
void FXWindow::setHeight(FXint h){ 
  height=FXMAX(h,1); 
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


// Return visual
Visual* FXWindow::getDefaultVisual(){ 
  return DefaultVisual(getDisplay(),DefaultScreen(getDisplay())); 
  }


// Get depth
FXint FXWindow::getDefaultDepth(){
  return DefaultDepth(getDisplay(),DefaultScreen(getDisplay()));
  }


// Return colormap
FXID FXWindow::getDefaultColormap(){ 
  return DefaultColormap(getDisplay(),DefaultScreen(getDisplay())); 
  }


// Window does override-redirect
FXbool FXWindow::doesOverrideRedirect() const { 
  return 0; 
  }


// Window does save-unders
FXbool FXWindow::doesSaveUnder() const { 
  return 0; 
  }


// Modal events to this window should be passed
FXbool FXWindow::passModalEvents() const {
  // We're assuming [for now] only popups do save-unders!!!
  return !getApp()->invocation || getApp()->invocation->window==shell->xid || shell->doesSaveUnder();
  }


// Add hot key to closest ancestor's accelerator table
void FXWindow::addHotKey(FXHotKey code){
  FXAccelTable *accel; FXWindow *win=this;
  while((accel=win->getAccelTable())==NULL && win!=shell) win=win->parent;
  if(accel) accel->addAccel(code,this,MKUINT(ID_HOTKEY,SEL_KEYPRESS),MKUINT(ID_HOTKEY,SEL_KEYRELEASE));
  }


// Remove hot key from closest ancestor's accelerator table
void FXWindow::remHotKey(FXHotKey code){
  FXAccelTable *accel; FXWindow *win=this;
  while((accel=win->getAccelTable())==NULL && win!=shell) win=win->parent;
  if(accel) accel->removeAccel(code);
  }


// Check if child is related
FXbool FXWindow::containsChild(const FXWindow* child) const {
  while(child){ 
    if(child==this) return 1;
    child=child->parent;
    }
  return 0;
  }


// Check if window is related to parent window
FXbool FXWindow::isChildOf(const FXWindow* window) const {
  const FXWindow* w=this;
  while(w){
    if(w==window) return 1;
    w=w->parent;
    }
  return 0;
  }


// Get child at x,y
FXWindow* FXWindow::getChildAt(FXint x,FXint y) const {
  register FXWindow* child;
  for(child=first; child; child=child->next){
    if(child->shown() && child->FXWindow::contains(x,y)) return child;
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



// Clear rectangle of window to background
void FXWindow::clearArea(FXint x,FXint y,FXint w,FXint h){
  if(!xid){ fxerror("%s::clearArea: window has not yet been created\n.",getClassName()); }
  XClearArea(getDisplay(),xid,x,y,w,h,False);
  }


// Clear whole window to background
void FXWindow::clearWindow(){
  if(!xid){ fxerror("%s::clearWindow: window has not yet been created\n.",getClassName()); }
  XClearWindow(getDisplay(),xid);
  }


/*******************************************************************************/

// Add this window to the list of colormap windows
void FXWindow::addColormapWindows(){
  Window windows[2],*windowsReturn,*windowList;
  int countReturn;
  
  // Check to see if there is already a property
  Status status=XGetWMColormapWindows(getDisplay(),shell->id(),&windowsReturn,&countReturn);

  // If no property, just create one
  if(!status){
    windows[0]=id();
    windows[1]=getShell()->id();
    XSetWMColormapWindows(getDisplay(),getShell()->id(),windows,2);
    }

  // There was a property, add myself to the beginning
  else{
    windowList=(Window*)malloc((sizeof(Window))*(countReturn+1));
    windowList[0]=id();
    for(int i=0; i<countReturn; i++) windowList[i+1]=windowsReturn[i];
    XSetWMColormapWindows(getDisplay(),getShell()->id(),windowList,countReturn+1);
    XFree((char*)windowsReturn);
    free(windowList);
    }
  }


// Remove it from colormap windows
void FXWindow::removeColormapWindows(){
  Window *windowsReturn;
  int countReturn;
  Status status=XGetWMColormapWindows(getDisplay(),getShell()->id(),&windowsReturn,&countReturn);
  if(status){
    for(int i=0; i<countReturn; i++){
      if(windowsReturn[i]==id()){
        for(i++; i<countReturn; i++) windowsReturn[i-1]=windowsReturn[i];
        XSetWMColormapWindows(getDisplay(),getShell()->id(),windowsReturn,countReturn-1);
        break;
        }
      }
    XFree((char*)windowsReturn);
    }
  }


/*******************************************************************************/


// Set cursor
void FXWindow::setDefaultCursor(FXCursor* cur){
  if(cur==NULL){ fxerror("%s::setDefaultCursor: NULL cursor argument.\n",getClassName()); }
  if(xid){
    if(cur->id()==0){ fxerror("%s::setDefaultCursor: Cursor has not been created yet.\n",getClassName()); }
    XDefineCursor(getDisplay(),xid,cur->id());
    }
  defaultCursor=cur;
  }


// Set drag cursor
void FXWindow::setDragCursor(FXCursor* cur){
  if(cur==NULL){ fxerror("%s::setDragCursor: NULL cursor argument.\n",getClassName()); }
  if(xid){
    if(cur->id()==0){ fxerror("%s::setDragCursor: Cursor has not been created yet.\n",getClassName()); }
    if(grabbed()){ XChangeActivePointerGrab(getDisplay(),GRAB_EVENT_MASK,cur->id(),CurrentTime); }
    }
  dragCursor=cur;
  }


/*******************************************************************************/


// Unfailingly allocate a color
FXPixel FXWindow::acquireColor(FXColor clr){
  FXint i,dr,dg,db;
  FXuint dist,mindist,bestmatch,graym;
  XColor table[256],color;
  Visual *visual;
  Colormap colormap;
  
  // Test if display open
  if(!getDisplay()){ fxerror("%s::acquireColor: trying to create color before opening display.\n",getClassName()); }

  // Get visual
  visual=getDefaultVisual();
  
  // Get colormap
  colormap=getDefaultColormap();
  
  switch(visual->c_class){
    
    // True color
    case TrueColor:
      color.pixel=(visual->red_mask & ((visual->red_mask*FXREDVAL(clr))/255)) | 
                  (visual->green_mask & ((visual->green_mask*FXGREENVAL(clr))/255)) | 
                  (visual->blue_mask & ((visual->blue_mask*FXBLUEVAL(clr))/255));
//fprintf(stderr,"acquireColor(%06x) -> True %06x\n",clr,color.pixel);
      break;
   
    // Gray ramp
    case StaticGray:
      graym=(1<<visual->bits_per_rgb)-1;
      color.pixel=(graym*(77*FXREDVAL(clr)+152*FXGREENVAL(clr)+28*FXBLUEVAL(clr)))/65535;
//fprintf(stderr,"acquireColor(%06x) -> Gray %06x\n",clr,color.pixel);
      break;

    // Changable map; need to allocate it so it won't be changed by other client
    default:
  
      // If this turns out to be a performance bottleneck
      // we should cache (color+colormap->pixel) mappings.
      color.red=FXREDVAL(clr)*257;
      color.green=FXGREENVAL(clr)*257;
      color.blue=FXBLUEVAL(clr)*257;

      // First try just using XAllocColor 
      if(XAllocColor(getDisplay(),colormap,&color)==0){

        // Test just to make sure
        if(visual->map_entries>256) fxerror("%s::makeColor: bigger colormap than expected.\n",getClassName());

        // Get colors in the map
        for(i=0; i<visual->map_entries; i++) table[i].pixel=i;
        XQueryColors(getDisplay(),colormap,table,visual->map_entries);

        // Find best (weighted) match; also gives good colors for grey-ramps
        for(mindist=4294967295U,bestmatch=0,i=0; i<visual->map_entries; i++){
          dr=19660*(color.red-table[i].red);
          dg=38666*(color.green-table[i].green);
          db=7209*(color.blue-table[i].blue);
          dist=(FXuint)FXABS(dr) + (FXuint)FXABS(dg) + (FXuint)FXABS(db);
          if(dist<mindist){
            bestmatch=i;
            mindist=dist;
            }
          }
 
        // Return result
        color.red=table[bestmatch].red;
        color.green=table[bestmatch].green;
        color.blue=table[bestmatch].blue;

//fprintf(stderr,"acquireColor(%06x) -> Best %06x\n",clr,color.pixel);

        // Try to allocate the closest match color.  This should only
        // fail if the cell is read/write.  Otherwise, we're incrementing
        // the cell's reference count.
        if(!XAllocColor(getDisplay(),colormap,&color)){
          color.pixel = bestmatch;
          color.red   = table[bestmatch].red;
          color.green = table[bestmatch].green;
          color.blue  = table[bestmatch].blue;
          color.flags = DoRed | DoGreen | DoBlue;
          }
        }
//fprintf(stderr,"acquireColor(%06x) -> Other %06x\n",clr,color.pixel);
      break;
    }
  return color.pixel;
  }

 
// Releases color
void FXWindow::releaseColor(FXPixel pix){
  Visual *visual;
  Colormap colormap;

  // Test if display open
  if(!getDisplay()){ fxerror("%s::releaseColor: trying to create color before opening display.\n",getClassName()); }

  // Get visual
  visual=getDefaultVisual();
  
  // Get colormap
  colormap=getDefaultColormap();
  
  // Free color, if it was allocated
  switch(visual->c_class){
    default:
      XFreeColors(getDisplay(),colormap,&pix,1,0);
    case TrueColor:
    case StaticGray:
      break;
    }
  }


// Find color of pixel
FXColor FXWindow::pixelColor(FXPixel pix){
  XColor color;
  Visual *visual;
  Colormap colormap;
  FXuint r,g,b,graym;

  // Test if display open
  if(!getDisplay()){ fxerror("%s::releaseColor: trying to create color before opening display.\n",getClassName()); }

  // Get visual
  visual=getDefaultVisual();
  
  // Get colormap
  colormap=getDefaultColormap();

  // Get color back
  switch(visual->c_class){
    case TrueColor:
      r=(255*(pix&visual->red_mask))/visual->red_mask;
      g=(255*(pix&visual->green_mask))/visual->green_mask;
      b=(255*(pix&visual->blue_mask))/visual->blue_mask;
      break;
    case StaticGray:
      graym=(1<<visual->bits_per_rgb)-1;
      r=(255*pix)/graym;
      b=r;
      g=r;
      break;
    default:
      color.pixel=pix;
      XQueryColor(getDisplay(),colormap,&color);
      r=color.red/257;
      g=color.green/257;
      b=color.blue/257;
      break;
    }
  return FXRGB(r,g,b);
  }
  
  

// Set window background
void FXWindow::setBackColor(FXPixel clr){
  backColor=clr;
  if(xid){
    XSetWindowBackground(getDisplay(),xid,backColor);
    update(0,0,width,height);
    }
  }


/*******************************************************************************/


// Has this window the selection
FXbool FXWindow::hasSelection() const {
  return (flags&FLAG_SELECTION)!=0;
  }


// Lost the selection
long FXWindow::onSelectionLost(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onSelectionLost %x\n",getClassName(),this);
  flags&=~FLAG_SELECTION;
  if(target && target->handle(this,MKUINT(message,SEL_SELECTION_LOST),ptr)) return 1;
  return 1;
  }


// Gained the selection
long FXWindow::onSelectionGained(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onSelectionGained %x\n",getClassName(),this);
  flags|=FLAG_SELECTION;
  if(target && target->handle(this,MKUINT(message,SEL_SELECTION_GAINED),ptr)) return 1;
  return 1;
  }


// Somebody wants our the selection
long FXWindow::onSelectionRequest(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onSelectionRequest %x\n",getClassName(),this);
  if(target && target->handle(this,MKUINT(message,SEL_SELECTION_REQUEST),ptr)) return 1;
  return 0;
  }


// Acquire the selection
void FXWindow::acquireSelection(){
  if(!(flags&FLAG_SELECTION)){
    if(xid){
//fprintf(stderr,"%s::acquireSelection %x\n",getClassName(),this);
      XSetSelectionOwner(getDisplay(),XA_PRIMARY,None,getApp()->event.time);
      XSetSelectionOwner(getDisplay(),XA_PRIMARY,xid,getApp()->event.time);
      if(XGetSelectionOwner(getDisplay(),XA_PRIMARY)==xid){
        handle(this,MKUINT(0,SEL_SELECTION_GAINED),&getApp()->event);
        }
      }
    }
  }


// Release the selection
void FXWindow::releaseSelection(){
  if(flags&FLAG_SELECTION){
    if(xid){ 
//fprintf(stderr,"%s::releaseSelection %x\n",getClassName(),this);
      if(XGetSelectionOwner(getDisplay(),XA_PRIMARY)==xid){
        XSetSelectionOwner(getDisplay(),XA_PRIMARY,None,CurrentTime);
        }
      }
    }
  }


/*******************************************************************************/


// Make transient for some other window
void FXWindow::setTransientFor(FXWindow* main){
  if(xid && main && main->id()){
    XSetTransientForHint(getDisplay(),xid,main->id());
    }
  }


// Get pointer location
FXint FXWindow::getCursorPosition(FXint& x,FXint& y,FXuint& buttons) const {
  if(xid!=0){
    Window dum; int rx,ry;
    return XQueryPointer(getDisplay(),xid,&dum,&dum,&rx,&ry,&x,&y,&buttons);
    }
  return FALSE;
  }


// Recalculate layout if marked as dirty during some previous interaction.
// This one should ALWAYS be called, as it it responsible for widget layout!
// If updating is not blocked [e.g. during a user-interaction], call upon
// the target to update this widget also.
// Return 0 if we're updatable but no target was present or the target
// didn't handle the update message; we need this for auto-gray or auto-hide
// capability.
long FXWindow::onUpdate(FXObject*,FXSelector,void*){
//if(flags&FLAG_DIRTY) fprintf(stderr,"%s::onUpdate %08x (dirty=%x)\n",getClassName(),this,(flags&FLAG_DIRTY));
  if(flags&FLAG_DIRTY) layout();
  if(flags&FLAG_UPDATE){
    return target && target->handle(this,MKUINT(message,SEL_UPDATE),NULL);
    }
  return 1;
  }


// Propagate window size change
void FXWindow::layout(){ 
  flags&=~FLAG_DIRTY;
  }


// Propagate size change upward
// We also schedule an eventual refresh
void FXWindow::recalc(){
  getApp()->refresh();
  flags|=FLAG_DIRTY;
  parent->recalc(); 
  }


// Update dirty rectangle
void FXWindow::update(FXint x,FXint y,FXint w,FXint h){ 
  if(xid){
    
    // Probably got an update before window was laid out; we correct by 
    // building a big expose rectangle which covers the whole window.
    if(w<=0) w=4096;
    if(h<=0) h=4096;
    
    // Append the rectangle; it is a synthetic expose event!!
    getApp()->addRepaint(xid,x,y,w,h,1);
    }
  }


// Move and resize
void FXWindow::position(FXint x,FXint y,FXint w,FXint h){
  if((flags&FLAG_DIRTY) || (x!=xpos) || (y!=ypos) || (w!=width) || (h!=height)){
    xpos=x;
    ypos=y;
    width=FXMAX(w,1);
    height=FXMAX(h,1);
    if(xid){
      XMoveResizeWindow(getDisplay(),xid,xpos,ypos,width,height);
      layout();
      }
    }
  }


// Move window
void FXWindow::move(int x,int y){
  if((x!=xpos) || (y!=ypos)){
    xpos=x;
    ypos=y;
    if(xid){
      XMoveWindow(getDisplay(),xid,xpos,ypos);
      }
    }
  }


// Resize
void FXWindow::resize(int w,int h) {
  if((flags&FLAG_DIRTY) || (w!=width) || (h!=height)){
    width=FXMAX(w,1);
    height=FXMAX(h,1);
    if(xid){
      XResizeWindow(getDisplay(),xid,width,height);
      layout();
      }
    }
  }


// Reparent window under a new parent
void FXWindow::reparent(FXComposite* newparent){
  if(newparent==NULL){ fxerror("%s::reparent: NULL parent specified.\n",getClassName()); }
  if(parent==NULL){ fxerror("%s::reparent: cannot reparent root window.\n",getClassName()); }
  if(newparent!=parent){
    
    // Check for funny cases
    if(containsChild(newparent)){ fxerror("%s::reparent: new parent is child of window.\n",getClassName()); }
    
    // Both windows created or both non-created
    if(xid && !newparent->xid){ fxerror("%s::reparent: new parent not created yet.\n",getClassName()); }
    if(!xid && newparent->xid){ fxerror("%s::reparent: window not created yet.\n",getClassName()); }
    
    // Kill focus chain through this window
    killFocus();
    
    // Flag old parent as to be recalculated
    parent->recalc();
    
    // Unlink from old parent
    if(prev) prev->next=next; else if(parent) parent->first=next;
    if(next) next->prev=prev; else if(parent) parent->last=prev;
    
    // Link to new parent
    parent=newparent;
    prev=parent->last;
    next=NULL;
    parent->last=this;
    if(prev) prev->next=this; else parent->first=this;
    
    // New toplevel shell too
    shell=parent->getShell();
    
    // Hook up to new window in server too
    if(xid && parent->xid){
      XReparentWindow(getDisplay(),xid,parent->xid,0,0);
      }
    
    // Flag as to be recalculated
    recalc();
    }
  }


// Move window in chain before a sibling
void FXWindow::linkBefore(FXWindow* sibling){
  if(!sibling){fxerror("%s::linkBefore: NULL argument.\n",getClassName());}
  if(sibling->parent!=parent){fxerror("%s::linkBefore: windows should have same parent.\n",getClassName());}
  if(sibling==this){fxerror("%s::linkBefore: same window.\n",getClassName());}
  if(prev) prev->next=next; else if(parent) parent->first=next;
  if(next) next->prev=prev; else if(parent) parent->last=prev; 
  prev=sibling->prev;
  next=sibling;
  if(prev) prev->next=this; else parent->first=this;
  sibling->prev=this;
  recalc();
  }


// Move window in chain after a sibling
void FXWindow::linkAfter(FXWindow* sibling){
  if(!sibling){fxerror("%s::linkAfter: NULL argument.\n",getClassName());}
  if(sibling->parent!=parent){fxerror("%s::linkAfter: windows should have same parent.\n",getClassName());}
  if(sibling==this){fxerror("%s::linkAfter: same window.\n",getClassName());}
  if(prev) prev->next=next; else if(parent) parent->first=next;
  if(next) next->prev=prev; else if(parent) parent->last=prev; 
  next=sibling->next;
  prev=sibling;
  if(next) next->prev=this; else parent->last=this;
  sibling->next=this;
  recalc();
  }


// Map window
void FXWindow::show(){
  if(!(flags&FLAG_SHOWN)){
    if(xid){
      XMapWindow(getDisplay(),xid);
      flags|=FLAG_SHOWN;
      }
    }
  }


// Unmap
void FXWindow::hide(){
  if(flags&FLAG_SHOWN){
    if(xid){
      XUnmapWindow(getDisplay(),xid);
      flags&=~FLAG_SHOWN;
      }
    }
  }


// Check if mapped
FXbool FXWindow::shown() const { 
  return (flags&FLAG_SHOWN)!=0; 
  }


// Test if logically inside
FXbool FXWindow::contains(FXint parentx,FXint parenty) const { 
  return xpos<=parentx && parentx<xpos+width && ypos<=parenty && parenty<ypos+height;
  }


// Enable the window
void FXWindow::enable(){
  if(!(flags&FLAG_ENABLED)){
    if(xid){
      FXuint events=BASIC_EVENT_MASK|ENABLED_EVENT_MASK;
      if(shell==this) events|=SHELL_EVENT_MASK;
      XSelectInput(getDisplay(),xid,events);
      }
    flags|=FLAG_ENABLED;
    }
  }


// Disable the window
void FXWindow::disable(){ 
  if(flags&FLAG_ENABLED){
    if(xid){
      FXuint events=BASIC_EVENT_MASK;
      if(shell==this) events|=SHELL_EVENT_MASK;
      XSelectInput(getDisplay(),xid,events);
      }
    flags&=~FLAG_ENABLED;
    }
  }


// Is window enabled
FXbool FXWindow::isEnabled() const { 
  return (flags&FLAG_ENABLED)!=0; 
  }


// Raise
void FXWindow::raise(){ 
  if(xid){ XRaiseWindow(getDisplay(),xid); }
  }
  

// Lower
void FXWindow::lower(){ 
  if(xid){ XLowerWindow(getDisplay(),xid); }
  }


// Locate window at root coordinate position x,y
FXID FXWindow::getWindowAt(FXint x,FXint y) const {
  Window win,child=XDefaultRootWindow(getDisplay());
  int dropx,dropy;
  while(1){
    if(!XTranslateCoordinates(getDisplay(),XDefaultRootWindow(getDisplay()),child,x,y,&dropx,&dropy,&win)){
      return XDefaultRootWindow(getDisplay());
      }
    if(win==None) break;
    child=win;
    }
  return child;
  }


// Get coordinates from another window (for symmetry)
void FXWindow::translateCoordinatesFrom(FXint& tox,FXint& toy,const FXWindow* fromwindow,FXint fromx,FXint fromy) const {
  Window tmp;
  if(fromwindow==NULL){ fxerror("%s::translateCoordinatesFrom: from-window is NULL.\n",getClassName()); }
  if(xid && fromwindow->xid){
    XTranslateCoordinates(getDisplay(),fromwindow->xid,xid,fromx,fromy,&tox,&toy,&tmp);
    }
  }


// Get coordinates to another window (for symmetry)
void FXWindow::translateCoordinatesTo(FXint& tox,FXint& toy,const FXWindow* towindow,FXint fromx,FXint fromy) const {
  Window tmp;
  if(towindow==NULL){ fxerror("%s::translateCoordinatesTo: to-window is NULL.\n",getClassName()); }
  if(xid && towindow->xid){
    XTranslateCoordinates(getDisplay(),xid,towindow->xid,fromx,fromy,&tox,&toy,&tmp);
    }
  }


// Clip drawing into window by children
void FXWindow::clipByChildren(FXuint yes){
  if(xid){
    XSetSubwindowMode(getDisplay(),getGC(),!yes);
    }
  }


/*******************************************************************************/


// Acquire grab
void FXWindow::grab(){
  if(xid){
//fprintf(stderr,"%s::grab\n",getClassName());
    if(dragCursor->id()==0){ fxerror("%s::grab: Cursor has not been created yet.\n",getClassName()); }
    if(!(flags&FLAG_SHOWN)){ fxwarning("%s::grab: Window is not visible.\n",getClassName()); }
    if(GrabSuccess!=XGrabPointer(getDisplay(),xid,FALSE,GRAB_EVENT_MASK,GrabModeAsync,GrabModeAsync,None,dragCursor->id(),CurrentTime)){ fxwarning("%s::grab: grab failed!\n",getClassName()); }
    getApp()->grabWindow=this;
    }
  }


// Release grab
void FXWindow::ungrab(){
  if(xid){
//fprintf(stderr,"%s::ungrab\n",getClassName());
    XUngrabPointer(getDisplay(),CurrentTime);
    getApp()->grabWindow=NULL;
    }
  }


// Return true if active grab is in effect
FXbool FXWindow::grabbed() const {
  return getApp()->grabWindow==this;
  }


/*******************************************************************************/


// To pass arguments to matcher
struct XMatch {
  Window window;
  Atom   selection;
  int    gotit;
  };


// Enable widget as drop target
void FXWindow::dropEnable(){
  if(xid==0){ fxerror("%s::dropEnable: window has not yet been created.\n",getClassName()); }
  if(!(flags&FLAG_DROPTARGET)){
    FXID propdata=(FXID)XDND_PROTOCOL_VERSION;
    FXASSERT(getApp()->xdndAware);
    XChangeProperty(getDisplay(),xid,getApp()->xdndAware,XA_ATOM,32,PropModeReplace,(FXuchar*)&propdata,1);
    flags|=FLAG_DROPTARGET;
    }
  }


// Disable widget as drop target
void FXWindow::dropDisable(){
  if(xid==0){ fxerror("%s::dropDisable: window has not yet been created.\n",getClassName()); }
  if(flags&FLAG_DROPTARGET){
    FXASSERT(getApp()->xdndAware);
    XDeleteProperty(getDisplay(),xid,getApp()->xdndAware);
    flags&=~FLAG_DROPTARGET;
    }
  }


// Is window drop enabled
FXbool FXWindow::isDropEnabled() const {
  return (flags&FLAG_DROPTARGET)!=0;
  }


// Set drag rectangle; we'll get no more DND_MOTION events while cursor is inside
void FXWindow::setDragRectangle(FXint x,FXint y,FXint w,FXint h,FXbool wantupdates){
  int tox,toy; Window tmp;
  if(xid==0){ fxerror("%s::setDragRectangle: window has not yet been created.\n",getClassName()); }
  XTranslateCoordinates(getDisplay(),parent->id(),XDefaultRootWindow(getDisplay()),x,y,&tox,&toy,&tmp);
  getApp()->xdndRect.x=tox;
  getApp()->xdndRect.y=toy;
  getApp()->xdndRect.w=w;
  getApp()->xdndRect.h=h;
  getApp()->xdndWantUpdates=wantupdates;
  }


// Clear drag rectangle
// Target always wants SEL_DND_MOTION messages
void FXWindow::clearDragRectangle(){
  if(xid==0){ fxerror("%s::clearDragRectangle: window has not yet been created.\n",getClassName()); }
  getApp()->xdndRect.x=0;
  getApp()->xdndRect.y=0;
  getApp()->xdndRect.w=0;
  getApp()->xdndRect.h=0;
  getApp()->xdndWantUpdates=FALSE;
  }


// Inquire about the DND type we're supposed to furnish to the requestor
FXDragType FXWindow::getDNDType() const {
  if(xid==0){ fxerror("%s::getDNDType: window has not yet been created.\n",getClassName()); }
  return (FXDragType)getApp()->ddeTarget;
  }


// Wait for arrival of dynamically exchanged data
static Bool notifyevent(Display *,XEvent *event,XPointer ptr){
  XMatch *match=(XMatch*)ptr;
  if(event->type==DestroyNotify && event->xdestroywindow.window==match->window){
    match->gotit=0;
    return True;
    }
  if(event->type==SelectionNotify && event->xselection.selection==match->selection){
    return True;
    }
  return False;
  }


// Get dropped data; called in response to DND enter or DND drop
FXbool FXWindow::getDNDData(FXDragType targettype,FXuchar*& data,FXuint& size){
  unsigned long  nitems,alloc_incr,bytes_after,transfersize,offset;
  FXWindow      *ownerobject;
  Window         ownerwindow;
  XEvent         ev;
  XMatch         match;
  Bool           status;
  Atom           actualtype;
  int            actualformat;
  FXuchar       *ptr;

  if(xid==0){ fxerror("%s::getDNDData: window has not yet been created.\n",getClassName()); }

  // Stuff about us
  getApp()->ddeRequestor=xid;
  getApp()->ddeTarget=targettype;
  getApp()->ddeProperty=getApp()->ddeAtom;
  
  data=NULL;
  size=0;
//fprintf(stderr,"%s::getDNDData ddeSelection=%d\n",getClassName(),ddeSelection);
  
  // If no owner, we're done quickly
  ownerwindow=XGetSelectionOwner(getDisplay(),getApp()->ddeSelection);
  if(ownerwindow == None) return FALSE;
//fprintf(stderr,"owner=%d\n",ownerwindow);
  
  // Window is in same application; bypass protocol by returning data from the mailbox
  // The caller is responsible for deletion.
  if(XFindContext(getDisplay(),ownerwindow,getApp()->wcontext,(XPointer*)&ownerobject)==0){
    FXASSERT(ownerobject);
//fprintf(stderr,"from local %s\n",ownerobject->getClassName());
    getApp()->event.type=SEL_SELECTION_REQUEST;
    getApp()->event.window=ownerwindow;
    ownerobject->handle(getApp(),MKUINT(0,SEL_SELECTION_REQUEST),&getApp()->event);
    data=getApp()->ddeData;
    size=getApp()->ddeSize;
    getApp()->ddeData=NULL;
    getApp()->ddeSize=0;
    return data!=NULL;
    }
  
  // Window is in other application
//fprintf(stderr,"from remote %d\n",ownerwindow);

  // Send message to the selection owner
  XConvertSelection(getDisplay(),getApp()->ddeSelection,getApp()->ddeTarget,getApp()->ddeAtom,getApp()->ddeRequestor,getApp()->event.time);
  
  // Wait for a response
  match.window=getApp()->ddeRequestor;
  match.selection=getApp()->ddeSelection;
  match.gotit=1;
  status=XCheckIfEvent(getDisplay(),&ev,notifyevent,(XPointer)&match);

  // Broke out of event loop
  if(!match.gotit) return FALSE;
  if(!status){
    match.gotit=1;
    XIfEvent(getDisplay(),&ev,notifyevent,(XPointer)&match);
    if(!match.gotit) return FALSE;
    }
  
  // There was no selection or no data has been specified
  if(ev.xselection.property==None){
    return FALSE;
    }

  // Prepare for download of big property data
  transfersize=XMaxRequestSize(getDisplay());
  if(!FXMALLOC(&getApp()->ddeData,FXuchar,8)) return FALSE;

  // Loop while we didn't get it all
  offset=0;
  getApp()->ddeSize=0;
  bytes_after=1;
  while(bytes_after){
    if(XGetWindowProperty(getDisplay(),getApp()->ddeRequestor,getApp()->ddeAtom,offset,transfersize,False,AnyPropertyType,&actualtype,&actualformat,&nitems,&bytes_after,&ptr)!=Success){
      FXFREE(&getApp()->ddeData);
      getApp()->ddeData=NULL;
      getApp()->ddeSize=0;
      return FALSE;
      }
    if(ptr == NULL || nitems == 0){
      XDeleteProperty(getDisplay(),getApp()->ddeRequestor,getApp()->ddeAtom);
      if(ptr) XFree(ptr);
      FXFREE(&getApp()->ddeData);
      getApp()->ddeData=NULL;
      getApp()->ddeSize=0;
      return FALSE;
      }
    alloc_incr = (actualformat==32) ? nitems<<2 : (actualformat==16) ? nitems<<1 : nitems;
    if(!FXRESIZE(&getApp()->ddeData,FXuchar,getApp()->ddeSize+alloc_incr+1)){
      XDeleteProperty(getDisplay(),getApp()->ddeRequestor,getApp()->ddeAtom);
      XFree(ptr);
      getApp()->ddeData=NULL;
      getApp()->ddeSize=0;
      return FALSE;
      }
    memcpy(&getApp()->ddeData[offset],ptr,alloc_incr);
    getApp()->ddeSize += alloc_incr;
    offset += (actualformat==32) ? nitems : (actualformat==16) ? nitems>>1 : nitems>>2;
    XFree(ptr);
    }
  XDeleteProperty(getDisplay(),getApp()->ddeRequestor,getApp()->ddeAtom);
  getApp()->ddeData[getApp()->ddeSize]=0;
  
  // Now we may have some data, return it
  data=getApp()->ddeData;
  size=getApp()->ddeSize;
  getApp()->ddeData=NULL;
  getApp()->ddeSize=0;
  return TRUE;
  }



// Set drop data; called in response to data request
// Data array will be deleted by the system automatically!!
FXbool FXWindow::setDNDData(FXDragType targettype,FXuchar* data,FXuint size){
  unsigned long  transfersize,offset;
  unsigned long  nunits,tlen,mode;
  FXWindow      *reqobject;
  
  if(xid==0){ fxerror("%s::setDNDData: window has not yet been created.\n",getClassName()); }
  
  // Must be true
  FXASSERT(getApp()->ddeRequestor);
  FXASSERT(getApp()->ddeProperty);
  FXASSERT(targettype);
  
//fprintf(stderr,"%s::setDNDData\n",getClassName());

  // Stuff about us
  getApp()->ddeTarget=targettype;
  getApp()->ddeData=data;
  getApp()->ddeSize=size;

  // Window is in same application; bypass protocol by storing data into the mailbox
  // The requestor taking it back out from the mailbox is responsible for the deletion.
  if(XFindContext(getDisplay(),getApp()->ddeRequestor,getApp()->wcontext,(XPointer*)&reqobject)==0){
    FXASSERT(reqobject);
//fprintf(stderr,"to local %s\n",reqobject->getClassName());
    return TRUE;
    }
  
  // Window in another application.  Stick the data into the indicated property,
  // then delete our local copy.  A new data array will be created in the remote
  // application.
  if(getApp()->ddeData!=NULL && size!=0){
    transfersize=XMaxRequestSize(getDisplay());
    mode=PropModeReplace;
    nunits=transfersize<<2;
    offset=0;
    FXASSERT(getApp()->ddeSize!=0);
//fprintf(stderr,"to remote %d\n",xid);
    do{
      if(getApp()->ddeSize<nunits) tlen=getApp()->ddeSize; else tlen=nunits;
      XChangeProperty(getDisplay(),getApp()->ddeRequestor,getApp()->ddeProperty,getApp()->ddeTarget,8,mode,&getApp()->ddeData[offset],tlen);
      getApp()->ddeSize-=tlen;
      mode=PropModeAppend;
      offset+=transfersize;
      }
    while(getApp()->ddeSize!=0);
    FXFREE(&getApp()->ddeData);
    }
  
  // Nothing to send
  else{
    getApp()->ddeProperty=None;
    }
  getApp()->ddeData=NULL;
  getApp()->ddeSize=0;
  return TRUE;
  }


// Target says accepts the drop
void FXWindow::acceptDrop(FXDragAction action){
  if(xid==0){ fxerror("%s::acceptDrop: window has not yet been created.\n",getClassName()); }
  if(getApp()->xdndTarget!=xid){ fxerror("%s::acceptDrop: window is not the drop target.\n",getClassName()); }

  // Accept anything or suggest a specific action?
  if(action!=DRAG_ACCEPT){
    if(action==DRAG_COPY) getApp()->xdndAction=getApp()->xdndActionCopy;
    if(action==DRAG_MOVE) getApp()->xdndAction=getApp()->xdndActionMove;
    if(action==DRAG_LINK) getApp()->xdndAction=getApp()->xdndActionLink;
    }
  
  // Accept a drop?
  getApp()->xdndAccepts=(action!=DRAG_REJECT);
  }



// Inquire about types being dragged over this window
FXbool FXWindow::inquireDNDTypes(const FXDragType*& types,FXuint& numtypes) const {
  if(xid==0){ fxerror("%s::inquireDNDTypes: window has not yet been created.\n",getClassName()); }
  if(getApp()->xdndTarget!=xid){ fxerror("%s::inquireDNDTypes: window is not the drop target.\n",getClassName()); }
  types=NULL;
  numtypes=0;
  if(getApp()->xdndNumTypes){
    types=getApp()->xdndTypeList;
    numtypes=getApp()->xdndNumTypes;
    return TRUE;
    }
  return FALSE;
  }


// Is the drop target being offered a certain DND type?
FXbool FXWindow::offeredDNDType(FXDragType type) const {
  if(xid==0){ fxerror("%s::offeredDNDType: window has not yet been created.\n",getClassName()); }
  if(getApp()->xdndTarget!=xid){ fxerror("%s::offeredDNDType: window is not the drop target.\n",getClassName()); }
  for(FXuint i=0; i<getApp()->xdndNumTypes; i++){
    if(type==getApp()->xdndTypeList[i]) return TRUE;
    }
  return FALSE;
  }


// What is the drag action being performed?
FXDragAction FXWindow::inquireDNDAction() const {
  FXDragAction action=DRAG_COPY;
  if(xid==0){ fxerror("%s::inquireDNDAction: window has not yet been created.\n",getClassName()); }
  if(getApp()->xdndTarget!=xid){ fxerror("%s::inquireDNDAction: window is not the drop target.\n",getClassName()); }
  if(getApp()->xdndAction==getApp()->xdndActionCopy) action=DRAG_COPY;
  if(getApp()->xdndAction==getApp()->xdndActionMove) action=DRAG_MOVE;
  if(getApp()->xdndAction==getApp()->xdndActionLink) action=DRAG_LINK;
  return action;
  }


// Return non-zero if we can drop on window
FXuint FXWindow::mayDropOn(FXID dropwindow) const {
  unsigned long itemCount,remainingBytes;
  unsigned char* rawData=NULL;
  Atom actualType;
  FXuint version=0;
  FXuint i,j;
  int actualFormat;
  if(XGetWindowProperty(getDisplay(),dropwindow,getApp()->xdndAware,0,1024,False,XA_ATOM,&actualType,&actualFormat,&itemCount,&remainingBytes,&rawData)==Success){
    if(rawData){
      if(actualType==XA_ATOM && actualFormat==32 && itemCount>0){
        
        // Version of the protocol
        version=*((FXuint*)rawData) | 0x80000000;
        
        // Did target specify specific types?
        if(itemCount>1){
          
          // Try match up the type lists
          for(i=0; i<getApp()->xdndNumTypes; i++){
            for(j=1; j<itemCount; j++){
              if(getApp()->xdndTypeList[i]==((FXuint*)rawData)[j]) goto ok;
              }
            }
          
          // Too bad, didn't find it
          version=0;
          }
        }
ok:   XFree(rawData);
      }
    }
  return version;
  }


// Source wants to find out if target accepted
FXDragAction FXWindow::didAccept() const {
  FXDragAction action=DRAG_REJECT;
  if(xid==0){ fxerror("%s::didAccept: window has not yet been created.\n",getClassName()); }
  if(getApp()->xdndSource!=xid){ fxerror("%s::didAccept: window is not a drag source.\n",getClassName()); }
  if(getApp()->xdndAccepts){
    if(getApp()->xdndAction==getApp()->xdndActionCopy) action=DRAG_COPY;
    if(getApp()->xdndAction==getApp()->xdndActionMove) action=DRAG_MOVE;
    if(getApp()->xdndAction==getApp()->xdndActionLink) action=DRAG_LINK;
    }
  return action;
  }


// True if we're in a drag operation
FXbool FXWindow::isDragging() const { 
  return (flags&FLAG_DRAGGING)!=0; 
  }


// True if this window is drop target
FXbool FXWindow::isDropTarget() const {
  if(xid==0){ fxerror("%s::isDropTarget: window has not yet been created.\n",getClassName()); }
  return xid==getApp()->xdndTarget;
  }
  

// Start a drag on the types mentioned
FXbool FXWindow::beginDrag(const FXDragType *types,FXuint numtypes){
  FXuint num;
  if(xid==0){ fxerror("%s::beginDrag: window has not yet been created.\n",getClassName()); }
  if(!(flags&FLAG_DRAGGING)){
    if(numtypes<1){ fxerror("%s::beginDrag: should have at least one type to drag.\n",getClassName()); }
    XSetSelectionOwner(getDisplay(),getApp()->xdndSelection,xid,getApp()->event.time);
    if(XGetSelectionOwner(getDisplay(),getApp()->xdndSelection)!=xid){
      fxwarning("%s::beginDrag: failed to acquire DND selection.\n",getClassName());
      return FALSE;
      }
    getApp()->xdndTypeList=(FXID*)malloc(sizeof(FXID)*numtypes);
    if(!getApp()->xdndTypeList) return FALSE;
    getApp()->xdndNumTypes=numtypes;
    for(num=0; num<numtypes; num++){
      getApp()->xdndTypeList[num]=types[num];
      }
    XChangeProperty(getDisplay(),xid,getApp()->xdndTypes,XA_ATOM,32,PropModeReplace,(const FXuchar*)getApp()->xdndTypeList,getApp()->xdndNumTypes);
    getApp()->xdndSource=xid;
    getApp()->xdndTarget=xid;
    getApp()->xdndAccepts=FALSE;
    getApp()->xdndSendPosition=FALSE;
    getApp()->xdndStatusPending=FALSE;
    getApp()->xdndStatusReceived=FALSE;
    getApp()->xdndWantUpdates=FALSE;
    getApp()->xdndVersion=XDND_PROTOCOL_VERSION;
    getApp()->xdndAction=getApp()->xdndActionCopy;
    getApp()->xdndRect.x=0;
    getApp()->xdndRect.y=0;
    getApp()->xdndRect.w=0;
    getApp()->xdndRect.h=0;
    getApp()->xdndXPos=0;
    getApp()->xdndYPos=0;
    flags|=FLAG_DRAGGING;
    return TRUE;
    }
  return FALSE;
  }

// Drag to new position
FXbool FXWindow::handleDrag(FXint x,FXint y,FXDragAction action){
  FXuint version=XDND_PROTOCOL_VERSION;
  FXuint forcepos=0;
  FXWindow *tgt;
  Window child,tmp;
  FXEvent event;
  XEvent se;
  if(xid==0){ fxerror("%s::handleDrag: window has not yet been created.\n",getClassName()); }
  if(flags&FLAG_DRAGGING){
    if(getApp()->xdndSource!=xid){ fxerror("%s::handleDrag: window is not a drag source.\n",getClassName()); }
    if(action<DRAG_COPY || DRAG_LINK<action){ fxerror("%s::handleDrag: illegal drag action.\n",getClassName()); }
  
    // Get drop window
    child=getWindowAt(x,y);
    
    // Got proper drop window?
    if(child!=XDefaultRootWindow(getDisplay())){
      
      // Test for XDND awareness
      version=mayDropOn(child);
      if(!version){
        child=XDefaultRootWindow(getDisplay());
        }
      }
    
    version&=0xff;
    
    // Moved to different window?
    if(child!=getApp()->xdndTarget){
      
      // Moving OUT of XDND aware window?
      if(getApp()->xdndTarget!=XDefaultRootWindow(getDisplay())){
        
        // If local, bypass protocol
        if(XFindContext(getDisplay(),getApp()->xdndTarget,getApp()->wcontext,(XPointer*)&tgt)==0){
          FXASSERT(tgt);
          event.type=SEL_DND_LEAVE;
          event.window=getApp()->xdndTarget;
          getApp()->ddeSelection=getApp()->xdndSelection;
          tgt->handle(this,MKUINT(0,SEL_DND_LEAVE),&event);
          getApp()->ddeSelection=XA_PRIMARY;
          }
        
        // Else send through protocol
        else{
          se.xclient.type=ClientMessage;
          se.xclient.display=getDisplay();
          se.xclient.message_type=getApp()->xdndLeave;
          se.xclient.format=32;
          se.xclient.window=getApp()->xdndTarget;
          se.xclient.data.l[0]=getApp()->xdndSource;
          se.xclient.data.l[1]=0;
          se.xclient.data.l[2]=0;
          se.xclient.data.l[3]=0;
          se.xclient.data.l[4]=0;
          XSendEvent(getDisplay(),getApp()->xdndTarget,True,NoEventMask,&se);
          }
        }
      
      // Reset XDND variables
      getApp()->xdndAccepts=FALSE;
      getApp()->xdndSendPosition=FALSE;
      getApp()->xdndStatusPending=FALSE;
      getApp()->xdndStatusReceived=FALSE;
      getApp()->xdndVersion=FXMIN(version,XDND_PROTOCOL_VERSION);
      getApp()->xdndAction=getApp()->xdndActionCopy;
      getApp()->xdndWantUpdates=FALSE;
      getApp()->xdndRect.x=0;
      getApp()->xdndRect.y=0;
      getApp()->xdndRect.w=0;
      getApp()->xdndRect.h=0;
      getApp()->xdndXPos=0;
      getApp()->xdndYPos=0;
      getApp()->xdndTarget=child;
      
      // Moving INTO XDND aware window?
      if(getApp()->xdndTarget!=XDefaultRootWindow(getDisplay())){
        
        // If local, bypass protocol
        if(XFindContext(getDisplay(),getApp()->xdndTarget,getApp()->wcontext,(XPointer*)&tgt)==0){
          FXASSERT(tgt);
          event.type=SEL_DND_ENTER;
          event.window=getApp()->xdndTarget;
          getApp()->ddeSelection=getApp()->xdndSelection;
          tgt->handle(this,MKUINT(0,SEL_DND_ENTER),&event);
          getApp()->ddeSelection=XA_PRIMARY;
          }
        
        // Else send through protocol
        else{
          se.xclient.type=ClientMessage;
          se.xclient.display=getDisplay();
          se.xclient.message_type=getApp()->xdndEnter;
          se.xclient.format=32;
          se.xclient.window=getApp()->xdndTarget;
          se.xclient.data.l[0]=getApp()->xdndSource;
          se.xclient.data.l[1]=getApp()->xdndVersion<<24;
          se.xclient.data.l[2]=getApp()->xdndNumTypes>=1 ? getApp()->xdndTypeList[0] : None;
          se.xclient.data.l[3]=getApp()->xdndNumTypes>=2 ? getApp()->xdndTypeList[1] : None;
          se.xclient.data.l[4]=getApp()->xdndNumTypes>=3 ? getApp()->xdndTypeList[2] : None;
          if(getApp()->xdndNumTypes>3) se.xclient.data.l[1]|=1;
          XSendEvent(getDisplay(),getApp()->xdndTarget,True,NoEventMask,&se);
          }
        forcepos=1;
        }
      }

    // Send target a position update
    if(getApp()->xdndTarget!=XDefaultRootWindow(getDisplay())){
      
      // Send position if we're outside the mouse box or ignoring it
      if(forcepos || getApp()->xdndRect.w==0 || getApp()->xdndRect.h==0 || (getApp()->xdndWantUpdates && getApp()->xdndRect.x<=x && getApp()->xdndRect.y<=y && x<getApp()->xdndRect.x+getApp()->xdndRect.w && y<getApp()->xdndRect.y+getApp()->xdndRect.h)){

        // No outstanding status message, so send new pos right away      
        if(!getApp()->xdndStatusPending){
          
          // Update drag action being performed
          if(action==DRAG_COPY) getApp()->xdndAction=getApp()->xdndActionCopy;
          if(action==DRAG_MOVE) getApp()->xdndAction=getApp()->xdndActionMove;
          if(action==DRAG_LINK) getApp()->xdndAction=getApp()->xdndActionLink;
          
          // New position
          getApp()->xdndXPos=x;
          getApp()->xdndYPos=y;
          
          // If local, bypass protocol
          if(XFindContext(getDisplay(),getApp()->xdndTarget,getApp()->wcontext,(XPointer*)&tgt)==0){
            FXASSERT(tgt);
            event.type=SEL_DND_MOTION;
            event.window=getApp()->xdndTarget;
            event.root_x=x;
            event.root_y=y;
            XTranslateCoordinates(getDisplay(),XDefaultRootWindow(getDisplay()),getApp()->xdndTarget,x,y,&event.win_x,&event.win_y,&tmp);
            getApp()->ddeSelection=getApp()->xdndSelection;
            getApp()->xdndAccepts=FALSE;
            tgt->handle(this,MKUINT(0,SEL_DND_MOTION),&event);
            getApp()->ddeSelection=XA_PRIMARY;
            getApp()->xdndStatusReceived=TRUE;
            getApp()->xdndStatusPending=FALSE;      // We know it got there, as its local
            }
          
          // Else send through protocol
          else{
            se.xclient.type=ClientMessage;
            se.xclient.display=getDisplay();
            se.xclient.message_type=getApp()->xdndPosition;
            se.xclient.format=32;
            se.xclient.window=getApp()->xdndTarget;
            se.xclient.data.l[0]=getApp()->xdndSource;
            se.xclient.data.l[1]=0;
            se.xclient.data.l[2]=MKUINT(y,x);                               // Coordinates
            se.xclient.data.l[3]=getApp()->event.time;                      // Time stamp
            se.xclient.data.l[4]=getApp()->xdndAction;                      // Drag and Drop action
            XSendEvent(getDisplay(),getApp()->xdndTarget,True,NoEventMask,&se);
            getApp()->xdndStatusPending=TRUE;       // Waiting for the other app to respond
            }
          }

        // Send it later
        else{
          getApp()->xdndSendPosition=TRUE;
          }
        }
      }
    return TRUE;
    }
  return FALSE;
  }


// Scan for status and selection requests...
static Bool matchevent(Display* display,XEvent* event,XPointer match){
  if(event->type == ClientMessage && event->xclient.message_type == *((Atom*)match)) return True;
  if(event->type == SelectionRequest) return True;
  return False;
  }


// Compare times
static inline int operator<(const struct timeval& a,const struct timeval& b){
  return (a.tv_sec<b.tv_sec) || (a.tv_sec==b.tv_sec && a.tv_usec<b.tv_usec);
  }


// Terminate the drag; if drop flag is false, don't drop even if accepted.
FXbool FXWindow::endDrag(FXbool drop){
  FXbool dropped=FALSE;
  FXbool nodrop=TRUE;
  FXWindow *tgt;
  FXEvent event;
  XEvent se;
  FXuint loops;
  if(xid==0){ fxerror("%s::endDrag: window has not yet been created.\n",getClassName()); }
  if(flags&FLAG_DRAGGING){
    if(getApp()->xdndSource!=xid){ fxerror("%s::endDrag: window is not a drag source.\n",getClassName()); }

    // Ever received a status
    if(getApp()->xdndStatusReceived && drop){
      
      // If a status message is still pending, wait for it
      if(getApp()->xdndStatusPending){
        getApp()->xdndSendPosition=FALSE;          // No more position updates sent to this target
        loops=1000;
        while(loops){
          if(XCheckIfEvent(getDisplay(),&se,matchevent,(char*)&getApp()->xdndStatus)){
            getApp()->dispatchEvent(se);
            if(se.xany.type==ClientMessage && se.xclient.data.l[0]==getApp()->xdndTarget){
//fprintf(stderr,"Got last status message!\n");
              getApp()->xdndStatusPending=FALSE;
              break;
              }
 
            // We either got a selection request or a status message for a stale target.
            // Since there is thus still hope for a message, lets wait a bit longer...
            loops=1000;
            }
          fxsleep(20000);
          loops--;
          }
        }
      
      // Got our status message
      if(!getApp()->xdndStatusPending && getApp()->xdndAccepts){
        
        // If local, bypass protocol
        if(XFindContext(getDisplay(),getApp()->xdndTarget,getApp()->wcontext,(XPointer*)&tgt)==0){
          FXASSERT(tgt);
          event.type=SEL_DND_DROP;
          event.window=getApp()->xdndTarget;
          getApp()->ddeSelection=getApp()->xdndSelection;
          tgt->handle(this,MKUINT(0,SEL_DND_DROP),&event);
          getApp()->ddeSelection=XA_PRIMARY;
          getApp()->xdndFinishPending=FALSE;        // We just handled the callback
          dropped=TRUE;                             // We're sure, as its handled directly!
          }
 
        // Else send through protocol
        else{
          se.xclient.type=ClientMessage;
          se.xclient.display=getDisplay();
          se.xclient.message_type=getApp()->xdndDrop;
          se.xclient.format=32;
          se.xclient.window=getApp()->xdndTarget;
          se.xclient.data.l[0]=getApp()->xdndSource;
          se.xclient.data.l[1]=0;
          se.xclient.data.l[2]=getApp()->event.time;
          se.xclient.data.l[3]=0;
          se.xclient.data.l[4]=0;
          XSendEvent(getDisplay(),getApp()->xdndTarget,True,NoEventMask,&se);
          getApp()->xdndFinishPending=TRUE;         // Waiting for the drop confirmation!
          }
        
        // We wait till the drop has been processed...
        // The target should request the data at some point;
        // We should wait here till the target has processed the drop message
        // or some amount of timeout has expired....
        // Some error code should be returned if the timer expired...
        if(getApp()->xdndFinishPending){
          loops=1000;
          while(loops){
            if(XCheckIfEvent(getDisplay(),&se,matchevent,(char*)&getApp()->xdndFinished)){
              getApp()->dispatchEvent(se);
              if(se.xany.type==ClientMessage && se.xclient.data.l[0]==getApp()->xdndTarget){
//fprintf(stderr,"Got finish message!\n");
                getApp()->xdndFinishPending=FALSE;
                dropped=TRUE;
                break;
                }
 
              // We either got a selection request or a status message for a stale target.
              // Since there is thus still hope for a message, lets wait a bit longer...
              loops=1000;
              }
            fxsleep(20000);
            loops--;
            }
          }
        
        // We tried a drop
        nodrop=FALSE;
        }
      }
    
    // Didn't drop, or didn't get any response, so just send a leave
    if(nodrop){
      
      // If local, bypass protocol
      if(XFindContext(getDisplay(),getApp()->xdndTarget,getApp()->wcontext,(XPointer*)&tgt)==0){
        FXASSERT(tgt);
        event.type=SEL_DND_LEAVE;
        event.window=getApp()->xdndTarget;
        getApp()->ddeSelection=getApp()->xdndSelection;
        tgt->handle(this,MKUINT(0,SEL_DND_LEAVE),&event);
        getApp()->ddeSelection=XA_PRIMARY;
        }
      
      // Else send through protocol
      else{
        se.xclient.type=ClientMessage;
        se.xclient.display=getDisplay();
        se.xclient.message_type=getApp()->xdndLeave;
        se.xclient.format=32;
        se.xclient.window=getApp()->xdndTarget;
        se.xclient.data.l[0]=getApp()->xdndSource;                  // Source window
        se.xclient.data.l[1]=0;
        se.xclient.data.l[2]=0;
        se.xclient.data.l[3]=0;
        se.xclient.data.l[4]=0;
        XSendEvent(getDisplay(),getApp()->xdndTarget,True,NoEventMask,&se);
        }
      }
      
    // Clean up
    if(getApp()->xdndTypeList) XFree(getApp()->xdndTypeList);
    getApp()->xdndTypeList=NULL;
    getApp()->xdndNumTypes=0;
    XDeleteProperty(getDisplay(),xid,getApp()->xdndTypes);
    getApp()->xdndSource=0;
    getApp()->xdndTarget=0;
    getApp()->xdndAccepts=FALSE;
    getApp()->xdndSendPosition=FALSE;
    getApp()->xdndStatusPending=FALSE;
    getApp()->xdndFinishPending=FALSE;
    getApp()->xdndStatusReceived=FALSE;
    getApp()->xdndWantUpdates=FALSE;
    getApp()->xdndVersion=XDND_PROTOCOL_VERSION;
    getApp()->xdndRect.x=0;
    getApp()->xdndRect.y=0;
    getApp()->xdndRect.w=0;
    getApp()->xdndRect.h=0;
    getApp()->xdndXPos=0;
    getApp()->xdndYPos=0;
    flags&=~FLAG_DRAGGING;
    }
  return dropped;
  }


/*******************************************************************************/


// Delete window
FXWindow::~FXWindow(){
  if(prev) prev->next=next; else if(parent) parent->first=next;
  if(next) next->prev=prev; else if(parent) parent->last=prev; 
  if(parent && parent->focus==this) parent->focus=NULL;
  if(getApp()->refresher==this) getApp()->refresher=parent;
  if(getApp()->cursorWindow==this) getApp()->cursorWindow=parent;
  if(getApp()->focusWindow==this) getApp()->focusWindow=parent;
  if(getApp()->grabWindow==this) getApp()->grabWindow=NULL;
  if(parent) parent->recalc();
  delete accelTable;
  destroy(); 
  parent=(FXWindow*)-1;
  shell=(FXWindow*)-1;
  first=last=(FXWindow*)-1;
  next=prev=(FXWindow*)-1;
  focus=(FXWindow*)-1;
  defaultCursor=(FXCursor*)-1;
  dragCursor=(FXCursor*)-1;
  accelTable=(FXAccelTable*)-1;
  target=(FXObject*)-1;
  }

