/********************************************************************************
*                                                                               *
*                         S c r o l l   W i n d o w                             *
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
* $Id: FXScrollWindow.cpp,v 1.29 1998/10/23 06:20:09 jeroen Exp $                *
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
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXCanvas.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"


/*
  To do:
  - Fix to take advantage of new scroll information
  - Perhaps scroll windows should observe FRAME_SUNKEN etc?
  - Potential optimization by compressing scoll events
    and delayed blit as well as delayed repaint
  - Solution for delayed scroll:
      - When sliders move, update pos_x,pos_y.
      - In onUpdate(), blit&post paint.
  - In new HSCROLLING_OFF mode, default width should be computed
    from contents (new virtual for that), and presence of scrollbars
    (determined by flags, as well as need).
  - The original content size should be returned from getContentWidth(),
    and getContentHeight(). The content_w and content_h member variables
    reflect the area involved in the scrolling; this is determined by layout().
  - Autoscroll needs to change, but don't know how yet. Idea:- pass in x,y, and
    start autoscroll if x,y near wall, stop if reaching final position or calling stopAutoScroll...
*/


#define AUTOSCROLL_FUDGE  5       // Proximity to wall at which we start autoscrolling


/*******************************************************************************/


FXDEFMAP(FXScrollArea) FXScrollAreaMap[]={
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_HSCROLLED,FXScrollArea::onHScrollerChanged),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_VSCROLLED,FXScrollArea::onVScrollerChanged),
  FXMAPFUNC(SEL_CHANGED,FXWindow::ID_HSCROLLED,FXScrollArea::onHScrollerDragged),
  FXMAPFUNC(SEL_CHANGED,FXWindow::ID_VSCROLLED,FXScrollArea::onVScrollerDragged),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXScrollArea::onAutoScroll),
  };


// Object implementation
FXIMPLEMENT(FXScrollArea,FXComposite,FXScrollAreaMap,ARRAYNUMBER(FXScrollAreaMap))

  
// Deserialization
FXScrollArea::FXScrollArea(){
  horizontal=NULL;
  vertical=NULL;
  corner=NULL;
  scrolltimer=NULL;
  viewport_w=1;
  viewport_h=1;
  content_w=1;
  content_h=1;
  pos_x=0;
  pos_y=0;
  }


// Construct and init
FXScrollArea::FXScrollArea(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h){
  horizontal=new FXScrollbar(this,this,FXWindow::ID_HSCROLLED,SCROLLBAR_HORIZONTAL);
  vertical=new FXScrollbar(this,this,FXWindow::ID_VSCROLLED,SCROLLBAR_VERTICAL);
  corner=new FXScrollCorner(this);
  scrolltimer=NULL;
  viewport_w=1;
  viewport_h=1;
  content_w=1;
  content_h=1;
  pos_x=0;
  pos_y=0;
  }


// Create X window
void FXScrollArea::create(){
  FXComposite::create();
  setBackColor(acquireColor(FXRGB(255,255,255)));
  show();
  }


// This should really add the scroll bar size only when required; however, 
// that depends on the actual size.  We are potentially being called at
// a moment when this is not known yet, so we return a size which reflects
// the situation when the scrollbars have been placed; this way, we should
// at least have enough space to fully see the contents, and a bit extra
// when the scrollbars turn out to have been unnecessary.

// Get default width
FXint FXScrollArea::getDefaultWidth(){
  FXint w=1;
  if((options&HSCROLLER_NEVER)&&(options&HSCROLLER_ALWAYS)) w=getContentWidth();
  if(!(options&VSCROLLER_NEVER)) w+=vertical->getDefaultWidth();
  if(!(options&HSCROLLER_NEVER)) w+=horizontal->getDefaultWidth();
  return w; 
  }


// Get default height
FXint FXScrollArea::getDefaultHeight(){ 
  FXint h=1;
  if((options&VSCROLLER_NEVER)&&(options&VSCROLLER_ALWAYS)) h=getContentHeight();
  if(!(options&HSCROLLER_NEVER)) h+=horizontal->getDefaultHeight();
  if(!(options&VSCROLLER_NEVER)) h+=vertical->getDefaultHeight();
  return h; 
  }


// Move content
void FXScrollArea::moveContents(FXint x,FXint y){
  if(!(flags&FLAG_INVISIBLE)){
    FXint dx,dy,tx,ty,fx,fy,ex,ey;
    if(pos_x<x){ 
      dx=x-pos_x; 
      fx=0; 
      tx=dx; 
      ex=0;
      }
    else{
      dx=pos_x-x; 
      fx=dx; 
      tx=0; 
      ex=viewport_w-dx;
      }
    if(pos_y<y){
      dy=y-pos_y; 
      fy=0; 
      ty=dy; 
      ey=0;
      }
    else{
      dy=pos_y-y; 
      fy=dy; 
      ty=0; 
      ey=viewport_h-dy;
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
      drawArea(this,fx,fy,viewport_w-dx,viewport_h-dy,tx,ty);
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
  }


// Changed
long FXScrollArea::onHScrollerChanged(FXObject*,FXSelector,void* ptr){
  FXint new_x=-(FXint)ptr;
  if(new_x!=pos_x){
    moveContents(new_x,pos_y);
    }
  return 1;
  }


// Changed
long FXScrollArea::onVScrollerChanged(FXObject*,FXSelector,void* ptr){
  FXint new_y=-(FXint)ptr;
  if(new_y!=pos_y){
    moveContents(pos_x,new_y);
    }
  return 1;
  }


// Dragged
long FXScrollArea::onHScrollerDragged(FXObject*,FXSelector,void* ptr){
  if(!(options&SCROLLERS_DONT_TRACK)){
    FXint new_x=-(FXint)ptr;
    if(new_x!=pos_x){
      moveContents(new_x,pos_y);
      }
    }
  return 1;
  }


// Dragged
long FXScrollArea::onVScrollerDragged(FXObject*,FXSelector,void* ptr){
  if(!(options&SCROLLERS_DONT_TRACK)){
    FXint new_y=-(FXint)ptr;
    if(new_y!=pos_y){
      moveContents(pos_x,new_y);
      }
    }
  return 1;
  }


// Timeout
long FXScrollArea::onAutoScroll(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint dx,dy;
  
  scrolltimer=NULL;
  
  // Autoscroll while close to the wall
  dx=0;
  dy=0;
  
  if(ev->win_x<AUTOSCROLL_FUDGE) dx=AUTOSCROLL_FUDGE-ev->win_x;
  else if(viewport_w-AUTOSCROLL_FUDGE<=ev->win_x) dx=viewport_w-AUTOSCROLL_FUDGE-ev->win_x;
  
  if(ev->win_y<AUTOSCROLL_FUDGE) dy=AUTOSCROLL_FUDGE-ev->win_y;
  else if(viewport_h-AUTOSCROLL_FUDGE<=ev->win_y) dy=viewport_h-AUTOSCROLL_FUDGE-ev->win_y;
    
  // Keep autoscrolling
  if(dx || dy){
    setPosition(pos_x+dx,pos_y+dy); 
    scrolltimer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXWindow::ID_AUTOSCROLL);
    }
  return 1;
  }


// Start automatic scrolling
FXbool FXScrollArea::startAutoScroll(FXint x,FXint y,FXbool onlywheninside){
  FXbool autoscrolling=FALSE;
  if(horizontal->getPage()<horizontal->getRange()){
    if(x<AUTOSCROLL_FUDGE) autoscrolling=TRUE;
    else if(viewport_w-AUTOSCROLL_FUDGE<=x) autoscrolling=TRUE;
    }
  if(vertical->getPage()<vertical->getRange()){
    if(y<AUTOSCROLL_FUDGE) autoscrolling=TRUE;
    else if(viewport_h-AUTOSCROLL_FUDGE<=y) autoscrolling=TRUE;
    }
  if(onlywheninside && (x<0 || y<0 || viewport_w<=x || viewport_h<=y)) autoscrolling=FALSE;
  if(autoscrolling){
    if(!scrolltimer){ scrolltimer=getApp()->addTimeout(getApp()->scrollSpeed,this,FXWindow::ID_AUTOSCROLL); }
    }
  else{
    if(scrolltimer){ scrolltimer=getApp()->removeTimeout(scrolltimer); }
    }
  return autoscrolling;
  }


// Stop automatic scrolling
void FXScrollArea::stopAutoScroll(){
  if(scrolltimer){scrolltimer=getApp()->removeTimeout(scrolltimer);}
  }


// Set scroll style
void FXScrollArea::setScrollStyle(FXuint style){
  FXuint opts=(options&~SCROLLER_MASK) | (style&SCROLLER_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Get scroll style
FXuint FXScrollArea::getScrollStyle() const {
  return (options&SCROLLER_MASK); 
  }


// True if horizontally scrollable enabled
FXbool FXScrollArea::isHorizontalScrollable() const {
  return !((options&HSCROLLER_NEVER) && (options&HSCROLLER_ALWAYS));
  }


// True if vertically scrollable enabled
FXbool FXScrollArea::isVerticalScrollable() const {
  return !((options&VSCROLLER_NEVER) && (options&VSCROLLER_ALWAYS));
  }


  
// Default viewport width
FXint FXScrollArea::getViewportWidth(){
  return width;
  }


// Default viewport height
FXint FXScrollArea::getViewportHeight(){
  return height;
  }


// Determine minimum content width of scroll area
FXint FXScrollArea::getContentWidth(){ 
  return 1; 
  }


// Determine minimum content height of scroll area
FXint FXScrollArea::getContentHeight(){ 
  return 1; 
  }


// Recalculate layout
void FXScrollArea::layout(){
  FXint sh_h=0;
  FXint sv_w=0;
  
  // Inviolate
  FXASSERT(pos_x<=0 && pos_y<=0);

  // Initial viewport size
  viewport_w=getViewportWidth();
  viewport_h=getViewportHeight();
  //viewport_w=width;
  //viewport_h=height;
  
  // ALWAYS determine content size
  content_w=getContentWidth();
  content_h=getContentHeight();

  // Get dimensions of the scroll bars
  if(!(options&HSCROLLER_NEVER)) sh_h=horizontal->getDefaultHeight();
  if(!(options&VSCROLLER_NEVER)) sv_w=vertical->getDefaultWidth();

  // Should we disable the scroll bars?
  // A bit tricky as the scrollbars may influence each other's presence
  if(!(options&(HSCROLLER_ALWAYS|VSCROLLER_ALWAYS)) && (content_w<=viewport_w) && (content_h<=viewport_h)){sh_h=sv_w=0;}
  //if(!(options&HSCROLLER_ALWAYS) && (content_w<=width-sv_w)) sh_h=0;
  //if(!(options&VSCROLLER_ALWAYS) && (content_h<=height-sh_h)) sv_w=0;
  //if(!(options&HSCROLLER_ALWAYS) && (content_w<=width-sv_w)) sh_h=0;
  if(!(options&HSCROLLER_ALWAYS) && (content_w<=getViewportWidth()-sv_w)) sh_h=0;
  if(!(options&VSCROLLER_ALWAYS) && (content_h<=getViewportHeight()-sh_h)) sv_w=0;
  if(!(options&HSCROLLER_ALWAYS) && (content_w<=getViewportWidth()-sv_w)) sh_h=0;

  // Viewport size with scroll bars taken into account
  viewport_w-=sv_w;
  viewport_h-=sh_h;

  // Adjust content size, now that we know about those scroll bars
  if((options&HSCROLLER_NEVER)&&(options&HSCROLLER_ALWAYS)) content_w=viewport_w;
  if((options&VSCROLLER_NEVER)&&(options&VSCROLLER_ALWAYS)) content_h=viewport_h;
  
  // Furthermore, content size won't be smaller than the viewport
  if(content_w<viewport_w) content_w=viewport_w;
  if(content_h<viewport_h) content_h=viewport_h;
  
  // Content size
  horizontal->setRange(content_w);
  vertical->setRange(content_h);
  
  // Page size may have changed
  horizontal->setPage(viewport_w);
  vertical->setPage(viewport_h);
  
  // Positions may have changed
  pos_x=-horizontal->getPosition();
  pos_y=-vertical->getPosition();
  
  // Hide or show horizontal scroll bar
  if(sh_h){
    horizontal->position(0,height-sh_h,width-sv_w,sh_h); 
    horizontal->show();
    horizontal->raise();
    }
  else{
    horizontal->hide();
    }

  // Hide or show vertical scroll bar
  if(sv_w){
    vertical->position(width-sv_w,0,sv_w,height-sh_h);
    vertical->show();
    vertical->raise();
    }
  else{
    vertical->hide();
    }
  
  // Hide or show scroll corner
  if(sv_w && sh_h){
    corner->position(width-sv_w,height-sh_h,sv_w,sh_h);
    corner->show();
    corner->raise();
    }
  else{
    corner->hide();
    }
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Set position
void FXScrollArea::setPosition(FXint x,FXint y){
  FXint new_x,new_y;

  // Set scroll bars
  horizontal->setPosition(-x);
  vertical->setPosition(-y);

  // Then read back valid position from scroll bars
  new_x=-horizontal->getPosition();
  new_y=-vertical->getPosition();

  // Move content if there's a change
  if(new_x!=pos_x || new_y!=pos_y){
    moveContents(new_x,new_y);
    }
  }


// The content has changed size
void FXScrollArea::recalc(){
  
  // Need to refresh
  getApp()->refresh();
  
  // Mark this widget as dirty
  flags|=FLAG_DIRTY;
  
  // If scrolling is locked, propagate dirty marking upward
  if(((options&HSCROLLER_NEVER)&&(options&HSCROLLER_ALWAYS)) || ((options&VSCROLLER_NEVER)&&(options&VSCROLLER_ALWAYS))){
    getParent()->recalc();
    }
  }


// Clean up
FXScrollArea::~FXScrollArea(){
  if(scrolltimer){getApp()->removeTimeout(scrolltimer);}
  horizontal=(FXScrollbar*)-1;
  vertical=(FXScrollbar*)-1;
  corner=(FXScrollCorner*)-1;
  scrolltimer=(FXTimer*)-1;
  }


/*******************************************************************************/



// Object implementation
FXIMPLEMENT(FXScrollWindow,FXScrollArea,NULL,0)


// Construct and init
FXScrollWindow::FXScrollWindow(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  }


// Get content window; may be NULL
FXWindow* FXScrollWindow::contentWindow() const { 
  return corner->getNext(); 
  }


// Determine content width of scroll area
FXint FXScrollWindow::getContentWidth(){ 
  return contentWindow() ? contentWindow()->getDefaultWidth() : 1;
  }


// Determine content height of scroll area
FXint FXScrollWindow::getContentHeight(){ 
  return contentWindow() ? contentWindow()->getDefaultHeight() : 1;
  }


// Move contents; moves child window
void FXScrollWindow::moveContents(FXint x,FXint y){
  FXWindow* contents=contentWindow();
  if(contents) contents->move(x,y);
  pos_x=x;
  pos_y=y;
  }


// Recalculate layout
void FXScrollWindow::layout(){

  // Layout scroll bars and viewport
  FXScrollArea::layout();

  // Resize contents
  if(contentWindow()){
    
    // Reposition content window
    contentWindow()->position(pos_x,pos_y,content_w,content_h);
    
    // Make sure its under the scroll bars
    contentWindow()->lower();
    }
  flags&=~FLAG_DIRTY;
  }
