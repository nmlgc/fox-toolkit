/********************************************************************************
*                                                                               *
*                P a c k e r   C o n t a i n e r   O b j e c t                  *
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
* $Id: FXPacker.cpp,v 1.20 1998/09/18 22:07:16 jvz Exp $                      *
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
#include "FXPacker.h"
#include "FXShell.h"

  
/*
  To do:
  - Should FXPacker send GUI update messages????
*/



/*******************************************************************************/


// Map
FXDEFMAP(FXPacker) FXPackerMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXPacker::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXPacker,FXComposite,FXPackerMap,ARRAYNUMBER(FXPackerMap))


// Create child frame window
FXPacker::FXPacker(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXComposite(p,opts,x,y,w,h){
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  padtop=pt;
  padbottom=pb;
  padleft=pl;
  padright=pr;
  hspacing=hs;
  vspacing=vs;
  border=(options&FRAME_THICK)?2:(options&(FRAME_SUNKEN|FRAME_RAISED))?1:0;
  }


// Create window
void FXPacker::create(){
  FXComposite::create();
  baseColor=acquireColor(getApp()->backColor);
  hiliteColor=acquireColor(getApp()->hiliteColor);
  shadowColor=acquireColor(getApp()->shadowColor);
  borderColor=acquireColor(getApp()->borderColor);
  show();
  }


// Draw simple line border
void FXPacker::drawBorderRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(borderColor);
  drawRectangle(x,y,w,h);
  }


// Draw 1 pixel raised border
void FXPacker::drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(shadowColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }


// Draw 1 pixel sunken border
void FXPacker::drawSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(hiliteColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }


// Draw 2 pixel ridge border
void FXPacker::drawRidgeRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+1,x+w-2,y+h-2);
  setForeground(shadowColor);
  drawLine(x+1,y+1,x+w-3,y+1);
  drawLine(x+1,y+1,x+1,y+h-3);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }


// Draw 2 pixel groove border
void FXPacker::drawGrooveRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+1,x+w-2,y+h-2);
  setForeground(hiliteColor);
  drawLine(x+1,y+1,x+w-2,y+1);
  drawLine(x+1,y+1,x+1,y+h-2);
  drawLine(x+1,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y+1,x+w-1,y+h-1);
  }


// Draw 2 pixel double raised border
void FXPacker::drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(hiliteColor);
  drawLine(x,y,x+w-2,y);
  drawLine(x,y,x,y+h-2);
  setForeground(baseColor);
  drawLine(x+1,y+1,x+w-3,y+1);
  drawLine(x+1,y+1,x+1,y+h-3);
  setForeground(shadowColor);
  drawLine(x+1,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+h-2,x+w-2,y+1);
  setForeground(borderColor);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  }


// Draw 2 pixel double sunken border
void FXPacker::drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h){
  FXASSERT(xid);
  setForeground(shadowColor);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  setForeground(borderColor);
  drawLine(x+1,y+1,x+w-2,y+1);
  drawLine(x+1,y+1,x+1,y+h-2);
  setForeground(hiliteColor);
  drawLine(x+1,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y+h-1,x+w-1,y+1);
  setForeground(baseColor);
  drawLine(x+2,y+h-2,x+w-2,y+h-2);
  drawLine(x+w-2,y+2,x+w-2,y+h-2);
  }


// Draw border
void FXPacker::drawFrame(FXint x,FXint y,FXint w,FXint h){
  switch(options&FRAME_MASK) {
    case FRAME_LINE: drawBorderRectangle(x,y,w,h); break;
    case FRAME_SUNKEN: drawSunkenRectangle(x,y,w,h); break;
    case FRAME_RAISED: drawRaisedRectangle(x,y,w,h); break;
    case FRAME_GROOVE: drawGrooveRectangle(x,y,w,h); break;
    case FRAME_RIDGE: drawRidgeRectangle(x,y,w,h); break;
    case FRAME_SUNKEN|FRAME_THICK: drawDoubleSunkenRectangle(x,y,w,h); break;
    case FRAME_RAISED|FRAME_THICK: drawDoubleRaisedRectangle(x,y,w,h); break;
    }
  }


// Handle repaint 
long FXPacker::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXComposite::onPaint(sender,sel,ptr);
  drawFrame(0,0,width,height);
  return 1;
  }


// Change frame border style
void FXPacker::setFrameStyle(FXuint style){
  options=(options&~FRAME_MASK) | (style&FRAME_MASK);
  border=(options&FRAME_THICK) ? 2 : (options&(FRAME_SUNKEN|FRAME_RAISED)) ? 1 : 0;
  recalc();
  update(0,0,width,height);
  }


// Get frame style
FXuint FXPacker::getFrameStyle() const { 
  return (options&FRAME_MASK); 
  }


// Set base color
void FXPacker::setBaseColor(FXPixel clr){
  baseColor=clr;
  update(0,0,width,height);
  }


// Set highlight color
void FXPacker::setHiliteColor(FXPixel clr){
  hiliteColor=clr;
  update(0,0,width,height);
  }


// Set shadow color
void FXPacker::setShadowColor(FXPixel clr){
  shadowColor=clr;
  update(0,0,width,height);
  }


// Set border color
void FXPacker::setBorderColor(FXPixel clr){
  borderColor=clr;
  update(0,0,width,height);
  }


// Change top padding
void FXPacker::setPadTop(FXint pt){
  if(padtop!=pt){
    padtop=pt;
    recalc();
    update(0,0,width,height);
    }
  }


// Change bottom padding
void FXPacker::setPadBottom(FXint pb){
  if(padbottom!=pb){
    padbottom=pb;
    recalc();
    update(0,0,width,height);
    }
  }


// Change left padding
void FXPacker::setPadLeft(FXint pl){
  if(padleft!=pl){
    padleft=pl;
    recalc();
    update(0,0,width,height);
    }
  }


// Change right padding
void FXPacker::setPadRight(FXint pr){
  if(padright!=pr){
    padright=pr;
    recalc();
    update(0,0,width,height);
    }
  }


// Change horizontal spacing
void FXPacker::setHSpacing(FXint hs){
  if(hspacing!=hs){
    hspacing=hs;
    recalc();
    update(0,0,width,height);
    }
  }


// Change vertical spacing
void FXPacker::setVSpacing(FXint vs){
  if(vspacing!=vs){
    vspacing=vs;
    recalc();
    update(0,0,width,height);
    }
  }


// Compute minimum width based on child layout hints
int FXPacker::getDefaultWidth(){
  register FXint t,wcum,wmax,mw=0;
  register FXWindow* child;
  register FXuint hints,side;
  wmax=wcum=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getLast(); child; child=child->getPrev()){
    if(child->shown()){
      hints=child->getLayoutHints();
      side=hints&LAYOUT_SIDE_MASK;
      if(hints&LAYOUT_FIX_WIDTH) t=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) t=mw;
      else t=child->getDefaultWidth();
      if(hints&LAYOUT_FIX_X){ 
        t=child->getX()+t;
        if(t>wmax) wmax=t;
        }
      else if(side==LAYOUT_SIDE_LEFT || side==LAYOUT_SIDE_RIGHT){
        if(child->getNext()) wcum+=hspacing;
        wcum+=t;
        }
      else{
        if(t>wcum) wcum=t;
        }
      }
    }
  return padleft+padright+(border<<1)+FXMAX(wcum,wmax);
  }


// Compute minimum height based on child layout hints
int FXPacker::getDefaultHeight(){
  register FXint t,hcum,hmax,mh=0;
  register FXWindow* child;
  register FXuint hints,side;
  hmax=hcum=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getLast(); child; child=child->getPrev()){
    if(child->shown()){
      hints=child->getLayoutHints();
      side=hints&LAYOUT_SIDE_MASK;
      if(hints&LAYOUT_FIX_HEIGHT) t=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) t=mh;
      else t=child->getDefaultHeight();
      if(hints&LAYOUT_FIX_Y){
        t=child->getY()+t;
        if(t>hmax) hmax=t;
        }
      else if(side==LAYOUT_SIDE_TOP || side==LAYOUT_SIDE_BOTTOM){
        if(child->getNext()) hcum+=vspacing;
        hcum+=t;
        }
      else{
        if(t>hcum) hcum=t;
        }
      }
    }
  return padtop+padbottom+(border<<1)+FXMAX(hcum,hmax);
  }


// Recalculate layout
void FXPacker::layout(){
  FXint left,right,top,bottom;
  FXint mw=0,mh=0;
  FXWindow* child;
  FXint x,y,w,h;
  FXint extra_space,total_space;
  FXuint hints,side;
  
  // Placement rectangle; right/bottom non-inclusive
  left=border+padleft;
  right=width-border-padright;
  top=border+padtop;
  bottom=height-border-padbottom;
  
  // Get maximum child size
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  
  // Pack them in the cavity
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      side=hints&LAYOUT_SIDE_MASK;
      x=child->getX();
      y=child->getY();
      
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      
      if(side==LAYOUT_SIDE_LEFT){           // Left side
        if(!(hints&LAYOUT_FIX_Y)){
          extra_space=0;
          if((hints&LAYOUT_FILL_Y) && !(hints&LAYOUT_FIX_HEIGHT)){
            h=bottom-top;
            if(h<0) h=0;
            }
          else if(hints&LAYOUT_CENTER_Y){
            if(h<(bottom-top)) extra_space=(bottom-top-h)/2;
            }
          if(hints&LAYOUT_BOTTOM)
            y=bottom-extra_space-h;
          else /*hints&LAYOUT_TOP*/
            y=top+extra_space;
          }
        extra_space=0;
        total_space=0;
        if((hints&LAYOUT_FILL_X) && !(hints&LAYOUT_FIX_WIDTH)){
          w=right-left;
          if(w<0) w=0;
          }
        else if(hints&LAYOUT_CENTER_X){
          if(w<(right-left)){
            total_space=right-left-w;
            extra_space=total_space/2;
            }
          }
        x=left+extra_space;
        left+=(w+hspacing+total_space);
        }
      
      else if(side==LAYOUT_SIDE_RIGHT){     // Right side
        if(!(hints&LAYOUT_FIX_Y)){
          extra_space=0;
          if((hints&LAYOUT_FILL_Y) && !(hints&LAYOUT_FIX_HEIGHT)){
            h=bottom-top;
            if(h<0) h=0;
            }
          else if(hints&LAYOUT_CENTER_Y){
            if(h<(bottom-top)) extra_space=(bottom-top-h)/2;
            }
          if(hints&LAYOUT_BOTTOM)
            y=bottom-extra_space-h;
          else /*hints&LAYOUT_TOP*/
            y=top+extra_space;
          }
        extra_space=0;
        total_space=0;
        if((hints&LAYOUT_FILL_X) && !(hints&LAYOUT_FIX_WIDTH)){
          w=right-left;
          if(w<0) w=0;
          }
        else if(hints&LAYOUT_CENTER_X){
          if(w<(right-left)){
            total_space=right-left-w;
            extra_space=total_space/2;
            }
          }
        x=right-w-extra_space;
        right-=(w+hspacing+total_space);
        }
      
      else if(side==LAYOUT_SIDE_BOTTOM){    // Bottom side
        if(!(hints&LAYOUT_FIX_X)){
          extra_space=0;
          if((hints&LAYOUT_FILL_X) && !(hints&LAYOUT_FIX_WIDTH)){
            w=right-left;
            if(w<0) w=0;
            }
          else if(hints&LAYOUT_CENTER_X){
            if(w<(right-left)) extra_space=(right-left-w)/2;
            }
          if(hints&LAYOUT_RIGHT)
            x=right-extra_space-w;
          else /*hints&LAYOUT_LEFT*/
            x=left+extra_space;
          }
        extra_space=0;
        total_space=0;
        if((hints&LAYOUT_FILL_Y) && !(hints&LAYOUT_FIX_HEIGHT)){
          h=bottom-top;
          if(h<0) h=0;
          }
        else if(hints&LAYOUT_CENTER_Y){
          if(h<(bottom-top)){
            total_space=bottom-top-h;
            extra_space=total_space/2;
            }
          }
        y=bottom-h-extra_space;
        bottom-=(h+vspacing+total_space);
        }
      
      else if(side==LAYOUT_SIDE_TOP){       // Top side
        if(!(hints&LAYOUT_FIX_X)){
          extra_space=0;
          if((hints&LAYOUT_FILL_X) && !(hints&LAYOUT_FIX_WIDTH)){
            w=right-left;
            if(w<0) w=0;
            }
          else if(hints&LAYOUT_CENTER_X){
            if(w<(right-left)) extra_space=(right-left-w)/2;
            }
          if(hints&LAYOUT_RIGHT)
            x=right-extra_space-w;
          else /*hints&LAYOUT_LEFT*/
            x=left+extra_space;
          }
        extra_space=0;
        total_space=0;
        if((hints&LAYOUT_FILL_Y) && !(hints&LAYOUT_FIX_HEIGHT)){
          //if(h<(bottom-top)) h=bottom-top;
          h=bottom-top;
          if(h<0) h=0;
          }
        else if(hints&LAYOUT_CENTER_Y){
          if(h<(bottom-top)){
            total_space=bottom-top-h;
            extra_space=total_space/2;
            }
          }
        y=top+extra_space;
        top+=(h+vspacing+total_space);
        }
      
      // Place it
      child->position(x,y,w,h);
      }
    }
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Save object to stream
void FXPacker::save(FXStream& store) const {
  FXComposite::save(store);
  store << padtop << padbottom << padleft << padright << hspacing << vspacing << border;
  }


// Load object from stream
void FXPacker::load(FXStream& store){
  FXComposite::load(store);
  store >> padtop >> padbottom >> padleft >> padright >> hspacing >> vspacing >> border;
  }  
