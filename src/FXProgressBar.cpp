/********************************************************************************
*                                                                               *
*                      P r o g r e s s B a r   W i d g e t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* Contributed by: Jonathan Bush                                                 *
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
* $Id: FXProgressBar.cpp,v 1.10 1998/08/28 22:58:51 jvz Exp $                  *
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
#include "FXLabel.h"
#include "FXProgressBar.h"


/*
  Notes:
  - Reduced flicker by not drawing background at all.
  - Reduced flicker by setting clip rectangle to only redraw interior.
  - Progress bar has a target, as it can send update messages.
*/


/*******************************************************************************/

    
// Map
FXDEFMAP(FXProgressBar) FXProgressBarMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXProgressBar::onPaint),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXProgressBar::onCmdSetValue),
  };


// Object implementation
FXIMPLEMENT(FXProgressBar,FXCell,FXProgressBarMap,ARRAYNUMBER(FXProgressBarMap))    

  

// Make progress bar
FXProgressBar::FXProgressBar(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXCell(p,opts,x,y,w,h,pl,pr,pt,pb){
  target=tgt;
  message=sel;
  progress=0;
  total=100;
  barsize=5;
  font=getApp()->normalFont;
  barBGColor=0;
  barColor=0;
  textNumColor=0;
  textAltColor=0;
  }


// Get minimum width
FXint FXProgressBar::getDefaultWidth(){
  FXint w=barsize;
  if(options&PROGRESSBAR_PERCENTAGE){
    w=font->getTextWidth("100%",4);
    if(w<barsize) w=barsize;
    }
  return w+padleft+padright+border;  
  }


// Get minimum height
FXint FXProgressBar::getDefaultHeight(){
  FXint h=barsize;
  if(options&PROGRESSBAR_PERCENTAGE){
    h=font->getFontHeight();
    if(h<barsize) h=barsize;
    }
  return h+padtop+padbottom+border;  
  }


// Create X window
void FXProgressBar::create(){ 
  FXCell::create();
  barColor=acquireColor(FXRGB(0,0,255));  //default colors blue&white
  textNumColor=acquireColor(FXRGB(0,0,255));
  barBGColor=acquireColor(FXRGB(255,255,255));
  textAltColor=acquireColor(FXRGB(255,255,255));
  setBackground(barBGColor);
  font->create();
  }


// Update progress value from a message
long FXProgressBar::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setProgress((FXuint)ptr);
  return 1;
  }


// Draw the progress bar
long FXProgressBar::onPaint(FXObject*,FXSelector,void *ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint percent,barlength,barfilled,tx,ty,tw,th,n;
  FXchar numtext[5];

  // Only draw what's changed
  setClipRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  
  // Just draw the frame, not the background
  drawFrame(0,0,width,height);
    
  // Vertical
  if(options&PROGRESSBAR_VERTICAL){
    
    // If total is 0, it's 100%
    barlength=height-border-border;
    barfilled=barlength;
    percent=100;
    if(total!=0){
      barfilled=(FXuint) (((double)progress * (double)barlength) / (double)total);
      percent=(FXuint) (((double)progress * 100.0) / (double)total);
      }
    
    // Draw completed bar
    if(0<barfilled){
      setForeground(barColor);
      fillRectangle(border,border,width-(border<<1),barfilled);
      }
    
    // Draw uncompleted bar
    if(barfilled<barlength){
      setForeground(barBGColor);
      fillRectangle(border,border+barfilled,width-(border<<1),barlength-barfilled);
      }
    
    // Draw text 
    if(options&PROGRESSBAR_PERCENTAGE){
      sprintf(numtext,"%d%%",percent);
      n=strlen(numtext);
      tw=font->getTextWidth(numtext,n);
      th=font->getFontHeight();
      ty=(height-th)/2+font->getFontAscent();
      tx=(width-tw)/2;
      if(border+barfilled<=ty){           // In right side
        setForeground(textNumColor);
        drawText(tx,ty,numtext,n);
        }
      else if(ty+th<=border+barfilled){   // In left side
        setForeground(textAltColor);
        drawText(tx,ty,numtext,n);
        }
      else{                               // In between!
        setForeground(textAltColor);
        setClipRectangle(border,border,width,barfilled);
        drawText(tx,ty,numtext,n);
        setForeground(textNumColor);
        setClipRectangle(border,border+barfilled,width,barlength-barfilled);
        drawText(tx,ty,numtext,n);
        clearClipRectangle();
        }
      }
    }
  
  // Horizontal
  else{
    
    // If total is 0, it's 100%
    barlength=width-border-border;
    barfilled=barlength;
    percent=100;
    if(total!=0){
      barfilled=(FXuint) (((double)progress * (double)barlength) / (double)total);
      percent=(FXuint) (((double)progress * 100.0) / (double)total);
      }
    
    // Draw completed bar
    if(0<barfilled){
      setForeground(barColor);
      fillRectangle(border,border,barfilled,height-(border<<1));
      }
    
    // Draw uncompleted bar
    if(barfilled<barlength){
      setForeground(barBGColor);
      fillRectangle(border+barfilled,border,barlength-barfilled,height-(border<<1));
      }
    
    // Draw text 
    if(options&PROGRESSBAR_PERCENTAGE){
      sprintf(numtext,"%d%%",percent);
      n=strlen(numtext);
      tw=font->getTextWidth(numtext,n);
      th=font->getFontHeight();
      ty=(height-th)/2+font->getFontAscent();
      tx=(width-tw)/2;
      if(border+barfilled<=tx){           // In right side
        setForeground(textNumColor);
        drawText(tx,ty,numtext,n);
        }
      else if(tx+tw<=border+barfilled){   // In left side
        setForeground(textAltColor);
        drawText(tx,ty,numtext,n);
        }
      else{                               // In between!
        setForeground(textAltColor);
        setClipRectangle(border,border,barfilled,height);
        drawText(tx,ty,numtext,n);
        setForeground(textNumColor);
        setClipRectangle(border+barfilled,border,barlength-barfilled,height);
        drawText(tx,ty,numtext,n);
        clearClipRectangle();
        }
      }
    }
  clearClipRectangle();
  return 1;
  }
    

// Set amount of progress made
void FXProgressBar::setProgress(FXuint value){ 
  if(value!=progress){
    progress=value; 
    if(progress>total) progress=total; 
    update(border,border,width-(border<<1),height-(border<<1)); 
    }
  }


// Increment amount of progress
void FXProgressBar::increment(FXuint value){ 
  if(value!=0){
    progress+=value; 
    if(progress>total) progress=total;
    update(border,border,width-(border<<1),height-(border<<1)); 
    }
  }


// Set total amount to completion
void FXProgressBar::setTotal(FXuint value){
  if(value!=total){
    total=value;
    update(border,border,width-(border<<1),height-(border<<1)); 
    }
  }


// Change bar color
void FXProgressBar::setBarColor(FXPixel clr){  
  barColor=clr;  
  update(border,border,width-(border<<1),height-(border<<1)); 
  }


// Change bar background color
void FXProgressBar::setBarBGColor(FXPixel clr){  
  barBGColor=clr;  
  update(border,border,width-(border<<1),height-(border<<1)); 
  }


// Change text foreground color
void FXProgressBar::setTextColor(FXPixel clr) {   
  textNumColor=clr;   
  update(0,0,width,height);
  }


// Change alternate text color
void FXProgressBar::setTextAltColor(FXPixel clr) {  
  textAltColor=clr;   
  update(0,0,width,height);
  }


// Hide percentage display
void FXProgressBar::hideNumber(){ 
  if(options&PROGRESSBAR_PERCENTAGE){
    options&=~PROGRESSBAR_PERCENTAGE;
    recalc();
    update(0,0,width,height);
    }
  }


// Show percentage display
void FXProgressBar::showNumber(){ 
  if(!(options&PROGRESSBAR_PERCENTAGE)){
    options|=PROGRESSBAR_PERCENTAGE;
    recalc();
    update(0,0,width,height);
    }
  }


void FXProgressBar::setBarSize(FXint size){
  if(size<1){ fxerror("%s::setBarSize: zero or negative barsize specified.\n",getClassName()); }
  if(barsize!=size){
    barsize=size;
    recalc();
    update(0,0,width,height);
    }
  }


// Change the font
void FXProgressBar::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Destroy
FXProgressBar::~FXProgressBar(){
  font=(FXFont*)-1;
  }

  
