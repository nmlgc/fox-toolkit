/********************************************************************************
*                                                                               *
*                         S t a t u s b a r   O b j e c t s                     *
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
* $Id: FXStatusbar.cpp,v 1.19 1998/10/19 21:10:46 jvz Exp $                   *
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
#include "FXComposite.h"
#include "FXStatusbar.h"


#define CORNERSIZE    17
#define PANESPACING   4
#define UPPERSPACING  0

/* 
  To do:
  - Split basic C++ libaries off from platform specifics (X/UNIX).
  - Need to include whatever for strlen()
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXDragCorner) FXDragCornerMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXDragCorner::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXDragCorner::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXDragCorner::onLeftBtnRelease),
  FXMAPFUNC(SEL_MOTION,0,FXDragCorner::onMotion),
  };


// Object implementation
FXIMPLEMENT(FXDragCorner,FXFrame,FXDragCornerMap,ARRAYNUMBER(FXDragCornerMap))


// Construct and init
FXDragCorner::FXDragCorner(FXComposite* p):
  FXFrame(p,LAYOUT_RIGHT|LAYOUT_BOTTOM){
  defaultCursor=getApp()->resizeCursor;
  flags|=FLAG_ENABLED;
  oldw=0;
  oldh=0;
  xoff=0;
  yoff=0;
  }


// Create X Window
void FXDragCorner::create(){
  FXFrame::create();
  show();
  }


// Get default width
FXint FXDragCorner::getDefaultWidth(){
  return CORNERSIZE;
  }


// Get default height
FXint FXDragCorner::getDefaultHeight(){
  return CORNERSIZE;
  }


// Slightly different from Frame border
long FXDragCorner::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onPaint(sender,sel,ptr);
  setLineAttributes(1);
  setForeground(shadowColor);
  drawLine(width-2,height-1,width,height-3);
  drawLine(width-8,height-1,width,height-9);
  drawLine(width-14,height-1,width,height-15);
  //drawLine(0,0,width,0);
  setForeground(hiliteColor);
  drawLine(width-5,height-1,width,height-6);
  drawLine(width-11,height-1,width,height-12);
  drawLine(width-17,height-1,width,height-18);
  //drawLine(0,height-1,width-17,height-1);
  //drawLine(width-1,1,width-1,height-17);
  setLineAttributes(0);
  return 1;
  }


// Pressed LEFT button 
long FXDragCorner::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint xx,yy;
  xoff=width-event->win_x;
  yoff=height-event->win_y;
  oldw=width;
  oldh=height;
  getRoot()->setFunction(BLT_NOT_DST);     // Does this always show up?
  getRoot()->clipByChildren(FALSE);
  getShell()->translateCoordinatesTo(xx,yy,getRoot(),0,0);
  getRoot()->drawRectangle(xx,yy,oldw,oldh);
  getRoot()->clipByChildren(TRUE);
  getRoot()->setFunction(BLT_SRC);
  flags|=FLAG_PRESSED;
  return 1;
  }


// Released LEFT button 
long FXDragCorner::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint xx,yy,wx,wy;
  getShell()->translateCoordinatesTo(xx,yy,getRoot(),0,0);
  translateCoordinatesTo(wx,wy,getShell(),event->win_x,event->win_y);
  getRoot()->setFunction(BLT_NOT_DST);     // Does this always show up?
  getRoot()->clipByChildren(FALSE);
  getRoot()->drawRectangle(xx,yy,oldw,oldh);
  getRoot()->clipByChildren(TRUE);
  getRoot()->setFunction(BLT_SRC);
  getShell()->resize(wx+xoff,wy+yoff);
  flags&=~FLAG_PRESSED;
  return 1;
  }


// Moved
long FXDragCorner::onMotion(FXObject*,FXSelector,void* ptr){
  if(flags&FLAG_PRESSED){
    FXEvent *event=(FXEvent*)ptr;
    FXint xx,yy,wx,wy;
    getShell()->translateCoordinatesTo(xx,yy,getRoot(),0,0);
    translateCoordinatesTo(wx,wy,getShell(),event->win_x,event->win_y);
    getRoot()->setFunction(BLT_NOT_DST);     // Does this always show up?
    getRoot()->clipByChildren(FALSE);
    getRoot()->drawRectangle(xx,yy,oldw,oldh);
    oldw=wx+xoff;
    oldh=wy+yoff;
    getRoot()->drawRectangle(xx,yy,oldw,oldh);
    getRoot()->clipByChildren(TRUE);
    getRoot()->setFunction(BLT_SRC);
    return 1;
    }
  return 0;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXStatusline) FXStatuslineMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXStatusline::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXStatusline::onUpdate),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXStatusline::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXStatusline::onCmdGetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXStatusline,FXFrame,FXStatuslineMap,ARRAYNUMBER(FXStatuslineMap))


// Construct and init
FXStatusline::FXStatusline(FXComposite* p,FXObject* tgt,FXSelector sel):
  FXFrame(p,LAYOUT_LEFT|LAYOUT_FILL_Y|LAYOUT_FILL_X){
  font=getApp()->normalFont;
  textColor=0;
  target=tgt;
  message=sel;
  }


// Create X Window
void FXStatusline::create(){
  FXFrame::create();
  textColor=acquireColor(getApp()->foreColor);
  font->create();
  show();
  }


// Get default width; as text changes often, exact content does not matter
FXint FXStatusline::getDefaultWidth(){
  return 8;
  }


// Get default height; just care about font height
FXint FXStatusline::getDefaultHeight(){
  return 4+font->getFontHeight();
  }


// Slightly different from Frame border
long FXStatusline::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint ty=(height-font->getFontHeight())/2;
  FXWindow::onPaint(sender,sel,ptr);
  setTextFont(font);
  setForeground(textColor);
  if(status.text()){
    drawText(4,ty+font->getFontAscent(),status.text(),status.length());
    }
  drawSunkenRectangle(0,0,width,height);
  return 1;
  }


// If the cursor is inside a widget, flash its help text;
// Otherwise, unflash back to the regular status message.
long FXStatusline::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *helpsource;
  
  // Regular GUI update
  FXFrame::onUpdate(sender,sel,ptr);
  
  // Ask the help source for a new status text first
  helpsource=getApp()->getCursorWindow();
  if(helpsource){
    if(helpsource->handle(this,MKUINT(FXWindow::ID_QUERY_HELP,SEL_UPDATE),NULL)){
      return 1;
      }
    }
  
  // Next, try focus window
  helpsource=getApp()->getFocusWindow();
  if(helpsource){
    if(helpsource->handle(this,MKUINT(FXWindow::ID_QUERY_HELP,SEL_UPDATE),NULL)){
      return 1;
      }
    }
  
  // Ask target; this should be the normal help text
  // indicating the state the program is in currently.
  if(target){
    if(target->handle(this,MKUINT(FXWindow::ID_QUERY_HELP,SEL_UPDATE),NULL)){
      return 1;
      }
    }
  
  // Otherwise, supply our own
  setText("Ready.");
  return 1;
  }


// Update value from a message
long FXStatusline::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setText(*((FXString*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXStatusline::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXString*)ptr) = getText(); }
  return 1;
  }


// Set permanently displayed message
void FXStatusline::setText(const FXchar *text){
  if(status!=text){
    status=text;
    update(0,0,width,height);
    }
  }


// Change the font
void FXStatusline::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Set text color
void FXStatusline::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Destruct
FXStatusline::~FXStatusline(){
  font=(FXFont*)-1;
  }


/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXStatusbar,FXComposite,NULL,0)


// Make a status bar
FXStatusbar::FXStatusbar(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts,x,y,w,h){
  corner=new FXDragCorner(this);
  status=new FXStatusline(this);
  }


// Get default width
FXint FXStatusbar::getDefaultWidth(){
  FXint t,wcum,numc;
  FXWindow* child;
  FXuint hints;
  wcum=numc=0;
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) t=child->getWidth(); else t=child->getDefaultWidth();
      wcum+=t;
      numc++;
      }
    }
  if(numc>1) wcum+=(numc-1)*PANESPACING;
  return wcum;
  }


// Get default height
FXint FXStatusbar::getDefaultHeight(){
  FXint t,hmax;
  FXWindow* child;
  FXuint hints;
  hmax=0;
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_HEIGHT) t=child->getHeight(); else t=child->getDefaultHeight();
      if(hmax<t) hmax=t;
      }
    }
  return hmax+UPPERSPACING+1;
  }


// Recalculate layout
void FXStatusbar::layout(){
  FXint left,right;
  FXint remain,room,error;
  FXint extra_space,total_space;
  FXint x,y,w,h;
  FXint numc=0;
  FXint nb_expand=0;
  FXuint hints;
  FXWindow* child;

  flags&=~FLAG_DIRTY;
  
  if(getFirst()==NULL) return;

  // Placement rectangle; right/bottom non-inclusive
  left=0;
  right=width;
  remain=right-left;

  // Find number of paddable children and total width
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); else w=child->getDefaultWidth();
      if(hints&LAYOUT_FILL_X) nb_expand++;
      remain-=w;
      numc++;
      }
    }

  // Child spacing
  // Expand FXStatusline if its alone!
  if(numc==2) remain+=corner->getDefaultWidth();
  if(numc>2) remain-=PANESPACING*(numc-1);

  // For expansion
  room=0;
  error=0;
  if(nb_expand>0){
    room=remain/nb_expand;
    error=remain%nb_expand;
    }

  // Do the layout
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();

      // Layout child in Y
      y=child->getY();
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); else h=child->getDefaultHeight();
      extra_space=0;
      if(hints&LAYOUT_FILL_Y){
        if(h<height-UPPERSPACING) h=height-UPPERSPACING;
        }
      if(hints&LAYOUT_CENTER_Y){
        if(h<height-UPPERSPACING) extra_space=(height-UPPERSPACING-h)/2;
        }
      if(hints&LAYOUT_BOTTOM)
        y=height-extra_space-h;
      else 
        y=UPPERSPACING+extra_space;

      // Layout child in X
      x=child->getX();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); else w=child->getDefaultWidth();
      extra_space=0;
      total_space=0;
      if(hints&LAYOUT_FILL_X){
        if(remain>0){
          w+=room;
          if(error){ w++; error--; }
          }
        }
      else if(hints&LAYOUT_CENTER_X){
        if(remain>0){
          total_space=room;
          if(error){ total_space++; error--; }
          extra_space=total_space/2;
          }
        }
      if(hints&LAYOUT_RIGHT){
        x=right-w-extra_space;
        right=right-w-PANESPACING-total_space;
        }
      else{
        x=left+extra_space;
        left=left+w+PANESPACING+total_space;
        }
      child->position(x,y,w,h);
      }
    }
  
  // Just make sure corner grip's on top
  corner->raise();
  }


// Create X window
void FXStatusbar::create(){
  FXComposite::create();
  show();
  }


// Destruct
FXStatusbar::~FXStatusbar(){
  corner=(FXDragCorner*)-1;
  status=(FXStatusline*)-1;
  }



