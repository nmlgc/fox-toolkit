/********************************************************************************
*                                                                               *
*                S p l i t t e r   W i n d o w   O b j e c t                    *
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
* $Id: FXSplitter.cpp,v 1.9 1998/09/16 19:12:19 jvz Exp $                     *
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
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXSplitter.h"

#define BARWIDTH 4

/*
  Notes:
  - Reversed split option starts parting out from right [bottom].
  - Minimal partition of a split is 1 pixel.
  - Minimum width/height determined by number of visible children & BARWIDTH.
  - For convenience, width/height of zero replaced by minimum width/height
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXSplitter) FXSplitterMap[]={
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXSplitter::onLButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXSplitter::onLButtonRelease),
  FXMAPFUNC(SEL_MOTION,0,FXSplitter::onMotion),
  };


// Object implementation
FXIMPLEMENT(FXSplitter,FXComposite,FXSplitterMap,ARRAYNUMBER(FXSplitterMap))

  
// Make a splitter
FXSplitter::FXSplitter(){
  flags|=FLAG_ENABLED;
  win=NULL;
  split=0;
  off=0;
  }


// Make a splitter; it has no interior padding, and no borders
FXSplitter::FXSplitter(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXComposite(p,opts&~FRAME_MASK,x,y,w,h){
  defaultCursor=(options&SPLITTER_VERTICAL) ? getApp()->vsplitCursor : getApp()->hsplitCursor;
  dragCursor=defaultCursor;
  flags|=FLAG_ENABLED;
  win=NULL;
  split=0;
  off=0;
  }


// Create X window
void FXSplitter::create(){
  FXComposite::create();
  show();
  }


// Get default width
FXint FXSplitter::getDefaultWidth(){
  FXWindow* child;
  FXint numc,w;
  if(options&SPLITTER_VERTICAL){
    w=1;
    }
  else{
    numc=w=0;
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){ w+=1; numc++; }
      }
    if(numc>1) w+=(numc-1)*BARWIDTH;
    }
  return w;
  }
  

// Get default height
FXint FXSplitter::getDefaultHeight(){
  FXWindow* child;
  FXint numc,h;
  if(options&SPLITTER_VERTICAL){
    numc=h=0;
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){ h+=1; numc++; }
      }
    if(numc>1) h+=(numc-1)*BARWIDTH;
    }
  else{
    h=1;
    }
  return h;
  }


// Recompute layout
void FXSplitter::layout(){
  FXint pos,w,h;
  FXWindow* child;
  if(options&SPLITTER_VERTICAL){          // Vertical
    if(options&SPLITTER_REVERSED){
      pos=height;
      child=getLast();
      while(child){
        if(child->shown()){
          h=child->getHeight();
          if(h<=1) h=child->getDefaultHeight();
          if(child==getFirst()){ h=pos; if(h<1) h=1; }
          child->position(0,pos-h,width,h);
          pos=pos-h-BARWIDTH;
          }
        child=child->getPrev();
        }
      }
    else{
      pos=0;
      child=getFirst();
      while(child){
        if(child->shown()){
          h=child->getHeight();
          if(h<=1) h=child->getDefaultHeight();
          if(child==getLast()){ h=height-pos; if(h<1) h=1; }
          child->position(0,pos,width,h);
          pos=pos+h+BARWIDTH;
          }
        child=child->getNext();
        }
      }
    }
  else{                                   // Horizontal
    if(options&SPLITTER_REVERSED){
      pos=width;
      child=getLast();
      while(child){
        if(child->shown()){
          w=child->getWidth();
          if(w<=1) w=child->getDefaultWidth();
          if(child==getFirst()){ w=pos; if(w<1) w=1; }
          child->position(pos-w,0,w,height);
          pos=pos-w-BARWIDTH;
          }
        child=child->getPrev();
        }
      }
    else{
      pos=0;
      child=getFirst();
      while(child){
        if(child->shown()){
          w=child->getWidth();
          if(w<=1) w=child->getDefaultWidth();
          if(child==getLast()){ w=width-pos; if(w<1) w=1; }
          child->position(pos,0,w,height);
          pos=pos+w+BARWIDTH;
          }
        child=child->getNext();
        }
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Adjust horizontal layout
void FXSplitter::adjustHLayout(){
  FXWindow *child;
  FXint w,pos;
  if(options&SPLITTER_REVERSED){
    pos=win->getX()+win->getWidth();
    win->position(split,0,pos-split,height);
    pos=split-BARWIDTH;
    for(child=win->getPrev(); child; child=child->getPrev()){
      if(child->shown()){
        w=child->getWidth();
        if(w<=1) w=child->getDefaultWidth();
        if(child==getFirst()){ w=pos; if(w<1) w=1; }
        child->position(pos-w,0,w,height);
        pos=pos-w-BARWIDTH;
        }
      }
    }
  else{
    pos=win->getX();
    win->position(pos,0,split-pos,height);
    pos=split+BARWIDTH;
    for(child=win->getNext(); child; child=child->getNext()){
      if(child->shown()){
        w=child->getWidth();
        if(w<=1) w=child->getDefaultWidth();
        if(child==getLast()){ w=width-pos; if(w<1) w=1; }
        child->position(pos,0,w,height);
        pos=pos+w+BARWIDTH;
        }
      }
    }
  }


// Adjust vertical layout
void FXSplitter::adjustVLayout(){
  FXWindow *child;
  FXint h,pos;
  if(options&SPLITTER_REVERSED){
    pos=win->getY()+win->getHeight();
    win->position(0,split,width,pos-split);
    pos=split-BARWIDTH;
    for(child=win->getPrev(); child; child=child->getPrev()){
      if(child->shown()){
        h=child->getHeight();
        if(h<=1) h=child->getDefaultHeight();
        if(child==getFirst()){ h=pos; if(h<1) h=1; }
        child->position(0,pos-h,width,h);
        pos=pos-h-BARWIDTH;
        }
      }
    }
  else{
    pos=win->getY();
    win->position(0,pos,width,split-pos);
    pos=split+BARWIDTH;
    for(child=win->getNext(); child; child=child->getNext()){
      if(child->shown()){
        h=child->getHeight();
        if(h<=1) h=child->getDefaultHeight();
        if(child==getLast()){ h=height-pos; if(h<1) h=1; }
        child->position(0,pos,width,h);
        pos=pos+h+BARWIDTH;
        }
      }
    }
  }


// Find child just before split
FXWindow* FXSplitter::findHSplit(FXint pos){
  FXWindow* child;
  if(options&SPLITTER_REVERSED){
    child=getFirst();
    while(child){
      if(child->shown()){
        if(child->getX()-BARWIDTH<=pos && pos<child->getX()) return child;
        }
      child=child->getNext();
      }
    }
  else{
    child=getFirst();
    while(child){
      if(child->shown()){
        if(child->getX()+child->getWidth()<=pos && pos<child->getX()+child->getWidth()+BARWIDTH) return child;
        }
      child=child->getNext();
      }
    }
  return NULL;
  }


// Find child just before split
FXWindow* FXSplitter::findVSplit(FXint pos){
  FXWindow* child;
  if(options&SPLITTER_REVERSED){
    child=getFirst();
    while(child){
      if(child->shown()){
        if(child->getY()-BARWIDTH<=pos && pos<child->getY()) return child;
        }
      child=child->getNext();
      }
    }
  else{
    child=getFirst();
    while(child){
      if(child->shown()){
        if(child->getY()+child->getHeight()<=pos && pos<child->getY()+child->getHeight()+BARWIDTH) return child;
        }
      child=child->getNext();
      }
    }
  return NULL;
  }


// Move the horizontal split intelligently
void FXSplitter::moveHSplit(FXint pos){
  FXint smin,smax;
  if(options&SPLITTER_REVERSED){
    smin=0;
    smax=win->getX()+win->getWidth()-1;
    }
  else{
    smin=win->getX()+1;
    smax=width-BARWIDTH-1;
    }
  split=pos;
  if(split<smin) split=smin;
  if(split>smax) split=smax;
  }


// Move the vertical split intelligently
void FXSplitter::moveVSplit(FXint pos){
  FXint smin,smax;
  if(options&SPLITTER_REVERSED){
    smin=0;
    smax=win->getY()+win->getHeight()-1;
    }
  else{
    smin=win->getY()+1;
    smax=width-BARWIDTH-1;
    }
  split=pos;
  if(split<smin) split=smin;
  if(split>smax) split=smax;
  }


// Button being pressed
long FXSplitter::onLButtonPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  win=NULL; 
  if(options&SPLITTER_VERTICAL){
    pos=ev->win_y;
    win=findVSplit(pos);
    if(win){
      grab();
      if(options&SPLITTER_REVERSED)
        split=win->getY();
      else
        split=win->getY()+win->getHeight();
      off=pos-split;
      drawVSplit(split);
      return 1;
      }      
    }
  else{
    pos=ev->win_x;
    win=findHSplit(pos);
    if(win){
      grab();
      if(options&SPLITTER_REVERSED)
        split=win->getX();
      else
        split=win->getX()+win->getWidth();
      off=pos-split;
      drawHSplit(split);
      return 1;
      }      
    }
  return 0;
  }
  

// Button being released
long FXSplitter::onLButtonRelease(FXObject*,FXSelector,void*){
  if(win){
    ungrab();
    if(options&SPLITTER_VERTICAL){
      drawVSplit(split);
      adjustVLayout();
      }
    else{
      drawHSplit(split);
      adjustHLayout();
      }
    win=NULL;
    return 1;
    }
  return 0;
  }


// Button being released
long FXSplitter::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint cur,oldsplit;
  if(win){
    oldsplit=split;
    if(options&SPLITTER_VERTICAL){
      cur=ev->win_y;
      moveVSplit(cur-off);
      if(split!=oldsplit){
        drawVSplit(oldsplit);
        drawVSplit(split);
        }
      }
    else{
      cur=ev->win_x;
      moveHSplit(cur-off);
      if(split!=oldsplit){
        drawHSplit(oldsplit);
        drawHSplit(split);
        }
      }
    return 1;
    }
  return 0;
  }


// Draw the horizontal split
void FXSplitter::drawHSplit(FXint pos){
  setFunction(BLT_NOT_DST);     // Does this always show up?
  clipByChildren(FALSE);
  fillRectangle(pos,0,BARWIDTH,height);
  clipByChildren(TRUE);
  setFunction(BLT_SRC);
  }


// Draw the vertical split
void FXSplitter::drawVSplit(FXint pos){
  setFunction(BLT_NOT_DST);     // Does this always show up?
  clipByChildren(FALSE);
  fillRectangle(0,pos,width,BARWIDTH);
  clipByChildren(TRUE);
  setFunction(BLT_SRC);
  }
