/********************************************************************************
*                                                                               *
*                            L i s t   O b j e c t                              *
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
* $Id: FXList.cpp,v 1.81 1998/10/19 16:23:23 jvz Exp $                        *
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
#include "FXTextField.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXList.h"


// Private header files
#include "FXListItem.h"


/*

  Notes:
  - Draw stuff differently when disabled.
  - SEL_ACTIVATE instead of space and button.
  - Optimize element insert/delete: don't always have to redraw everything.
  - Move active item by means of focus navigation, or first letters typed.
  - Need tentative select/deselect etc.
  - PageUp/PageDn should also change currentitem.
  - Should items self-link [or unlink] when constructed/destructed?
  - Should support borders in ScrollWindow & derivatives.
  - Should issue callbacks from keyboard also.
  - Changes in currentitem are reported also.
  - Need distinguish various callbacks better:
     - Selection changes (all selected/unselected items or just changes???)
     - Changes of currentitem
     - Clicks on currentitem
  - You always get a SEL_COMMAND, even if no item.
  - May need one more mode [multiple select mode vs extended select mode]
*/


#define SIDE_SPACING             6    // Left or right spacing between items
#define LINE_SPACING             1    // Line spacing between items


/*******************************************************************************/

// Map
FXDEFMAP(FXList) FXListMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXList::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXList::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXList::onAutoScroll),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXList::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXList::onLeftBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXList::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXList::onKeyRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXList::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXList::onDeactivate),
  FXMAPFUNC(SEL_FOCUSIN,0,FXList::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXList::onFocusOut),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXList::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXList::onSelectionGained),
  FXMAPFUNC(SEL_CHANGED,0,FXList::onChanged),
  FXMAPFUNC(SEL_CLICKED,0,FXList::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXList::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXList::onTripleClicked),
  FXMAPFUNC(SEL_SELECTED,0,FXList::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXList::onDeselected),
  };


// Object implementation
FXIMPLEMENT(FXList,FXScrollArea,FXListMap,ARRAYNUMBER(FXListMap))

  
// List
FXList::FXList(){
  flags|=FLAG_ENABLED;
  firstitem=NULL;
  lastitem=NULL;
  anchoritem=NULL;
  currentitem=NULL;
  font=(FXFont*)-1;
  sortfunc=NULL;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  totalWidth=0;
  totalHeight=0;
  }

  
// List
FXList::FXList(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
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
  totalWidth=0;
  totalHeight=0;
  }


// Create X window
void FXList::create(){
  FXScrollArea::create();
  textColor=acquireColor(getApp()->foreColor);
  selbackColor=acquireColor(getApp()->selbackColor);
  seltextColor=acquireColor(getApp()->selforeColor);
  font->create();
  }


// Propagate size change
void FXList::recalc(){ 
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


// Can have focus
FXbool FXList::canFocus() const { return TRUE; }


// Create item
FXListItem* FXList::createItem(){ return new FXListItem; }


// Get number of items
FXint FXList::getNumItems() const {
  register FXListItem *item=firstitem;
  register FXint n=0;
  while(item){
    item=item->next;
    n++;
    }
  return n;
  }


// Get number of selected items
FXint FXList::getNumSelectedItems() const {
  register FXListItem *item=firstitem;
  register FXint n=0;
  while(item){
    if(item->state&LISTITEM_SELECTED) n++;
    item=item->next;
    }
  return n;
  }


// Get list of selected ite,s
FXListItem** FXList::getSelectedItems() const {
  register FXListItem *item=firstitem;
  register FXListItem **itemlist;
  register FXint i=0;
  FXMALLOC(&itemlist,FXListItem*,getNumSelectedItems()+1);
  while(item){
    if(item->state&LISTITEM_SELECTED) itemlist[i++]=item;
    item=item->next;
    }
  itemlist[i]=NULL;
  return itemlist;
  }
 


// Get item X
FXint FXList::getItemX(const FXListItem* item) const { 
  if(!item){ fxerror("%s::getItemX: item is NULL.\n",getClassName()); } 
  return 0; 
  }


// Get item Y
FXint FXList::getItemY(const FXListItem* item) const { 
  if(!item){ fxerror("%s::getItemY: item is NULL.\n",getClassName()); } 
  return item->y; 
  }


// Get item width
FXint FXList::getItemWidth(const FXListItem* item) const { 
  if(!item){ fxerror("%s::getItemWidth: item is NULL.\n",getClassName()); }
  return content_w;
  }


// Get Item height
FXint FXList::getItemHeight(const FXListItem* item) const { 
  if(!item){ fxerror("%s::getItemHeight: item is NULL.\n",getClassName()); } 
  return LINE_SPACING+font->getFontHeight(); 
  }


// True if item is selected
FXbool FXList::isItemSelected(const FXListItem* item) const { 
  if(!item){ fxerror("%s::isItemSelected: item is NULL.\n",getClassName()); } 
  return (item->state&LISTITEM_SELECTED)!=0; 
  }


// True if item is current
FXbool FXList::isItemCurrent(const FXListItem* item) const { 
  if(!item){ fxerror("%s::isItemCurrent: item is NULL.\n",getClassName()); } 
  return (item->state&LISTITEM_CURRENT)!=0; 
  }


// True if item (partially) visible
FXbool FXList::isItemVisible(const FXListItem* item) const { 
  if(!item){ fxerror("%s::isItemVisible: item is NULL.\n",getClassName()); } 
  return (0 < (pos_y+getItemY(item)+getItemHeight(item))) && ((pos_y+getItemY(item))) < viewport_h; 
  }


// Make item fully visible
void FXList::makeItemVisible(FXListItem* item){ 
  register FXint y,h;
  if(item){
    y=getItemY(item);
    h=getItemHeight(item);
    if((pos_y+y) < 0){
      setPosition(pos_x,-y);
      update(0,0,width,height);
      }
    else if(viewport_h<=(pos_y+y+h)){
      setPosition(pos_x,viewport_h-y-h);
      update(0,0,width,height);
      }
    }
  }


// Get item at position x,y
FXListItem* FXList::getItemAt(FXint,FXint y) const {
  register FXint h=LINE_SPACING+font->getFontHeight();
  register FXListItem* n=firstitem;
  register FXint yy=0;
  y=y-pos_y;
  while(n){
    if(y<yy) return NULL;
    yy+=h;
    if(y<yy) return n;
    n=n->next;
    }
  return NULL;
  }


// Get next item
FXListItem* FXList::getNextItem(const FXListItem* item) const { 
  if(!item){ fxerror("%s::getNextItem: item is NULL.\n",getClassName()); } 
  return item->next; 
  }


// Get previous item
FXListItem* FXList::getPrevItem(const FXListItem* item) const { 
  if(!item){ fxerror("%s::getPrevItem: item is NULL.\n",getClassName()); } 
  return item->prev; 
  }


// Sort the items based on the sort function
void FXList::sortItems(){
  FXItem* f=(FXItem*)firstitem;
  FXItem* l=(FXItem*)lastitem;
  fxsort((FXItem*&)firstitem,(FXItem*&)lastitem,f,l,sortfunc,getNumItems());
  recalc();
  }


// Repaint
void FXList::updateItem(FXListItem* item){
  update(pos_x+getItemX(item),pos_y+getItemY(item),getItemWidth(item),getItemHeight(item));
  }


// All turned on
FXbool FXList::selectItemRange(FXListItem* beg,FXListItem* end){
  register FXbool selected=FALSE;
  if(beg && end){
    register FXListItem *item=beg;
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
FXbool FXList::deselectItemRange(FXListItem* beg,FXListItem* end){
  register FXbool deselected=FALSE;
  if(beg && end){
    register FXListItem *item=beg;
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
FXbool FXList::toggleItemRange(FXListItem* beg,FXListItem* end){
  register FXbool toggled=FALSE;
  if(beg && end){
    register FXListItem *item=beg;
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
FXbool FXList::restoreItemRange(FXListItem* beg,FXListItem* end){
  register FXbool restored=FALSE;
  if(beg && end){
    register FXListItem *item=beg;
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
FXbool FXList::selectItem(FXListItem* item){
  if(item){
    if(!(item->state&LISTITEM_SELECTED)){
      item->state|=LISTITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// Deselect one item
FXbool FXList::deselectItem(FXListItem* item){
  if(item){
    if(item->state&LISTITEM_SELECTED){
      item->state&=~LISTITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// toggle one item
FXbool FXList::toggleItem(FXListItem* item){
  if(item){
    item->state^=LISTITEM_SELECTED;
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }


// Restore items to previous state
FXbool FXList::restoreItem(FXListItem* item){
  if(item){
    if((item->state&LISTITEM_MARK) && !(item->state&LISTITEM_SELECTED)){
      item->state|=LISTITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    if(!(item->state&LISTITEM_MARK) && (item->state&LISTITEM_SELECTED)){
      item->state&=~LISTITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// Mark items
void FXList::markItems(){
  register FXListItem* item;
  for(item=firstitem; item; item=item->next){
    (item->state&LISTITEM_SELECTED)?item->state|=LISTITEM_MARK:item->state&=~LISTITEM_MARK;
    }
  }


// Recompute interior 
void FXList::recompute(){
  register FXListItem* item;
  register FXint w,h;
  totalHeight=0;
  totalWidth=0;
  h=LINE_SPACING+font->getFontHeight();
  for(item=firstitem; item; item=item->next){
    item->y=totalHeight;
    w=SIDE_SPACING+font->getTextWidth(item->label.text(),item->label.length());
    if(w>totalWidth) totalWidth=w;
    totalHeight=totalHeight+h;
    }
  flags&=~FLAG_RECALC;
  }


// Determine content width of list
FXint FXList::getContentWidth(){ 
  if(flags&FLAG_RECALC) recompute();
  return totalWidth;
  }


// Determine content height of list
FXint FXList::getContentHeight(){
  if(flags&FLAG_RECALC) recompute();
  return totalHeight; 
  }


// Recalculate layout determines item locations and sizes
void FXList::layout(){
  
  // Calculate contents 
  FXScrollArea::layout();
  
  // Determine line size for scroll bars
  vertical->setLine(LINE_SPACING+font->getFontHeight());
  horizontal->setLine(1);
  
  // Force repaint
  update(0,0,width,height);
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Gained focus
long FXList::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(currentitem) updateItem(currentitem);
  return 1;
  }


// Lost focus
long FXList::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(currentitem) updateItem(currentitem);
  return 1;
  }


// We have the selection
long FXList::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionGained %x\n",getClassName(),this);
  FXScrollArea::onSelectionGained(sender,sel,ptr);
  //update(0,0,width,height);
  return 1;
  }


// We lost the selection
long FXList::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionLost %x\n",getClassName(),this);
  FXScrollArea::onSelectionLost(sender,sel,ptr);
  deselectItemRange(firstitem,lastitem);
  return 1;
  }


// Draw dashed focus rectangle
void FXList::drawFocusRectangle(FXint x,FXint y,FXint w,FXint h){
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


// Draw item list
long FXList::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXListItem* n=firstitem;
  FXint h=LINE_SPACING+font->getFontHeight();
  FXint x=pos_x;
  FXint y=pos_y;
  FXint eylo=event->rect.y-h;
  FXint eyhi=event->rect.y+event->rect.h;
  clearArea(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  setClipRectangle(0,0,viewport_w,viewport_h);
  setTextFont(font);
  while(n && y<eyhi){
    if(eylo<=y){
      if(n->label.text()){
        FXint len=n->label.length();
        if(n->state&LISTITEM_SELECTED){
          if(hasFocus() && (n->state&LISTITEM_CURRENT)){
            setForeground(selbackColor);
            fillRectangle(x+1,y+1,content_w-2,h-2);
            drawFocusRectangle(x,y,content_w,h);
            }
          else{
            setForeground(selbackColor);
            fillRectangle(x,y,content_w,h);
            }
          setForeground(seltextColor);
          drawText(x+(SIDE_SPACING/2),y+(LINE_SPACING/2)+font->getFontAscent(),n->label.text(),len);
          }
        else{
          if(hasFocus() && (n->state&LISTITEM_CURRENT)){
            drawFocusRectangle(x,y,content_w,h);
            }
          setForeground(textColor);
          drawText(x+(SIDE_SPACING/2),y+(LINE_SPACING/2)+font->getFontAscent(),n->label.text(),len);
          }
        }
      }
    y+=h;
    n=n->next;
    }
  clearClipRectangle();
  return 1;
  }


// Test if a is before b
FXbool FXList::before(FXListItem* a,FXListItem* b){
  while(a && a->next!=b) a=a->next;
  return a!=NULL;
  }


// Extend selection
FXbool FXList::extendSelection(FXListItem* item){
  FXbool changes=FALSE;
  if(item && anchoritem){
    
//fprintf(stderr,"extendselection anch=%s item=%s\n",anchoritem->label.text(),item->label.text());
      
    // Browse select mode
    if((options&LIST_BROWSESELECT) && !(options&LIST_SINGLESELECT)){
      changes|=deselectItemRange(firstitem,item->prev);
      changes|=deselectItemRange(item->next,lastitem);
      changes|=selectItem(item);
      }

    // Extended select mode
    else if(!(options&(LIST_SINGLESELECT|LIST_BROWSESELECT))){
      if(item==anchoritem){
        changes|=restoreItemRange(firstitem,anchoritem->prev);
        changes|=restoreItemRange(anchoritem->next,lastitem);
        }
      else if(before(item,anchoritem)){
        changes|=restoreItemRange(firstitem,item->prev);
        changes|=restoreItemRange(anchoritem->next,lastitem);
        if(anchoritem->state&LISTITEM_SELECTED){
          changes|=selectItemRange(item,anchoritem->prev);
          }
        else{
          changes|=deselectItemRange(item,anchoritem->prev);
          }
        }
      else{
        changes|=restoreItemRange(firstitem,anchoritem->prev);
        changes|=restoreItemRange(item->next,lastitem);
        if(anchoritem->state&LISTITEM_SELECTED){
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
long FXList::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXListItem *item;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
//fprintf(stderr,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state);
  switch(event->code){
    case KEY_Up:
    case KEY_Down:
    case KEY_Home:
    case KEY_End:
      item=currentitem;
      switch(event->code){
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
        if((options&LIST_BROWSESELECT) && !(options&LIST_SINGLESELECT)){
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
long FXList::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  flags|=FLAG_UPDATE;
//fprintf(stderr,"%s::onKeyRelease keysym=0x%04x\n",getClassName(),event->code);
  if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
  switch(event->code){
    case KEY_Up:
    case KEY_Down:
    case KEY_Home:
    case KEY_End:
      if(flags&FLAG_KEY){
        if((options&LIST_BROWSESELECT) && !(options&LIST_SINGLESELECT)){
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


// Automatic scroll
long FXList::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXListItem *item;
  FXScrollArea::onAutoScroll(sender,sel,ptr);
  
  // Find item
  item=getItemAt(event->win_x,event->win_y);
  
  // Got item and different from last time
  if(item && item!=currentitem){
    
    // Make it the current item
    handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
 
    // Extend the selection to it
    extendSelection(item);
    }
  return 1;
  }


// Mouse moved
long FXList::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXListItem *item;
  if(flags&FLAG_PRESSED){
    
    // Start auto scrolling?
    if(startAutoScroll(event->win_x,event->win_y,FALSE)) return 1;
    
    // Find item
    item=getItemAt(event->win_x,event->win_y);
    
    // Got an item different from before
    if(item && item!=currentitem){
      
      // Make it the current item
      handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
      
      // Extend the selection to it
      extendSelection(item);
      }
    return 1;
    }
  return 0;
  }
 

// Pressed button
long FXList::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXListItem *item;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    if(options&LIST_AUTOSELECT) return 1;///////// FIX
    item=getItemAt(event->win_x,event->win_y);
    if(item!=currentitem) handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
    handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    flags&=~FLAG_UPDATE;
    flags|=FLAG_PRESSED;
    return 1;
    }
  return 0;
  }


// Released button
long FXList::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    flags&=~FLAG_PRESSED;
    flags|=FLAG_UPDATE;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    return 1;
    }
  return 0;
  }


// Current item changed
long FXList::onChanged(FXObject*,FXSelector,void* ptr){
  makeItemVisible((FXListItem*)ptr);
  setCurrentItem((FXListItem*)ptr);
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  return 1;
  }


// Selected items
long FXList::onSelected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
  }


// Deselected items
long FXList::onDeselected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
  }


// Clicked in list
long FXList::onClicked(FXObject*,FXSelector,void* ptr){
  register FXListItem **selectedlist,**deselectedlist,*item;
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
    if((item->state&LISTITEM_SELECTED) && !(item->state&LISTITEM_HISTORY)) numselected++;
    if(!(item->state&LISTITEM_SELECTED) && (item->state&LISTITEM_HISTORY)) numdeselected++;
    }
 
  // Make some room
  FXMALLOC(&selectedlist,FXListItem*,numselected+1);
  FXMALLOC(&deselectedlist,FXListItem*,numdeselected+1);
 
  // Add items to the proper lists
  for(item=firstitem,i=j=0; item; item=item->next){
    if((item->state&LISTITEM_SELECTED) && !(item->state&LISTITEM_HISTORY)) selectedlist[i++]=item;
    if(!(item->state&LISTITEM_SELECTED) && (item->state&LISTITEM_HISTORY)) deselectedlist[j++]=item;
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


// Double clicked in list; ptr may or may not point to an item
long FXList::onDoubleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr);
  }


// Triple clicked in list; ptr may or may not point to an item
long FXList::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }


// Button or Key activate
long FXList::onActivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXListItem *item;
  
  // Remember previous state
  for(item=firstitem; item; item=item->next){
    if(item->state&LISTITEM_SELECTED)
      item->state|=LISTITEM_HISTORY;
    else
      item->state&=~LISTITEM_HISTORY;
    }
  
  // Have currentitem
  if(currentitem){
    
    // Multiple selection mode
    if((options&LIST_SINGLESELECT)&&(options&LIST_BROWSESELECT)){
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
    else if(options&LIST_BROWSESELECT){
      deselectItemRange(firstitem,currentitem->prev);
      deselectItemRange(currentitem->next,lastitem);
      selectItem(currentitem);
      setAnchorItem(currentitem);
      }

    // Single selection mode
    else if(options&LIST_SINGLESELECT){
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
      setAnchorItem(currentitem);
      markItems();
      }
    }
  
  // No currentitem & not multiple or browse select mode
  else if(!(options&LIST_BROWSESELECT)){
    
    // If not add or toggle mode, turn them off
    if(!(event->state&(SHIFTMASK|CONTROLMASK))){
      deselectItemRange(firstitem,lastitem);
      }
    setAnchorItem(currentitem);
    }
  
  // If we want selection, see wether to acquire or release it
  if(options&LIST_WANTSELECTION){
    if(getNumSelectedItems())
      acquireSelection();
    else
      releaseSelection();
    }
  return 1;
  }
  

// Button or Key deactivate
long FXList::onDeactivate(FXObject*,FXSelector,void* ptr){
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


// Add item as first
FXListItem* FXList::addItemFirst(const char* text,void* ptr){
  FXListItem *item=createItem();
  item->prev=NULL;
  item->next=firstitem;
  if(item->next) item->next->prev=item; else lastitem=item;
  firstitem=item;
  item->label=text;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Add item as last
FXListItem* FXList::addItemLast(const char* text,void* ptr){
  FXListItem *item=createItem();
  item->prev=lastitem;
  item->next=NULL;
  if(item->prev) item->prev->next=item; else firstitem=item;
  lastitem=item;
  item->label=text;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Add item after other
FXListItem* FXList::addItemAfter(FXListItem* other,const char* text,void* ptr){
  if(other==NULL){ fxerror("%s::addItemAfter: other item is NULL\n",getClassName()); }
  FXListItem *item=createItem();
  item->prev=other;
  item->next=other->next;
  other->next=item;
  if(item->next) item->next->prev=item; else lastitem=item; 
  item->label=text;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Add item before other
FXListItem* FXList::addItemBefore(FXListItem* other,const char* text,void* ptr){
  if(other==NULL){ fxerror("%s::addItemBefore: other item is NULL\n",getClassName()); }
  FXListItem *item=createItem();
  item->next=other;
  item->prev=other->prev;
  other->prev=item;
  if(item->prev) item->prev->next=item; else firstitem=item;
  item->label=text;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Remove node from list
void FXList::removeItem(FXListItem* item){
  if(item){
    if(item->prev) item->prev->next=item->next; else firstitem=item->next;
    if(item->next) item->next->prev=item->prev; else lastitem=item->prev;
    if(currentitem==item) currentitem=NULL;
    if(anchoritem==item) anchoritem=NULL;
    delete item;
    recalc();
    }
  }


// Remove items [fm..to]
void FXList::removeItems(FXListItem* fm,FXListItem* to){
  if(fm && to){
    register FXListItem *item;
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
void FXList::removeAllItems(){
  register FXListItem *item;
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
void FXList::setCurrentItem(FXListItem* item){
  if(item!=currentitem){ 
    if(item){
      item->state|=LISTITEM_CURRENT;
      updateItem(item);
      }
    if(currentitem){
      currentitem->state&=~LISTITEM_CURRENT;
      updateItem(currentitem);
      }
    currentitem=item;
    }
  }


// Set anchor item
void FXList::setAnchorItem(FXListItem* item){
  anchoritem=item;
  }


void FXList::setItemText(FXListItem* item,const char* text){
  if(item==NULL){ fxerror("%s::setItemText: item is NULL\n",getClassName()); }
  item->label=text;
  recalc();
  }


const char* FXList::getItemText(const FXListItem* item) const {
  if(item==NULL){ fxerror("%s::getItemText: item is NULL\n",getClassName()); }
  return item->label.text();
  }


void FXList::setItemData(FXListItem* item,void* ptr) const {
  if(item==NULL){ fxerror("%s::setItemData: item is NULL\n",getClassName()); }
  item->data=ptr;
  }


void* FXList::getItemData(const FXListItem* item) const {
  if(item==NULL){ fxerror("%s::getItemData: item is NULL\n",getClassName()); }
  return item->data;
  }


// Change the font
void FXList::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  }


// Set text color
void FXList::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Set select background color
void FXList::setSelBackColor(FXPixel clr){
  selbackColor=clr;
  update(0,0,width,height);
  }


// Set selected text color
void FXList::setSelTextColor(FXPixel clr){
  seltextColor=clr;
  update(0,0,width,height);
  }


// Change list style
void FXList::setListStyle(FXuint style){
  options=(options&~LIST_MASK) | (style&LIST_MASK);
  }


// Get list style
FXuint FXList::getListStyle() const { 
  return (options&LIST_MASK); 
  }


// Clean up
FXList::~FXList(){
  removeAllItems();
  firstitem=(FXListItem*)-1;
  lastitem=(FXListItem*)-1;
  anchoritem=(FXListItem*)-1;
  currentitem=(FXListItem*)-1;
  font=(FXFont*)-1;
  }


