/********************************************************************************
*                                                                               *
*                     A r r o w   B u t t o n    O b j e c t                    *
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
* $Id: FXArrowButton.cpp,v 1.5 1998/10/30 05:29:49 jeroen Exp $                 *
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
#include "FXLabel.h"
#include "FXButton.h"
#include "FXArrowButton.h"
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenu.h"
#include "FXTooltip.h"

/*
  To do:
  - Add ``flat'' toolbar style also
*/


/*******************************************************************************/


// Map
FXDEFMAP(FXArrowButton) FXArrowButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXArrowButton::onPaint),
  FXMAPFUNC(SEL_ACTIVATE,0,FXArrowButton::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXArrowButton::onDeactivate),
  FXMAPFUNC(SEL_TIMEOUT,FXArrowButton::ID_REPEAT,FXArrowButton::onRepeat),
  };


// Object implementation
FXIMPLEMENT(FXArrowButton,FXButton,FXArrowButtonMap,ARRAYNUMBER(FXArrowButtonMap))


// For deserialization
FXArrowButton::FXArrowButton(){
  repeater=NULL;
  }


// Make a text button
FXArrowButton::FXArrowButton(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXButton(p,NULL,NULL,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb){
  repeater=NULL;
  }


// Get default size
FXint FXArrowButton::getDefaultWidth(){
  return padleft+padright+9+(border<<1);
  }


FXint FXArrowButton::getDefaultHeight(){
  return padtop+padbottom+9+(border<<1);
  }

  
// Handle repaint 
long FXArrowButton::onPaint(FXObject*,FXSelector,void*){
  FXPoint points[3];
  int xx,yy,ww,hh,q;
  FXASSERT(xid!=0);
  
  // With borders
  if(options&(FRAME_RAISED|FRAME_SUNKEN)){
    if(isDefault()){
      drawBorderRectangle(0,0,width,height);
      if(!isEnabled() || (state==STATE_UP)){
        setForeground(backColor);
        fillRectangle(border+1,border+1,width-border*2-2,height-border*2-2);
        if(options&FRAME_THICK) drawDoubleRaisedRectangle(1,1,width-1,height-1);
        else drawRaisedRectangle(1,1,width-1,height-1);
        }
      else{
        if(state==STATE_ENGAGED) setForeground(hiliteColor); else setForeground(backColor);
        fillRectangle(border,border,width-border*2-1,height-border*2-1);
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width-1,height-1);
        else drawSunkenRectangle(0,0,width-1,height-1);
        }
      }
    else{
      if(!isEnabled() || (state==STATE_UP)){
        setForeground(backColor);
        fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleRaisedRectangle(0,0,width,height);
        else drawRaisedRectangle(0,0,width,height);
        }
      else{
        if(state==STATE_ENGAGED) setForeground(hiliteColor); else setForeground(backColor);
        fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(0,0,width,height);
        else drawSunkenRectangle(0,0,width,height);
        }
      }
    }
  
  // No borders
  else{
    if(isEnabled() && (state==STATE_ENGAGED)){
      setForeground(hiliteColor);
      fillRectangle(0,0,width,height);
      }
    else{
      setForeground(backColor);
      fillRectangle(0,0,width,height);
      }
    }

  // Compute size of the arrows....
  ww=width-padleft-padright-(border<<1);
  hh=height-padtop-padbottom-(border<<1);
  if(options&(ARROW_UP|ARROW_DOWN)){
    q=ww|1; if(q>(hh<<1)) q=(hh<<1)-1;
    ww=q; hh=q>>1;
    } 
  else{
    q=hh|1; if(q>(ww<<1)) q=(ww<<1)-1;
    ww=q>>1; hh=q;
    }

  if(options&JUSTIFY_LEFT) xx=padleft+border;
  else if(options&JUSTIFY_RIGHT) xx=width-ww-padright-border;
  else xx=(width-ww)/2;

  if(options&JUSTIFY_TOP) yy=padtop+border;
  else if(options&JUSTIFY_BOTTOM) yy=height-hh-padbottom-border;
  else yy=(height-hh)/2;

  if(state){ ++xx; ++yy; }

  // Draw background
  if(state==STATE_ENGAGED){
    setForeground(hiliteColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    setForeground(hiliteColor);
    drawLine(border,border,width-border-1,border);
    }
  else{
    setForeground(backColor);
    fillRectangle(border,border,width-border*2,height-border*2);
    }

  if(isEnabled())
    setForeground(textColor);
  else
    setForeground(shadowColor);

  // NB Size of arrow should stretch
  if(options&ARROW_UP){
    points[0].x=xx+(ww>>1);
    points[0].y=yy-1;
    points[1].x=xx;
    points[1].y=yy+hh;
    points[2].x=xx+ww;
    points[2].y=yy+hh;
    }
  else if(options&ARROW_DOWN){
    points[0].x=xx+1;
    points[0].y=yy;
    points[1].x=xx+ww-1;
    points[1].y=yy;
    points[2].x=xx+(ww>>1);
    points[2].y=yy+hh;
    }
  else if(options&ARROW_LEFT){
    points[0].x=xx+ww;
    points[0].y=yy;
    points[1].x=xx+ww;
    points[1].y=yy+hh-1;
    points[2].x=xx;
    points[2].y=yy+(hh>>1);
    }
  else{ /*options&ARROW_RIGHT*/
    points[0].x=xx;
    points[0].y=yy;
    points[1].x=xx;
    points[1].y=yy+hh-1;
    points[2].x=xx+ww;
    points[2].y=yy+(hh>>1);
    }
  fillPolygon(points,3);
  return 1;
  }


// Button being activated
long FXArrowButton::onActivate(FXObject* sender,FXSelector sel,void* ptr){
  flags|=FLAG_PRESSED;
  flags&=~FLAG_UPDATE;
  flags&=~FLAG_FIRED;
  if(state!=STATE_ENGAGED) setState(STATE_DOWN);
  if(options&ARROW_REPEAT){ repeater=getApp()->addTimeout(getApp()->scrollSpeed,this,ID_REPEAT); }
  return 1;
  }
  

// Button being deactivated
long FXArrowButton::onDeactivate(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXuint click=(state==STATE_DOWN);
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  if(state!=STATE_ENGAGED) setState(STATE_UP);
  if(repeater) getApp()->removeTimeout(repeater);
  if(!(flags&FLAG_FIRED)){ handle(this,MKUINT(0,SEL_CLICKED),(void*)click); }
  flags&=~FLAG_FIRED;
  return 1;
  }


// Repeat a click automatically
long FXArrowButton::onRepeat(FXObject* sender,FXSelector sel,void* ptr){
  FXuint click=(state==STATE_DOWN);
  handle(this,MKUINT(0,SEL_CLICKED),(void*)click);
  repeater=getApp()->addTimeout(getApp()->scrollSpeed,this,ID_REPEAT);
  flags|=FLAG_FIRED;
  return 1;
  }


// Kill the timer
FXArrowButton::~FXArrowButton(){
  if(repeater) getApp()->removeTimeout(repeater);
  repeater=(FXTimer*)-1;
  }
