/********************************************************************************
*                                                                               *
*                               H e a d e r   O b j e c t                       *
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
* $Id: FXHeader.cpp,v 1.18 1998/09/16 22:02:03 jvz Exp $                      *
********************************************************************************/
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
#include "FXHeader.h"

// Private header files
#include "FXHeaderItem.h"

#define FUDGE   8


/*
  To do:
  - Ability to have button behaviour
  - Icon drawing
  - Justify modes
  - Honor frame border styles
  - Need to have font class
  - Should probably derive from FXCell!!
  - FXHeaderItem should perhaps derive from FXItem.
  - Should be able to be driven by keyboard.
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXHeader) FXHeaderMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXHeader::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXHeader::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXHeader::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXHeader::onLeftBtnRelease),
  };


// Object implementation
FXIMPLEMENT(FXHeader,FXFrame,FXHeaderMap,ARRAYNUMBER(FXHeaderMap))

  
// Make a Header
FXHeader::FXHeader(){
  flags|=FLAG_ENABLED;
  firstitem=NULL;
  lastitem=NULL;
  before=NULL;
  font=(FXFont*)-1;
  textColor=0;
  split=0;
  off=0;
  }


// Make a Header
FXHeader::FXHeader(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFrame(p,opts,x,y,w,h){
  defaultCursor=getApp()->arrowCursor;
  dragCursor=getApp()->hsplitCursor;
  font=getApp()->normalFont;
  textColor=0;
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  firstitem=NULL;
  lastitem=NULL;
  before=NULL;
  split=0;
  off=0;
  }


// Create X window
void FXHeader::create(){
  FXHeaderItem* item;
  FXFrame::create();
  textColor=acquireColor(getApp()->foreColor);
  font->create();
  for(item=firstitem; item; item=item->next){
    if(item->icon){item->icon->create();}
    }
  show();
  }


// Get default width
FXint FXHeader::getDefaultWidth(){
  FXHeaderItem* item;
  FXint wmax;
  for(item=firstitem,wmax=0; item; item=item->next){
    wmax+=item->size;
    }
  return wmax;
  }


// Get default height
FXint FXHeader::getDefaultHeight(){
  FXHeaderItem* item;
  FXint th,ih,hmax;
  for(item=firstitem,hmax=0; item; item=item->next){
    if(item->label.text()){ th=font->getFontHeight(); if(th>hmax) hmax=th; }
    if(item->icon){ ih=item->icon->getHeight(); if(ih>hmax) hmax=ih; }
    }
  return 4+hmax+(border<<1);
  }


// Create item
FXHeaderItem* FXHeader::createItem(){ return new FXHeaderItem; }


// Get number of items
FXint FXHeader::getNumItems() const {
  register FXHeaderItem *item=firstitem;
  register FXint n=0;
  while(item){
    item=item->next;
    n++;
    }
  return n;
  }


// Get item X
FXint FXHeader::getItemX(const FXHeaderItem* item) const {
  return item->pos;
  }


// Get item Y
FXint FXHeader::getItemY(const FXHeaderItem* item) const {
  return 0;
  }


// Get item width
FXint FXHeader::getItemWidth(const FXHeaderItem* item) const {
  return item->size;
  }


// Get Item height
FXint FXHeader::getItemHeight(const FXHeaderItem*) const {
  return height;
  }


// Get next item
FXHeaderItem* FXHeader::getNextItem(const FXHeaderItem* item) const {
  return item->next;
  }


// Get previous item
FXHeaderItem* FXHeader::getPrevItem(const FXHeaderItem* item) const {
  return item->prev;
  }


// Add item as first
FXHeaderItem* FXHeader::addItemFirst(const char* text,FXIcon* icon,FXint size,FXObject* tgt,FXSelector sel){
  FXHeaderItem *item=createItem();
  item->prev=NULL;
  item->next=firstitem;
  if(item->next) item->next->prev=item; else lastitem=item;
  firstitem=item;
  item->label=text;
  item->icon=icon;
  item->target=tgt;
  item->message=sel;
  item->size=size;
  item->pos=0;
  recalc();
  return item;
  }


// Add item as last
FXHeaderItem* FXHeader::addItemLast(const char* text,FXIcon* icon,FXint size,FXObject* tgt,FXSelector sel){
  FXHeaderItem *item=createItem();
  item->prev=lastitem;
  item->next=NULL;
  if(item->prev) item->prev->next=item; else firstitem=item;
  lastitem=item;
  item->label=text;
  item->icon=icon;
  item->target=tgt;
  item->message=sel;
  item->size=size;
  item->pos=0;
  recalc();
  return item;
  }


// Remove node from list
void FXHeader::removeItem(FXHeaderItem* item){
  if(item){
    if(item->prev) item->prev->next=item->next; else firstitem=item->next;
    if(item->next) item->next->prev=item->prev; else lastitem=item->prev;
    delete item;
    recalc();
    }
  }


// Remove all items from [fm,to]
void FXHeader::removeItems(FXHeaderItem* fm,FXHeaderItem* to){
  if(fm && to){
    register FXHeaderItem *item;
    if(fm->prev) fm->prev->next=to->next; else firstitem=to->next;
    if(to->next) to->next->prev=fm->prev; else lastitem=fm->prev;
    do{
      item=fm;
      fm=fm->next;
      delete item;
      }
    while(item!=to);
    recalc();
    }
  }


// Remove all items
void FXHeader::removeAllItems(){
  register FXHeaderItem *item;
  while(firstitem){
    item=firstitem;
    firstitem=firstitem->next;
    delete item;
    }
  firstitem=NULL;
  lastitem=NULL;
  recalc();
  }


void FXHeader::setItemText(FXHeaderItem* item,const char* text){
  if(item==NULL){ fxerror("%s::setItemText: item is NULL\n",getClassName()); }
  item->label=text;
  update(item->pos,0,item->pos+item->size,height);
  }


const char* FXHeader::getItemText(const FXHeaderItem* item) const {
  if(item==NULL){ fxerror("%s::getItemText: item is NULL\n",getClassName()); }
  return item->label.text();
  }

void FXHeader::setItemTarget(FXHeaderItem* item,FXObject* tgt){
  if(item==NULL){ fxerror("%s::setItemTarget: item is NULL\n",getClassName()); }
  item->target=tgt;
  }


FXObject* FXHeader::getItemTarget(const FXHeaderItem* item) const {
  if(item==NULL){ fxerror("%s::getItemTarget: item is NULL\n",getClassName()); }
  return item->target;
  }


void FXHeader::setItemSelector(FXHeaderItem* item,FXSelector sel){
  if(item==NULL){ fxerror("%s::setItemSelector: item is NULL\n",getClassName()); }
  item->message=sel;
  }


FXSelector FXHeader::getItemSelector(const FXHeaderItem* item) const{
  if(item==NULL){ fxerror("%s::getItemSelector: item is NULL\n",getClassName()); }
  return item->message;
  }


void FXHeader::setItemIcon(FXHeaderItem* item,FXIcon* icon){
  if(item==NULL){ fxerror("%s::setItemIcon: item is NULL\n",getClassName()); }
  item->icon=icon;
  update(item->pos,0,item->pos+item->size,height);
  }


FXIcon* FXHeader::getItemIcon(const FXHeaderItem* item) const {
  if(item==NULL){ fxerror("%s::getItemIcon: item is NULL\n",getClassName()); }
  return item->icon;
  }


// Do layout
void FXHeader::layout(){
  FXHeaderItem* item;
  FXint pos;
  
  // Recompute item positions
  for(item=firstitem,pos=0; item; item=item->next){
    item->pos=pos;
    pos+=item->size;
    }
  
  // Force repaint
  update(0,0,width,height);
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Adjust horizontal layout
void FXHeader::adjustLayout(){
  FXHeaderItem* item;
  FXint pos;
  if(before){
    before->size=split+border-before->pos;
    for(item=before->next,pos=before->pos+before->size; item; item=item->next){
      item->pos=pos;
      pos+=item->size;
      }
    update(before->pos,0,width,height);
    }
  }


// Move the horizontal split intelligently
void FXHeader::moveSplit(FXint pos){
  FXint smin=before->pos;
  FXint smax=width-(border<<1);
  split=pos;
  if(split<smin) split=smin;
  if(split>smax) split=smax;
  }


// Handle repaint 
long FXHeader::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXHeaderItem* item;
  FXint x,dw,tw,rs,num,space,s;
  dw=font->getTextWidth("...",3);
  space=(border<<1);
  rs=0;
  clearArea(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  setTextFont(font);
  for(item=firstitem; item && item->pos<width; item=item->next){
    x=item->pos+border+3;
    s=item->size-space-6;
    if(item->icon){
      if(item->icon->getWidth()<=s){
        drawIcon(item->icon,x,2+border);
        x+=item->icon->getWidth()+4;
        }
      s-=item->icon->getWidth()+4;
      }
    if(item->label.text() && s>0){
      num=item->label.length();
      if(font->getTextWidth(item->label.text(),num)>s){
        while(num>0 && (tw=font->getTextWidth(item->label.text(),num))>(s-dw)) num--;
        if(num>0){
          setForeground(textColor);
          drawText(x,2+border+font->getFontAscent(),item->label.text(),num);
          drawText(x+tw,2+border+font->getFontAscent(),"...",3);
          }
        else{
          if(font->getTextWidth(item->label.text(),1)<=s){
            setForeground(textColor);
            drawText(x,2+border+font->getFontAscent(),item->label.text(),1);
            }
          }
        }
      else{
        setForeground(textColor);
        drawText(x,2+border+font->getFontAscent(),item->label.text(),num);
        }
      }
    drawFrame(item->pos,0,item->size,height);
    rs=item->pos+item->size;
    }
  if(rs<width){
    drawFrame(rs,0,width-rs,height);
    }
  return 1;
  }


// Button being pressed
long FXHeader::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  for(before=firstitem; before; before=before->next){
    if(before->pos+before->size-FUDGE<=ev->win_x && ev->win_x<=before->pos+before->size+FUDGE){
      split=before->pos+before->size-border;
      off=ev->win_x-split;
      grab();
      drawSplit(split);
      return 1;
      }
    }
  return 1;
  }
  

// Button being released
long FXHeader::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXHeaderItem* item;
  if(before){
    ungrab();
    drawSplit(split);
    adjustLayout();
    if(target) target->handle(this,MKUINT(message,SEL_CHANGED),before);
    before=NULL;
    }
  else{
    for(item=firstitem; item; item=item->next){
      if(item->pos+FUDGE<=ev->win_x && ev->win_x<=item->pos+item->size-FUDGE){
        if(item->target){
          item->target->handle(this,MKUINT(item->message,SEL_COMMAND),ptr);
          break;
          }
        }
      }
    }
  return 1;
  }


// Button being moved
long FXHeader::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXHeaderItem* item;
  if(before){
    FXint cur,oldsplit;
    oldsplit=split;
    cur=ev->win_x;
    moveSplit(cur-off);
    if(split!=oldsplit){
      drawSplit(oldsplit);
      drawSplit(split);
      if(target){
        target->handle(this,MKUINT(message,SEL_DRAGGED),before);
        }
      }
    }
  else{
    for(item=firstitem; item; item=item->next){
      if(item->pos+item->size-FUDGE <= ev->win_x && ev->win_x <= item->pos+item->size+FUDGE){
        setDefaultCursor(getApp()->hsplitCursor);
        return 1;
        }
      }
    setDefaultCursor(getApp()->arrowCursor);
    }
  return 1;
  }


// Draw the horizontal split
void FXHeader::drawSplit(FXint pos){
  if(border){
    FXint px,py;
    FXint b=border<<1;
    translateCoordinatesTo(px,py,getParent(),pos,0);
    setFunction(BLT_NOT_DST);
    fillRectangle(pos,0,b,height);
    getParent()->fillRectangle(px,0,b,getParent()->getHeight());
    setFunction(BLT_SRC);
    }
  }


// Change the font
void FXHeader::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  update(0,0,width,height);
  }


// Set text color
void FXHeader::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Clean up
FXHeader::~FXHeader(){
  removeAllItems();
  firstitem=(FXHeaderItem*)-1;
  lastitem=(FXHeaderItem*)-1;
  before=(FXHeaderItem*)-1;
  font=(FXFont*)-1;
  }

