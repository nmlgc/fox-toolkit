/********************************************************************************
*                                                                               *
*                          I c o n L i s t   O b j e c t                        *
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
* $Id: FXIconList.cpp,v 1.86 1998/10/19 16:23:23 jvz Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXComposite.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXHeader.h"
#include "FXIconList.h"

// Private header
#include "FXIconItem.h"


/*
  To do:
  - Item size should depend on mode
  - Update Items should be optimized
  - Scrolling stuff should be cleaned up
  - When switching to detail mode, there is interaction between sb's and header
  - Need to handle mouse actions properly
  - Need to fix click/double click etc. stuff.
  - drawIconShaded should be made to work correctly for colormapped displays
  - Need to use selection history here too! (e.g. selectionrectangle)
  - Slight overlap between update rectangles?
  - Need restore item stuff back?
  - In detail-mode, some items should be left, some right, and some centered in the field.
  - You always get a SEL_COMMAND, even if no item.
  - Slight optimization:- only recompute() when needed.
  - Text layout not good if bigger texts.
*/



#define SIDE_SPACING             4    // Left or right spacing between items

#define DETAIL_LINE_SPACING      0    // Line spacing in detail mode
#define DETAIL_TEXT_SPACING      4    // Spacing between text and icon in detail icon mode

#define MINI_LINE_SPACING        1    // Line spacing in mini icon mode
#define MINI_TEXT_SPACING        4    // Spacing between text and icon in mini icon mode

#define BIG_LINE_SPACING         6    // Line spacing in big icon mode
#define BIG_TEXT_SPACING         4    // Spacing between text and icon in big icon mode

#define MINI_ICON_WIDTH         16    // Default mini icon width
#define MINI_ICON_HEIGHT        16    // Default mini icon height

#define BIG_ICON_WIDTH          32    // Default big icon width
#define BIG_ICON_HEIGHT         32    // Default big icon height

#define ITEM_SPACE             128    // Default space for item

/*******************************************************************************/

// Map
FXDEFMAP(FXIconList) FXIconListMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXIconList::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXIconList::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXIconList::onAutoScroll),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXIconList::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXIconList::onLeftBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXIconList::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXIconList::onKeyRelease),
  FXMAPFUNC(SEL_FOCUSIN,0,FXIconList::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXIconList::onFocusOut),
  FXMAPFUNC(SEL_ACTIVATE,0,FXIconList::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXIconList::onDeactivate),
  FXMAPFUNC(SEL_CHANGED,0,FXIconList::onChanged),
  FXMAPFUNC(SEL_CLICKED,0,FXIconList::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXIconList::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXIconList::onTripleClicked),
  FXMAPFUNC(SEL_SELECTED,0,FXIconList::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXIconList::onDeselected),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXIconList::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXIconList::onSelectionGained),
  FXMAPFUNC(SEL_COMMAND,FXIconList::ID_SHOW_DETAILS,FXIconList::onCmdShowDetails),
  FXMAPFUNC(SEL_UPDATE,FXIconList::ID_SHOW_DETAILS,FXIconList::onUpdShowDetails),
  FXMAPFUNC(SEL_COMMAND,FXIconList::ID_SHOW_MINI_ICONS,FXIconList::onCmdShowMiniIcons),
  FXMAPFUNC(SEL_UPDATE,FXIconList::ID_SHOW_MINI_ICONS,FXIconList::onUpdShowMiniIcons),
  FXMAPFUNC(SEL_COMMAND,FXIconList::ID_SHOW_BIG_ICONS,FXIconList::onCmdShowBigIcons),
  FXMAPFUNC(SEL_UPDATE,FXIconList::ID_SHOW_BIG_ICONS,FXIconList::onUpdShowBigIcons),
  FXMAPFUNC(SEL_COMMAND,FXIconList::ID_ARRANGE_BY_ROWS,FXIconList::onCmdArrangeByRows),
  FXMAPFUNC(SEL_UPDATE,FXIconList::ID_ARRANGE_BY_ROWS,FXIconList::onUpdArrangeByRows),
  FXMAPFUNC(SEL_COMMAND,FXIconList::ID_ARRANGE_BY_COLUMNS,FXIconList::onCmdArrangeByColumns),
  FXMAPFUNC(SEL_UPDATE,FXIconList::ID_ARRANGE_BY_COLUMNS,FXIconList::onUpdArrangeByColumns),
  FXMAPFUNC(SEL_COMMAND,FXIconList::ID_ARRANGE_AT_RANDOM,FXIconList::onCmdArrangeAtRandom),
  FXMAPFUNC(SEL_UPDATE,FXIconList::ID_ARRANGE_AT_RANDOM,FXIconList::onUpdArrangeAtRandom),
  FXMAPFUNC(SEL_CHANGED,100,FXIconList::onHeaderChanged),
  };


// Object implementation
FXIMPLEMENT(FXIconList,FXScrollArea,FXIconListMap,ARRAYNUMBER(FXIconListMap))

  
// Icon List
FXIconList::FXIconList(){
  flags|=FLAG_ENABLED;
  header=NULL;
  firstitem=NULL;
  lastitem=NULL;
  anchoritem=NULL;
  currentitem=NULL;
  font=(FXFont*)-1;
  sortfunc=NULL;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  iconWidth=BIG_ICON_WIDTH;
  iconHeight=BIG_ICON_HEIGHT;
  miniIconWidth=MINI_ICON_WIDTH;
  miniIconHeight=MINI_ICON_HEIGHT;
  itemSpace=ITEM_SPACE;
  itemWidth=0;
  itemHeight=0;
  totalWidth=0;
  totalHeight=0;
  anchorx=0;
  anchory=0;
  currentx=0;
  currenty=0;
  }


// Icon List
FXIconList::FXIconList(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  header=new FXHeader(this,this,100,FRAME_RAISED|FRAME_THICK);
  target=tgt;
  message=sel;
  firstitem=NULL;
  lastitem=NULL;
  anchoritem=NULL;
  currentitem=NULL;
  font=getApp()->normalFont;
  sortfunc=NULL;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  iconWidth=BIG_ICON_WIDTH;
  iconHeight=BIG_ICON_HEIGHT;
  miniIconWidth=MINI_ICON_WIDTH;
  miniIconHeight=MINI_ICON_HEIGHT;
  itemSpace=ITEM_SPACE;
  itemWidth=0;
  itemHeight=0;
  totalWidth=0;
  totalHeight=0;
  anchorx=0;
  anchory=0;
  currentx=0;
  currenty=0;
  }


// Create X window
void FXIconList::create(){
  FXIconItem *item;
  FXScrollArea::create();
  selbackColor=acquireColor(getApp()->selbackColor);
  seltextColor=acquireColor(getApp()->selforeColor);
  textColor=acquireColor(getApp()->foreColor);
  font->create();
  for(item=firstitem; item; item=item->next){
    if(item->icon){item->icon->create();}
    if(item->miniIcon){item->miniIcon->create();}
    }
  }


// Propagate size change
void FXIconList::recalc(){ 
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


// If window can have focus
FXbool FXIconList::canFocus() const { return 1; }


// Create item
FXIconItem* FXIconList::createItem(){ return new FXIconItem; }


// Get number of items
FXint FXIconList::getNumItems() const {
  register FXIconItem *item=firstitem;
  register FXint n=0;
  while(item){
    item=item->next;
    n++;
    }
  return n;
  }

// Get number of selected items
FXint FXIconList::getNumSelectedItems() const {
  register FXIconItem *item=firstitem;
  register FXint n=0;
  while(item){
    if(item->state&ICONITEM_SELECTED) n++;
    item=item->next;
    }
  return n;
  }


// Get list of selected ite,s
FXIconItem** FXIconList::getSelectedItems() const {
  register FXIconItem *item=firstitem;
  register FXIconItem **itemlist;
  register FXint i=0;
  FXMALLOC(&itemlist,FXIconItem*,getNumSelectedItems()+1);
  while(item){
    if(item->state&ICONITEM_SELECTED) itemlist[i++]=item;
    item=item->next;
    }
  itemlist[i]=NULL;
  return itemlist;
  }
 

// Helper function
static FXint itemnamelen(const FXchar* name){
  register FXint len;
  for(len=0; name[len] && name[len]!='\t'; len++);
  return len;
  }


// Get item X
FXint FXIconList::getItemX(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::getItemX: item is NULL.\n",getClassName()); } 
  return item->x; 
  }


// Get item Y
FXint FXIconList::getItemY(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::getItemY: item is NULL.\n",getClassName()); } 
  return item->y; 
  }


// Get item width
FXint FXIconList::getItemWidth(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::getItemWidth: item is NULL.\n",getClassName()); } 
  return itemWidth; 
  }


// Get Item height
FXint FXIconList::getItemHeight(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::getItemHeight: item is NULL.\n",getClassName()); } 
  return itemHeight; 
  }


// True if item is selected
FXbool FXIconList::isItemSelected(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::isItemSelected: item is NULL.\n",getClassName()); } 
  return (item->state&ICONITEM_SELECTED)!=0; 
  }


// True if item is current
FXbool FXIconList::isItemCurrent(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::isItemCurrent: item is NULL.\n",getClassName()); } 
  return (item->state&ICONITEM_CURRENT)!=0; 
  }


// True if item visible
FXbool FXIconList::isItemVisible(const FXIconItem* item) const {
  if(!item){ fxerror("%s::isItemVisible: item is NULL.\n",getClassName()); } 
  return 0<(pos_x+getItemX(item)+getItemWidth(item)) && (pos_x+getItemX(item))<viewport_w && 0<(pos_y+getItemY(item)+getItemHeight(item)) && (pos_y+getItemY(item))<viewport_h;
  }  


// Make item fully visible
void FXIconList::makeItemVisible(FXIconItem* item){
  register FXint x,y,w,h,hh,px,py;
  if(item){
    px=pos_x;
    py=pos_y;
    if(options&(ICONLIST_BIG_ICONS|ICONLIST_MINI_ICONS)){
      x=getItemX(item);
      y=getItemY(item);
      w=getItemWidth(item);
      h=getItemHeight(item);
      if(px+x+w >= viewport_w) px=viewport_w-x-w;
      if(px+x <= 0) px=-x;
      if(py+y+h >= viewport_h) py=viewport_h-y-h;
      if(py+y <= 0) py=-y;
      }
    else{
      y=getItemY(item);
      h=getItemHeight(item);
      hh=header->getDefaultHeight();
      if(py+y+h >= viewport_h+hh) py=hh+viewport_h-y-h;
      if(py+y <= hh) py=hh-y;
      }
    setPosition(px,py);
    }
  }


// Get item at position x,y (NULL if none)
FXIconItem* FXIconList::getItemAt(FXint x,FXint y) const {
  register FXint off,to,tw;
  register FXIconItem* n;
  y-=pos_y;
  x-=pos_x;
  if(options&ICONLIST_BIG_ICONS){
    off=(itemSpace-iconWidth)/2;
    for(n=firstitem; n; n=n->next){
      if(y<n->y || n->y+itemHeight<=y || x<n->x || n->x+itemWidth<=x) continue;
      if(y<n->y+iconHeight && (x<n->x+off || n->x+off+iconWidth<=x)) continue;
      tw=font->getTextWidth(n->label.text(),itemnamelen(n->label.text()));
      if(tw>itemWidth) tw=itemWidth;
      to=(itemWidth-tw)/2;
      if(x<n->x+to || n->x+to+tw<=x) continue;
      return n;
      }
    }
  else if(options&ICONLIST_MINI_ICONS){
    for(n=firstitem; n; n=n->next){
      if(x<n->x || n->x+itemWidth<=x || y<n->y || n->y+itemHeight<=y) continue;
      if(x<n->x+miniIconWidth+4+font->getTextWidth(n->label.text(),itemnamelen(n->label.text()))) return n;
      }
    }
  else{
    for(n=firstitem; n; n=n->next){
      if(n->y<=y && y<n->y+itemHeight) return n;
      }
    }
  return NULL;
  }



// Get next item
FXIconItem* FXIconList::getNextItem(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::getNextItem: item is NULL.\n",getClassName()); } 
  return item->next; 
  }


// Get previous item
FXIconItem* FXIconList::getPrevItem(const FXIconItem* item) const { 
  if(!item){ fxerror("%s::getPrevItem: item is NULL.\n",getClassName()); } 
  return item->prev; 
  }


// Sort the items based on the sort function
void FXIconList::sortItems(){
  FXItem *f=(FXItem*)firstitem;
  FXItem *l=(FXItem*)lastitem;
  fxsort((FXItem*&)firstitem,(FXItem*&)lastitem,f,l,sortfunc,getNumItems());
  recalc();
  }


// Repaint
void FXIconList::updateItem(FXIconItem* item){
  update(pos_x+getItemX(item),pos_y+getItemY(item),getItemWidth(item),getItemHeight(item));
  }


// All turned on
FXbool FXIconList::selectItemRange(FXIconItem* beg,FXIconItem* end){
  register FXbool selected=FALSE;
  if(beg && end){
    register FXIconItem *item=beg;
    do{
      selected|=selectItem(item);
      if(item==end) break;
      item=item->next;
      }
    while(item);
    }
  return selected;
  }


// All turned off
FXbool FXIconList::deselectItemRange(FXIconItem* beg,FXIconItem* end){
  register FXbool deselected=FALSE;
  if(beg && end){
    register FXIconItem *item=beg;
    do{
      deselected|=deselectItem(item);
      if(item==end) break;
      item=item->next;
      }
    while(item);
    }
  return deselected;
  }


// Toggle items
FXbool FXIconList::toggleItemRange(FXIconItem* beg,FXIconItem* end){
  register FXbool toggled=FALSE;
  if(beg && end){
    register FXIconItem *item=beg;
    do{
      toggled|=toggleItem(item);
      if(item==end) break;
      item=item->next;
      }
    while(item);
    }
  return toggled;
  }


// Restore items to previous state
FXbool FXIconList::restoreItemRange(FXIconItem* beg,FXIconItem* end){
  register FXbool restored=FALSE;
  if(beg && end){
    register FXIconItem *item=beg;
    do{
      restored|=restoreItem(item);
      if(item==end) break;
      item=item->next;
      }
    while(item);
    }
  return restored;
  }


// Select one item
FXbool FXIconList::selectItem(FXIconItem* item){
  if(item){
    if(!(item->state&ICONITEM_SELECTED)){
      item->state|=ICONITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// Deselect one item
FXbool FXIconList::deselectItem(FXIconItem* item){
  if(item){
    if(item->state&ICONITEM_SELECTED){
      item->state&=~ICONITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// toggle one item
FXbool FXIconList::toggleItem(FXIconItem* item){
  if(item){
    item->state^=ICONITEM_SELECTED;
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }


// Restore items to previous state
FXbool FXIconList::restoreItem(FXIconItem* item){
  if(item){
    if((item->state&ICONITEM_MARK) && !(item->state&ICONITEM_SELECTED)){
      item->state|=ICONITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    if(!(item->state&ICONITEM_MARK) && (item->state&ICONITEM_SELECTED)){
      item->state&=~ICONITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// Mark items
void FXIconList::markItems(){
  register FXIconItem* item;
  for(item=firstitem; item; item=item->next){
    (item->state&ICONITEM_SELECTED)?item->state|=ICONITEM_MARK:item->state&=~ICONITEM_MARK;
    }
  }


// Little helper function; 1=select, 0=deselect, 2=toggle, 3=restore, 4=nop
FXbool FXIconList::performSelectionChange(FXIconItem* item,FXuint code){
  switch(code){
    case 0: return deselectItem(item);
    case 1: return selectItem(item);
    case 2: return toggleItem(item);
    case 3: return restoreItem(item);
    }
  return FALSE;
  }


// Select, deselect, or toggle items in rectangle
FXbool FXIconList::performSelectionRectangle(FXint lx,FXint ty,FXint rx,FXint by,FXuint in,FXuint out){
  register FXbool changed=FALSE;
  register FXint ix,iy,off;
  register FXIconItem* n;
  if(lx<rx && ty<by){
    if(options&ICONLIST_BIG_ICONS){
      off=(itemSpace-iconWidth)/2;
      for(n=firstitem; n; n=n->next){
        ix=n->x;
        iy=n->y;
        if(ix+off<=rx && iy<=by && lx<=ix+off+iconWidth && ty<=iy+iconHeight){
          changed|=performSelectionChange(n,in);
          }
        else{
          changed|=performSelectionChange(n,out);
          }
        }
      }
    else if(options&ICONLIST_MINI_ICONS){
      for(n=firstitem; n; n=n->next){
        ix=n->x;
        iy=n->y;
        if(ix<=rx && iy<=by && lx<=ix+miniIconWidth && ty<=iy+miniIconHeight){
          changed|=performSelectionChange(n,in);
          }
        else{
          changed|=performSelectionChange(n,out);
          }
        }
      }
    else{
      for(n=firstitem; n; n=n->next){
        ix=n->x;
        iy=n->y;
        if(ix<=rx && iy<=by && lx<=ix+itemWidth && ty<=iy+itemHeight){
          changed|=performSelectionChange(n,in);
          }
        else{
          changed|=performSelectionChange(n,out);
          }
        }
      }
    }
  return changed;
  }


// Select items in rectangle
FXbool FXIconList::selectInRectangle(FXint x,FXint y,FXint w,FXint h){
  return performSelectionRectangle(x,y,x+w-1,y+h-1,1,4);
  }

// Deselect items in rectangle
FXbool FXIconList::deselectInRectangle(FXint x,FXint y,FXint w,FXint h){
  return performSelectionRectangle(x,y,x+w-1,y+h-1,0,4);
  }


// Toggle items in rectangle
FXbool FXIconList::toggleInRectangle(FXint x,FXint y,FXint w,FXint h){
  return performSelectionRectangle(x,y,x+w-1,y+h-1,2,4);
  }


// Toggle items in rectangle
FXbool FXIconList::restoreInRectangle(FXint x,FXint y,FXint w,FXint h){
  return performSelectionRectangle(x,y,x+w-1,y+h-1,3,4);
  }


// Move content
void FXIconList::moveContents(FXint x,FXint y){
  if(!(flags&FLAG_INVISIBLE)){
    FXint dx,dy,tx,ty,fx,fy,ex,ey;
    FXint top=0;
    if(!(options&(ICONLIST_MINI_ICONS|ICONLIST_BIG_ICONS))){
      top=header->getDefaultHeight();
      header->move(x,0);
      }
    if(pos_x<x){
      dx=x-pos_x; fx=0; tx=dx; ex=0;
      }
    else{
      dx=pos_x-x; fx=dx; tx=0; ex=viewport_w-dx;
      }
    if(pos_y<y){
      dy=y-pos_y; fy=top; ty=top+dy; ey=top;
      }
    else{
      dy=pos_y-y; fy=top+dy; ty=top; ey=top+viewport_h-dy;
      }
    pos_x=x;
    pos_y=y;
    if((flags&FLAG_OBSCURED) || (viewport_w<=dx) || (viewport_h<=dy)){
      update(0,top,viewport_w,viewport_h);
      }
    else{
      FXEvent ev;
      ev.type=SEL_PAINT;
      ev.window=xid;
      drawArea(this,fx,fy,viewport_w-dx,viewport_h-dy,tx,ty);
      if(dy){
        ev.rect.x=0;
        ev.rect.y=ey;
        ev.rect.w=viewport_w;
        ev.rect.h=dy;
        handle(this,MKUINT(0,SEL_PAINT),&ev);
        }
      if(dx){
        ev.rect.x=ex;
        ev.rect.y=top;
        ev.rect.w=dx;
        ev.rect.h=viewport_h;
        handle(this,MKUINT(0,SEL_PAINT),&ev);
        }
      }
    }
  }


// Size of a possible column caption
FXint FXIconList::getViewportHeight(){
  return (options&(ICONLIST_MINI_ICONS|ICONLIST_BIG_ICONS)) ? height : height-header->getDefaultHeight();
  }


// Recompute interior 
void FXIconList::recompute(){
  register FXint y,nh,nv,r,k,th,num;
  register FXIconItem* n;
//fprintf(stderr,"%s::recompute\n",getClassName());

  totalWidth=0;
  totalHeight=0;
  th=1+font->getFontHeight();
  num=getNumItems();
  
  // Big Icons
  if(options&ICONLIST_BIG_ICONS){
    itemHeight=iconHeight+th+BIG_LINE_SPACING+BIG_TEXT_SPACING;
    itemWidth=FXMAX(iconWidth,itemSpace)+SIDE_SPACING;
    }
 
  // Mini Icons
  else if(options&ICONLIST_MINI_ICONS){
    itemHeight=FXMAX(miniIconHeight,th)+MINI_LINE_SPACING;
    itemWidth=miniIconWidth+itemSpace+MINI_TEXT_SPACING+SIDE_SPACING;
    }
 
  // Detail list
  else{
    itemHeight=FXMAX(miniIconHeight,th)+DETAIL_LINE_SPACING;
    itemWidth=header->getDefaultWidth();
    }
  
  // For icon modes
  if(options&(ICONLIST_MINI_ICONS|ICONLIST_BIG_ICONS)){

    // Arrangement at random
    if(options&ICONLIST_RANDOM){
      for(n=firstitem; n; n=n->next){
        if(n->x+itemWidth>totalWidth) totalWidth=n->x+itemWidth;
        if(n->y+itemHeight>totalHeight) totalHeight=n->y+itemHeight;
        }
      }

    // Arrangement by columns
    else if(options&ICONLIST_COLUMNS){
      nh=width/itemWidth;
      if(nh<1) nh=1;
      nv=(num+nh-1)/nh;
      if(nv*itemHeight > height){
        nh=(width-vertical->getDefaultWidth())/itemWidth;
        if(nh<1) nh=1;
        nv=(num+nh-1)/nh;
        }
      for(n=firstitem,r=0,k=0; n; n=n->next){
        n->x=k*itemWidth;
        n->y=r*itemHeight;
        if(++k==nh){k=0;r++;}
        }
      totalWidth=itemWidth*nh;
      totalHeight=itemHeight*nv;
      }

    // Arrangement by rows
    else{
      nv=height/itemHeight;
      if(nv<1) nv=1;
      nh=(num+nv-1)/nv;
      if(nh*itemWidth > width){
        nv=(height-horizontal->getDefaultHeight())/itemHeight;
        if(nv<1) nv=1;
        nh=(num+nv-1)/nv;
        }
      for(n=firstitem,r=0,k=0; n; n=n->next){
        n->x=k*itemWidth;
        n->y=r*itemHeight;
        if(++r==nv){r=0;k++;}
        }
      totalWidth=itemWidth*nh;
      totalHeight=itemHeight*nv;
      }
    }

  // For detail mode
  else{
    for(n=firstitem,y=header->getDefaultHeight(); n; y+=itemHeight,n=n->next){
      n->x=0;
      n->y=y;
      }
    totalWidth=itemWidth;
    totalHeight=num*itemHeight;//+header->getDefaultHeight();
    }
  
  // No more recalc 
  flags&=~FLAG_RECALC;
  }


// Determine content width of tree list
FXint FXIconList::getContentWidth(){
  if(flags&FLAG_RECALC) recompute();
  return totalWidth;
  }


// Determine content height of tree list
FXint FXIconList::getContentHeight(){
  if(flags&FLAG_RECALC) recompute();
  return totalHeight;  
  }


// Recalculate layout
void FXIconList::layout(){
  
  ////// Should be set ONLY when in row/col mode & rows/cols have changed
  flags|=FLAG_RECALC;
  
  // Update scroll bars
  FXScrollArea::layout();

  // In detail mode
  if(!(options&(ICONLIST_MINI_ICONS|ICONLIST_BIG_ICONS))){ 
    header->position(pos_x,0,2048,header->getDefaultHeight());
    header->show();
    if(itemWidth<viewport_w) itemWidth=viewport_w;
    }
  else{
    header->hide();
    }

  // Set line size
  vertical->setLine(itemHeight);
  horizontal->setLine(1);
  
  // Force repaint
  update(0,0,width,height);
  
  flags&=~FLAG_DIRTY;
  }


// Gained focus
long FXIconList::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(currentitem) updateItem(currentitem);
  return 1;
  }


// Lost focus
long FXIconList::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(currentitem) updateItem(currentitem);
  return 1;
  }


// We have the selection
long FXIconList::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionGained %x\n",getClassName(),this);
  FXScrollArea::onSelectionGained(sender,sel,ptr);
  //update(0,0,width,height);
  return 1;
  }


// We lost the selection
long FXIconList::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionLost %x\n",getClassName(),this);
  FXScrollArea::onSelectionLost(sender,sel,ptr);
  deselectItemRange(firstitem,lastitem);
  return 1;
  }


// Draw dashed focus rectangle
void FXIconList::drawFocusRectangle(FXint x,FXint y,FXint w,FXint h){
  static const char onoff[]={1,2};
  FXASSERT(xid);
  setForeground(textColor);
  setDashes(0,onoff,2);
  setLineAttributes(0,LINE_ONOFF_DASH,CAP_BUTT,JOIN_MITER);
  drawLine(x,y,x+w-1,y);
  drawLine(x,y,x,y+h-1);
  drawLine(x,y+h-1,x+w-1,y+h-1);
  drawLine(x+w-1,y,x+w-1,y+h-1);
  setLineAttributes(0,LINE_SOLID,CAP_BUTT,JOIN_MITER);
  }


// Draw Lasso rectangle
void FXIconList::drawLasso(FXint x0,FXint y0,FXint x1,FXint y1){
  setFunction(BLT_NOT_DST);
  x0+=pos_x;
  x1+=pos_x;
  y0+=pos_y;
  y1+=pos_y;
  drawLine(x0,y0,x1,y0);
  drawLine(x1,y0,x1,y1);
  drawLine(x1,y1,x0,y1);
  drawLine(x0,y1,x0,y0);
  setFunction(BLT_SRC);
  }


// Draw big icon
void FXIconList::drawBigIcon(FXint x,FXint y,FXIconItem* item){
  register FXint iconoffx,iconoffy,textoffx,textoffy,tw,th,dw,tlen,tdrw,s;
  register const FXchar* text;
  if(item->icon){
    iconoffx=x+((itemSpace-item->icon->getWidth())/2)+(SIDE_SPACING/2);
    iconoffy=y+(BIG_LINE_SPACING/2);
    if(item->state&ICONITEM_SELECTED){
      drawIconShaded(item->icon,iconoffx,iconoffy);
      }
    else{
      drawIcon(item->icon,iconoffx,iconoffy);
      }
    }
  text=item->label.text();
  if(text){
    for(tlen=0; text[tlen] && text[tlen]!='\t'; tlen++);
    if(tlen>0){
      th=font->getFontHeight();
      tw=font->getTextWidth(text,tlen);
      tdrw=tlen;
      dw=0;
      if(tw>itemSpace){
        dw=font->getTextWidth("...",3);
        s=itemSpace-dw;
        while((tw=font->getTextWidth(text,tdrw))>s && tdrw>1) --tdrw;
        if(tw>s) dw=0;
        }
      if(tw<=itemSpace){
        textoffy=y+iconHeight+BIG_TEXT_SPACING;
        textoffx=x+((itemSpace-tw-dw)/2)+(SIDE_SPACING/2);
        if(item->state&ICONITEM_SELECTED){
          if(hasFocus() && (item->state&ICONITEM_CURRENT)){
            setForeground(selbackColor);
            fillRectangle(textoffx+1,textoffy+1,tw+dw,th-1);
            drawFocusRectangle(textoffx,textoffy,tw+dw+2,th+1);
            }
          else{
            setForeground(selbackColor);
            fillRectangle(textoffx,textoffy,tw+dw+2,th+1);
            }
          setForeground(seltextColor);
          drawText(textoffx+1,textoffy+font->getFontAscent(),text,tdrw);
          if(dw) drawText(textoffx+tw+1,textoffy+font->getFontAscent(),"...",3);
          }
        else{
          if(hasFocus() && (item->state&ICONITEM_CURRENT)){
            drawFocusRectangle(textoffx,textoffy,tw+dw+2,th+1);
            }
          setForeground(textColor);
          drawText(textoffx+1,textoffy+font->getFontAscent(),text,tdrw);
          if(dw) drawText(textoffx+tw+1,textoffy+font->getFontAscent(),"...",3);
          }
        }
      }
    }
  }


// Draw mini icon
void FXIconList::drawMiniIcon(FXint x,FXint y,FXIconItem* item){
  register FXint iconoffx,iconoffy,textoffx,textoffy,tw,th,dw,tlen,tdrw,s;
  register const FXchar* text;
  if(item->miniIcon){
    iconoffx=x+(SIDE_SPACING/2);
    iconoffy=y+(MINI_LINE_SPACING/2)+(itemHeight-item->miniIcon->getHeight())/2;
    if(item->state&ICONITEM_SELECTED){
      drawIconShaded(item->miniIcon,iconoffx,iconoffy);
      }
    else{
      drawIcon(item->miniIcon,iconoffx,iconoffy);
      }
    }
  text=item->label.text();
  if(text){
    for(tlen=0; text[tlen] && text[tlen]!='\t'; tlen++);
    if(tlen>0){
      th=font->getFontHeight();
      tw=font->getTextWidth(text,tlen);
      tdrw=tlen;
      dw=0;
      if(tw>itemSpace){
        dw=font->getTextWidth("...",3);
        s=itemSpace-dw;
        while((tw=font->getTextWidth(text,tdrw))>s && tdrw>1) --tdrw;
        if(tw>s) dw=0;
        }
      if(tw<=itemSpace){
        textoffx=x+miniIconWidth+(SIDE_SPACING/2)+MINI_TEXT_SPACING;
        textoffy=y+(MINI_LINE_SPACING/2)+(itemHeight-th)/2;
        if(item->state&ICONITEM_SELECTED){
          if(hasFocus() && (item->state&ICONITEM_CURRENT)){
            setForeground(selbackColor);
            fillRectangle(textoffx+1,textoffy+1,tw+dw,th-1);
            drawFocusRectangle(textoffx,textoffy,tw+dw+2,th+1);
            }
          else{
            setForeground(selbackColor);
            fillRectangle(textoffx,textoffy,tw+dw+2,th+1);
            }
          setForeground(seltextColor);
          drawText(textoffx+1,textoffy+font->getFontAscent(),text,tdrw);
          if(dw) drawText(textoffx+tw+1,textoffy+font->getFontAscent(),"...",3);
          }
        else{
          if(hasFocus() && (item->state&ICONITEM_CURRENT)){
            drawFocusRectangle(textoffx,textoffy,tw+dw+2,th+1);
            }
          setForeground(textColor);
          drawText(textoffx+1,textoffy+font->getFontAscent(),text,tdrw);
          if(dw) drawText(textoffx+tw+1,textoffy+font->getFontAscent(),"...",3);
          }
        }
      }
    }
  }


// Draw detail
void FXIconList::drawDetails(FXint x,FXint y,FXIconItem* item){
  register FXint iconoffx,iconoffy,textoffx,textoffy,tw,th,dw,ddw,tlen,tdrw,space,s;
  register const FXchar* text=item->label.text();
  FXHeaderItem* hi=header->getFirstItem();
  if(!hi) return;
  if(item->miniIcon){
    iconoffx=x+(SIDE_SPACING/2);
    iconoffy=y+(DETAIL_LINE_SPACING/2)+(itemHeight-item->miniIcon->getHeight())/2;
    if(item->state&ICONITEM_SELECTED){
      drawIconShaded(item->miniIcon,iconoffx,iconoffy);
      }
    else{
      drawIcon(item->miniIcon,iconoffx,iconoffy);
      }
    }
  if(text){
    th=font->getFontHeight();
    textoffx=x+miniIconWidth+(SIDE_SPACING/2)+DETAIL_TEXT_SPACING;
    textoffy=y+(DETAIL_LINE_SPACING/2)+(itemHeight-th)/2;
    if(item->state&ICONITEM_SELECTED){
      if(hasFocus() && (item->state&ICONITEM_CURRENT)){
        setForeground(selbackColor);
        fillRectangle(textoffx+1,textoffy+1,itemWidth-2,th-1);
        drawFocusRectangle(textoffx,textoffy,itemWidth,th+1);
        }
      else{
        setForeground(selbackColor);
        fillRectangle(textoffx,textoffy,itemWidth,th+1);
        }
      }
    else{
      if(hasFocus() && (item->state&ICONITEM_CURRENT)){
        drawFocusRectangle(textoffx,textoffy,itemWidth,th+1);
        }
      }
    setForeground((item->state&ICONITEM_SELECTED) ? seltextColor : textColor);
    ddw=font->getTextWidth("...",3);
    space=header->getItemWidth(hi)-miniIconWidth-DETAIL_TEXT_SPACING;
    while(1){
      for(tlen=0; text[tlen] && text[tlen]!='\t'; tlen++);
      if(tlen>0){
        tw=font->getTextWidth(text,tlen);
        tdrw=tlen;
        dw=0;
        if(tw>(space-10)){
          dw=ddw;
          s=space-10-dw;
          while((tw=font->getTextWidth(text,tdrw))>s && tdrw>1) --tdrw;
          if(tw>(space-10)) dw=0;
          }
        if(tw<=(space-10)){
          drawText(textoffx+1,textoffy+font->getFontAscent(),text,tdrw);
          if(dw) drawText(textoffx+tw+1,textoffy+font->getFontAscent(),"...",3);
          }
        }
      if(!text[tlen]) break;
      hi=header->getNextItem(hi);
      if(!hi) break;
      textoffx+=space;
      space=header->getItemWidth(hi);
      text=text+tlen+1;
      }
    }
  }


// Draw item list
long FXIconList::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXint xmin,xmax,ymin,ymax;
  register FXIconItem* n;
  register FXint x,y;
  clearArea(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  setTextFont(font);
  
  // Possible overlap
  xmin=event->rect.x-itemWidth;
  ymin=event->rect.y-itemHeight;
  xmax=event->rect.x+event->rect.w;
  ymax=event->rect.y+event->rect.h;
 
  // Big icons
  if(options&ICONLIST_BIG_ICONS){
    for(n=firstitem; n; n=n->next){
      x=pos_x+n->x;
      y=pos_y+n->y;
      if(x<=xmax && y<=ymax && xmin<=x && ymin<=y) drawBigIcon(x,y,n);
      }
    }
 
  // Mini icons
  else if(options&ICONLIST_MINI_ICONS){
    for(n=firstitem; n; n=n->next){
      x=pos_x+n->x;
      y=pos_y+n->y;
      if(x<=xmax && y<=ymax && xmin<=x && ymin<=y) drawMiniIcon(x,y,n);
      }
    }
 
  // Detail mode
  else{
    for(n=firstitem; n; n=n->next){
      x=pos_x+n->x;
      y=pos_y+n->y;
      if(y<=ymax && ymin<=y) drawDetails(x,y,n);
      }
    }
  return 1;
  }


// Arrange by rows
long FXIconList::onCmdArrangeByRows(FXObject*,FXSelector,void*){
  options&=~ICONLIST_COLUMNS;
  options&=~ICONLIST_RANDOM;
  recalc();
  return 1;
  }


// Update sender
long FXIconList::onUpdArrangeByRows(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&(ICONLIST_COLUMNS|ICONLIST_RANDOM))?ID_UNCHECK:ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

// Arrange by columns
long FXIconList::onCmdArrangeByColumns(FXObject*,FXSelector,void*){
  options&=~ICONLIST_RANDOM;
  options|=ICONLIST_COLUMNS;
  recalc();
  return 1;
  }

// Update sender
long FXIconList::onUpdArrangeByColumns(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&ICONLIST_COLUMNS)?ID_CHECK:ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Arrange randomly
long FXIconList::onCmdArrangeAtRandom(FXObject*,FXSelector,void*){
  options&=~ICONLIST_COLUMNS;
  options|=ICONLIST_RANDOM;
  recalc();
  return 1;
  }


// Update sender
long FXIconList::onUpdArrangeAtRandom(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&ICONLIST_RANDOM)?ID_CHECK:ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Show detailed list
long FXIconList::onCmdShowDetails(FXObject*,FXSelector,void*){
  options&=~ICONLIST_MINI_ICONS;
  options&=~ICONLIST_BIG_ICONS;
  options&=~ICONLIST_RANDOM;
  recalc();
  return 1;
  }


// Update sender
long FXIconList::onUpdShowDetails(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&(ICONLIST_MINI_ICONS|ICONLIST_BIG_ICONS))?ID_UNCHECK:ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Show big icons
long FXIconList::onCmdShowBigIcons(FXObject*,FXSelector,void*){
  options&=~ICONLIST_MINI_ICONS;
  options|=ICONLIST_BIG_ICONS;
  options&=~ICONLIST_RANDOM;
  recalc();
  return 1;
  }


// Update sender
long FXIconList::onUpdShowBigIcons(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&ICONLIST_BIG_ICONS)?ID_CHECK:ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Show small icons
long FXIconList::onCmdShowMiniIcons(FXObject*,FXSelector,void*){
  options|=ICONLIST_MINI_ICONS;
  options&=~ICONLIST_BIG_ICONS;
  options&=~ICONLIST_RANDOM;
  recalc();
  return 1;
  }


// Update sender
long FXIconList::onUpdShowMiniIcons(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(options&ICONLIST_MINI_ICONS)?ID_CHECK:ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Header subdivision has changed
long FXIconList::onHeaderChanged(FXObject*,FXSelector,void*){
  recalc();
  return 1;
  }


// Add item as first one under parent p
FXIconItem* FXIconList::addItemFirst(const char* text,FXIcon* ic,FXIcon* mi,void* ptr){
  FXIconItem *item=createItem();
  item->prev=NULL;
  item->next=firstitem;
  if(item->next) item->next->prev=item; else lastitem=item;
  firstitem=item;
  item->label=text;
  item->icon=ic;
  item->miniIcon=mi;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Add item as last one under parent p
FXIconItem* FXIconList::addItemLast(const char* text,FXIcon* ic,FXIcon* mi,void* ptr){
  FXIconItem *item=createItem();
  item->prev=lastitem;
  item->next=NULL;
  if(item->prev) item->prev->next=item; else firstitem=item;
  lastitem=item;
  item->label=text;
  item->icon=ic;
  item->miniIcon=mi;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Link item after other
FXIconItem* FXIconList::addItemAfter(FXIconItem* other,const char* text,FXIcon* ic,FXIcon* mi,void* ptr){
  if(other==NULL){ fxerror("%s::addItemAfter: other item is NULL.\n",getClassName()); }
  FXIconItem *item=createItem();
  item->prev=other;
  item->next=other->next;
  other->next=item;
  if(item->next) item->next->prev=item; else lastitem=item; 
  item->label=text;
  item->icon=ic;
  item->miniIcon=mi;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Link item before other 
FXIconItem* FXIconList::addItemBefore(FXIconItem* other,const char* text,FXIcon* ic,FXIcon* mi,void* ptr){
  if(other==NULL){ fxerror("%s::addItemBefore: other item is NULL.\n",getClassName()); }
  FXIconItem *item=createItem();
  item->next=other;
  item->prev=other->prev;
  other->prev=item;
  if(item->prev) item->prev->next=item; else firstitem=item;
  item->label=text;
  item->icon=ic;
  item->miniIcon=mi;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Remove node from list
void FXIconList::removeItem(FXIconItem* item){
  if(item){
    if(item->prev) item->prev->next=item->next; else firstitem=item->next;
    if(item->next) item->next->prev=item->prev; else lastitem=item->prev;
    if(currentitem==item) currentitem=NULL;
    if(anchoritem==item) anchoritem=NULL;
    delete item;
    recalc();
    }
  }


// Remove all siblings from [fm,to]
void FXIconList::removeItems(FXIconItem* fm,FXIconItem* to){
  if(fm && to){
    register FXIconItem *item;
    if(fm->prev) fm->prev->next=to->next; else firstitem=to->next;
    if(to->next) to->next->prev=fm->prev; else lastitem=fm->prev;
    do{
      item=fm;
      if(currentitem==item) currentitem=NULL;
      if(anchoritem==item) anchoritem=NULL;
      fm=fm->next;
      delete item;
      }
    while(item!=to);
    recalc();
    }
  }



// Remove all items
void FXIconList::removeAllItems(){
  register FXIconItem *item;
  while(firstitem){
    item=firstitem;
    firstitem=firstitem->next;
    delete item;
    }
  firstitem=NULL;
  lastitem=NULL;
  currentitem=NULL;
  anchoritem=NULL;
  recalc();
  }


// Set current item
void FXIconList::setCurrentItem(FXIconItem* item){
  if(item!=currentitem){ 
    if(item){
      item->state|=ICONITEM_CURRENT;
      updateItem(item);
      }
    if(currentitem){
      currentitem->state&=~ICONITEM_CURRENT;
      updateItem(currentitem);
      }
    currentitem=item;
    }
  }


// Set anchor item
void FXIconList::setAnchorItem(FXIconItem* item){
  anchoritem=item;
  }


void FXIconList::setItemText(FXIconItem* item,const char* text){
  if(!item){ fxerror("%s::setItemText: NULL item argument.\n",getClassName()); } 
  item->label=text;
  recalc();
  }


const char* FXIconList::getItemText(const FXIconItem* item) const {
  if(!item){ fxerror("%s::getItemText: NULL item argument.\n",getClassName()); } 
  return item->label.text();
  }


void FXIconList::setItemMiniIcon(FXIconItem* item,FXIcon* icon){
  if(!item){ fxerror("%s::setItemMiniIcon: NULL item argument.\n",getClassName()); } 
  item->miniIcon=icon;
  updateItem(item);
  }


FXIcon* FXIconList::getItemMiniIcon(const FXIconItem* item) const {
  if(!item){ fxerror("%s::getItemMiniIcon: NULL item argument.\n",getClassName()); } 
  return item->miniIcon;
  }


void FXIconList::setItemIcon(FXIconItem* item,FXIcon* icon){
  if(item==NULL){ fxerror("%s::setItemIcon: NULL item argument.\n",getClassName()); }
  item->icon=icon;
  updateItem(item);
  }


FXIcon* FXIconList::getItemIcon(const FXIconItem* item) const {
  if(item==NULL){ fxerror("%s::getItemIcon: NULL item argument.\n",getClassName()); }
  return item->icon;
  }


void FXIconList::setItemData(FXIconItem* item,void* ptr) const {
  if(item==NULL){ fxerror("%s::setItemData: NULL item argument.\n",getClassName()); }
  item->data=ptr;
  }


void* FXIconList::getItemData(const FXIconItem* item) const {
  if(item==NULL){ fxerror("%s::getItemData: NULL item argument.\n",getClassName()); }
  return item->data;
  }


// Test if a is before b
FXbool FXIconList::before(FXIconItem* a,FXIconItem* b){
  while(a && a->next!=b) a=a->next;
  return a!=NULL;
  }


// Extend selection
FXbool FXIconList::extendSelection(FXIconItem* item){
  FXbool changes=FALSE;
  if(item && anchoritem){

    // Browse select mode
    if((options&ICONLIST_BROWSESELECT) && !(options&ICONLIST_SINGLESELECT)){
      changes|=deselectItemRange(firstitem,item->prev);
      changes|=deselectItemRange(item->next,lastitem);
      changes|=selectItem(item);
      }

    // Extended number selected
    else if(!(options&(ICONLIST_SINGLESELECT|ICONLIST_BROWSESELECT))){
      if(item==anchoritem){
        changes|=restoreItemRange(firstitem,anchoritem->prev);
        changes|=restoreItemRange(anchoritem->next,lastitem);
        }
      else if(before(item,anchoritem)){
        changes|=restoreItemRange(firstitem,item->prev);
        changes|=restoreItemRange(anchoritem->next,lastitem);
        if(anchoritem->state&ICONITEM_SELECTED){
          changes|=selectItemRange(item,anchoritem->prev);
          }
        else{
          changes|=deselectItemRange(item,anchoritem->prev);
          }
        }
      else{
        changes|=restoreItemRange(firstitem,anchoritem->prev);
        changes|=restoreItemRange(item->next,lastitem);
        if(anchoritem->state&ICONITEM_SELECTED){
          changes|=selectItemRange(anchoritem->next,item);
          }
        else{
          changes|=deselectItemRange(anchoritem->next,item);
          }
        }
      }
    }
  return changes;
  }


// Key Press
long FXIconList::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXIconItem *item;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
//fprintf(stderr,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state);
  switch(event->code){
    case KEY_Right:
    case KEY_Left:
    case KEY_Up:
    case KEY_Down:
    case KEY_Home:
    case KEY_End:
      item=currentitem;
      switch(event->code){
        case KEY_Right:
          if(!item) item=firstitem; 
          break;
        case KEY_Left:
          if(!item) item=lastitem; 
          break;
        case KEY_Up: 
          if(item && item->prev) item=item->prev; 
          if(!item) item=lastitem; 
          break;
        case KEY_Down:
          if(item && item->next) item=item->next; 
          if(!item) item=firstitem; 
          break;
        case KEY_Home: 
          item=firstitem; 
          break;
        case KEY_End:  
          item=lastitem; 
          break;
        }
      if(item && item!=currentitem){
        handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
        if((options&ICONLIST_BROWSESELECT) && !(options&ICONLIST_SINGLESELECT)){
          handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
          }
        flags&=~FLAG_UPDATE;
        flags|=FLAG_KEY;
        }
      return 1;
    case KEY_space:
      handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
      flags&=~FLAG_UPDATE;
      flags|=FLAG_KEY;
      return 1;
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Shift_L:
    case KEY_Shift_R:
      flags|=FLAG_KEY;
      return 1;
    }
  return 0;
  }


// Key Release 
long FXIconList::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  flags|=FLAG_UPDATE;
  if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
//fprintf(stderr,"%s::onKeyRelease keysym=0x%04x\n",getClassName(),event->code);
  switch(event->code){
    case KEY_Right:
    case KEY_Left:
    case KEY_Up:
    case KEY_Down:
    case KEY_Home:
    case KEY_End:
      if(flags&FLAG_KEY){
        if((options&ICONLIST_BROWSESELECT) && !(options&ICONLIST_SINGLESELECT)){
          handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
          }
        flags&=~FLAG_KEY;
        }
      return 1;
    case KEY_space:
      handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
      flags&=~FLAG_KEY;
      return 1;
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Control_L:
    case KEY_Control_R:
      flags&=~FLAG_KEY;
      return 1;
    }
  return 0;
  }


// Timeout
long FXIconList::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXint lx,rx,ty,by;
  
  // Hide the lasso before scrolling
  drawLasso(anchorx,anchory,currentx,currenty);
  FXScrollArea::onAutoScroll(sender,sel,ptr);
  
  currentx=event->win_x-pos_x;
  currenty=event->win_y-pos_y;
  FXMINMAX(lx,rx,anchorx,currentx);
  FXMINMAX(ty,by,anchory,currenty);
 
  // Perform selection
  if(event->state&SHIFTMASK){
    performSelectionRectangle(lx,ty,rx,by,1,3);
    }
  else if(event->state&CONTROLMASK){
    performSelectionRectangle(lx,ty,rx,by,0,3);
    }
  else{
    performSelectionRectangle(lx,ty,rx,by,1,3);
    }
 
  getApp()->flush();
 
  // Show lasso again
  drawLasso(anchorx,anchory,currentx,currenty);
  return 1;
  }


// Mouse moved
long FXIconList::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXIconItem *item;
  if(flags&FLAG_PRESSED){
    FXint lx,rx,ty,by;
    
    // Start auto scrolling?
    if(startAutoScroll(event->win_x,event->win_y,FALSE)) return 1;
    
    // Find item under cursor
    //item=getItemAt(event->win_x,event->win_y);
    
    // Hide lasso
    drawLasso(anchorx,anchory,currentx,currenty);
    currentx=event->win_x-pos_x;
    currenty=event->win_y-pos_y;
    FXMINMAX(lx,rx,anchorx,currentx);
    FXMINMAX(ty,by,anchory,currenty);
    
    // Perform selection
    if(event->state&SHIFTMASK){
      performSelectionRectangle(lx,ty,rx,by,1,3);
      }
    else if(event->state&CONTROLMASK){
      performSelectionRectangle(lx,ty,rx,by,0,3);
      }
    else{
      performSelectionRectangle(lx,ty,rx,by,1,3);
      }
    
    // Change current item
    //if(item && item!=currentitem){
    //  handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
    //  }
    getApp()->flush();
    
    // Show lasso again
    drawLasso(anchorx,anchory,currentx,currenty);
    return 1;
    }
  return 0; 
  }


// Pressed a button
long FXIconList::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXIconItem *item;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    item=getItemAt(event->win_x,event->win_y);
    if(item!=currentitem) handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
    handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    flags&=~FLAG_UPDATE;
    if(!item && !(options&(ICONLIST_SINGLESELECT|ICONLIST_BROWSESELECT))){
      anchorx=currentx=event->win_x-pos_x;
      anchory=currenty=event->win_y-pos_y;
      drawLasso(anchorx,anchory,currentx,currenty);
      flags|=FLAG_PRESSED;
      }
    markItems();
    return 1;
    }
  return 0;
  }


// Released button
long FXIconList::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    flags|=FLAG_UPDATE;
    if(flags&FLAG_PRESSED) drawLasso(anchorx,anchory,currentx,currenty);
    flags&=~FLAG_PRESSED;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    return 1; 
    }
  return 0;
  }


// Button or Key activate
long FXIconList::onActivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXIconItem* item;
  
  // Remember previous state
  for(item=firstitem; item; item=item->next){
    if(item->state&ICONITEM_SELECTED)
      item->state|=ICONITEM_HISTORY;
    else
      item->state&=~ICONITEM_HISTORY;
    }
  
  // Have currentitem
  if(currentitem){
    
    // Multiple selection mode
    if((options&ICONLIST_SINGLESELECT)&&(options&ICONLIST_BROWSESELECT)){
      if(isItemSelected(currentitem)){
        deselectItem(currentitem);
        }
      else{
        selectItem(currentitem);
        }
      markItems();
      setAnchorItem(currentitem);
      }

    // Browse select mode
    else if(options&ICONLIST_BROWSESELECT){
      deselectItemRange(firstitem,currentitem->prev);
      deselectItemRange(currentitem->next,lastitem);
      selectItem(currentitem);
      setAnchorItem(currentitem);
      }

    // Single selection mode
    else if(options&ICONLIST_SINGLESELECT){
      if(isItemSelected(currentitem)){
        deselectItemRange(firstitem,lastitem);
        }
      else{
        deselectItemRange(firstitem,currentitem->prev);
        deselectItemRange(currentitem->next,lastitem);
        selectItem(currentitem);
        }
      setAnchorItem(currentitem);
      }

    // Add selection mode
    else if(event->state&SHIFTMASK){
      if(anchoritem){
        selectItem(anchoritem);
        markItems();
        extendSelection(currentitem);
        }
      else{
        setAnchorItem(currentitem);
        selectItem(currentitem);
        markItems();
        }
      }
    
    // Toggle selection mode
    else if(event->state&CONTROLMASK){
      setAnchorItem(currentitem);
      toggleItem(currentitem);
      markItems();
      }
    
    // Extended selection mode
    else{
      deselectItemRange(firstitem,currentitem->prev);
      deselectItemRange(currentitem->next,lastitem);
      selectItem(currentitem);
      markItems();
      setAnchorItem(currentitem);
      }
    }
  
  // No currentitem & not multiple or browse select mode
  else if(!(options&ICONLIST_BROWSESELECT)){
    
    // If not add or toggle mode, turn them off
    if(!(event->state&(SHIFTMASK|CONTROLMASK))){
      deselectItemRange(firstitem,lastitem);
      }
    markItems();
    setAnchorItem(currentitem);
    }
  
  // If we want selection, see wether to acquire or release it
  if(options&ICONLIST_WANTSELECTION){
    if(getNumSelectedItems())
      acquireSelection();
    else
      releaseSelection();
    }
  return 1;
  }
  

// Button or Key deactivate
long FXIconList::onDeactivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  
  // New anchor item
  setAnchorItem(currentitem);
  
  // Command callbacks
  if(event->click_count==1) 
    handle(this,MKUINT(0,SEL_CLICKED),currentitem);
  else if(event->click_count==2) 
    handle(this,MKUINT(0,SEL_DOUBLECLICKED),currentitem);
  else if(event->click_count==3) 
    handle(this,MKUINT(0,SEL_TRIPLECLICKED),currentitem);
  return 1;
  }



// Current item changed
long FXIconList::onChanged(FXObject*,FXSelector,void* ptr){
  makeItemVisible((FXIconItem*)ptr);
  setCurrentItem((FXIconItem*)ptr);
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  return 1;
  }


// Selected items
long FXIconList::onSelected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
  }


// Deselected items
long FXIconList::onDeselected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
  }


// Clicked in list
long FXIconList::onClicked(FXObject*,FXSelector,void* ptr){
  register FXIconItem **selectedlist,**deselectedlist,*item;
  register FXint numselected,numdeselected,i,j;
  
  // Notify target of the click
  if(target){
    
    // Clicked message indicates a click anywhere in the widget
    if(target->handle(this,MKUINT(message,SEL_CLICKED),ptr)) return 1;
    
    // Command message indicates click on an item
    if(ptr && target->handle(this,MKUINT(message,SEL_COMMAND),ptr)) return 1;
    }
    
  // Find out number of items whose selection state changed
  for(item=firstitem,numselected=numdeselected=0; item; item=item->next){
    if((item->state&ICONITEM_SELECTED) && !(item->state&ICONITEM_HISTORY)) numselected++;
    if(!(item->state&ICONITEM_SELECTED) && (item->state&ICONITEM_HISTORY)) numdeselected++;
    }
 
  // Make some room
  FXMALLOC(&selectedlist,FXIconItem*,numselected+1);
  FXMALLOC(&deselectedlist,FXIconItem*,numdeselected+1);
 
  // Add items to the proper lists
  for(item=firstitem,i=j=0; item; item=item->next){
    if((item->state&ICONITEM_SELECTED) && !(item->state&ICONITEM_HISTORY)) selectedlist[i++]=item;
    if(!(item->state&ICONITEM_SELECTED) && (item->state&ICONITEM_HISTORY)) deselectedlist[j++]=item;
    }
 
  // Close off lists
  selectedlist[i]=NULL;
  deselectedlist[j]=NULL;
 
  // Tell the target about the newly selected items
  handle(this,MKUINT(0,SEL_SELECTED),selectedlist);
 
  // Tell the target about the newly deselected items
  handle(this,MKUINT(0,SEL_DESELECTED),deselectedlist);
 
  // Free the lists
  FXFREE(&selectedlist);
  FXFREE(&deselectedlist);
  return 1;
  }


// Double Clicked in list; ptr may or may not point to an item
long FXIconList::onDoubleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr);
  }


// Triple Clicked in list; ptr may or may not point to an item
long FXIconList::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }



// Change the font
void FXIconList::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  }


// Set text color
void FXIconList::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Set select background color
void FXIconList::setSelBackColor(FXPixel clr){
  selbackColor=clr;
  update(0,0,width,height);
  }


// Set selected text color
void FXIconList::setSelTextColor(FXPixel clr){
  seltextColor=clr;
  update(0,0,width,height);
  }


// Set text width
void FXIconList::setItemSpace(FXint s){
  if(itemSpace!=s){
    itemSpace=s;
    recalc();
    }
  }


// Set BIG icon width
void FXIconList::setIconWidth(FXint w){
  if(iconWidth!=w){
    iconWidth=w;
    recalc();
    }
  }


// Set BIG icon height
void FXIconList::setIconHeight(FXint h){
  if(iconHeight!=h){
    iconHeight=h;
    recalc();
    }
  }


// Set MINI icon width
void FXIconList::setMiniIconWidth(FXint w){
  if(miniIconWidth!=w){
    miniIconWidth=w;
    recalc();
    }
  }


// Set MINI icon height
void FXIconList::setMiniIconHeight(FXint h){
  if(miniIconHeight!=h){
    miniIconHeight=h;
    recalc();
    }
  }


// Change list style
void FXIconList::setListStyle(FXuint style){
  FXuint opts=(options&~ICONLIST_MASK) | (style&ICONLIST_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Get list style
FXuint FXIconList::getListStyle() const { 
  return (options&ICONLIST_MASK); 
  }


// Cleanup
FXIconList::~FXIconList(){
  removeAllItems();
  header=(FXHeader*)-1;
  firstitem=(FXIconItem*)-1;
  lastitem=(FXIconItem*)-1;
  anchoritem=(FXIconItem*)-1;
  currentitem=(FXIconItem*)-1;
  font=(FXFont*)-1;
  }


