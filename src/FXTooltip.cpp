/********************************************************************************
*                                                                               *
*                           T o o l t i p   W i d g e t                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXTooltip.cpp,v 1.22 2002/01/18 22:43:07 jeroen Exp $                    *
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
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXCursor.h"
#include "FXTooltip.h"

/*
  Notes:
  - Initial colors are now obtained from FXApp and therefore
    from the system registry.
*/

#define HSPACE  4
#define VSPACE  2


/*******************************************************************************/

// Map
FXDEFMAP(FXTooltip) FXTooltipMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTooltip::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXTooltip::onUpdate),
  FXMAPFUNC(SEL_TIMEOUT,FXTooltip::ID_TIP_SHOW,FXTooltip::onTipShow),
  FXMAPFUNC(SEL_TIMEOUT,FXTooltip::ID_TIP_HIDE,FXTooltip::onTipHide),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXTooltip::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXTooltip::onCmdGetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXTooltip,FXShell,FXTooltipMap,ARRAYNUMBER(FXTooltipMap))


// Deserialization
FXTooltip::FXTooltip(){
  font=NULL;
  textColor=0;
  timer=NULL;
  popped=FALSE;
  }

// Create a toplevel window
FXTooltip::FXTooltip(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXShell(a,opts,x,y,w,h),label("Tooltip"){
  font=getApp()->getNormalFont();
  textColor=getApp()->getTipforeColor();
  backColor=getApp()->getTipbackColor();
  timer=NULL;
  popped=FALSE;
  }


// Tooltips do override-redirect
FXbool FXTooltip::doesOverrideRedirect() const { return TRUE; }


// Tooltips do save-unders
FXbool FXTooltip::doesSaveUnder() const { return TRUE; }


#ifdef WIN32
const char* FXTooltip::GetClass() const { return "FXPopup"; }
#endif


// Create window
void FXTooltip::create(){
  FXShell::create();
  font->create();
  }


// Detach window
void FXTooltip::detach(){
  FXShell::detach();
  font->detach();
  }


// Show window
void FXTooltip::show(){
  FXShell::show();
  raise();
  }


// Get default width
FXint FXTooltip::getDefaultWidth(){
  const FXchar *beg,*end;
  FXint w,tw=0;
  beg=label.text();
  if(beg){
    do{
      end=beg;
      while(*end!='\0' && *end!='\n') end++;
      if((w=font->getTextWidth(beg,end-beg))>tw) tw=w;
      beg=end+1;
      }
    while(*end!='\0');
    }
  return tw+HSPACE+HSPACE+2;
  }


// Get default height
FXint FXTooltip::getDefaultHeight(){
  const FXchar *beg,*end;
  FXint th=0;
  beg=label.text();
  if(beg){
    do{
      end=beg;
      while(*end!='\0' && *end!='\n') end++;
      th+=font->getFontHeight();
      beg=end+1;
      }
    while(*end!='\0');
    }
  return th+VSPACE+VSPACE+2;
  }


// Handle repaint
long FXTooltip::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  const FXchar *beg,*end;
  FXint tx,ty;
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  dc.setForeground(textColor);
  dc.setTextFont(font);
  dc.drawRectangle(0,0,width-1,height-1);
  beg=label.text();
  if(beg){
    tx=1+HSPACE;
    ty=1+VSPACE+font->getFontAscent();
    do{
      end=beg;
      while(*end!='\0' && *end!='\n') end++;
      dc.drawText(tx,ty,beg,end-beg);
      ty+=font->getFontHeight();
      beg=end+1;
      }
    while(*end!='\0');
    }
  return 1;
  }


// Place the tool tip
void FXTooltip::place(FXint x,FXint y){
  FXint rw=getRoot()->getWidth();
  FXint rh=getRoot()->getHeight();
  FXint w=getDefaultWidth();
  FXint h=getDefaultHeight();
  FXint px,py;
  px=x+16-w/3;
  py=y+20;
  if(px+w>rw) px=rw-w;
  if(px<0) px=0;
  if(py+h>rh){ py=rh-h; if(py<=y && y<py+h) py=y-h-10; }
  if(py<0) py=0;
  position(px,py,w,h);
  }


// Automatically place tooltip
void FXTooltip::autoplace(){
  FXint x,y; FXuint state;
  getRoot()->getCursorPosition(x,y,state);
  place(x,y);
  }


// Update tooltip based on widget under cursor
long FXTooltip::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *helpsource=getApp()->getCursorWindow();
  FXWindow::onUpdate(sender,sel,ptr);
  if(helpsource && helpsource->handle(this,MKUINT(FXWindow::ID_QUERY_TIP,SEL_UPDATE),ptr)){
    if(!popped){
      popped=TRUE;
      if(!shown()){
        if(timer) getApp()->removeTimeout(timer);
        timer=getApp()->addTimeout(getApp()->getTooltipPause(),this,ID_TIP_SHOW);
        return 1;
        }
      autoplace();
      }
    return 1;
    }
  if(timer){getApp()->removeTimeout(timer);timer=NULL;}
  popped=FALSE;
  hide();
  return 1;
  }


// Pop the tool tip now
long FXTooltip::onTipShow(FXObject*,FXSelector,void*){
  timer=NULL;
  if(!label.empty()){
    autoplace();
    show();
    if(!(options&TOOLTIP_PERMANENT)){
      FXint timeoutms=getApp()->getTooltipTime();
      // Text length dependent tooltip display time;
      // Contributed by: leonard@hipgraphics.com
      if(options&TOOLTIP_VARIABLE){
        timeoutms=timeoutms/4+(timeoutms*label.length())/64;
        }
      timer=getApp()->addTimeout(timeoutms,this,ID_TIP_HIDE);
      }
    }
  return 1;
  }


// Tip should hide now
long FXTooltip::onTipHide(FXObject*,FXSelector,void*){
  timer=NULL;
  hide();
  return 1;
  }


// Change value
long FXTooltip::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdSetStringValue: NULL pointer.\n",getClassName()); }
  setText(*((FXString*)ptr));
  return 1;
  }


// Obtain value
long FXTooltip::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdGetStringValue: NULL pointer.\n",getClassName()); }
  *((FXString*)ptr)=getText();
  return 1;
  }


// Change text
void FXTooltip::setText(const FXString& text){
  if(label!=text){
    label=text;
    recalc();
    popped=FALSE;       // If text changes, pop it up again
    update();
    }
  }


// Change the font
void FXTooltip::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Set text color
void FXTooltip::setTextColor(FXColor clr){
  if(clr!=textColor){
    textColor=clr;
    update();
    }
  }


// Save data
void FXTooltip::save(FXStream& store) const {
  FXShell::save(store);
  store << label;
  store << font;
  store << textColor;
  }


// Load data
void FXTooltip::load(FXStream& store){
  FXShell::load(store);
  store >> label;
  store >> font;
  store >> textColor;
  }


// Destroy label
FXTooltip::~FXTooltip(){
  if(timer) getApp()->removeTimeout(timer);
  font=(FXFont*)-1;
  timer=(FXTimer*)-1;
  }
