/********************************************************************************
*                                                                               *
*              H o r i z o n t a l   C o n t a i n e r   O b j e c t            *
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
* $Id: FXHorizontalFrame.cpp,v 1.13 1998/09/10 15:15:50 jvz Exp $             *
********************************************************************************/
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
#include "FXHorizontalFrame.h"
#include "FXShell.h"

  
/*
  Notes:
  - Filled items shrink as well as stretch.
  - Stretch is proportional to default size; this way, at default size, 
    it is exactly correct.
*/




/*******************************************************************************/

// Map
FXDEFMAP(FXHorizontalFrame) FXHorizontalFrameMap[]={
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXHorizontalFrame::onFocusLeft),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXHorizontalFrame::onFocusRight),
  };


// Object implementation
FXIMPLEMENT(FXHorizontalFrame,FXPacker,FXHorizontalFrameMap,ARRAYNUMBER(FXHorizontalFrameMap))


// Make a horizontal one
FXHorizontalFrame::FXHorizontalFrame(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXPacker(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  }


// Focus moved to left
long FXHorizontalFrame::onFocusLeft(FXObject* sender,FXSelector sel,void* ptr){
  return FXPacker::onFocusPrev(sender,sel,ptr);
  }


// Focus moved to right
long FXHorizontalFrame::onFocusRight(FXObject* sender,FXSelector sel,void* ptr){
  return FXPacker::onFocusNext(sender,sel,ptr);
  }


// Compute minimum width based on child layout hints
int FXHorizontalFrame::getDefaultWidth(){
  register FXint w,wcum,wmax,numc,mw=0;
  register FXWindow* child;
  register FXuint hints;
  wcum=wmax=numc=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if(hints&LAYOUT_FIX_X){ w=child->getX()+w; } else { wcum+=w; numc++; }
      if(wmax<w) wmax=w;
      }
    }
  if(numc>1) wcum+=(numc-1)*hspacing;
  if(wmax<wcum) wmax=wcum;
  return padleft+padright+wmax+(border<<1);
  }
  

// Compute minimum height based on child layout hints
int FXHorizontalFrame::getDefaultHeight(){
  register FXint h,hmax,mh=0;
  register FXWindow* child;
  register FXuint hints;
  hmax=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if(hints&LAYOUT_FIX_Y){ h=child->getY()+h; }
      if(hmax<h) hmax=h;
      }
    }
  return padtop+padbottom+hmax+(border<<1);
  }


// Recalculate layout
void FXHorizontalFrame::layout(){
  FXint left,right,top,bottom;
  FXint mw=0,mh=0;
  FXint remain,extra_space,total_space,t;
  FXint x,y,w,h;
  FXint numc=0;
  FXint sumexpand=0;
  FXint e=0;
  FXuint hints;
  FXWindow* child;

  if(getFirst()){

    // Placement rectangle; right/bottom non-inclusive
    left=border+padleft;
    right=width-border-padright;
    top=border+padtop;
    bottom=height-border-padbottom;
    remain=right-left;

    // Get maximum child size
    if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
    if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();

    // Find number of paddable children and total width
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(!(hints&LAYOUT_FIX_X)){
          if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
          else if(options&PACK_UNIFORM_WIDTH) w=mw;
          else w=child->getDefaultWidth();
          FXASSERT(w>0);
          if((hints&LAYOUT_CENTER_X) || ((hints&LAYOUT_FILL_X) && !(hints&LAYOUT_FIX_WIDTH))) sumexpand+=w; else remain-=w;
          numc++;
          }
        }
      }

    // Child spacing
    if(numc>1) remain-=hspacing*(numc-1);

    // Do the layout
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();

        // Layout child in Y
        y=child->getY();
        if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
        else if(options&PACK_UNIFORM_HEIGHT) h=mh;
        else h=child->getDefaultHeight();
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

        // Layout child in X
        x=child->getX();
        if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
        else if(options&PACK_UNIFORM_WIDTH) w=mw;
        else w=child->getDefaultWidth();
        if(!(hints&LAYOUT_FIX_X)){
          extra_space=0;
          total_space=0;
          if((hints&LAYOUT_FILL_X) && !(hints&LAYOUT_FIX_WIDTH)){
            t=w*remain;
            FXASSERT(sumexpand>0);
            w=t/sumexpand;
            e+=t%sumexpand;
            if(e>=sumexpand){w++;e-=sumexpand;}
            }
          else if(hints&LAYOUT_CENTER_X){
            t=w*remain;
            FXASSERT(sumexpand>0);
            total_space=t/sumexpand-w;
            e+=t%sumexpand;
            if(e>=sumexpand){total_space++;e-=sumexpand;}
            extra_space=total_space/2;
            }
          if(hints&LAYOUT_RIGHT){
            x=right-w-extra_space;
            right=right-w-hspacing-total_space;
            }
          else{/*hints&LAYOUT_LEFT*/
            x=left+extra_space;
            left=left+w+hspacing+total_space;
            }
          }
        child->position(x,y,w,h);
        }
      }
    }
  flags&=~FLAG_DIRTY;
  }


