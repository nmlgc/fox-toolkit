/********************************************************************************
*                                                                               *
*                               H e a d e r   O b j e c t                       *
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
* $Id: FXHeader.cpp,v 1.31 2002/01/18 22:43:00 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXIcon.h"
#include "FXHeader.h"




/*
  Notes:
  - Perhaps, some way to drive via keyboard?
  - Allow some header items to be stretchable.
  - Add minimum, maximum size constraints to items.
  - Should up/down arrows go sideways when vertically oriented?
  - Perhaps maintain button state in item.
  - Perhaps perform drawing of border in item also.
*/


#define FUDGE         8
#define ARROW_SPACING 8
#define ICON_SPACING  4
#define HEADER_MASK   (HEADER_BUTTON|HEADER_TRACKING|HEADER_VERTICAL)


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXHeaderItem,FXObject,NULL,0)


// Draw item
void FXHeaderItem::draw(const FXHeader* header,FXDC& dc,FXint x,FXint y,FXint w,FXint h){
  FXFont *font=header->getFont();

  // Clip to inside of header control
  dc.setClipRectangle(x,y,w,h);

  // Account for borders
  w=w-(header->getPadLeft()+header->getPadRight()+(header->getBorderWidth()<<1));
  h=h-(header->getPadTop()+header->getPadBottom()+(header->getBorderWidth()<<1));
  x+=header->getBorderWidth()+header->getPadLeft();
  y+=header->getBorderWidth()+header->getPadTop();

  // Draw icon
  if(icon){
    if(icon->getWidth()<=w){
      dc.drawIcon(icon,x,y+(h-icon->getHeight())/2);
      x+=icon->getWidth();
      w-=icon->getWidth();
      }
    }

  // Draw text
  if(!label.empty()){
    FXint dw=font->getTextWidth("...",3);
    FXint num=label.length();
    FXint tw=font->getTextWidth(label.text(),num);
    FXint th=font->getFontHeight();
    FXint ty=y+(h-th)/2+font->getFontAscent();
    dc.setTextFont(font);
    if(icon){ x+=ICON_SPACING; w-=ICON_SPACING; }
    if(tw<=w){
      dc.setForeground(header->getTextColor());
      dc.drawText(x,ty,label.text(),num);
      x+=tw;
      w-=tw;
      }
    else{
      while(num>0 && (tw=font->getTextWidth(label.text(),num))>(w-dw)) num--;
      if(num>0){
        dc.setForeground(header->getTextColor());
        dc.drawText(x,ty,label.text(),num);
        dc.drawText(x+tw,ty,"...",3);
        x+=tw+dw;
        w-=tw+dw;
        }
      else{
        tw=font->getTextWidth(label.text(),1);
        if(tw<=w){
          dc.setForeground(header->getTextColor());
          dc.drawText(x,ty,label.text(),1);
          x+=tw;
          w-=tw;
          }
        }
      }
    }

  // Draw arrows
  if(arrow!=MAYBE){
    FXint aa=(font->getFontHeight()-3)|1;
    if(icon || !label.empty()){ x+=ARROW_SPACING; w-=ARROW_SPACING; }
    if(w>aa){
      if(arrow==TRUE){
        y=y+(h-aa)/2;
        dc.setForeground(header->getHiliteColor());
        dc.drawLine(x+aa/2,y,x+aa-1,y+aa);
        dc.drawLine(x,y+aa,x+aa,y+aa);
        dc.setForeground(header->getShadowColor());
        dc.drawLine(x+aa/2,y,x,y+aa);
        }
      else{
        y=y+(h-aa)/2;
        dc.setForeground(header->getHiliteColor());
        dc.drawLine(x+aa/2,y+aa,x+aa-1,y);
        dc.setForeground(header->getShadowColor());
        dc.drawLine(x+aa/2,y+aa,x,y);
        dc.drawLine(x,y,x+aa,y);
        }
      }
    }

  // Restore original clip path
  dc.clearClipRectangle();
  }


// Create icon
void FXHeaderItem::create(){ if(icon) icon->create(); }


// No op, we don't own icon
void FXHeaderItem::destroy(){ /*if(icon) icon->destroy();*/ }


// Detach from icon resource
void FXHeaderItem::detach(){ if(icon) icon->detach(); }


// Get width of item
FXint FXHeaderItem::getWidth(const FXHeader* header) const {
  register FXint w;
  if(header->getHeaderStyle()&HEADER_VERTICAL){
    w=0;
    if(icon) w=icon->getWidth();
    if(!label.empty()){ w+=header->getFont()->getTextWidth(label.text(),label.length()); if(icon) w+=ICON_SPACING; }
    return w+header->getPadLeft()+header->getPadRight()+(header->getBorderWidth()<<1);
    }
  return size;
  }


// Get height of item
FXint FXHeaderItem::getHeight(const FXHeader* header) const {
  register FXint th,ih;
  if(!(header->getHeaderStyle()&HEADER_VERTICAL)){
    th=ih=0;
    if(!label.empty()) th=header->getFont()->getFontHeight();
    if(icon) ih=icon->getHeight();
    return FXMAX(th,ih)+header->getPadTop()+header->getPadBottom()+(header->getBorderWidth()<<1);
    }
  return size;
  }


// Save data
void FXHeaderItem::save(FXStream& store) const {
  FXObject::save(store);
  store << label;
  store << icon;
  store << size;
  store << arrow;
  }


// Load data
void FXHeaderItem::load(FXStream& store){
  FXObject::load(store);
  store >> label;
  store >> icon;
  store >> size;
  store >> arrow;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXHeader) FXHeaderMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXHeader::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXHeader::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXHeader::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXHeader::onLeftBtnRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXHeader::onUngrabbed),
  FXMAPFUNC(SEL_TIMEOUT,FXHeader::ID_TIPTIMER,FXHeader::onTipTimer),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXHeader::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXHeader::onQueryHelp),
  };


// Object implementation
FXIMPLEMENT(FXHeader,FXFrame,FXHeaderMap,ARRAYNUMBER(FXHeaderMap))


// Make a Header
FXHeader::FXHeader(){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  items=NULL;
  nitems=0;
  textColor=0;
  state=FALSE;
  font=(FXFont*)-1;
  timer=NULL;
  active=-1;
  activepos=0;
  activesize=0;
  off=0;
  }


// Make a Header
FXHeader::FXHeader(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  flags|=FLAG_ENABLED|FLAG_SHOWN;
  target=tgt;
  message=sel;
  items=NULL;
  nitems=0;
  textColor=getApp()->getForeColor();
  font=getApp()->getNormalFont();
  timer=NULL;
  state=FALSE;
  active=-1;
  activepos=0;
  activesize=0;
  off=0;
  }


// Create window
void FXHeader::create(){
  register FXint i;
  FXFrame::create();
  font->create();
  for(i=0; i<nitems; i++){if(items[i]->icon){items[i]->icon->create();}}
  }


// Detach window
void FXHeader::detach(){
  register FXint i;
  FXFrame::detach();
  font->detach();
  for(i=0; i<nitems; i++){if(items[i]->icon){items[i]->icon->detach();}}
  }


// Get default width
FXint FXHeader::getDefaultWidth(){
  register FXint w,i,t;
  if(options&HEADER_VERTICAL){
    for(i=0,w=0; i<nitems; i++){if((t=items[i]->getWidth(this))>w) w=t;}
    }
  else{
    for(i=0,w=0; i<nitems; i++){w+=items[i]->getWidth(this);}
    }
  return w;
  }


// Get default height
FXint FXHeader::getDefaultHeight(){
  register FXint h,i,t;
  if(options&HEADER_VERTICAL){
    for(i=0,h=0; i<nitems; i++){h+=items[i]->getHeight(this);}
    }
  else{
    for(i=0,h=0; i<nitems; i++){if((t=items[i]->getHeight(this))>h) h=t;}
    }
  return h;
  }


// Create custom item
FXHeaderItem *FXHeader::createItem(const FXString& text,FXIcon* icon,FXint size,void* ptr){
  return new FXHeaderItem(text,icon,size,ptr);
  }


// Retrieve item
FXHeaderItem *FXHeader::retrieveItem(FXint index) const {
  if(index<0 || nitems<index){ fxerror("%s::retrieveItem: index out of range.\n",getClassName()); }
  return items[index];
  }


// Replace item with another
FXint FXHeader::replaceItem(FXint index,FXHeaderItem* item,FXbool notify){

  // Must have item
  if(!item){ fxerror("%s::replaceItem: item is NULL.\n",getClassName()); }

  // Must be in range
  if(index<0 || nitems<=index){ fxerror("%s::replaceItem: index out of range.\n",getClassName()); }

  // Notify item will be replaced
  if(notify && target){target->handle(this,MKUINT(message,SEL_REPLACED),(void*)index);}

  // Copy the size over
  item->setSize(items[index]->getSize());

  // Delete old
  delete items[index];

  // Add new
  items[index]=item;

  // Redo layout
  recalc();
  return index;
  }


// Replace item with another
FXint FXHeader::replaceItem(FXint index,const FXString& text,FXIcon *icon,FXint size,void* ptr,FXbool notify){
  return replaceItem(index,createItem(text,icon,FXMAX(size,0),ptr),notify);
  }


// Insert item
FXint FXHeader::insertItem(FXint index,FXHeaderItem* item,FXbool notify){

  // Must have item
  if(!item){ fxerror("%s::insertItem: item is NULL.\n",getClassName()); }

  // Must be in range
  if(index<0 || nitems<index){ fxerror("%s::insertItem: index out of range.\n",getClassName()); }

  // Add item to list
  FXRESIZE(&items,FXHeaderItem*,nitems+1);
  memmove(&items[index+1],&items[index],sizeof(FXHeaderItem*)*(nitems-index));
  items[index]=item;
  nitems++;

  // Notify item has been inserted
  if(notify && target){target->handle(this,MKUINT(message,SEL_INSERTED),(void*)index);}

  // Redo layout
  recalc();

  return index;
  }


// Insert item
FXint FXHeader::insertItem(FXint index,const FXString& text,FXIcon *icon,FXint size,void* ptr,FXbool notify){
  return insertItem(index,createItem(text,icon,FXMAX(size,0),ptr),notify);
  }


// Append item
FXint FXHeader::appendItem(FXHeaderItem* item,FXbool notify){
  return insertItem(nitems,item,notify);
  }


// Append item
FXint FXHeader::appendItem(const FXString& text,FXIcon *icon,FXint size,void* ptr,FXbool notify){
  return insertItem(nitems,createItem(text,icon,FXMAX(size,0),ptr),notify);
  }


// Prepend item
FXint FXHeader::prependItem(FXHeaderItem* item,FXbool notify){
  return insertItem(0,item,notify);
  }

// Prepend item
FXint FXHeader::prependItem(const FXString& text,FXIcon *icon,FXint size,void* ptr,FXbool notify){
  return insertItem(0,createItem(text,icon,FXMAX(size,0),ptr),notify);
  }


// Remove node from list
void FXHeader::removeItem(FXint index,FXbool notify){

  // Must be in range
  if(index<0 || nitems<=index){ fxerror("%s::removeItem: index out of range.\n",getClassName()); }

  // Notify item will be deleted
  if(notify && target){target->handle(this,MKUINT(message,SEL_DELETED),(void*)index);}

  // Remove item from list
  nitems--;
  delete items[index];
  memmove(&items[index],&items[index+1],sizeof(FXHeaderItem*)*(nitems-index));

  // Redo layout
  recalc();
  }


// Remove all items
void FXHeader::clearItems(FXbool notify){

  // Delete items
  for(FXint index=0; index<nitems; index++){
    if(notify && target){target->handle(this,MKUINT(message,SEL_DELETED),(void*)index);}
    delete items[index];
    }

  // Free array
  FXFREE(&items);
  nitems=0;

  // Redo layout
  recalc();
  }


// Get item at offset
FXint FXHeader::getItemAt(FXint offset) const {
  register FXint x,y,i,w,h;
  if(options&HEADER_VERTICAL){
    for(i=0,y=0; i<nitems; i++){
      h=items[i]->getHeight(this);
      if(y<=offset && offset<y+h) return i;
      y+=h;
      }
    }
  else{
    for(i=0,x=0; i<nitems; i++){
      w=items[i]->getWidth(this);
      if(x<=offset && offset<x+w) return i;
      x+=w;
      }
    }
  return -1;
  }


// Change item's text
void FXHeader::setItemText(FXint index,const FXString& text){
  if(index<0 || nitems<=index){ fxerror("%s::setItemText: index out of range.\n",getClassName()); }
  if(items[index]->getText()!=text){
    items[index]->setText(text);
    update();
    }
  }


// Get item's text
FXString FXHeader::getItemText(FXint index) const {
  if(index<0 || nitems<=index){ fxerror("%s::getItemText: index out of range.\n",getClassName()); }
  return items[index]->getText();
  }


// Change item's icon
void FXHeader::setItemIcon(FXint index,FXIcon* icon){
  if(index<0 || nitems<=index){ fxerror("%s::setItemIcon: index out of range.\n",getClassName()); }
  if(items[index]->getIcon()!=icon){
    items[index]->setIcon(icon);
    update();
    }
  }


// Get item's icon
FXIcon* FXHeader::getItemIcon(FXint index) const {
  if(index<0 || nitems<=index){ fxerror("%s::getItemIcon: index out of range.\n",getClassName()); }
  return items[index]->getIcon();
  }


// Change item's size
void FXHeader::setItemSize(FXint index,FXint size){
  if(index<0 || nitems<=index){ fxerror("%s::setItemSize: index out of range.\n",getClassName()); }
  if(size<0) size=0;
  if(items[index]->getSize()!=size){
    items[index]->setSize(size);
    recalc();
    }
  }


// Get item's size
FXint FXHeader::getItemSize(FXint index) const {
  if(index<0 || nitems<=index){ fxerror("%s::getItemSize: index out of range.\n",getClassName()); }
  return items[index]->getSize();
  }


// Get item's offset
FXint FXHeader::getItemOffset(FXint index) const {
  register FXint off,i;
  if(index<0 || nitems<=index){ fxerror("%s::getItemOffset: index out of range.\n",getClassName()); }
  if(options&HEADER_VERTICAL){
    for(i=0,off=0; i<index; i++){off+=items[i]->getHeight(this);}
    }
  else{
    for(i=0,off=0; i<index; i++){off+=items[i]->getWidth(this);}
    }
  return off;
  }


// Set item data
void FXHeader::setItemData(FXint index,void* ptr){
  if(index<0 || nitems<=index){ fxerror("%s::setItemData: index out of range.\n",getClassName()); }
  items[index]->setData(ptr);
  }


// Get item data
void* FXHeader::getItemData(FXint index) const {
  if(index<0 || nitems<=index){ fxerror("%s::getItemData: index out of range.\n",getClassName()); }
  return items[index]->getData();
  }


// Change sort direction
void FXHeader::setArrowDir(FXint index,FXbool dir){
  if(index<0 || nitems<=index){ fxerror("%s::setArrowDir: index out of range.\n",getClassName()); }
  if(items[index]->getArrowDir()!=dir){
    items[index]->setArrowDir(dir);
    update();
    }
  }


// Return sort direction
FXbool FXHeader::getArrowDir(FXint index) const {
  if(index<0 || nitems<=index){ fxerror("%s::getArrowDir: index out of range.\n",getClassName()); }
  return items[index]->getArrowDir();
  }


// Do layout
void FXHeader::layout(){

  // Force repaint
  update();

  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Handle repaint
long FXHeader::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint x,y,w,h,i;
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  if(options&HEADER_VERTICAL){
    for(i=0,y=0; i<nitems; i++){
      h=items[i]->getHeight(this);
      if(ev->rect.y<y+h && y<ev->rect.y+ev->rect.h){
        items[i]->draw(this,dc,0,y,width,h);
        if(i==active && state){
          if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc,0,y,width,h);
          else drawSunkenRectangle(dc,0,y,width,h);
          }
        else{
          if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,0,y,width,h);
          else drawRaisedRectangle(dc,0,y,width,h);
          }
        }
      y+=h;
      }
    if(y<height){
      if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,0,y,width,height-y);
      else drawRaisedRectangle(dc,0,y,width,height-y);
      }
    }
  else{
    for(i=0,x=0; i<nitems; i++){
      w=items[i]->getWidth(this);
      if(ev->rect.x<x+w && x<ev->rect.x+ev->rect.w){
        items[i]->draw(this,dc,x,0,w,height);
        if(i==active && state){
          if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc,x,0,w,height);
          else drawSunkenRectangle(dc,x,0,w,height);
          }
        else{
          if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,x,0,w,height);
          else drawRaisedRectangle(dc,x,0,w,height);
          }
        }
      x+=w;
      }
    if(x<width){
      if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,x,0,width-x,height);
      else drawRaisedRectangle(dc,x,0,width-x,height);
      }
    }
  return 1;
  }


// We were asked about tip text
long FXHeader::onQueryTip(FXObject* sender,FXSelector,void*){
  FXint index,cx,cy; FXuint btns;
  if(flags&FLAG_TIP){
    getCursorPosition(cx,cy,btns);
    index=getItemAt((options&HEADER_VERTICAL)?cy:cx);
    if(0<=index){
      FXString string=items[index]->getText();
      sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&string);
      return 1;
      }
    }
  return 0;
  }


// We were asked about status text
long FXHeader::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(!help.empty() && (flags&FLAG_HELP)){
    FXTRACE((250,"%s::onQueryHelp %p\n",getClassName(),this));
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We timed out, i.e. the user didn't move for a while
long FXHeader::onTipTimer(FXObject*,FXSelector,void*){
  FXTRACE((250,"%s::onTipTimer %p\n",getClassName(),this));
  timer=NULL;
  flags|=FLAG_TIP;
  return 1;
  }



// Button being pressed
long FXHeader::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint i,x,y,w,h;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(options&HEADER_VERTICAL){
      for(i=0,y=0; i<nitems; i++){
        h=items[i]->getHeight(this);

        // Hit a header button?
        if((options&HEADER_BUTTON) && y+FUDGE<=ev->win_y && ev->win_y<y+h-FUDGE){
          active=i;
          activepos=y;
          activesize=h;
          state=TRUE;
          update(0,activepos,width,activesize);
          flags&=~FLAG_UPDATE;
          flags|=FLAG_PRESSED;
          break;
          }

        // Upper end of a button
        if(y+h-FUDGE<=ev->win_y && ev->win_y<y+h){
          setDragCursor(getApp()->getDefaultCursor(DEF_VSPLIT_CURSOR));
          active=i;
          activepos=y;
          activesize=h;
          flags|=FLAG_DODRAG;
          flags&=~FLAG_UPDATE;
          flags|=FLAG_PRESSED;
          break;
          }

        // Lower end of last button at this place
        if(y+h<=ev->win_y && ev->win_y<y+h+FUDGE){
          setDragCursor(getApp()->getDefaultCursor(DEF_VSPLIT_CURSOR));
          active=i;
          activepos=y;
          activesize=h;
          flags|=FLAG_DODRAG;
          flags&=~FLAG_UPDATE;
          flags|=FLAG_PRESSED;
          }

        // Hit nothing, next item
        y+=h;
        }

      // Dragging
      if(flags&FLAG_DODRAG){
        off=ev->win_y-activepos-activesize;
        if(!(options&HEADER_TRACKING)) drawSplit(activepos+activesize);
        }
      }
    else{
      for(i=0,x=0; i<nitems; i++){
        w=items[i]->getWidth(this);

        // Hit a header button?
        if((options&HEADER_BUTTON) && x+FUDGE<=ev->win_x && ev->win_x<x+w-FUDGE){
          active=i;
          activepos=x;
          activesize=w;
          state=TRUE;
          update(activepos,0,activesize,height);
          flags&=~FLAG_UPDATE;
          flags|=FLAG_PRESSED;
          break;
          }

        // Upper end of a button
        if(x+w-FUDGE<=ev->win_x && ev->win_x<x+w){
          setDragCursor(getApp()->getDefaultCursor(DEF_HSPLIT_CURSOR));
          active=i;
          activepos=x;
          activesize=w;
          flags|=FLAG_DODRAG;
          flags&=~FLAG_UPDATE;
          flags|=FLAG_PRESSED;
          break;
          }

        // Lower end of last button at this place
        if(x+w<=ev->win_x && ev->win_x<x+w+FUDGE){
          setDragCursor(getApp()->getDefaultCursor(DEF_HSPLIT_CURSOR));
          active=i;
          activepos=x;
          activesize=w;
          flags|=FLAG_DODRAG;
          flags&=~FLAG_UPDATE;
          flags|=FLAG_PRESSED;
          }

        // Hit nothing, next item
        x+=w;
        }

      // Dragging
      if(flags&FLAG_DODRAG){
        off=ev->win_x-activepos-activesize;
        if(!(options&HEADER_TRACKING)) drawSplit(activepos+activesize);
        }
      }
    return 1;
    }
  return 0;
  }


// Button being released
long FXHeader::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  if(isEnabled()){
    ungrab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(flags&FLAG_DODRAG){
      setDragCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
      if(!(options&HEADER_TRACKING)){
        drawSplit(activepos+activesize);
        setItemSize(active,activesize);
        if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)active);
        }
      flags&=~FLAG_DODRAG;
      }
    else if(state){
      state=FALSE;
      if(options&HEADER_VERTICAL)
        update(0,activepos,width,activesize);
      else
        update(activepos,0,activesize,height);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)active);
      }
    return 1;
    }
  return 0;
  }


// Button being moved
long FXHeader::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint i,x,y,w,h,oldsplit,newsplit;
  FXuint flg=flags;

  // Kill the tip
  flags&=~FLAG_TIP;

  // Kill the tip timer
  if(timer) timer=getApp()->removeTimeout(timer);

  // Had the button pressed
  if(flags&FLAG_PRESSED){

    // We were dragging a split
    if(flags&FLAG_DODRAG){
      oldsplit=activepos+activesize;
      if(options&HEADER_VERTICAL)
        activesize=ev->win_y-off-activepos;
      else
        activesize=ev->win_x-off-activepos;
      if(activesize<0) activesize=0;
      newsplit=activepos+activesize;
      if(newsplit!=oldsplit){
        if(!(options&HEADER_TRACKING)){
          drawSplit(oldsplit);
          drawSplit(newsplit);
          }
        else{
          setItemSize(active,activesize);
          if(target) target->handle(this,MKUINT(message,SEL_CHANGED),(void*)active);
          }
        }
      }

    // We pressed the button
    else{
      if(options&HEADER_VERTICAL){
        if(activepos<=ev->win_y && ev->win_y<activepos+activesize && 0<=ev->win_x && ev->win_x<width){
          if(!state){
            state=TRUE;
            update(0,activepos,width,activesize);
            }
          }
        else{
          if(state){
            state=FALSE;
            update(0,activepos,width,activesize);
            }
          }
        }
      else{
        if(activepos<=ev->win_x && ev->win_x<activepos+activesize && 0<=ev->win_y && ev->win_y<height){
          if(!state){
            state=TRUE;
            update(activepos,0,activesize,height);
            }
          }
        else{
          if(state){
            state=FALSE;
            update(activepos,0,activesize,height);
            }
          }
        }
      }
    return 1;
    }

  // If over a split, show split cursor
  if(isEnabled()){
    if(options&HEADER_VERTICAL){
      for(i=0,y=0; i<nitems; i++){
        h=items[i]->getHeight(this);
        if(y+h-FUDGE<=ev->win_y && ev->win_y<y+h+FUDGE){
          setDefaultCursor(getApp()->getDefaultCursor(DEF_VSPLIT_CURSOR));
          return 1;
          }
        y+=h;
        }
      }
    else{
      for(i=0,x=0; i<nitems; i++){
        w=items[i]->getWidth(this);
        if(x+w-FUDGE<=ev->win_x && ev->win_x<x+w+FUDGE){
          setDefaultCursor(getApp()->getDefaultCursor(DEF_HSPLIT_CURSOR));
          return 1;
          }
        x+=w;
        }
      }
    }

  // Not over a split, back to normal cursor
  setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));

  // Reset tip timer if nothing's going on
  timer=getApp()->addTimeout(getApp()->getMenuPause(),this,ID_TIPTIMER);

  // Force GUI update only when needed
  return (flg&FLAG_TIP);
  }


// The widget lost the grab for some reason
long FXHeader::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onUngrabbed(sender,sel,ptr);
  flags&=~FLAG_PRESSED;
  flags&=~FLAG_CHANGED;
  flags|=FLAG_UPDATE;
  return 1;
  }


// Draw the split
void FXHeader::drawSplit(FXint pos){
  FXDCWindow dc(getParent());
  FXint px,py;
  translateCoordinatesTo(px,py,getParent(),pos,pos);
  dc.clipChildren(FALSE);
  dc.setFunction(BLT_NOT_DST);
  if(options&HEADER_VERTICAL){
    dc.fillRectangle(0,py,getParent()->getWidth(),2);
    }
  else{
    dc.fillRectangle(px,0,2,getParent()->getHeight());
    }
  }


// Change the font
void FXHeader::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Set text color
void FXHeader::setTextColor(FXColor clr){
  if(textColor!=clr){
    textColor=clr;
    update();
    }
  }


// Header style change
void FXHeader::setHeaderStyle(FXuint style){
  FXuint opts=(options&~HEADER_MASK) | (style&HEADER_MASK);
  if(options!=opts){    // Thanks to: Hartmut Scholz <hartmut.scholz@firemail.de>
    options=opts;
    recalc();
    update();
    }
  }


// Get header style
FXuint FXHeader::getHeaderStyle() const {
  return (options&HEADER_MASK);
  }


// Change help text
void FXHeader::setHelpText(const FXString& text){
  help=text;
  }


// Save object to stream
void FXHeader::save(FXStream& store) const {
  register FXint i;
  FXFrame::save(store);
  store << nitems;
  for(i=0; i<nitems; i++){store<<items[i];}
  store << textColor;
  store << font;
  store << help;
  }



// Load object from stream
void FXHeader::load(FXStream& store){
  register FXint i;
  FXFrame::load(store);
  store >> nitems;
  FXRESIZE(&items,FXHeaderItem*,nitems);
  for(i=0; i<nitems; i++){store>>items[i];}
  store >> textColor;
  store >> font;
  store >> help;
  }


// Clean up
FXHeader::~FXHeader(){
  if(timer){getApp()->removeTimeout(timer);}
  clearItems();
  items=(FXHeaderItem**)-1;
  font=(FXFont*)-1;
  timer=(FXTimer*)-1;
  }
