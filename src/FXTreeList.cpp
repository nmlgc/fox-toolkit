/********************************************************************************
*                                                                               *
*                          T r e e L i s t   O b j e c t                        *
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
* $Id: FXTreeList.cpp,v 1.85.4.3 2003/03/03 16:12:41 fox Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXIcon.h"
#include "FXScrollbar.h"
#include "FXTreeList.h"


/*
  Notes:
  - Tooltip should pop up exactly on top of current item.
  - Clicking on + does not make it current.
  - Need translate right-clicks into message with item figured out...
  - In autoselect mode, all items are expanded.
  - Sortfunc's will be hard to serialize.
  - Perhaps simplify things by having fixed root item embedded inside,
    which is not visible (i.e. no icon or text); just an idea at this stage.
  - It may be convenient to have ways to move items around.
  - Need insertSorted() API to add item in the right place based on current
    sort function.
*/


#define ICON_SPACING        4         // Spacing between parent and child in x direction
#define TEXT_SPACING        4         // Spacing between icon and text
#define SIDE_SPACING        4         // Spacing between side and item

#define DEFAULT_INDENT      8         // Indent between parent and child


#define SELECT_MASK     (TREELIST_SINGLESELECT|TREELIST_BROWSESELECT)
#define TREELIST_MASK   (SELECT_MASK|TREELIST_AUTOSELECT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_ROOT_BOXES)

/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXTreeItem,FXObject,NULL,0)



// Draw item
void FXTreeItem::draw(const FXTreeList* list,FXDC& dc,FXint x,FXint y,FXint,FXint h) const {
  register FXFont *font=list->getFont();
  register FXIcon *icon=(state&OPENED)?openIcon:closedIcon;
  register FXint th=0,ih=0,tw,len;
  if(icon) ih=icon->getHeight();
  if(!label.empty()) th=4+font->getFontHeight();
  x+=SIDE_SPACING/2;
  if(icon){
    dc.drawIcon(icon,x,y+(h-ih)/2);
    x+=ICON_SPACING+icon->getWidth();
    }
  if(!label.empty()){
    len=label.length();
    tw=4+font->getTextWidth(label.text(),len);
    y+=(h-th)/2;
    if(isSelected()){
      dc.setForeground(list->getSelBackColor());
      dc.fillRectangle(x,y,tw,th);
      if(!isEnabled())
        dc.setForeground(makeShadowColor(list->getBackColor()));
      else
        dc.setForeground(list->getSelTextColor());
      }
    else{
      if(!isEnabled())
        dc.setForeground(makeShadowColor(list->getBackColor()));
      else
        dc.setForeground(list->getTextColor());
      }
    dc.drawText(x+2,y+font->getFontAscent()+2,label.text(),len);
    if(hasFocus()){
      dc.drawFocusRectangle(x+1,y+1,tw-2,th-2);
      }
    }
  }


// See if item got hit, and where:- 1 is icon, 2 is text
FXint FXTreeItem::hitItem(const FXTreeList* list,FXint x,FXint y) const {
  register FXint oiw=0,ciw=0,oih=0,cih=0,tw=0,th=0,iw,ih,ix,iy,tx,ty,h;
  register FXFont *font=list->getFont();
  if(openIcon){
    oiw=openIcon->getWidth();
    oih=openIcon->getHeight();
    }
  if(closedIcon){
    ciw=closedIcon->getWidth();
    cih=closedIcon->getHeight();
    }
  if(!label.empty()){
    tw=4+font->getTextWidth(label.text(),label.length());
    th=4+font->getFontHeight();
    }
  iw=FXMAX(oiw,ciw);
  ih=FXMAX(oih,cih);
  h=FXMAX(th,ih);
  ix=SIDE_SPACING/2;
  tx=SIDE_SPACING/2;
  if(iw) tx+=iw+ICON_SPACING;
  iy=(h-ih)/2;
  ty=(h-th)/2;

  // In icon?
  if(ix<=x && iy<=y && x<ix+iw && y<iy+ih) return 1;

  // In text?
  if(tx<=x && ty<=y && x<tx+tw && y<ty+th) return 2;

  // Outside
  return 0;
  }


// Set or kill focus
void FXTreeItem::setFocus(FXbool focus){
  if(focus) state|=FOCUS; else state&=~FOCUS;
  }

// Select or deselect item
void FXTreeItem::setSelected(FXbool selected){
  if(selected) state|=SELECTED; else state&=~SELECTED;
  }

// Set item opened
void FXTreeItem::setOpened(FXbool opened){
  if(opened) state|=OPENED; else state&=~OPENED;
  }

// Set item expanded
void FXTreeItem::setExpanded(FXbool expanded){
  if(expanded) state|=EXPANDED; else state&=~EXPANDED;
  }

// Enable or disable the item
void FXTreeItem::setEnabled(FXbool enabled){
  if(enabled) state&=~DISABLED; else state|=DISABLED;
  }

// Icon is draggable
void FXTreeItem::setDraggable(FXbool draggable){
  if(draggable) state|=DRAGGABLE; else state&=~DRAGGABLE;
  }

// Icons owner by item
void FXTreeItem::setIconOwned(FXuint owned){
  state=(state&~(OPENICONOWNED|CLOSEDICONOWNED))|(owned&(OPENICONOWNED|CLOSEDICONOWNED));
  }


// Create icon
void FXTreeItem::create(){
  if(openIcon) openIcon->create();
  if(closedIcon) closedIcon->create();
  }


// Destroy icon
void FXTreeItem::destroy(){
  if((state&OPENICONOWNED) && openIcon) openIcon->destroy();
  if((state&CLOSEDICONOWNED) && closedIcon) closedIcon->destroy();
  }


// Detach from icon resource
void FXTreeItem::detach(){
  if(openIcon) openIcon->detach();
  if(closedIcon) closedIcon->detach();
  }


// Get number of child items
FXint FXTreeItem::getNumChildren() const {
  register FXTreeItem *item=first;
  register FXint n=0;
  while(item){item=item->next;n++;}
  return n;
  }



// Get item (logically) below this one
FXTreeItem* FXTreeItem::getBelow() const {
  register FXTreeItem* item=(FXTreeItem*)this;
  if(first) return first;
  while(!item->next && item->parent) item=item->parent;
  return item->next;
  }


// Get item (logically) above this one
FXTreeItem* FXTreeItem::getAbove() const {
  register FXTreeItem* item=prev;
  if(!item) return parent;
  while(item->last) item=item->last;
  return item;
  }


// Get item width
FXint FXTreeItem::getWidth(const FXTreeList* list) const {
  register FXint w=0,oiw=0,ciw=0;
  if(openIcon)   oiw=openIcon->getWidth();
  if(closedIcon) ciw=closedIcon->getWidth();
  w=FXMAX(oiw,ciw);
  if(!label.empty()){
    if(w) w+=ICON_SPACING;
    w+=4+list->getFont()->getTextWidth(label.text(),label.length());
    }
  return SIDE_SPACING+w;
  }


// Get item height
FXint FXTreeItem::getHeight(const FXTreeList* list) const {
  register FXint th=0,oih=0,cih=0;
  if(openIcon)   oih=openIcon->getHeight();
  if(closedIcon) cih=closedIcon->getHeight();
  if(!label.empty()) th=4+list->getFont()->getFontHeight();
  return FXMAX3(th,oih,cih);
  }


// Save data
void FXTreeItem::save(FXStream& store) const {
  FXObject::save(store);
  store << prev;
  store << next;
  store << parent;
  store << first;
  store << last;
  store << label;
  store << openIcon;
  store << closedIcon;
  store << state;
  }


// Load data
void FXTreeItem::load(FXStream& store){
  FXObject::load(store);
  store >> prev;
  store >> next;
  store >> parent;
  store >> first;
  store >> last;
  store >> label;
  store >> openIcon;
  store >> closedIcon;
  store >> state;
  }


// Delete icons if owned
FXTreeItem::~FXTreeItem(){
  if(state&OPENICONOWNED) delete openIcon;
  if(state&CLOSEDICONOWNED) delete closedIcon;
  }

/*******************************************************************************/

// Map
FXDEFMAP(FXTreeList) FXTreeListMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTreeList::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXTreeList::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXTreeList::onAutoScroll),
  FXMAPFUNC(SEL_TIMEOUT,FXTreeList::ID_TIPTIMER,FXTreeList::onTipTimer),
  FXMAPFUNC(SEL_TIMEOUT,FXTreeList::ID_LOOKUPTIMER,FXTreeList::onLookupTimer),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTreeList::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTreeList::onLeftBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXTreeList::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXTreeList::onRightBtnRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXTreeList::onUngrabbed),
  FXMAPFUNC(SEL_KEYPRESS,0,FXTreeList::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXTreeList::onKeyRelease),
  FXMAPFUNC(SEL_ENTER,0,FXTreeList::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXTreeList::onLeave),
  FXMAPFUNC(SEL_FOCUSIN,0,FXTreeList::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXTreeList::onFocusOut),
  FXMAPFUNC(SEL_SELECTED,0,FXTreeList::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXTreeList::onDeselected),
  FXMAPFUNC(SEL_OPENED,0,FXTreeList::onOpened),
  FXMAPFUNC(SEL_CLOSED,0,FXTreeList::onClosed),
  FXMAPFUNC(SEL_EXPANDED,0,FXTreeList::onExpanded),
  FXMAPFUNC(SEL_COLLAPSED,0,FXTreeList::onCollapsed),
  FXMAPFUNC(SEL_CLICKED,0,FXTreeList::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXTreeList::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXTreeList::onTripleClicked),
  FXMAPFUNC(SEL_COMMAND,0,FXTreeList::onCommand),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXTreeList::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXTreeList::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXTreeList,FXScrollArea,FXTreeListMap,ARRAYNUMBER(FXTreeListMap))


/*******************************************************************************/


// Tree List
FXTreeList::FXTreeList(){
  flags|=FLAG_ENABLED;
  firstitem=NULL;
  lastitem=NULL;
  anchoritem=NULL;
  currentitem=NULL;
  extentitem=NULL;
  cursoritem=NULL;
  font=(FXFont*)-1;
  sortfunc=NULL;
  textColor=0;
  selbackColor=0;
  seltextColor=0;
  lineColor=0;
  treeWidth=0;
  treeHeight=0;
  visible=0;
  indent=DEFAULT_INDENT;
  grabx=0;
  graby=0;
  timer=NULL;
  lookuptimer=NULL;
  state=FALSE;
  }


// Tree List
FXTreeList::FXTreeList(FXComposite *p,FXint nvis,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  firstitem=NULL;
  lastitem=NULL;
  anchoritem=NULL;
  currentitem=NULL;
  extentitem=NULL;
  cursoritem=NULL;
  font=getApp()->getNormalFont();
  sortfunc=NULL;
  textColor=getApp()->getForeColor();
  selbackColor=getApp()->getSelbackColor();
  seltextColor=getApp()->getSelforeColor();
  lineColor=getApp()->getShadowColor();
  treeWidth=0;
  treeHeight=0;
  visible=FXMAX(nvis,0);
  indent=DEFAULT_INDENT;
  grabx=0;
  graby=0;
  timer=NULL;
  lookuptimer=NULL;
  state=FALSE;
  }


// Create window
void FXTreeList::create(){
  register FXTreeItem *item=firstitem;
  FXScrollArea::create();
  while(item){
    item->create();
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  font->create();
  }


// Detach window
void FXTreeList::detach(){
  register FXTreeItem *item=firstitem;
  FXScrollArea::detach();
  while(item){
    item->detach();
    if(item->first){item=item->first;continue;}
    while(!item->next && item->parent){item=item->parent;}
    item=item->next;
    }
  font->detach();
  }


// Can have focus
FXbool FXTreeList::canFocus() const { return TRUE; }


// Into focus chain
void FXTreeList::setFocus(){
  FXScrollArea::setFocus();
  setDefault(TRUE);
  }


// Out of focus chain
void FXTreeList::killFocus(){
  FXScrollArea::killFocus();
  setDefault(MAYBE);
  }


// Get default width
FXint FXTreeList::getDefaultWidth(){
  return FXScrollArea::getDefaultWidth();
  }


// Get default height
FXint FXTreeList::getDefaultHeight(){
  if(visible) return visible*(4+font->getFontHeight());
  return FXScrollArea::getDefaultHeight();
  }


// Propagate size change
void FXTreeList::recalc(){
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  cursoritem=NULL;
  }


// List is multiple of nitems
void FXTreeList::setNumVisible(FXint nvis){
  if(nvis<0) nvis=0;
  if(visible!=nvis){
    visible=nvis;
    recalc();
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


// Recompute interior
void FXTreeList::recompute(){
  register FXTreeItem* item;
  register FXint x,y,w,h;
  x=y=0;
  treeWidth=0;
  treeHeight=0;
  item=firstitem;
  if(options&TREELIST_ROOT_BOXES) x+=(4+indent);
  while(item){
    item->x=x;
    item->y=y;
    w=item->getWidth(this);
    h=item->getHeight(this);
    if(x+w>treeWidth) treeWidth=x+w;
    y+=h;
    if(item->first && ((options&TREELIST_AUTOSELECT) || item->isExpanded())){
      x+=(indent+h/2);
      item=item->first;
      continue;
      }
    while(!item->next && item->parent){
      item=item->parent;
      x-=(indent+item->getHeight(this)/2);
      }
    item=item->next;
    }
  treeHeight=y;
  flags&=~FLAG_RECALC;
  }


// Determine content width of tree list
FXint FXTreeList::getContentWidth(){
  if(flags&FLAG_RECALC) recompute();
  return treeWidth;
  }


// Determine content height of tree list
FXint FXTreeList::getContentHeight(){
  if(flags&FLAG_RECALC) recompute();
  return treeHeight;
  }


// Recalculate layout
void FXTreeList::layout(){

  // Repaint when content size changed
  //if(flags&FLAG_RECALC) update();

  // Calculate contents
  FXScrollArea::layout();

  // Set line size based on item size
  if(firstitem){
    vertical->setLine(firstitem->getHeight(this));
    horizontal->setLine(firstitem->getWidth(this)/10);
    }

  // Force repaint
  update();

  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Set item text
void FXTreeList::setItemText(FXTreeItem* item,const FXString& text){
  if(item==NULL){ fxerror("%s::setItemText: item is NULL.\n",getClassName()); }
  item->setText(text);
  recalc();
  }


// Get item text
FXString FXTreeList::getItemText(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemText: item is NULL.\n",getClassName()); }
  return item->getText();
  }


// Set item open icon
void FXTreeList::setItemOpenIcon(FXTreeItem* item,FXIcon* icon){
  if(item==NULL){ fxerror("%s::setItemOpenIcon: item is NULL.\n",getClassName()); }
  item->setOpenIcon(icon);
  recalc();
  }


// Get item open icon
FXIcon* FXTreeList::getItemOpenIcon(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemOpenIcon: item is NULL.\n",getClassName()); }
  return item->getOpenIcon();
  }


// Set item closed icon
void FXTreeList::setItemClosedIcon(FXTreeItem* item,FXIcon* icon){
  if(item==NULL){ fxerror("%s::setItemClosedIcon: item is NULL.\n",getClassName()); }
  item->setClosedIcon(icon);
  recalc();
  }


// Get item closed icon
FXIcon* FXTreeList::getItemClosedIcon(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemClosedIcon: item is NULL.\n",getClassName()); }
  return item->getClosedIcon();
  }


// Set item data
void FXTreeList::setItemData(FXTreeItem* item,void* ptr) const {
  if(item==NULL){ fxerror("%s::setItemData: item is NULL.\n",getClassName()); }
  item->setData(ptr);
  }


// Get item data
void* FXTreeList::getItemData(const FXTreeItem* item) const {
  if(item==NULL){ fxerror("%s::getItemData: item is NULL.\n",getClassName()); }
  return item->getData();
  }


// True if item is selected
FXbool FXTreeList::isItemSelected(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemSelected: item is NULL.\n",getClassName()); }
  return item->isSelected();
  }


// True if item is current
FXbool FXTreeList::isItemCurrent(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemCurrent: item is NULL.\n",getClassName()); }
  return currentitem==item;
  }


// Check if item is expanded
FXbool FXTreeList::isItemExpanded(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemExpanded: item is NULL.\n",getClassName()); }
  return (options&TREELIST_AUTOSELECT) || item->isExpanded();
  }


// Is item a leaf item
FXbool FXTreeList::isItemLeaf(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemLeaf: item is NULL.\n",getClassName()); }
  return item->first==NULL;
  }


// Check if item is enabled
FXbool FXTreeList::isItemEnabled(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemEnabled: item is NULL.\n",getClassName()); }
  return item->isEnabled();
  }


// Check item is open
FXbool FXTreeList::isItemOpened(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemOpen: item is NULL.\n",getClassName()); }
  return item->isOpened();
  }


// True if item (partially) visible
FXbool FXTreeList::isItemVisible(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::isItemVisible: item is NULL.\n",getClassName()); }
  return 0<(pos_y+item->y+item->getHeight(this)) && (pos_y+item->y)<viewport_h;
  }


// Make item fully visible
void FXTreeList::makeItemVisible(FXTreeItem* item){
  FXint x,y,w,h;
  if(item){

    // Expand parents of this node
    if(!(options&TREELIST_AUTOSELECT)){
      FXTreeItem *par=item->parent;
      FXbool expanded=FALSE;
      while(par){
        if(!par->isExpanded()){
          par->setExpanded(TRUE);
          expanded=TRUE;
          }
        par=par->parent;
        }

      // If any nodes have expanded that weren't previously, recompute list size
      if(expanded){
        recalc();
        if(xid) layout();
        }
      }

    // Now we adjust the scrolled position to fit everything
    if(xid){
      x=pos_x;
      y=pos_y;

      w=item->getWidth(this);
      h=item->getHeight(this);

      if(viewport_w<=x+item->x+w) x=viewport_w-item->x-w;
      if(x+item->x<=0) x=-item->x;

      if(viewport_h<=y+item->y+h) y=viewport_h-item->y-h;
      if(y+item->y<=0) y=-item->y;

      setPosition(x,y);
      }
    }
  }


// Return item width
FXint FXTreeList::getItemWidth(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::getItemWidth: item is NULL.\n",getClassName()); }
  return item->getWidth(this);
  }


// Return item height
FXint FXTreeList::getItemHeight(const FXTreeItem* item) const {
  if(!item){ fxerror("%s::getItemHeight: item is NULL.\n",getClassName()); }
  return item->getHeight(this);
  }


// Get item at position x,y
FXTreeItem* FXTreeList::getItemAt(FXint,FXint y) const {
  register FXTreeItem* item=firstitem;
  register FXint ix,iy,iw,ih;
  ix=pos_x;
  iy=pos_y;
  if(options&TREELIST_ROOT_BOXES) ix+=(4+indent);
  while(item && iy<=y){
    iw=item->getWidth(this);
    ih=item->getHeight(this);
    if(y<iy+ih) return item;
    iy+=ih;
    if(item->first && ((options&TREELIST_AUTOSELECT) || item->isExpanded())){
      ix+=(indent+ih/2);
      item=item->first;
      continue;
      }
    while(!item->next && item->parent){
      item=item->parent;
      ix-=(indent+item->getHeight(this)/2);
      }
    item=item->next;
    }
  return NULL;
  }


// Did we hit the item, and which part of it did we hit (0=outside, 1=icon, 2=text, 3=box)
FXint FXTreeList::hitItem(const FXTreeItem* item,FXint x,FXint y) const {
  FXint ix,iy,iw,ih,xh,yh,hit=0;
  if(item){
    x-=pos_x;
    y-=pos_y;
    ix=item->x;
    iy=item->y;
    iw=item->getWidth(this);
    ih=item->getHeight(this);
    if(iy<=y && y<iy+ih){
      if((options&TREELIST_SHOWS_BOXES) && ((item->state&FXTreeItem::HASITEMS) || item->first)){
        xh=ix-indent+(SIDE_SPACING/2);
        yh=iy+ih/2;
        if(xh-4<=x && x<=xh+4 && yh-4<=y && y<=yh+4) return 3;
        }
      hit=item->hitItem(this,x-ix,y-iy);
      }
    }
  return hit;
  }


// Repaint
void FXTreeList::updateItem(FXTreeItem* item){
  if(item){
    update(0,pos_y+item->y,content_w,item->getHeight(this));
    }
  }


// Enable one item
FXbool FXTreeList::enableItem(FXTreeItem* item){
  if(!item){ fxerror("%s::enableItem: item is NULL.\n",getClassName()); }
  if(!item->isEnabled()){
    item->setEnabled(TRUE);
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }


// Disable one item
FXbool FXTreeList::disableItem(FXTreeItem* item){
  if(!item){ fxerror("%s::disableItem: item is NULL.\n",getClassName()); }
  if(item->isEnabled()){
    item->setEnabled(FALSE);
    updateItem(item);
    return TRUE;
    }
  return FALSE;
  }


// Select one item
FXbool FXTreeList::selectItem(FXTreeItem* item,FXbool notify){
  if(!item){ fxerror("%s::selectItem: item is NULL.\n",getClassName()); }
  if(!item->isSelected()){
    switch(options&SELECT_MASK){
      case TREELIST_SINGLESELECT:
      case TREELIST_BROWSESELECT:
        killSelection(notify);
        item->setSelected(TRUE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)item);}
        break;
      case TREELIST_EXTENDEDSELECT:
      case TREELIST_MULTIPLESELECT:
        item->setSelected(TRUE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)item);}
        break;
      }
    return TRUE;
    }
  return FALSE;
  }


// Deselect one item
FXbool FXTreeList::deselectItem(FXTreeItem* item,FXbool notify){
  if(!item){ fxerror("%s::deselectItem: item is NULL.\n",getClassName()); }
  if(item->isSelected()){
    switch(options&SELECT_MASK){
      case TREELIST_EXTENDEDSELECT:
      case TREELIST_MULTIPLESELECT:
      case TREELIST_SINGLESELECT:
        item->setSelected(FALSE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_DESELECTED),(void*)item);}
        break;
      }
    return TRUE;
    }
  return FALSE;
  }


// toggle one item
FXbool FXTreeList::toggleItem(FXTreeItem* item,FXbool notify){
  if(!item){ fxerror("%s::toggleItem: item is NULL.\n",getClassName()); }
  switch(options&SELECT_MASK){
    case TREELIST_BROWSESELECT:
      if(!item->isSelected()){
        killSelection(notify);
        item->setSelected(TRUE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)item);}
        }
      break;
    case TREELIST_SINGLESELECT:
      if(!item->isSelected()){
        killSelection(notify);
        item->setSelected(TRUE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)item);}
        }
      else{
        item->setSelected(FALSE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_DESELECTED),(void*)item);}
        }
      break;
    case TREELIST_EXTENDEDSELECT:
    case TREELIST_MULTIPLESELECT:
      if(!item->isSelected()){
        item->setSelected(TRUE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)item);}
        }
      else{
        item->setSelected(FALSE);
        updateItem(item);
        if(notify){handle(this,MKUINT(0,SEL_DESELECTED),(void*)item);}
        }
      break;
    }
  return TRUE;
  }


// Open item
FXbool FXTreeList::openItem(FXTreeItem* item,FXbool notify){
  if(item==NULL){ fxerror("%s::openItem: item is NULL.\n",getClassName()); }
  if(!item->isOpened()){
    item->setOpened(TRUE);
    updateItem(item);
    if(notify){handle(this,MKUINT(0,SEL_OPENED),(void*)item);}
    return TRUE;
    }
  return FALSE;
  }


// Close item
FXbool FXTreeList::closeItem(FXTreeItem* item,FXbool notify){
  if(item==NULL){ fxerror("%s::closeItem: item is NULL.\n",getClassName()); }
  if(item->isOpened()){
    item->setOpened(FALSE);
    updateItem(item);
    if(notify){handle(this,MKUINT(0,SEL_CLOSED),(void*)item);}
    return TRUE;
    }
  return FALSE;
  }


// Collapse all subtrees under item
FXbool FXTreeList::collapseTree(FXTreeItem* tree,FXbool notify){
  if(tree==NULL){ fxerror("%s::collapseTree: tree is NULL.\n",getClassName()); }
  if(tree->isExpanded()){
    tree->setExpanded(FALSE);
    if(!(options&TREELIST_AUTOSELECT)){     // In autoselect, already shown as expanded!
      if(tree->first){
        recalc();
        }
      else{
        updateItem(tree);
        }
      }
    if(notify){handle(this,MKUINT(0,SEL_COLLAPSED),(void*)tree);}
    return TRUE;
    }
  return FALSE;
  }


// Expand subtree under item
FXbool FXTreeList::expandTree(FXTreeItem* tree,FXbool notify){
  if(tree==NULL){ fxerror("%s::expandTree: tree is NULL.\n",getClassName()); }
  if(!tree->isExpanded()){
    tree->setExpanded(TRUE);
    if(!(options&TREELIST_AUTOSELECT)){     // In autoselect, already shown as expanded!
      if(tree->first){
        recalc();
        }
      else{
        updateItem(tree);
        }
      }
    if(notify){handle(this,MKUINT(0,SEL_EXPANDED),(void*)tree);}
    return TRUE;
    }
  return FALSE;
  }


// Reparent item under a new parent
void FXTreeList::reparentItem(FXTreeItem* item,FXTreeItem* p){
  if(!item){ fxerror("%s::reparentItem: item is NULL.\n",getClassName()); }
  if(item->parent!=p){
    if(item->prev) item->prev->next=item->next; else if(item->parent) item->parent->first=item->next; else firstitem=item->next;
    if(item->next) item->next->prev=item->prev; else if(item->parent) item->parent->last=item->prev; else lastitem=item->prev;
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
    recalc();
    }
  }


// Start motion timer while in this window
long FXTreeList::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onEnter(sender,sel,ptr);
  if(!timer){timer=getApp()->addTimeout(getApp()->getMenuPause(),this,ID_TIPTIMER);}
  cursoritem=NULL;
  return 1;
  }


// Stop motion timer when leaving window
long FXTreeList::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onLeave(sender,sel,ptr);
  if(timer){timer=getApp()->removeTimeout(timer);}
  cursoritem=NULL;
  return 1;
  }


// Gained focus
long FXTreeList::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(currentitem){
    currentitem->setFocus(TRUE);
    updateItem(currentitem);
    }
  return 1;
  }


// Lost focus
long FXTreeList::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(currentitem){
    currentitem->setFocus(FALSE);
    updateItem(currentitem);
    }
  return 1;
  }


// Draw item list
long FXTreeList::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem* item=firstitem;
  FXTreeItem* p;
  FXint yh,xh,x,y,w,h,xp,hh;
  FXDCWindow dc(this,event);
  dc.setTextFont(font);
  x=pos_x;
  y=pos_y;
  if(options&TREELIST_ROOT_BOXES) x+=(4+indent);
  while(item && y<event->rect.y+event->rect.h){
    w=item->getWidth(this);
    h=item->getHeight(this);
    if(event->rect.y<=y+h){

      // Draw item
      dc.setForeground(backColor);
      dc.fillRectangle(pos_x,y,content_w,h);
      item->draw(this,dc,x,y,w,h);

      // Show other paraphernalia such as dotted lines and expand-boxes
      if((options&(TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES)) && (item->parent || (options&TREELIST_ROOT_BOXES))){
        hh=h/2;
        yh=y+hh;
        xh=x-indent+(SIDE_SPACING/2);
        dc.setForeground(lineColor);
        dc.setStipple(STIPPLE_GRAY,pos_x&1,pos_y&1);
        if(options&TREELIST_SHOWS_LINES){                   // Connect items with lines
          p=item->parent;
          xp=xh;
          dc.setFillStyle(FILL_STIPPLED);
          while(p){
            xp-=(indent+p->getHeight(this)/2);
            if(p->next) dc.fillRectangle(xp,y,1,h);
            p=p->parent;
            }
          if((options&TREELIST_SHOWS_BOXES) && ((item->state&FXTreeItem::HASITEMS) || item->first)){
            if(item->prev || item->parent) dc.fillRectangle(xh,y,1,yh-y-4);
            if(item->next) dc.fillRectangle(xh,yh+4,1,y+h-yh-4);
            }
          else{
            if(item->prev || item->parent) dc.fillRectangle(xh,y,1,hh);
            if(item->next) dc.fillRectangle(xh,yh,1,h);
            dc.fillRectangle(xh,yh,x+(SIDE_SPACING/2)-2-xh,1);
            }
          dc.setFillStyle(FILL_SOLID);
          }

        // Boxes before items for expand/collapse of item
        if((options&TREELIST_SHOWS_BOXES) && ((item->state&FXTreeItem::HASITEMS) || item->first)){
          dc.setFillStyle(FILL_STIPPLED);
          dc.fillRectangle(xh+4,yh,(SIDE_SPACING/2)-2,1);
          dc.setFillStyle(FILL_SOLID);
          dc.drawRectangle(xh-4,yh-4,8,8);
          dc.setForeground(textColor);
          dc.fillRectangle(xh-2,yh,5,1);
          if(!(options&TREELIST_AUTOSELECT) && !item->isExpanded()){
            dc.fillRectangle(xh,yh-2,1,5);
            }
          }
        }
      }

    y+=h;

    // Move on to the next item
    if(item->first && ((options&TREELIST_AUTOSELECT) || item->isExpanded())){
      x+=(indent+h/2);
      item=item->first;
      continue;
      }
    while(!item->next && item->parent){
      item=item->parent;
      x-=(indent+item->getHeight(this)/2);
      }
    item=item->next;
    }
  if(y<event->rect.y+event->rect.h){
    dc.setForeground(backColor);
    dc.fillRectangle(event->rect.x,y,event->rect.w,event->rect.y+event->rect.h-y);
    }
  return 1;
  }

// Zero out lookup string
long FXTreeList::onLookupTimer(FXObject*,FXSelector,void*){
  lookup=FXString::null;
  lookuptimer=NULL;
  return 1;
  }


// We were asked about tip text
long FXTreeList::onQueryTip(FXObject* sender,FXSelector,void*){
  FXint x,y; FXuint state;
  if((flags&FLAG_TIP) && !(options&TREELIST_AUTOSELECT)){   // No tip when autoselect!
    getCursorPosition(x,y,state);
    FXTreeItem *item=getItemAt(x,y);
    if(item){
      FXString string=item->getText();
      sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&string);
      return 1;
      }
    }
  return 0;
  }


// We were asked about status text
long FXTreeList::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(!help.empty() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// Key Press
long FXTreeList::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem *item=currentitem;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
  if(item==NULL) item=firstitem;
  switch(event->code){
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Alt_L:
    case KEY_Alt_R:
      if(flags&FLAG_DODRAG){handle(this,MKUINT(0,SEL_DRAGGED),ptr);}
      return 1;
    case KEY_Page_Up:
    case KEY_KP_Page_Up:
      lookup=FXString::null;
      setPosition(pos_x,pos_y+verticalScrollbar()->getPage());
      return 1;
    case KEY_Page_Down:
    case KEY_KP_Page_Down:
      lookup=FXString::null;
      setPosition(pos_x,pos_y-verticalScrollbar()->getPage());
      return 1;
    case KEY_Up:                          // Move up
    case KEY_KP_Up:
      if(item){
        if(item->prev){
          item=item->prev;
          while(item->first && ((options&TREELIST_AUTOSELECT) || item->isExpanded())) item=item->last;
          }
        else if(item->parent){
          item=item->parent;
          }
        }
      goto hop;
    case KEY_Down:                        // Move down
    case KEY_KP_Down:
      if(item){
        if(item->first && ((options&TREELIST_AUTOSELECT) || item->isExpanded())){
          item=item->first;
          }
        else{
          while(!item->next && item->parent) item=item->parent;
          item=item->next;
          }
        }
      goto hop;
    case KEY_Right:                       // Move right/down and open subtree
    case KEY_KP_Right:
      if(item){
        if(!(options&TREELIST_AUTOSELECT) && !item->isExpanded() && ((item->state&FXTreeItem::HASITEMS) || item->first)){
          expandTree(item,TRUE);
          }
        else if(item->first){
          item=item->first;
          }
        else{
          while(!item->next && item->parent) item=item->parent;
          item=item->next;
          }
        }
      goto hop;
    case KEY_Left:                        // Move left/up and close subtree
    case KEY_KP_Left:
      if(item){
        if(!(options&TREELIST_AUTOSELECT) && item->isExpanded() && ((item->state&FXTreeItem::HASITEMS) || item->first)){
          collapseTree(item,TRUE);
          }
        else if(item->parent){
          item=item->parent;
          }
        else if(item->prev){
          item=item->prev;
          }
        }
      goto hop;
    case KEY_Home:                        // Move to first
    case KEY_KP_Home:
      item=firstitem;
      goto hop;
    case KEY_End:                         // Move to last
    case KEY_KP_End:
      item=lastitem;
      while(item){
        if(item->last && ((options&TREELIST_AUTOSELECT) || item->isExpanded())){
          item=item->last;
          }
        else if(item->next){
          item=item->next;
          }
        else{
          break;
          }
        }
hop:  lookup=FXString::null;
      if(item){
        setCurrentItem(item,TRUE);
        makeItemVisible(item);
        if((options&SELECT_MASK)==TREELIST_EXTENDEDSELECT){
          if(item->isEnabled()){
            if(event->state&SHIFTMASK){
              if(anchoritem){
                selectItem(anchoritem,TRUE);
                extendSelection(item,TRUE);
                }
              else{
                selectItem(item,TRUE);
                setAnchorItem(item);
                }
              }
            else if(!(event->state&CONTROLMASK)){
              killSelection(TRUE);
              selectItem(item,TRUE);
              setAnchorItem(item);
              }
            }
          }
        }
      handle(this,MKUINT(0,SEL_CLICKED),(void*)currentitem);
      if(currentitem && currentitem->isEnabled()){
        handle(this,MKUINT(0,SEL_COMMAND),(void*)currentitem);
        }
      return 1;
    case KEY_space:
    case KEY_KP_Space:
      lookup=FXString::null;
      if(item && item->isEnabled()){
        switch(options&SELECT_MASK){
          case TREELIST_EXTENDEDSELECT:
            if(event->state&SHIFTMASK){
              if(anchoritem){
                selectItem(anchoritem,TRUE);
                extendSelection(item,TRUE);
                }
              else{
                selectItem(item,TRUE);
                }
              }
            else if(event->state&CONTROLMASK){
              toggleItem(item,TRUE);
              }
            else{
              killSelection(TRUE);
              selectItem(item,TRUE);
              }
            break;
          case TREELIST_MULTIPLESELECT:
          case TREELIST_SINGLESELECT:
            toggleItem(item,TRUE);
            break;
          }
        setAnchorItem(item);
        }
      handle(this,MKUINT(0,SEL_CLICKED),(void*)currentitem);
      if(currentitem && currentitem->isEnabled()){
        handle(this,MKUINT(0,SEL_COMMAND),(void*)currentitem);
        }
      return 1;
    case KEY_Return:
    case KEY_KP_Enter:
      lookup=FXString::null;
      handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)currentitem);
      if(currentitem && currentitem->isEnabled()){
        handle(this,MKUINT(0,SEL_COMMAND),(void*)currentitem);
        }
      return 1;
    default:
      if((event->state&(CONTROLMASK|ALTMASK)) || !isprint((FXuchar)event->text[0])) return 0;
      lookup.append(event->text);
      if(lookuptimer) getApp()->removeTimeout(lookuptimer);
      lookuptimer=getApp()->addTimeout(getApp()->getTypingSpeed(),this,ID_LOOKUPTIMER);
      item=findItem(lookup,currentitem,SEARCH_FORWARD|SEARCH_WRAP|SEARCH_PREFIX);
      if(item){
	setCurrentItem(item,TRUE);
	makeItemVisible(item);
	if((options&SELECT_MASK)==TREELIST_EXTENDEDSELECT){
	  if(item->isEnabled()){
	    killSelection(TRUE);
	    selectItem(item,TRUE);
	    }
	  }
	setAnchorItem(item);
        }
      handle(this,MKUINT(0,SEL_CLICKED),(void*)currentitem);
      if(currentitem && currentitem->isEnabled()){
	handle(this,MKUINT(0,SEL_COMMAND),(void*)currentitem);
	}
      return 1;
    }
  return 0;
  }


// Key Release
long FXTreeList::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
  switch(event->code){
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Alt_L:
    case KEY_Alt_R:
      if(flags&FLAG_DODRAG){handle(this,MKUINT(0,SEL_DRAGGED),ptr);}
      return 1;
    }
  return 0;
  }


// We timed out, i.e. the user didn't move for a while
long FXTreeList::onTipTimer(FXObject*,FXSelector,void*){
  FXTRACE((200,"%s::onTipTimer %p\n",getClassName(),this));
  timer=NULL;
  flags|=FLAG_TIP;
  return 1;
  }


// Scroll timer
long FXTreeList::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem *item;
  FXint xx,yy;

  // Scroll the content
  FXScrollArea::onAutoScroll(sender,sel,ptr);

  // Drag and drop mode
  if(flags&FLAG_DODRAG){
    handle(this,MKUINT(0,SEL_DRAGGED),ptr);
    return 1;
    }

  // In autoselect mode, stop scrolling when mouse outside window
  if((flags&FLAG_PRESSED) || (options&TREELIST_AUTOSELECT)){

    // Validated position
    xx=event->win_x; if(xx<0) xx=0; else if(xx>=viewport_w) xx=viewport_w-1;
    yy=event->win_y; if(yy<0) yy=0; else if(yy>=viewport_h) yy=viewport_h-1;

    // Find item
    item=getItemAt(xx,yy);

    // Got item and different from last time
    if(item && item!=currentitem){

      // Make it the current item
      setCurrentItem(item,TRUE);

      // Extend the selection
      if((options&SELECT_MASK)==TREELIST_EXTENDEDSELECT){
        state=FALSE;
        extendSelection(item,TRUE);
        }
      }
    return 1;
    }
  return 0;
  }


// Mouse motion
long FXTreeList::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem *oldcursoritem=cursoritem;
  FXuint flg=flags;
  FXTreeItem *item;

  // Kill the tip
  flags&=~FLAG_TIP;

  // Kill the tip timer
  if(timer) timer=getApp()->removeTimeout(timer);

  // Right mouse scrolling
  if(flags&FLAG_SCROLLING){
    setPosition(event->win_x-grabx,event->win_y-graby);
    return 1;
    }

  // Drag and drop mode
  if(flags&FLAG_DODRAG){
    if(startAutoScroll(event->win_x,event->win_y,TRUE)) return 1;
    handle(this,MKUINT(0,SEL_DRAGGED),ptr);
    return 1;
    }

  // Tentative drag and drop
  if((flags&FLAG_TRYDRAG) && event->moved){
    flags&=~FLAG_TRYDRAG;
    if(handle(this,MKUINT(0,SEL_BEGINDRAG),ptr)){
      flags|=FLAG_DODRAG;
      }
    return 1;
    }

  // Normal operation
  if((flags&FLAG_PRESSED) || (options&TREELIST_AUTOSELECT)){

    // Start auto scrolling?
    if(startAutoScroll(event->win_x,event->win_y,FALSE)) return 1;

    // Find item
    item=getItemAt(event->win_x,event->win_y);

    // Got an item different from before
    if(item && item!=currentitem){

      // Make it the current item
      setCurrentItem(item,TRUE);

      // Extend the selection
      if((options&SELECT_MASK)==TREELIST_EXTENDEDSELECT){
        state=FALSE;
        extendSelection(item,TRUE);
        }
      }
    return 1;
    }

  // Reset tip timer if nothing's going on
  timer=getApp()->addTimeout(getApp()->getMenuPause(),this,ID_TIPTIMER);

  // Get item we're over
  cursoritem=getItemAt(event->win_x,event->win_y);

  // Force GUI update only when needed
  return (cursoritem!=oldcursoritem)||(flg&FLAG_TIP);
  }


// Pressed a button
long FXTreeList::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTreeItem *item;
  FXint code;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    flags&=~FLAG_UPDATE;

    // First change callback
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;

    // Not autoselect mode
    if(options&TREELIST_AUTOSELECT) return 1;

    // Locate item
    item=getItemAt(event->win_x,event->win_y);

    // No item
    if(item==NULL) return 1;

    // Find out where hit
    code=hitItem(item,event->win_x,event->win_y);

    // Maybe clicked on box
    if(code==3){
      if(isItemExpanded(item))
        collapseTree(item,TRUE);
      else
        expandTree(item,TRUE);
      return 1;
      }

    // Change current item
    setCurrentItem(item,TRUE);

    // Change item selection
    state=item->isSelected();
    switch(options&SELECT_MASK){
      case TREELIST_EXTENDEDSELECT:
        if(event->state&SHIFTMASK){
          if(anchoritem){
            if(anchoritem->isEnabled()) selectItem(anchoritem,TRUE);
            extendSelection(item,TRUE);
            }
          else{
            if(item->isEnabled()) selectItem(item,TRUE);
            setAnchorItem(item);
            }
          }
        else if(event->state&CONTROLMASK){
          if(item->isEnabled() && !state) selectItem(item,TRUE);
          setAnchorItem(item);
          }
        else{
          if(item->isEnabled() && !state){ killSelection(TRUE); selectItem(item,TRUE); }
          setAnchorItem(item);
          }
        break;
      case TREELIST_MULTIPLESELECT:
      case TREELIST_SINGLESELECT:
        if(item->isEnabled() && !state) selectItem(item,TRUE);
        break;
      }

    // Start drag if actually pressed text or icon only
    if(code && item->isSelected() && item->isDraggable()){
      flags|=FLAG_TRYDRAG;
      }

    flags|=FLAG_PRESSED;
    return 1;
    }
  return 0;
  }


// Released button
long FXTreeList::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXuint flg=flags;
  if(isEnabled()){
    ungrab();
    stopAutoScroll();
    flags|=FLAG_UPDATE;
    flags&=~(FLAG_PRESSED|FLAG_TRYDRAG|FLAG_DODRAG);

    // First chance callback
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;

    // No activity
    if(!(flg&FLAG_PRESSED) && !(options&TREELIST_AUTOSELECT)) return 1;

    // Was dragging
    if(flg&FLAG_DODRAG){
      handle(this,MKUINT(0,SEL_ENDDRAG),ptr);
      return 1;
      }

    // Select only enabled item
    switch(options&SELECT_MASK){
      case TREELIST_EXTENDEDSELECT:
        if(currentitem && currentitem->isEnabled()){
          if(event->state&CONTROLMASK){
            if(state) deselectItem(currentitem,TRUE);
            }
          else if(!(event->state&SHIFTMASK)){
            if(state){ killSelection(TRUE); selectItem(currentitem,TRUE); }
            }
          }
        break;
      case TREELIST_MULTIPLESELECT:
      case TREELIST_SINGLESELECT:
        if(currentitem && currentitem->isEnabled()){
          if(state) deselectItem(currentitem,TRUE);
          }
        break;
      }

    // Scroll to make item visibke
    makeItemVisible(currentitem);

    // Update anchor
    setAnchorItem(currentitem);

    // Generate clicked callbacks
    if(event->click_count==1){
      handle(this,MKUINT(0,SEL_CLICKED),(void*)currentitem);
      }
    else if(event->click_count==2){
      handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)currentitem);
      }
    else if(event->click_count==3){
      handle(this,MKUINT(0,SEL_TRIPLECLICKED),(void*)currentitem);
      }

    // Command callback only when clicked on item
    if(currentitem && currentitem->isEnabled()){
      handle(this,MKUINT(0,SEL_COMMAND),(void*)currentitem);
      }
    return 1;
    }
  return 0;
  }


// Pressed right button
long FXTreeList::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    flags&=~FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    flags|=FLAG_SCROLLING;
    grabx=event->win_x-pos_x;
    graby=event->win_y-pos_y;
    return 1;
    }
  return 0;
  }


// Released right button
long FXTreeList::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_SCROLLING;
    flags|=FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }



// The widget lost the grab for some reason
long FXTreeList::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onUngrabbed(sender,sel,ptr);
  flags&=~(FLAG_DODRAG|FLAG_TRYDRAG|FLAG_PRESSED|FLAG_CHANGED|FLAG_SCROLLING);
  flags|=FLAG_UPDATE;
  stopAutoScroll();
  return 1;
  }


// Command message
long FXTreeList::onCommand(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_COMMAND),ptr);
  }


// Clicked in list
long FXTreeList::onClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_CLICKED),ptr);
  }


// Double clicked in list; ptr may or may not point to an item
long FXTreeList::onDoubleClicked(FXObject*,FXSelector,void* ptr){

  // Double click anywhere in the widget
  if(target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr)) return 1;

  // Double click on an item
  if(ptr){
    if(isItemExpanded((FXTreeItem*)ptr))
      collapseTree((FXTreeItem*)ptr,TRUE);
    else
      expandTree((FXTreeItem*)ptr,TRUE);
    }
  return 0;
  }


// Triple clicked in list; ptr may or may not point to an item
long FXTreeList::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }


// Item opened
long FXTreeList::onOpened(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_OPENED),ptr);
  }


// Item closed
long FXTreeList::onClosed(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_CLOSED),ptr);
  }


// Item expanded
long FXTreeList::onExpanded(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_EXPANDED),ptr);
  }


// Item collapsed
long FXTreeList::onCollapsed(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_COLLAPSED),ptr);
  }


// Selected item
long FXTreeList::onSelected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
  }


// Deselected item
long FXTreeList::onDeselected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
  }



// Extend selection
FXbool FXTreeList::extendSelection(FXTreeItem* item,FXbool notify){
  register FXTreeItem *it,*i1,*i2,*i3;
  register FXbool changes=FALSE;
  if(item && anchoritem && extentitem){
    it=firstitem;
    i1=i2=i3=NULL;
    FXTRACE((100,"extendSelection: anchor=%s extent=%s item=%s\n",anchoritem->label.text(),extentitem->label.text(),item->label.text()));

    // Find segments
    while(it){
      if(it==item){i1=i2;i2=i3;i3=it;}
      if(it==anchoritem){i1=i2;i2=i3;i3=it;}
      if(it==extentitem){i1=i2;i2=i3;i3=it;}
      it=it->getBelow();
      }

    FXASSERT(i1 && i2 && i3);

    // First segment
    it=i1;
    while(it!=i2){

      // item = extent - anchor
      // item = anchor - extent
      if(i1==item){
        if(!it->isSelected()){
          it->setSelected(TRUE);
          updateItem(it);
          changes=TRUE;
          if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)it);}
          }
        }

      // extent = anchor - item
      // extent = item   - anchor
      else if(i1==extentitem){
        if(it->isSelected()){
          it->setSelected(FALSE);
          updateItem(it);
          changes=TRUE;
          if(notify){handle(this,MKUINT(0,SEL_DESELECTED),(void*)it);}
          }
        }
      it=it->getBelow();
      }

    // Second segment
    it=i2;
    while(it!=i3){
      it=it->getBelow();

      // extent - anchor = item
      // anchor - extent = item
      if(i3==item){
        if(!it->isSelected()){
          it->setSelected(TRUE);
          updateItem(it);
          changes=TRUE;
          if(notify){handle(this,MKUINT(0,SEL_SELECTED),(void*)it);}
          }
        }

      // item   - anchor = extent
      // anchor - item   = extent
      else if(i3==extentitem){
        if(it->isSelected()){
          it->setSelected(FALSE);
          updateItem(it);
          changes=TRUE;
          if(notify){handle(this,MKUINT(0,SEL_DESELECTED),(void*)it);}
          }
        }
      }
    extentitem=item;
    }
  return changes;
  }


// Kill selection
FXbool FXTreeList::killSelection(FXbool notify){
  register FXTreeItem *item=firstitem;
  register FXbool changes=FALSE;
  while(item){
    if(item->isSelected()){
      item->setSelected(FALSE);
      updateItem(item);
      changes=TRUE;
      if(notify){handle(this,MKUINT(0,SEL_DESELECTED),(void*)item);}
      }
    item=item->getBelow();
    }
  return changes;
  }


// Sort items in ascending order
FXint FXTreeList::ascending(const FXTreeItem* a,const FXTreeItem* b){
  return compare(a->label,b->label);
  }


// Sort items in descending order
FXint FXTreeList::descending(const FXTreeItem* a,const FXTreeItem* b){
  return compare(b->label,a->label);
  }


// Sort items
void FXTreeList::sort(FXTreeItem*& f1,FXTreeItem*& t1,FXTreeItem*& f2,FXTreeItem*& t2,int n){
  FXTreeItem *ff1,*tt1,*ff2,*tt2,*q;
  FXint m;
  if(f2==NULL){
    f1=NULL;
    t1=NULL;
    return;
    }
  if(n>1){
    m=n/2;
    n=n-m;
    sort(ff1,tt1,f2,t2,n);  // 1 or more
    sort(ff2,tt2,f2,t2,m);  // 0 or more
    FXASSERT(ff1);
    if(ff2 && sortfunc(ff1,ff2)>0){
      f1=ff2;
      ff2->prev=NULL;
      ff2=ff2->next;
      }
    else{
      f1=ff1;
      ff1->prev=NULL;
      ff1=ff1->next;
      }
    t1=f1;
    t1->next=NULL;
    while(ff1 || ff2){
      if(ff1==NULL){ t1->next=ff2; ff2->prev=t1; t1=tt2; break; }
      if(ff2==NULL){ t1->next=ff1; ff1->prev=t1; t1=tt1; break; }
      if(sortfunc(ff1,ff2)>0){
        t1->next=ff2;
        ff2->prev=t1;
        t1=ff2;
        ff2=ff2->next;
        }
      else{
        t1->next=ff1;
        ff1->prev=t1;
        t1=ff1;
        ff1=ff1->next;
        }
      t1->next=NULL;
      }
    return;
    }
  FXASSERT(f2);
  f1=f2;
  t1=f2;
  f2=f2->next;
  while(f2){
    f2->prev=NULL;
    if(sortfunc(f2,t1)>0){
      t1->next=f2;
      f2->prev=t1;
      t1=f2;
      f2=f2->next;
      continue;
      }
    if(sortfunc(f1,f2)>0){
      q=f2;
      f2=f2->next;
      q->next=f1;
      f1->prev=q;
      f1=q;
      continue;
      }
    break;
    }
  FXASSERT(f1);
  FXASSERT(t1);
  f1->prev=NULL;
  t1->next=NULL;
  }


// Sort the items based on the sort function
void FXTreeList::sortItems(){
  if(sortfunc){
    FXTreeItem* f=firstitem;
    FXTreeItem* l=lastitem;
    sort(firstitem,lastitem,f,l,getNumItems());
    recalc();
    }
  }


// Sort child items
void FXTreeList::sortChildItems(FXTreeItem* item){
  if(sortfunc){
    FXTreeItem* f=item->first;
    FXTreeItem* l=item->last;
    sort(item->first,item->last,f,l,item->getNumChildren());
    if(item->isExpanded()) recalc();     // No need to recalc if it ain't visible!
    }
  }


// Set current item
void FXTreeList::setCurrentItem(FXTreeItem* item,FXbool notify){
  if(item!=currentitem){

    // Deactivate old item
    if(currentitem){

      // No visible change if it doen't have the focus
      if(hasFocus()){
        currentitem->setFocus(FALSE);
        updateItem(currentitem);
        }

      // Close old item
      closeItem(currentitem,notify);
      }

    currentitem=item;

    // Activate new item
    if(currentitem){

      // No visible change if it doen't have the focus
      if(hasFocus()){
        currentitem->setFocus(TRUE);
        updateItem(currentitem);
        }

      // Open new item
      openItem(currentitem,notify);
      }

    // Notify item change
    if(notify && target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)currentitem);}
    }

  // Select if browse mode
  if((options&SELECT_MASK)==TREELIST_BROWSESELECT && currentitem && currentitem->isEnabled()){
    selectItem(currentitem,notify);
    }
  }


// Set anchor item
void FXTreeList::setAnchorItem(FXTreeItem* item){
  anchoritem=item;
  extentitem=item;
  }



// Create item
FXTreeItem* FXTreeList::createItem(const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr){
  return new FXTreeItem(text,oi,ci,ptr);
  }


// Add item as first one under parent p
FXTreeItem* FXTreeList::addItemFirst(FXTreeItem* p,FXTreeItem* item,FXbool notify){
  register FXTreeItem* olditem=currentitem;

  // Must have item
  if(!item){ fxerror("%s::addItemFirst: item is NULL.\n",getClassName()); }

  // Add item to list
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
  item->x=0;
  item->y=0;

  // Make current if just added
  if(!currentitem && item==lastitem) currentitem=item;

  // Notify item has been inserted
  if(notify && target){target->handle(this,MKUINT(message,SEL_INSERTED),(void*)item);}

  // Current item may have changed
  if(olditem!=currentitem){
    if(notify && target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)currentitem);}
    }

  // Was new item
  if(currentitem==item){
    if(hasFocus()){
      currentitem->setFocus(TRUE);
      }
    if((options&SELECT_MASK)==TREELIST_BROWSESELECT && currentitem->isEnabled()){
      selectItem(currentitem,notify);
      }
    }

  // Redo layout
  recalc();
  return item;
  }


// Add item as first one under parent p
FXTreeItem* FXTreeList::addItemFirst(FXTreeItem* p,const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr,FXbool notify){
  return addItemFirst(p,createItem(text,oi,ci,ptr),notify);
  }


// Add item as last one under parent p
FXTreeItem* FXTreeList::addItemLast(FXTreeItem* p,FXTreeItem* item,FXbool notify){
  register FXTreeItem* olditem=currentitem;

  // Must have item
  if(!item){ fxerror("%s::addItemLast: item is NULL.\n",getClassName()); }

  // Add item to list
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
  item->x=0;
  item->y=0;

  // Make current if just added
  if(!currentitem && item==firstitem) currentitem=item;

  // Notify item has been inserted
  if(notify && target){target->handle(this,MKUINT(message,SEL_INSERTED),(void*)item);}

  // Current item may have changed
  if(olditem!=currentitem){
    if(notify && target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)currentitem);}
    }

  // Was new item
  if(currentitem==item){
    if(hasFocus()){
      currentitem->setFocus(TRUE);
      }
    if((options&SELECT_MASK)==TREELIST_BROWSESELECT && currentitem->isEnabled()){
      selectItem(currentitem,notify);
      }
    }

  // Redo layout
  recalc();
  return item;
  }


// Add item as last one under parent p
FXTreeItem* FXTreeList::addItemLast(FXTreeItem* p,const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr,FXbool notify){
  return addItemLast(p,createItem(text,oi,ci,ptr),notify);
  }


// Link item after other
FXTreeItem* FXTreeList::addItemAfter(FXTreeItem* other,FXTreeItem* item,FXbool notify){

  // Must have items
  if(!item){ fxerror("%s::addItemAfter: item is NULL.\n",getClassName()); }
  if(!other){ fxerror("%s::addItemAfter: other item is NULL.\n",getClassName()); }

  // Add item to list
  item->prev=other;
  item->next=other->next;
  other->next=item;
  if(item->next) item->next->prev=item; else if(other->parent) other->parent->last=item; else lastitem=item;
  item->parent=other->parent;
  item->first=NULL;
  item->last=NULL;
  item->x=0;
  item->y=0;

  // Notify item has been inserted
  if(notify && target){target->handle(this,MKUINT(message,SEL_INSERTED),(void*)item);}

  // Redo layout
  recalc();
  return item;
  }


// Link item after other
FXTreeItem* FXTreeList::addItemAfter(FXTreeItem* other,const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr,FXbool notify){
  return addItemAfter(other,createItem(text,oi,ci,ptr),notify);
  }


// Link item before other
FXTreeItem* FXTreeList::addItemBefore(FXTreeItem* other,FXTreeItem* item,FXbool notify){

  // Must have items
  if(!item){ fxerror("%s::addItemBefore: item is NULL.\n",getClassName()); }
  if(!other){ fxerror("%s::addItemBefore: other item is NULL.\n",getClassName()); }

  // Add item to list
  item->next=other;
  item->prev=other->prev;
  other->prev=item;
  if(item->prev) item->prev->next=item; else if(other->parent) other->parent->first=item; else firstitem=item;
  item->parent=other->parent;
  item->first=NULL;
  item->last=NULL;
  item->x=0;
  item->y=0;

  // Notify item has been inserted
  if(notify && target){target->handle(this,MKUINT(message,SEL_INSERTED),(void*)item);}

  // Redo layout
  recalc();
  return item;
  }


// Link item before other
FXTreeItem* FXTreeList::addItemBefore(FXTreeItem* other,const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr,FXbool notify){
  return addItemBefore(other,createItem(text,oi,ci,ptr),notify);
  }


// Remove node from list
void FXTreeList::removeItem(FXTreeItem* item,FXbool notify){
  register FXTreeItem* olditem=currentitem;
  if(item){

    // First remove children
    removeItems(item->first,item->last,notify);

    // Notify item will be deleted
    if(notify && target){target->handle(this,MKUINT(message,SEL_DELETED),(void*)item);}

    // Adjust pointers; suggested by Alan Ott <ott@acusoft.com>
    if(anchoritem==item){
      if(anchoritem->next) anchoritem=anchoritem->next;
      else if(anchoritem->prev) anchoritem=anchoritem->prev;
      else anchoritem=anchoritem->parent;
      }
    if(extentitem==item){
      if(extentitem->next) extentitem=extentitem->next;
      else if(extentitem->prev) extentitem=extentitem->prev;
      else extentitem=extentitem->parent;
      }
    if(currentitem==item){
      if(currentitem->next) currentitem=currentitem->next;
      else if(currentitem->prev) currentitem=currentitem->prev;
      else currentitem=currentitem->parent;
      }

    // Remove item from list
    if(item->prev) item->prev->next=item->next; else if(item->parent) item->parent->first=item->next; else firstitem=item->next;
    if(item->next) item->next->prev=item->prev; else if(item->parent) item->parent->last=item->prev; else lastitem=item->prev;

    // Hasta la vista, baby!
    delete item;

    // Current item has changed
    if(olditem!=currentitem){
      if(notify && target){target->handle(this,MKUINT(message,SEL_CHANGED),(void*)currentitem);}
      }

    // Deleted current item
    if(currentitem && item==olditem){
      if(hasFocus()){
        currentitem->setFocus(TRUE);
        }
      if((options&SELECT_MASK)==TREELIST_BROWSESELECT && currentitem->isEnabled()){
        selectItem(currentitem,notify);
        }
      }

    // Redo layout
    recalc();
    }
  }


// Remove all siblings from [fm,to]
void FXTreeList::removeItems(FXTreeItem* fm,FXTreeItem* to,FXbool notify){
  register FXTreeItem *item;
  if(fm && to){
    do{
      item=fm;
      fm=fm->next;
      removeItem(item,notify);
      }
    while(item!=to);
    }
  }


// Remove all items
void FXTreeList::clearItems(FXbool notify){
  removeItems(firstitem,lastitem,notify);
  }


typedef FXint (*FXCompareFunc)(const FXString&,const FXString &,FXint);


// Get item by name
FXTreeItem* FXTreeList::findItem(const FXString& text,FXTreeItem* start,FXuint flags) const {
  register FXCompareFunc comparefunc;
  register FXTreeItem *item,*s,*f,*l;
  register FXint len;
  if(firstitem){
    comparefunc=(flags&SEARCH_IGNORECASE) ? (FXCompareFunc)comparecase : (FXCompareFunc)compare;
    len=(flags&SEARCH_PREFIX)?text.length():2147483647;
    if(!(flags&SEARCH_BACKWARD)){
      s=f=firstitem;
      if(start){s=start;if(s->parent){f=s->parent->first;}}
      item=s;
      while(item){
        if((*comparefunc)(item->label,text,len)==0) return item;
        item=item->next;
        }
      if(!(flags&SEARCH_WRAP)) return NULL;
      item=f;
      while(item && item!=s){
        if((*comparefunc)(item->label,text,len)==0) return item;
        item=item->next;
        }
      }
    else{
      s=l=lastitem;
      if(start){s=start;if(s->parent){l=s->parent->last;}}
      item=s;
      while(item){
        if((*comparefunc)(item->label,text,len)==0) return item;
        item=item->prev;
        }
      if(!(flags&SEARCH_WRAP)) return NULL;
      item=l;
      while(item && item!=s){
        if((*comparefunc)(item->label,text,len)==0) return item;
        item=item->prev;
        }
      }
    }
  return NULL;
  }


// Change the font
void FXTreeList::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Change help text
void FXTreeList::setHelpText(const FXString& text){
  help=text;
  }


// Set text color
void FXTreeList::setTextColor(FXColor clr){
  if(clr!=textColor){
    textColor=clr;
    update();
    }
  }


// Set select background color
void FXTreeList::setSelBackColor(FXColor clr){
  if(clr!=selbackColor){
    selbackColor=clr;
    update();
    }
  }


// Set selected text color
void FXTreeList::setSelTextColor(FXColor clr){
  if(clr!=seltextColor){
    seltextColor=clr;
    update();
    }
  }


// Set line color
void FXTreeList::setLineColor(FXColor clr){
  if(clr!=lineColor){
    lineColor=clr;
    update();
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


// Save data
void FXTreeList::save(FXStream& store) const {
  FXScrollArea::save(store);
  store << firstitem;
  store << lastitem;
  store << anchoritem;
  store << currentitem;
  store << extentitem;
  store << font;
  store << textColor;
  store << selbackColor;
  store << seltextColor;
  store << lineColor;
  store << treeWidth;
  store << treeHeight;
  store << visible;
  store << indent;
  store << help;
  }


// Load data
void FXTreeList::load(FXStream& store){
  FXScrollArea::load(store);
  store >> firstitem;
  store >> lastitem;
  store >> anchoritem;
  store >> currentitem;
  store >> extentitem;
  store >> font;
  store >> textColor;
  store >> selbackColor;
  store >> seltextColor;
  store >> lineColor;
  store >> treeWidth;
  store >> treeHeight;
  store >> visible;
  store >> indent;
  store >> help;
  }


// Cleanup
FXTreeList::~FXTreeList(){
  if(timer) getApp()->removeTimeout(timer);
  if(lookuptimer){getApp()->removeTimeout(lookuptimer);}
  clearItems(FALSE);
  firstitem=(FXTreeItem*)-1;
  lastitem=(FXTreeItem*)-1;
  anchoritem=(FXTreeItem*)-1;
  currentitem=(FXTreeItem*)-1;
  extentitem=(FXTreeItem*)-1;
  font=(FXFont*)-1;
  timer=(FXTimer*)-1;
  lookuptimer=(FXTimer*)-1;
  }


