/********************************************************************************
*                                                                               *
*                      S e p a r a t o r   W i d g e t s                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXSeparator.cpp,v 1.13 2002/01/18 22:43:04 jeroen Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXSeparator.h"



/*
  Notes:
  - When changing icon/font/etc, we should only recalc and update when it's different.
  - When text changes, do we delete the hot key, or parse it from the new label?
  - It makes sense for certain ``passive'' widgets such as labels to have onUpdate;
    for example, to show/hide/whatever based on changing data structures.
*/

#define SEPARATOR_EXTRA 2

/*******************************************************************************/



// Map
FXDEFMAP(FXHorizontalSeparator) FXHorizontalSeparatorMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXHorizontalSeparator::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXHorizontalSeparator,FXFrame,FXHorizontalSeparatorMap,ARRAYNUMBER(FXHorizontalSeparatorMap))


// Construct and init
FXHorizontalSeparator::FXHorizontalSeparator(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  }


// Get default size
FXint FXHorizontalSeparator::getDefaultWidth(){
  return padleft+padright+(border<<1);
  }


FXint FXHorizontalSeparator::getDefaultHeight(){
  FXint h=(options&(SEPARATOR_GROOVE|SEPARATOR_RIDGE)) ? 2 : 1;
  return h+padtop+padbottom+(border<<1);
  }


// Handle repaint
long FXHorizontalSeparator::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint yy,hh;
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  drawFrame(dc,0,0,width,height);
  hh=(options&(SEPARATOR_GROOVE|SEPARATOR_RIDGE)) ? 2 : 1;
  yy=border+padtop+(height-padbottom-padtop-(border<<1)-hh)/2;
  if(options&SEPARATOR_GROOVE){
    dc.setForeground(shadowColor);
    dc.drawLine(border+padleft,yy,width-padright-padleft-(border<<1),yy);
    dc.setForeground(hiliteColor);
    dc.drawLine(border+padleft,yy+1,width-padright-padleft-(border<<1),yy+1);
    }
  else if(options&SEPARATOR_RIDGE){
    dc.setForeground(hiliteColor);
    dc.drawLine(border+padleft,yy,width-padright-padleft-(border<<1),yy);
    dc.setForeground(shadowColor);
    dc.drawLine(border+padleft,yy+1,width-padright-padleft-(border<<1),yy+1);
    }
  else if(options&SEPARATOR_LINE){
    dc.setForeground(borderColor);
    dc.drawLine(border+padleft,yy,width-padright-padleft-(border<<1),yy);
    }
  return 1;
  }


/*******************************************************************************/


// Map
FXDEFMAP(FXVerticalSeparator) FXVerticalSeparatorMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXVerticalSeparator::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXVerticalSeparator,FXFrame,FXVerticalSeparatorMap,ARRAYNUMBER(FXVerticalSeparatorMap))


// Construct and init
FXVerticalSeparator::FXVerticalSeparator(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  }


// Get default size
FXint FXVerticalSeparator::getDefaultWidth(){
  FXint w=(options&(SEPARATOR_GROOVE|SEPARATOR_RIDGE)) ? 2 : 1;
  return w+padleft+padright+(border<<1);
  }


FXint FXVerticalSeparator::getDefaultHeight(){
  return padtop+padbottom+(border<<1);
  }


// Handle repaint
long FXVerticalSeparator::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint ww,xx;
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  drawFrame(dc,0,0,width,height);
  ww=(options&(SEPARATOR_GROOVE|SEPARATOR_RIDGE)) ? 2 : 1;
  xx=border+padleft+(width-padleft-padright-(border<<1)-ww)/2;
  if(options&SEPARATOR_GROOVE){
    dc.setForeground(shadowColor);
    dc.drawLine(xx,padtop+border,xx,height-padtop-padbottom-(border<<1));
    dc.setForeground(hiliteColor);
    dc.drawLine(xx+1,padtop+border,xx+1,height-padtop-padbottom-(border<<1));
    }
  else if(options&SEPARATOR_RIDGE){
    dc.setForeground(hiliteColor);
    dc.drawLine(xx,padtop+border,xx,height-padtop-padbottom-(border<<1));
    dc.setForeground(shadowColor);
    dc.drawLine(xx+1,padtop+border,xx+1,height-padtop-padbottom-(border<<1));
    }
  else if(options&SEPARATOR_LINE){
    dc.setForeground(borderColor);
    dc.drawLine(xx,padtop+border,xx,height-padtop-padbottom-(border<<1));
    }
  return 1;
  }



