/********************************************************************************
*                                                                               *
*                 V e r t i c a l   C o n t a i n e r   O b j e c t             *
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
* $Id: FXVerticalFrame.cpp,v 1.13 1998/09/10 15:15:52 jvz Exp $               *
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
#include "FXVerticalFrame.h"
#include "FXShell.h"

  
/*
  Notes:
  - Filled items shrink as well as stretch.
  - Stretch is proportional to default size; this way, at default size, 
    it is exactly correct.
*/



/*******************************************************************************/

// Map
FXDEFMAP(FXVerticalFrame) FXVerticalFrameMap[]={
  FXMAPFUNC(SEL_FOCUS_UP,0,FXVerticalFrame::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXVerticalFrame::onFocusDown),
  };


// Object implementation
FXIMPLEMENT(FXVerticalFrame,FXPacker,FXVerticalFrameMap,ARRAYNUMBER(FXVerticalFrameMap))


// Make a vertical one
FXVerticalFrame::FXVerticalFrame(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXPacker(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  }


// Focus moved up
long FXVerticalFrame::onFocusUp(FXObject* sender,FXSelector sel,void* ptr){
  return FXPacker::onFocusPrev(sender,sel,ptr);
  }


// Focus moved down
long FXVerticalFrame::onFocusDown(FXObject* sender,FXSelector sel,void* ptr){
  return FXPacker::onFocusNext(sender,sel,ptr);
  }


// Compute minimum width based on child layout hints
int FXVerticalFrame::getDefaultWidth(){
  register FXint w,wmax,mw=0;
  register FXWindow* child;
  register FXuint hints;
  wmax=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if(hints&LAYOUT_FIX_X){ w=child->getX()+w; }
      if(wmax<w) wmax=w;
      }
    }
  return padleft+padright+wmax+(border<<1);
  }
  

// Compute minimum height based on child layout hints
int FXVerticalFrame::getDefaultHeight(){
  register FXint h,hcum,hmax,numc,mh=0;
  register FXWindow* child;
  register FXuint hints;
  hcum=hmax=numc=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if(hints&LAYOUT_FIX_Y){ h=child->getY()+h; } else { hcum+=h; numc++; }
      if(hmax<h) hmax=h;
      }
    }
  if(numc>1) hcum+=(numc-1)*vspacing;
  if(hmax<hcum) hmax=hcum;
  return padtop+padbottom+hmax+(border<<1);
  }


// Recalculate layout
void FXVerticalFrame::layout(){
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
    remain=bottom-top;

    // Get maximum child size
    if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
    if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();

    // Find number of paddable children and total height
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(!(hints&LAYOUT_FIX_Y)){
          if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
          else if(options&PACK_UNIFORM_HEIGHT) h=mh;
          else h=child->getDefaultHeight();
          FXASSERT(h>0);
          if((hints&LAYOUT_CENTER_Y) || ((hints&LAYOUT_FILL_Y) && !(hints&LAYOUT_FIX_HEIGHT))) sumexpand+=h; else remain-=h;
          numc++;
          }
        }
      }

    // Child spacing
    if(numc>1) remain-=vspacing*(numc-1);

    // Do the layout
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();

        // Layout child in X
        x=child->getX();
        if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
        else if(options&PACK_UNIFORM_WIDTH) w=mw;
        else w=child->getDefaultWidth();
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

        // Layout child in Y
        y=child->getY();
        if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
        else if(options&PACK_UNIFORM_HEIGHT) h=mh;
        else h=child->getDefaultHeight();
        if(!(hints&LAYOUT_FIX_Y)){
          extra_space=0;
          total_space=0;
          if((hints&LAYOUT_FILL_Y) && !(hints&LAYOUT_FIX_HEIGHT)){
            t=h*remain;
            FXASSERT(sumexpand>0);
            h=t/sumexpand;
            e+=t%sumexpand;
            if(e>=sumexpand){h++;e-=sumexpand;}
            }
          else if(hints&LAYOUT_CENTER_Y){
            t=h*remain;
            FXASSERT(sumexpand>0);
            total_space=t/sumexpand-h;
            e+=t%sumexpand;
            if(e>=sumexpand){total_space++;e-=sumexpand;}
            extra_space=total_space/2;
            }
          if(hints&LAYOUT_BOTTOM){
            y=bottom-h-extra_space;
            bottom=bottom-h-hspacing-total_space;
            }
          else{/*hints&LAYOUT_TOP*/
            y=top+extra_space;
            top=top+h+vspacing+total_space;
            }
          }
        child->position(x,y,w,h);
        }
      }
    }
  flags&=~FLAG_DIRTY;
  }

