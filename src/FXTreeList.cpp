/********************************************************************************
*                                                                               *
*                          T r e e L i s t   O b j e c t                        *
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
* $Id: FXTreeList.cpp,v 1.75 1998/10/19 21:38:23 jvz Exp $                    *
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
#include "FXStatusbar.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXTooltip.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXTreeList.h"

// Private header
#include "FXTreeItem.h"


/*
  To do:
  - Draw stuff differently when disabled.
  - Optimize element insert/delete: don't always have to redraw everything.
  - Tooltip should show text of current item, if any.
  - Tooltip should pop up exactly on top of current item.
  - Should allow for more flexible icon sizes.
  - You always get a SEL_COMMAND, even if no item.
  - Clicking on + does not make it current?
  - Text layout not good if bigger texts.
  - Need box on toplevel items, sometimes...
  - Need indent variable.
  - Should compute size of icons etc.
  - I guess select==open, but open is also open...
  - Need API to expand all parent of certain item so that it may be visible
*/

#define ICON_SPACING        4         // Spacing between parent and child in x direction
#define TEXT_SPACING        4         // Spacing between icon and text
#define LINE_SPACING        1         // Spacing between lines
#define SIDE_SPACING        4         // Spacing between side and item

#define DEFAULT_ICONWIDTH   16
#define DEFAULT_ICONHEIGHT  16
#define DEFAULT_INDENT      20        // Indent between parent and child

/*******************************************************************************/

// Map
FXDEFMAP(FXTreeList) FXTreeListMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTreeList::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXTreeList::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXTreeList::onAutoScroll),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTreeList::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTreeList::onLeftBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXTreeList::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXTreeList::onKeyRelease),
  FXMAPFUNC(SEL_ACTIVATE,0,FXTreeList::onActivate),
  FXMAPFUNC(SEL_DEACTIVATE,0,FXTreeList::onDeactivate),
  FXMAPFUNC(SEL_FOCUSIN,0,FXTreeList::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXTreeList::onFocusOut),
  FXMAPFUNC(SEL_OPENED,0,FXTreeList::onItemOpened),
  FXMAPFUNC(SEL_CLOSED,0,FXTreeList::onItemClosed),
  FXMAPFUNC(SEL_EXPANDED,0,FXTreeList::onItemExpanded),
  FXMAPFUNC(SEL_COLLAPSED,0,FXTreeList::onItemCollapsed),
  FXMAPFUNC(SEL_CHANGED,0,FXTreeList::onChanged),
  FXMAPFUNC(SEL_CLICKED,0,FXTreeList::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXTreeList::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXTreeList::onTripleClicked),
  FXMAPFUNC(SEL_SELECTED,0,FXTreeList::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXTreeList::onDeselected),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXTreeList::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXTreeList::onSelectionGained),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXTreeList::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXTreeList::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXTreeList,FXScrollArea,FXTreeListMap,ARRAYNUMBER(FXTreeListMap))

  
// Tree List
FXTreeList::FXTreeList(){
  firstitem=NULL;
  lastitem=NULL;
  anchoritem=NULL;
  currentitem=NULL;
  font=(FXFont*)-1;
  sortfunc=NULL;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  lineColor=0;
  iconWidth=DEFAULT_ICONWIDTH;
  iconHeight=DEFAULT_ICONHEIGHT;
  itemWidth=0;
  itemHeight=0;
  totalWidth=0;
  totalHeight=0;
  indent=DEFAULT_INDENT;
  }


// Tree List
FXTreeList::FXTreeList(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
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
  lineColor=0;
  iconWidth=DEFAULT_ICONWIDTH;
  iconHeight=DEFAULT_ICONHEIGHT;
  itemWidth=0;
  itemHeight=0;
  totalWidth=0;
  totalHeight=0;
  indent=DEFAULT_INDENT;
  }


// Create X window
void FXTreeList::create(){
  register FXTreeItem* n=firstitem;
  FXScrollArea::create();
  selbackColor=acquireColor(getApp()->selbackColor);
  seltextColor=acquireColor(getApp()->selforeColor);
  textColor=acquireColor(getApp()->foreColor);
  lineColor=acquireColor(getApp()->shadowColor);
  font->create();
  while(n){
    if(n->openIcon){n->openIcon->create();}
    if(n->closedIcon){n->closedIcon->create();}
    if(n->first){n=n->first;continue;}
    while(!n->next && n->parent){n=n->parent;}
    n=n->next;
    }
  }


// Get number of toplevel items
FXint FXTreeList::getNumItems() const {
  register FXTreeItem *item=firstitem;
  register FXint n=0;
  while(item){
    item=item->next;
    n++;
    }
  return n;
  }


// Get number of child items
FXint FXTreeList::getNumChildItems(FXTreeItem* par) const {
  if(!par){ fxerror("%s::getNumChildItems: item is NULL.\n",getClassName()); } 
  register FXTreeItem *item=par->first;
  register FXint n=0;
  while(item){
    item=item->next;
    n++;
    }
  return n;
  }


// Get number of selected items
FXint FXTreeList::getNumSelectedItems() const {
  register FXTreeItem *item=firstitem;
  register FXint n=0;
  while(item){
    if(item->state&TREEITEM_SELECTED) n++;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  return n;
  }


// Get list of selected ite,s
FXTreeItem** FXTreeList::getSelectedItems() const {
  register FXTreeItem *item=firstitem;
  register FXTreeItem **itemlist;
  register FXint i=0;
  FXMALLOC(&itemlist,FXTreeItem*,getNumSelectedItems()+1);
  while(item){
    if(item->state&TREEITEM_SELECTED) itemlist[i++]=item;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  itemlist[i]=NULL;
  return itemlist;
  }
 

// Propagate size change
void FXTreeList::recalc(){ 
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


// Can have focus
FXbool FXTreeList::canFocus() const { return TRUE; }


// Create item
FXTreeItem* FXTreeList::createItem(){ return new FXTreeItem; }


// Get item X
FXint FXTreeList::getItemX(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getItemX: item is NULL.\n",getClassName()); } 
  return item->x; 
  }


// Get item Y
FXint FXTreeList::getItemY(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getItemY: item is NULL.\n",getClassName()); } 
  return item->y; 
  }


// Get item width
FXint FXTreeList::getItemWidth(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getItemWidth: item is NULL.\n",getClassName()); } 
  return iconWidth+font->getTextWidth(item->label.text(),item->label.length())+TEXT_SPACING+SIDE_SPACING;
  }


// Get Item height
FXint FXTreeList::getItemHeight(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getItemHeight: item is NULL.\n",getClassName()); }
  return FXMAX(iconHeight,(font->getFontHeight()+1))+LINE_SPACING; 
  }


// True if item is selected
FXbool FXTreeList::isItemSelected(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::isItemSelected: item is NULL.\n",getClassName()); } 
  return (item->state&TREEITEM_SELECTED)!=0; 
  }


// True if item is current
FXbool FXTreeList::isItemCurrent(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::isItemCurrent: item is NULL.\n",getClassName()); } 
  return (item->state&TREEITEM_CURRENT)!=0; 
  }


// True if item (partially) visible
FXbool FXTreeList::isItemVisible(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::isItemVisible: item is NULL.\n",getClassName()); } 
  return 0<(pos_y+item->y+itemHeight) && (pos_y+item->y)<viewport_h; 
  }


// Check if item is expanded
FXbool FXTreeList::isItemExpanded(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemExpanded: item is NULL.\n",getClassName()); } 
  return (item->state&TREEITEM_EXPANDED)!=0; 
  }
  

// Is item a leaf item
FXbool FXTreeList::isItemLeaf(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemLeaf: item is NULL.\n",getClassName()); } 
  return item->first!=NULL; 
  }


// Get next item
FXTreeItem* FXTreeList::getNextItem(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getNextItem: item is NULL.\n",getClassName()); } 
  return item->next; 
  }


// Get previous item
FXTreeItem* FXTreeList::getPrevItem(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getPrevItem: item is NULL.\n",getClassName()); } 
  return item->prev; 
  }


// Get item (logically) above item
FXTreeItem* FXTreeList::getItemAbove(FXTreeItem* item) const {
  if(!item) return NULL;
  if(!item->prev) return item->parent;
  item=item->prev;
  while(item->last) item=item->last;
  return item;
  }


// Get item (logically) below
FXTreeItem* FXTreeList::getItemBelow(FXTreeItem* item) const {
  if(!item) return NULL;
  if(item->first) return item->first;
  while(!item->next && item->parent) item=item->parent;
  return item->next;
  }


// Get item's first child
FXTreeItem* FXTreeList::getFirstChildItem(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getFirstChildItem: item is NULL.\n",getClassName()); } 
  return item->first; 
  }


// Get item's last child
FXTreeItem* FXTreeList::getLastChildItem(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getLastChildItem: item is NULL.\n",getClassName()); } 
  return item->last; 
  }


// Get item's parent
FXTreeItem* FXTreeList::getParentItem(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::getParentItem: item is NULL.\n",getClassName()); } 
  return item->parent; 
  }


// Did we hit the item box
FXbool FXTreeList::hitItemBox(const FXTreeItem* item,FXint x,FXint y) const {
  if(!item){ fxerror("%s::hitItemBox: item is NULL.\n",getClassName()); }
  if((options&TREELIST_SHOWS_BOXES) && item->first){
    x=x-pos_x;
    y=y-pos_y;
    return item->y<=y && y<item->y+itemHeight && item->x-indent+iconWidth/2+(SIDE_SPACING/2)-4<=x && x<=item->x-indent+iconWidth/2+(SIDE_SPACING/2)+4;
    }
  return FALSE;
  }


// Did we hit the item
FXbool FXTreeList::hitItem(const FXTreeItem* item,FXint x,FXint y) const {
  if(!item){ fxerror("%s::hitItem: item is NULL.\n",getClassName()); } 
  x=x-pos_x;
  y=y-pos_y;
  return item->y<=y && y<item->y+itemHeight && item->x+(SIDE_SPACING/2)<=x && x<item->x+(SIDE_SPACING/2)+TEXT_SPACING+iconWidth+font->getTextWidth(item->label.text(),item->label.length());
  }


// Repaint
void FXTreeList::updateItem(FXTreeItem* item){
  update(0,pos_y+item->y,content_w,itemHeight);
  }

  
// Sort child items
void FXTreeList::sortChildItems(FXTreeItem* item){
  FXItem* f=(FXItem*)item->first;
  FXItem* l=(FXItem*)item->last;
  fxsort((FXItem*&)item->first,(FXItem*&)item->last,f,l,sortfunc,getNumChildItems(item));
  recalc();
  }


// Sort the items based on the sort function
void FXTreeList::sortItems(){
  FXItem* f=(FXItem*)firstitem;
  FXItem* l=(FXItem*)lastitem;
  fxsort((FXItem*&)firstitem,(FXItem*&)lastitem,f,l,sortfunc,getNumItems());
  recalc();
  }


// All turned on
FXbool FXTreeList::selectItemRange(FXTreeItem* beg,FXTreeItem* end){
  register FXbool selected=FALSE;
  register FXTreeItem *item=beg;
  while(item){
    selected|=selectItem(item);
    if(item==end) break;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  return selected;
  }


// All turned off
FXbool FXTreeList::deselectItemRange(FXTreeItem* beg,FXTreeItem* end){
  register FXbool deselected=FALSE;
  register FXTreeItem *item=beg;
  while(item){
    deselected|=deselectItem(item);
    if(item==end) break;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  return deselected;
  }



// Toggle items
FXbool FXTreeList::toggleItemRange(FXTreeItem* beg,FXTreeItem* end){
  register FXbool toggled=FALSE;
  register FXTreeItem *item=beg;
  while(item){
    toggled|=toggleItem(item);
    if(item==end) break;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  return toggled;
  }


// Restore items to previous state
FXbool FXTreeList::restoreItemRange(FXTreeItem* beg,FXTreeItem* end){
  register FXbool restored=FALSE;
  register FXTreeItem *item=beg;
  while(item){
    restored|=restoreItem(item);
    if(item==end) break;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  return restored;
  }


// Select one item
FXbool FXTreeList::selectItem(FXTreeItem* item){
  if(item){
    if(!(item->state&TREEITEM_SELECTED)){
      item->state|=TREEITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// Deselect one item
FXbool FXTreeList::deselectItem(FXTreeItem* item){
  if(item){
    if(item->state&TREEITEM_SELECTED){
      item->state&=~TREEITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// toggle one item
FXbool FXTreeList::toggleItem(FXTreeItem* item){
  if(item){
    item->state^=TREEITEM_SELECTED;
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }


// Restore items to previous state
FXbool FXTreeList::restoreItem(FXTreeItem* item){
  if(item){
    if((item->state&TREEITEM_MARK) && !(item->state&TREEITEM_SELECTED)){
      item->state|=TREEITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    if(!(item->state&TREEITEM_MARK) && (item->state&TREEITEM_SELECTED)){
      item->state&=~TREEITEM_SELECTED;
      updateItem(item);
      return TRUE;
      }
    }
  return FALSE;
  }


// Mark items
void FXTreeList::markItems(){
  register FXTreeItem *item=firstitem;
  while(item){
    (item->state&TREEITEM_SELECTED)?item->state|=TREEITEM_MARK:item->state&=~TREEITEM_MARK;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  }


// Recompute interior 
void FXTreeList::recompute(){
  register FXTreeItem* item;
  register FXint x,y,t;
//fprintf(stderr,"%s::recompute\n",getClassName());
  totalWidth=0;
  totalHeight=0;
  itemWidth=0;
  itemHeight=FXMAX(iconHeight,font->getFontHeight()+1)+LINE_SPACING;
  if(itemHeight&1) itemHeight++;    // Got to be even for the dotted lines!!
  x=y=0;
  if(options&TREELIST_ROOT_BOXES) x=indent;
  item=firstitem;
  while(item){
    item->x=x;
    item->y=y;
    y+=itemHeight;
    if(item->label.text()){
      t=x+iconWidth+TEXT_SPACING+SIDE_SPACING+font->getTextWidth(item->label.text(),item->label.length());
      if(t>itemWidth) itemWidth=t;
      }
    if(item->first && (item->state&TREEITEM_EXPANDED)){
      x+=indent;
      item=item->first;
      continue;
      }
    while(!item->next && item->parent){
      x-=indent;
      item=item->parent;
      }
    item=item->next;
    }
  totalWidth=itemWidth;
  totalHeight=y;
  flags&=~FLAG_RECALC;
  }

    
// Determine content width of tree list
FXint FXTreeList::getContentWidth(){
  if(flags&FLAG_RECALC) recompute();
  return totalWidth;
  }


// Determine content height of tree list
FXint FXTreeList::getContentHeight(){
  if(flags&FLAG_RECALC) recompute();
  return totalHeight;
  }


// Recalculate layout
void FXTreeList::layout(){

  // Calculate contents
  FXScrollArea::layout();
  
  // Set line size based on font
  vertical->setLine(itemHeight);
  horizontal->setLine(1);
  
  // Force repaint
  update(0,0,width,height);
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Gained focus
long FXTreeList::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(currentitem) updateItem(currentitem);
  return 1;
  }


// Lost focus
long FXTreeList::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(currentitem) updateItem(currentitem);
  return 1;
  }


// We have the selection
long FXTreeList::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionGained %x\n",getClassName(),this);
  FXScrollArea::onSelectionGained(sender,sel,ptr);
  //update(0,0,width,height);
  return 1;
  }


// We lost the selection
long FXTreeList::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onSelectionLost %x\n",getClassName(),this);
  FXScrollArea::onSelectionLost(sender,sel,ptr);
  deselectItemRange(firstitem,NULL);
  return 1;
  }



// Draw dashed focus rectangle
void FXTreeList::drawFocusRectangle(FXint x,FXint y,FXint w,FXint h){
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
long FXTreeList::onPaint(FXObject*,FXSelector,void* ptr){
  static const char onoff[]={1,1};
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem* n=firstitem;
  FXint eylo,eyhi,tw,th,yh,xh,x,y,xp,hh;
  FXint iconoffx,iconoffy,textoffx,textoffy;
  FXIcon *icon;
  eylo=event->rect.y-itemHeight;
  eyhi=event->rect.y+event->rect.h;
  th=font->getFontHeight();
  clearArea(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
  setClipRectangle(0,0,viewport_w,viewport_h);
  setTextFont(font);
  hh=(itemHeight/2)&~1;   // Got to be even for the dotted lines!
  while(n){
    x=n->x+pos_x;
    y=n->y+pos_y;
    if(eylo<=y && y<eyhi){
      
      // Show as open or closed
      if(n->state&TREEITEM_OPEN)
        icon=n->openIcon;
      else
        icon=n->closedIcon;
      
      // Show the icon
      if(icon){
        iconoffx=x+(SIDE_SPACING/2);
        iconoffy=y+(LINE_SPACING/2)+(itemHeight-icon->getHeight())/2;
        drawIcon(icon,iconoffx,iconoffy);
        }
      
      // Show the text
      if(n->label.text()){
        textoffx=x+(SIDE_SPACING/2)+TEXT_SPACING+iconWidth;
        textoffy=y+(LINE_SPACING/2)+(itemHeight-th)/2;
        tw=font->getTextWidth(n->label.text(),n->label.length());
        if(n->state&TREEITEM_SELECTED){
          if(hasFocus() && (n->state&TREEITEM_CURRENT)){
            setForeground(selbackColor);
            fillRectangle(textoffx+1,textoffy+1,tw,th-1);
            drawFocusRectangle(textoffx,textoffy,tw+2,th+1);
            }
          else{
            setForeground(selbackColor);
            fillRectangle(textoffx,textoffy,tw+2,th+1);
            }
          setForeground(seltextColor);
          drawText(textoffx+1,textoffy+font->getFontAscent(),n->label.text(),n->label.length());
          }
        else{
          if(n->state&TREEITEM_CURRENT){
            drawFocusRectangle(textoffx,textoffy,tw+2,th+1);
            }
          setForeground(textColor);
          drawText(textoffx+1,textoffy+font->getFontAscent(),n->label.text(),n->label.length());
          }
        }
      
      // Show other paraphernalia such as dotten lines and expand-boxes
      if((options&(TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES)) && (n->parent || (options&TREELIST_ROOT_BOXES))){
        yh=y+hh;
        xh=x-indent+(SIDE_SPACING/2)+iconWidth/2;
        setForeground(lineColor);
        setDashes(0,onoff,2);
        if(options&TREELIST_SHOWS_LINES){                   // Connect items with lines
          FXTreeItem* p=n->parent;
          xp=xh;
          setLineAttributes(0,LINE_ONOFF_DASH,CAP_BUTT,JOIN_MITER);
          while(p){
            xp-=indent;
            if(p->next) drawLine(xp,y,xp,y+itemHeight);
            p=p->parent;
            }
          if((options&TREELIST_SHOWS_BOXES) && n->first){
            if(n->prev || n->parent) drawLine(xh,y,xh,yh-4);
            if(n->next) drawLine(xh,yh+4,xh,y+itemHeight);
            }
          else{
            if(n->prev || n->parent) drawLine(xh,y,xh,yh);
            if(n->next) drawLine(xh,yh,xh,y+itemHeight);
            drawLine(xh,yh,x+(SIDE_SPACING/2)-2,yh);
            }
          setLineAttributes(0,LINE_SOLID,CAP_BUTT,JOIN_MITER);
          }
        if((options&TREELIST_SHOWS_BOXES) && n->first){     // Boxes before items for expand/collapse
          setLineAttributes(0,LINE_ONOFF_DASH,CAP_BUTT,JOIN_MITER);
          drawLine(xh+4,yh,x+(SIDE_SPACING/2)-2,yh);
          setLineAttributes(0,LINE_SOLID,CAP_BUTT,JOIN_MITER);
          drawLine(xh-4,yh-4,xh+4,yh-4);
          drawLine(xh-4,yh+4,xh+4,yh+4);
          drawLine(xh+4,yh-4,xh+4,yh+4);
          drawLine(xh-4,yh-4,xh-4,yh+4);
          setForeground(textColor);
          drawLine(xh-2,yh,xh+2,yh);
          if(!(n->state&TREEITEM_EXPANDED)){
            drawLine(xh,yh-2,xh,yh+2);
            }
          }
        }
      }
    
    // Move on to the next item
    if(n->first && (n->state&TREEITEM_EXPANDED)){n=n->first;continue;}
    while(!n->next && n->parent){n=n->parent;}
    n=n->next;
    }
  clearClipRectangle();
  return 1;
  }


// We were asked about tip text
long FXTreeList::onQueryTip(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
  if(flags&FLAG_TIP){
    if(tip.text()){
      sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&tip);
      return 1;
      }
//     if(itemunder && itemunder->label.text()){
//       ((FXTooltip*)sender)->setText(itemunder->label.text());
//       return 1;
//       }
    }
  return 0;
  }


// We were asked about status text
long FXTreeList::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// Extend selection
FXbool FXTreeList::extendSelection(FXTreeItem* item){
  FXbool changes=FALSE;
  if(item && anchoritem){
    
//fprintf(stderr,"extendselection anch=%s item=%s\n",anchoritem->label.text(),item->label.text());
      
    // Browse select mode
    if((options&TREELIST_BROWSESELECT) && !(options&TREELIST_SINGLESELECT)){
      changes|=deselectItemRange(firstitem,getItemAbove(item));
      changes|=deselectItemRange(getItemBelow(item),NULL);
      changes|=selectItem(item);
      }

    // Extended number selected
    else if(!(options&(TREELIST_SINGLESELECT|TREELIST_BROWSESELECT))){
      if(item==anchoritem){
        changes|=restoreItemRange(firstitem,getItemAbove(anchoritem));
        changes|=restoreItemRange(getItemBelow(anchoritem),NULL);
        }
      else if(item->y < anchoritem->y){
        changes|=restoreItemRange(firstitem,getItemAbove(item));
        changes|=restoreItemRange(getItemBelow(anchoritem),NULL);
        if(anchoritem->state&TREEITEM_SELECTED){
          changes|=selectItemRange(item,getItemAbove(anchoritem));
          }
        else{
          changes|=deselectItemRange(item,getItemAbove(anchoritem));
          }
        }
      else{
        changes|=restoreItemRange(firstitem,getItemAbove(anchoritem));
        changes|=restoreItemRange(getItemBelow(item),NULL);
        if(anchoritem->state&TREEITEM_SELECTED){
          changes|=selectItemRange(getItemBelow(anchoritem),item);
          }
        else{
          changes|=deselectItemRange(getItemBelow(anchoritem),item);
          }
        }
      }
    }
  return changes;
  }
  
  
// Get item at position x,y (NULL if none)
FXTreeItem* FXTreeList::getItemAt(FXint x,FXint y) const {
  FXTreeItem* n=firstitem;
  FXint yy=0;
  y=y-pos_y;
  x=x-pos_x;
  while(n){
    if(y<yy) return NULL;
    yy+=itemHeight;
    if(y<yy){
      if(x<n->x-indent) return NULL;
      if(n->x+iconWidth+TEXT_SPACING+SIDE_SPACING+font->getTextWidth(n->label.text(),n->label.length())<x) return NULL;
      return n;
      }
    if(n->first && (n->state&TREEITEM_EXPANDED)){n=n->first;continue;}
    while(!n->next && n->parent){n=n->parent;}
    n=n->next;
    }
  return NULL;
  }


// Make item fully visible
void FXTreeList::makeItemVisible(FXTreeItem* item){
  FXint x,y,w,h;
  if(item){
    x=pos_x;
    y=pos_y;

    w=iconWidth+font->getTextWidth(item->label.text(),item->label.length())+4;
    h=itemHeight;

    //if(viewport_w<=x+((FXTreeItem*)item)->x+w) x=viewport_w-((FXTreeItem*)item)->x-w;
    //if(x+((FXTreeItem*)item)->x<=0) x=-((FXTreeItem*)item)->x; 

    if(viewport_h<=y+item->y+h) y=viewport_h-item->y-h;
    if(y+item->y<=0) y=-item->y; 

    setPosition(x,y);
    }
  }


// Add item as first one under parent p
FXTreeItem* FXTreeList::addItemFirst(FXTreeItem* p,const char* text,FXIcon* oi,FXIcon* ci,void* ptr){
  FXTreeItem *item=createItem();
  if(p){
    item->prev=NULL;
    item->next=p->first;
    if(item->next) item->next->prev=item; else p->last=item;
    p->first=item;
    }
  else{
    item->prev=NULL;
    item->next=firstitem;
    if(item->next) item->next->prev=item; else lastitem=item;
    firstitem=item;
    }
  item->parent=p;
  item->first=NULL;
  item->last=NULL;
  item->label=text;
  item->openIcon=oi;
  item->closedIcon=ci;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Add item as last one under parent p
FXTreeItem* FXTreeList::addItemLast(FXTreeItem* p,const char* text,FXIcon* oi,FXIcon* ci,void* ptr){
  FXTreeItem *item=createItem();
  if(p){
    item->prev=p->last;
    item->next=NULL;
    if(item->prev) item->prev->next=item; else p->first=item;
    p->last=item;
    }
  else{
    item->prev=lastitem;
    item->next=NULL;
    if(item->prev) item->prev->next=item; else firstitem=item;
    lastitem=item;
    }
  item->parent=p;
  item->first=NULL;
  item->last=NULL;
  item->label=text;
  item->openIcon=oi;
  item->closedIcon=ci;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Link item after other
FXTreeItem* FXTreeList::addItemAfter(FXTreeItem* other,const char* text,FXIcon* oi,FXIcon* ci,void* ptr){
  if(other==NULL){ fxerror("%s::addItemAfter: other item is NULL.\n",getClassName()); }
  FXTreeItem *item=createItem();
  item->prev=other;
  item->next=other->next;
  other->next=item;
  if(item->next) item->next->prev=item; else if(other->parent) other->parent->last=item; else lastitem=item; 
  item->parent=other->parent;
  item->first=NULL;
  item->last=NULL;
  item->label=text;
  item->openIcon=oi;
  item->closedIcon=ci;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Link item before other 
FXTreeItem* FXTreeList::addItemBefore(FXTreeItem* other,const char* text,FXIcon* oi,FXIcon* ci,void* ptr){
  if(other==NULL){ fxerror("%s::addItemBefore: other item is NULL.\n",getClassName()); }
  FXTreeItem *item=createItem();
  item->next=other;
  item->prev=other->prev;
  other->prev=item;
  if(item->prev) item->prev->next=item; else if(other->parent) other->parent->first=item; else firstitem=item; 
  item->parent=other->parent;
  item->first=NULL;
  item->last=NULL;
  item->label=text;
  item->openIcon=oi;
  item->closedIcon=ci;
  item->state=0;
  item->data=ptr;
  recalc();
  return item;
  }


// Remove node from list
void FXTreeList::removeItem(FXTreeItem* item){
  if(item){
    if(item->prev) item->prev->next=item->next; else if(item->parent) item->parent->first=item->next; else firstitem=item->next;
    if(item->next) item->next->prev=item->prev; else if(item->parent) item->parent->last=item->prev; else lastitem=item->prev;
    if(currentitem==item) currentitem=NULL;
    if(anchoritem==item) anchoritem=NULL;
    removeItems(item->first,item->last);
    delete item;
    recalc();
    }
  }


// Remove all siblings from [fm,to]
void FXTreeList::removeItems(FXTreeItem* fm,FXTreeItem* to){
  if(fm && to){
    FXTreeItem *item;
    if(fm->prev) fm->prev->next=to->next; else if(fm->parent) fm->parent->first=to->next; else firstitem=to->next;
    if(to->next) to->next->prev=fm->prev; else if(to->parent) to->parent->last=fm->prev; else lastitem=fm->prev;
    do{
      item=fm;
      if(currentitem==item) currentitem=NULL;
      if(anchoritem==item) anchoritem=NULL;
      removeItems(item->first,item->last);
      fm=fm->next;
      delete item;
      }
    while(item!=to);
    recalc();
    }
  }


// Remove all items
void FXTreeList::removeAllItems(){
  removeItems(firstitem,lastitem);
  }


// Set current item
void FXTreeList::setCurrentItem(FXTreeItem* item){
  if(item!=currentitem){ 
    if(item){
      item->state|=TREEITEM_CURRENT;
      updateItem(item);
      }
    if(currentitem){
      currentitem->state&=~TREEITEM_CURRENT;
      updateItem(currentitem);
      }
    currentitem=item;
    }
  }


// Set anchor item
void FXTreeList::setAnchorItem(FXTreeItem* item){
  anchoritem=item;
  }


// Check item is open
FXbool FXTreeList::isItemOpen(const FXTreeItem* item) const { 
  if(!item){ fxerror("%s::isItemOpen: item is NULL.\n",getClassName()); } 
  return (item->state&TREEITEM_OPEN)!=0; 
  }


// Open item
FXbool FXTreeList::openItem(FXTreeItem* item){
  if(item==NULL){ fxerror("%s::openItem: item is NULL.\n",getClassName()); }
  if(!(item->state&TREEITEM_OPEN)){
    item->state|=TREEITEM_OPEN;
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }  
  
 
// Close item
FXbool FXTreeList::closeItem(FXTreeItem* item){
  if(item==NULL){ fxerror("%s::closeItem: item is NULL.\n",getClassName()); }
  if(item->state&TREEITEM_OPEN){
    item->state&=~TREEITEM_OPEN;
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }  


// Collapse all subtrees under item
FXbool FXTreeList::collapseTree(FXTreeItem* tree){
  if(tree==NULL){ fxerror("%s::collapseTree: tree is NULL.\n",getClassName()); }
  if(tree->state&TREEITEM_EXPANDED){
    tree->state&=~TREEITEM_EXPANDED;
    if(tree->first){
      recalc();
      }
    else{
      updateItem(tree);
      }
    return TRUE;
    }
  return FALSE;
  }

 
// Expand subtree under item
FXbool FXTreeList::expandTree(FXTreeItem* tree){
  if(tree==NULL){ fxerror("%s::expandTree: tree is NULL.\n",getClassName()); }
  if(!(tree->state&TREEITEM_EXPANDED)){
    tree->state|=TREEITEM_EXPANDED;
    if(tree->first){
      recalc();
      }
    else{
      updateItem(tree);
      }
    return TRUE;
    }
  return FALSE;
  }



void FXTreeList::setItemText(FXTreeItem* item,const char* text){
  if(item==NULL){ fxerror("%s::setItemText: item is NULL.\n",getClassName()); }
  item->label=text;
  recalc();
  }


const char* FXTreeList::getItemText(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemText: item is NULL.\n",getClassName()); }
  return item->label.text();
  }


void FXTreeList::setItemOpenIcon(FXTreeItem* item,FXIcon* icon){
  if(item==NULL){ fxerror("%s::setItemOpenIcon: item is NULL.\n",getClassName()); }
  item->openIcon=icon;
  updateItem(item);
  }


FXIcon* FXTreeList::getItemOpenIcon(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemOpenIcon: item is NULL.\n",getClassName()); }
  return item->openIcon;
  }


void FXTreeList::setItemClosedIcon(FXTreeItem* item,FXIcon* icon){
  if(item==NULL){ fxerror("%s::setItemClosedIcon: item is NULL.\n",getClassName()); }
  item->closedIcon=icon;
  updateItem(item);
  }


FXIcon* FXTreeList::getItemClosedIcon(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemClosedIcon: item is NULL.\n",getClassName()); }
  return item->closedIcon;
  }


void FXTreeList::setItemData(FXTreeItem* item,void* ptr) const {
  if(item==NULL){ fxerror("%s::setItemData: item is NULL.\n",getClassName()); }
  item->data=ptr;
  }


void* FXTreeList::getItemData(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemData: item is NULL.\n",getClassName()); }
  return item->data;
  }


// Key Press
long FXTreeList::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem *item,*tmp;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
//fprintf(stderr,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state);
  if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
  switch(event->code){
    case KEY_Up:
    case KEY_Down:
    case KEY_Right:
    case KEY_Left:
    case KEY_Home:
    case KEY_End:
      item=currentitem;
      switch(event->code){
        case KEY_Up:                      // Move up
          if(!item){ 
            item=lastitem;
            }
          else if(item->prev){
            item=item->prev; 
            while(item->first && (item->state&TREEITEM_EXPANDED)) item=item->last;
            }
          else if(item->parent){ 
            item=item->parent;
            }
          break;
        case KEY_Down:                    // Move down
          if(!item){
            item=firstitem;
            }
          else if(item->first && (item->state&TREEITEM_EXPANDED)){
            item=item->first;
            }
          else if(item->next){
            item=item->next;
            }
          else{
            tmp=item;
            while(item->parent && !item->parent->next) item=item->parent;
            if(item->parent && item->parent->next)
              item=item->parent->next;
            else
              item=tmp;
            }
          break;
        case KEY_Right:                   // Move right/down and open subtree
          if(!item){
            item=firstitem;
            }
          else if(item->first && !isItemExpanded(item)){
            handle(this,MKUINT(0,SEL_EXPANDED),item);
            }
          else if(item->first){
            item=item->first;
            }
          else if(item->next){
            item=item->next;
            }
          else{
            tmp=item;
            while(item->parent && !item->parent->next) item=item->parent;
            if(item->parent && item->parent->next)
              item=item->parent->next;
            else
              item=tmp;
            }
          break;
        case KEY_Left:                    // Move left/up and close subtree
          if(!item){
            item=firstitem;
            }
          else if(item->first && isItemExpanded(item)){
            handle(this,MKUINT(0,SEL_COLLAPSED),item);
            }
          else if(item->parent){
            item=item->parent;
            }
          else if(item->prev){
            item=item->prev;
            }
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
        if((options&TREELIST_BROWSESELECT) && !(options&TREELIST_SINGLESELECT)){
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
long FXTreeList::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
//fprintf(stderr,"%s::onKeyRelease keysym=0x%04x\n",getClassName(),event->code);
  switch(event->code){
    case KEY_Up:
    case KEY_Down:
    case KEY_Right:
    case KEY_Left:
    case KEY_Home:
    case KEY_End:
      if(flags&FLAG_KEY){
        if((options&TREELIST_BROWSESELECT) && !(options&TREELIST_SINGLESELECT)){
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



// Scroll timer
long FXTreeList::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXScrollArea::onAutoScroll(sender,sel,ptr);
//   FXTreeItem *item;
//   // Find item
//   item=getItemAt(x,y);
//   if(item && item!=currentitem){
//     
//     // Always one selected
//     if(options&LIST_ALWAYS_ONE){
//       deselectItemRange(firstitem,item->prev);
//       deselectItemRange(item->next,lastitem);
//       selectItem(item);
//       }
// 
//     // At most one selected
//     else if(options&LIST_SINGLESELECT){
//       if(currentitem && isItemSelected(currentitem)){
//         deselectItemRange(firstitem,item->prev);
//         deselectItemRange(item->next,lastitem);
//         selectItem(item);
//         }
//       else{
//         deselectItemRange(firstitem,lastitem);
//         }
//       }
// 
//     // Any number selected
//     else{
//       extendSelection(item);
//       }
//     
//     // Current item has changed
//     setCurrentItem(item);
//     if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)item);
//     }
  return 1;
  }


// Mouse motion
long FXTreeList::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(flags&FLAG_PRESSED){
    
    // Start auto scrolling?
    if(startAutoScroll(event->win_x,event->win_y,FALSE)) return 1;
//   if(event->moved && !event->state){
//     itemunder=getItemAt(event->win_x,event->win_y);
//     if(itemunder) flags|=FLAG_TIP;
//     }
    return 1;
    }
  return 0;
  }


// Pressed a button
long FXTreeList::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem *item;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    if(options&TREELIST_AUTOSELECT) return 1;///////// FIX
    item=getItemAt(event->win_x,event->win_y);
    if(item && hitItemBox(item,event->win_x,event->win_y)){
      if(isItemExpanded(item))
        handle(this,MKUINT(0,SEL_COLLAPSED),item);
      else
        handle(this,MKUINT(0,SEL_EXPANDED),item);
      return 1;
      }
    if(item!=currentitem) handle(this,MKUINT(0,SEL_CHANGED),(void*)item);
    handle(this,MKUINT(0,SEL_ACTIVATE),ptr);
    flags&=~FLAG_UPDATE;
    flags|=FLAG_PRESSED;
    return 1;
    }
  return 0;
  }


// Released button
long FXTreeList::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuint pressed=flags&FLAG_PRESSED;
  if(isEnabled()){
    flags|=FLAG_UPDATE;
    flags&=~FLAG_PRESSED;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(pressed) handle(this,MKUINT(0,SEL_DEACTIVATE),ptr);
    return 1;
    }
  return 0;
  }


// Button or Key activate
long FXTreeList::onActivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXTreeItem *item=firstitem;
  
  // Remember previous state
  while(item){
    if(item->state&TREEITEM_SELECTED)
      item->state|=TREEITEM_HISTORY;
    else
      item->state&=~TREEITEM_HISTORY;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }

  // Have currentitem
  if(currentitem){
    
    // Multiple selection mode
    if((options&TREELIST_SINGLESELECT)&&(options&TREELIST_BROWSESELECT)){
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
    else if(options&TREELIST_BROWSESELECT){
      deselectItemRange(firstitem,getItemAbove(currentitem));
      deselectItemRange(getItemBelow(currentitem),NULL);
      selectItem(currentitem);
      setAnchorItem(currentitem);
      }

    // Single selection mode
    else if(options&TREELIST_SINGLESELECT){
      if(isItemSelected(currentitem)){
        deselectItemRange(firstitem,NULL);
        }
      else{
        deselectItemRange(firstitem,getItemAbove(currentitem));
        deselectItemRange(getItemBelow(currentitem),NULL);
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
      deselectItemRange(firstitem,getItemAbove(currentitem));
      deselectItemRange(getItemBelow(currentitem),NULL);
      selectItem(currentitem);
      markItems();
      setAnchorItem(currentitem);
      }
    }
  
  // No currentitem & not multiple or browse select mode
  else if(!(options&TREELIST_BROWSESELECT)){
    
    // If not add or toggle mode, turn them off
    if(!(event->state&(SHIFTMASK|CONTROLMASK))){
      deselectItemRange(firstitem,NULL);
      }
    setAnchorItem(currentitem);
    }
  
  // If we want selection, see wether to acquire or release it
  if(options&TREELIST_WANTSELECTION){
    if(getNumSelectedItems())
      acquireSelection();
    else
      releaseSelection();
    }
  return 1;
  }
  

// Button or Key deactivate
long FXTreeList::onDeactivate(FXObject*,FXSelector,void* ptr){
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
long FXTreeList::onChanged(FXObject*,FXSelector,void* ptr){
  FXTreeItem *olditem=currentitem;
  makeItemVisible((FXTreeItem*)ptr);
  setCurrentItem((FXTreeItem*)ptr);
  if(target && target->handle(this,MKUINT(message,SEL_CHANGED),ptr)) return 1;
  if(olditem){
    handle(this,MKUINT(0,SEL_CLOSED),olditem);
    }
  if(currentitem){
    handle(this,MKUINT(0,SEL_OPENED),currentitem);
    }
  return 1;
  }


// Selected items
long FXTreeList::onSelected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
  }


// Deselected items
long FXTreeList::onDeselected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
  }


// Clicked in list
long FXTreeList::onClicked(FXObject*,FXSelector,void* ptr){
  register FXTreeItem **selectedlist,**deselectedlist,*item;
  register FXint numselected,numdeselected,i,j;
  
  // Notify target of the click
  if(target){
    
    // Clicked message indicates a click anywhere in the widget
    if(target->handle(this,MKUINT(message,SEL_CLICKED),ptr)) return 1;
  
    // Command message indicates click on an item
    if(ptr && target->handle(this,MKUINT(message,SEL_COMMAND),ptr)) return 1;
    }

  // Find out number of items whose selection state changed
  item=firstitem; numselected=numdeselected=0;
  while(item){
    if((item->state&TREEITEM_SELECTED) && !(item->state&TREEITEM_HISTORY)) numselected++;
    if(!(item->state&TREEITEM_SELECTED) && (item->state&TREEITEM_HISTORY)) numdeselected++;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }

  // Make some room
  FXMALLOC(&selectedlist,FXTreeItem*,numselected+1);
  FXMALLOC(&deselectedlist,FXTreeItem*,numdeselected+1);

  // Add items to the proper lists
  item=firstitem; i=j=0;
  while(item){
    if((item->state&TREEITEM_SELECTED) && !(item->state&TREEITEM_HISTORY)) selectedlist[i++]=item;
    if(!(item->state&TREEITEM_SELECTED) && (item->state&TREEITEM_HISTORY)) deselectedlist[j++]=item;
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
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
long FXTreeList::onDoubleClicked(FXObject*,FXSelector,void* ptr){
  
  // Double click anywhere in the widget
  if(target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr)) return 1;
  
  // Double click on an item
  if(ptr){
    
    // Expand the item 
    if(isItemExpanded((FXTreeItem*)ptr)){
      if(handle(this,MKUINT(0,SEL_COLLAPSED),ptr)) return 1;
      }
    else{
      if(handle(this,MKUINT(0,SEL_EXPANDED),ptr)) return 1;
      }
    }
  return 0;
  }


// Triple clicked in list; ptr may or may not point to an item
long FXTreeList::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }


// Item opened
long FXTreeList::onItemOpened(FXObject*,FXSelector,void* ptr){
  openItem((FXTreeItem*)ptr);
  return target && target->handle(this,MKUINT(message,SEL_OPENED),ptr);
  }


// Item closed
long FXTreeList::onItemClosed(FXObject*,FXSelector,void* ptr){
  closeItem((FXTreeItem*)ptr);
  return target && target->handle(this,MKUINT(message,SEL_CLOSED),ptr);
  }


// Item expanded
long FXTreeList::onItemExpanded(FXObject*,FXSelector,void* ptr){
  expandTree((FXTreeItem*)ptr);
  return target && target->handle(this,MKUINT(message,SEL_EXPANDED),ptr);
  }


// Item collapsed
long FXTreeList::onItemCollapsed(FXObject*,FXSelector,void* ptr){
  collapseTree((FXTreeItem*)ptr);
  return target && target->handle(this,MKUINT(message,SEL_COLLAPSED),ptr);
  }


// Change the font
void FXTreeList::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  font=fnt;
  recalc();
  }


// Change help text
void FXTreeList::setHelpText(const FXchar* text){
  help=text;
  }


// Change tip text
void FXTreeList::setTipText(const FXchar* text){
  tip=text;
  }


// Set text color
void FXTreeList::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Set select background color
void FXTreeList::setSelBackColor(FXPixel clr){
  selbackColor=clr;
  update(0,0,width,height);
  }


// Set selected text color
void FXTreeList::setSelTextColor(FXPixel clr){
  seltextColor=clr;
  update(0,0,width,height);
  }


// Set icon width
void FXTreeList::setIconWidth(FXint w){
  if(iconWidth!=w){
    iconWidth=w;
    recalc();
    }
  }


// Set icon height
void FXTreeList::setIconHeight(FXint h){
  if(iconHeight!=h){
    iconHeight=h;
    recalc();
    }
  }


// Set parent to child indent amount
void FXTreeList::setIndent(FXint in){
  if(indent!=in){
    indent=in;
    recalc();
    }
  }


// Change list style
void FXTreeList::setListStyle(FXuint style){
  FXuint opts=(options&~TREELIST_MASK) | (style&TREELIST_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Get list style
FXuint FXTreeList::getListStyle() const { 
  return (options&TREELIST_MASK); 
  }


// Cleanup
FXTreeList::~FXTreeList(){
  removeAllItems();
  firstitem=(FXTreeItem*)-1;
  lastitem=(FXTreeItem*)-1;
  anchoritem=(FXTreeItem*)-1;
  currentitem=(FXTreeItem*)-1;
  font=(FXFont*)-1;
  }


