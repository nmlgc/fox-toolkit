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
* $Id: FXList.h,v 1.41 1998/10/19 16:23:21 jvz Exp $                         *
********************************************************************************/
#ifndef FXLIST_H
#define FXLIST_H


// List options
enum FXListStyle {
  LIST_EXTENDEDSELECT = 0,                // Extended selection mode
  LIST_SINGLESELECT   = 0x00100000,       // At most one selected item
  LIST_BROWSESELECT   = 0x00200000,       // Always exactly one selected item
  LIST_MULTIPLESELECT = LIST_SINGLESELECT|LIST_BROWSESELECT,  // Multiple selection mode
  LIST_AUTOSELECT     = 0x00400000,       // Automatically select under cursor
  LIST_WANTSELECTION  = 0x00800000,       // Claim the selection
  LIST_MASK           = LIST_SINGLESELECT|LIST_BROWSESELECT|LIST_AUTOSELECT|LIST_WANTSELECTION
  };


// Opaque item type
struct FXListItem;



// Base of list
class FXList : public FXScrollArea {
  FXDECLARE(FXList)
protected:
  FXListItem    *firstitem;
  FXListItem    *lastitem;
  FXListItem    *anchoritem;
  FXListItem    *currentitem;
  FXFont        *font;
  FXItemSortFunc sortfunc;
  FXPixel        textColor;
  FXPixel        selbackColor;
  FXPixel        seltextColor;
  FXint          totalWidth;
  FXint          totalHeight;
protected:
  FXList();
  FXList(const FXList&){}
  static FXbool before(FXListItem* a,FXListItem* b);
  void drawFocusRectangle(FXint x,FXint y,FXint w,FXint h);
  virtual FXListItem* createItem();
  virtual void layout();
  void recompute();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onChanged(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);
  long onSelected(FXObject*,FXSelector,void*);
  long onDeselected(FXObject*,FXSelector,void*);
public:
  FXList(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void recalc();
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  virtual FXbool canFocus() const;
  FXint getNumItems() const;
  FXint getNumSelectedItems() const;
  FXListItem** getSelectedItems() const;
  FXListItem* getFirstItem() const { return firstitem; }
  FXListItem* getLastItem() const { return lastitem; }
  FXListItem* getNextItem(const FXListItem* item) const;
  FXListItem* getPrevItem(const FXListItem* item) const;
  virtual FXListItem* addItemFirst(const char* text,void* ptr=NULL);
  virtual FXListItem* addItemLast(const char* text,void* ptr=NULL);
  virtual FXListItem* addItemAfter(FXListItem* other,const char* text,void* ptr=NULL);
  virtual FXListItem* addItemBefore(FXListItem* other,const char* text,void* ptr=NULL);
  virtual void removeItem(FXListItem* item);
  virtual void removeItems(FXListItem* fm,FXListItem* to);
  virtual void removeAllItems();
  virtual FXListItem* getItemAt(FXint x,FXint y) const;
  virtual FXint getItemX(const FXListItem* item) const;
  virtual FXint getItemY(const FXListItem* item) const;
  virtual FXint getItemWidth(const FXListItem* item) const;
  virtual FXint getItemHeight(const FXListItem* item) const;
  void updateItem(FXListItem* item);
  void makeItemVisible(FXListItem* item);
  FXbool isItemSelected(const FXListItem* item) const;
  FXbool isItemCurrent(const FXListItem* item) const;
  FXbool isItemVisible(const FXListItem* item) const;
  void markItems();
  FXbool selectItemRange(FXListItem* beg,FXListItem* end);
  FXbool deselectItemRange(FXListItem* beg,FXListItem* end);
  FXbool toggleItemRange(FXListItem* beg,FXListItem* end);
  FXbool restoreItemRange(FXListItem* beg,FXListItem* end);
  FXbool selectItem(FXListItem* item);
  FXbool deselectItem(FXListItem* item);
  FXbool toggleItem(FXListItem* item);
  FXbool restoreItem(FXListItem* item);
  void sortItems();
  void setCurrentItem(FXListItem* item);
  FXListItem* getCurrentItem() const { return currentitem; }
  void setAnchorItem(FXListItem* item);
  FXListItem* getAnchorItem() const { return anchoritem; }
  FXbool extendSelection(FXListItem* item);
  void setItemText(FXListItem* item,const char* text);
  const char* getItemText(const FXListItem* item) const;
  void setItemData(FXListItem* item,void* ptr) const;
  void* getItemData(const FXListItem* item) const;
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  FXPixel getSelBackColor() const { return selbackColor; }
  void setSelBackColor(FXPixel clr);
  FXPixel getSelTextColor() const { return seltextColor; }
  void setSelTextColor(FXPixel clr);
  FXItemSortFunc getSortFunc() const { return sortfunc; }
  void setSortFunc(FXItemSortFunc func){ sortfunc=func; }
  FXuint getListStyle() const;
  void setListStyle(FXuint style);
  virtual ~FXList();
  };



#endif
