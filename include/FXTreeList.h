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
* $Id: FXTreeList.h,v 1.36 1998/10/19 16:23:21 jvz Exp $                     *
********************************************************************************/
#ifndef FXTREELIST_H
#define FXTREELIST_H


// Tree List options
enum FXTreeListStyle {
  TREELIST_EXTENDEDSELECT = 0,
  TREELIST_SINGLESELECT   = 0x00100000,       // Single selection mode
  TREELIST_BROWSESELECT   = 0x00200000,       // Browse selection mode
  TREELIST_MULTIPLESELECT = TREELIST_SINGLESELECT|TREELIST_BROWSESELECT,  // Multiple select
  TREELIST_AUTOSELECT     = 0x00400000,       // Automatically select under cursor
  TREELIST_SHOWS_LINES    = 0x00800000,       // Lines shown
  TREELIST_SHOWS_BOXES    = 0x01000000,       // Boxes to expand shown
  TREELIST_ROOT_BOXES     = 0x02000000,       // Display root boxes also
  TREELIST_WANTSELECTION  = 0x04000000,       // Claim the selection
  TREELIST_MASK           = TREELIST_MULTIPLESELECT|TREELIST_AUTOSELECT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_WANTSELECTION
  };


// Opaque item type
struct FXTreeItem;



// Generic tab item
class FXTreeList : public FXScrollArea {
  FXDECLARE(FXTreeList)
protected:
  FXTreeItem    *firstitem;
  FXTreeItem    *lastitem;
  FXTreeItem    *anchoritem;
  FXTreeItem    *currentitem;
  FXFont        *font;
  FXItemSortFunc sortfunc;
  FXPixel        textColor;
  FXPixel        selbackColor;
  FXPixel        seltextColor;
  FXPixel        lineColor;
  FXint          iconWidth;
  FXint          iconHeight;
  FXint          itemWidth;
  FXint          itemHeight;
  FXint          totalWidth;
  FXint          totalHeight;
  FXint          indent;
  FXString       tip;
  FXString       help;
protected:
  FXTreeList();
  FXTreeList(const FXTreeList&){}
  void recompute();
  virtual FXTreeItem* createItem();
  void drawFocusRectangle(FXint x,FXint y,FXint w,FXint h);
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
  long onItemOpened(FXObject*,FXSelector,void*);
  long onItemClosed(FXObject*,FXSelector,void*);
  long onItemExpanded(FXObject*,FXSelector,void*);
  long onItemCollapsed(FXObject*,FXSelector,void*);
  long onChanged(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);
  long onSelected(FXObject*,FXSelector,void*);
  long onDeselected(FXObject*,FXSelector,void*);
public:
  FXTreeList(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void recalc();
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  virtual FXbool canFocus() const;
  FXint getNumItems() const;
  FXint getNumChildItems(FXTreeItem* par) const;
  FXint getNumSelectedItems() const;
  FXTreeItem** getSelectedItems() const;
  FXTreeItem* getFirstItem() const { return firstitem; }
  FXTreeItem* getLastItem() const { return lastitem; }
  FXTreeItem* getNextItem(const FXTreeItem* item) const;
  FXTreeItem* getPrevItem(const FXTreeItem* item) const;
  FXTreeItem* getFirstChildItem(const FXTreeItem* item) const;
  FXTreeItem* getLastChildItem(const FXTreeItem* item) const;
  FXTreeItem* getParentItem(const FXTreeItem* item) const;
  FXTreeItem* getItemAbove(FXTreeItem* item) const;
  FXTreeItem* getItemBelow(FXTreeItem* item) const;
  virtual FXTreeItem* addItemFirst(FXTreeItem* p,const char* text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL);
  virtual FXTreeItem* addItemLast(FXTreeItem* p,const char* text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL);
  virtual FXTreeItem* addItemAfter(FXTreeItem* other,const char* text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL);
  virtual FXTreeItem* addItemBefore(FXTreeItem* other,const char* text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL);
  virtual void removeItem(FXTreeItem* item);
  virtual void removeItems(FXTreeItem* fm,FXTreeItem* to);
  virtual void removeAllItems();
  virtual FXTreeItem* getItemAt(FXint x,FXint y) const;
  FXbool openItem(FXTreeItem* item);
  FXbool closeItem(FXTreeItem* item);
  FXbool collapseTree(FXTreeItem* tree);
  FXbool expandTree(FXTreeItem* tree);
  virtual FXint getItemX(const FXTreeItem* item) const;
  virtual FXint getItemY(const FXTreeItem* item) const;
  virtual FXint getItemWidth(const FXTreeItem* item) const;
  virtual FXint getItemHeight(const FXTreeItem* item) const;
  void updateItem(FXTreeItem* item);
  void makeItemVisible(FXTreeItem* item);
  FXbool isItemSelected(const FXTreeItem* item) const;
  FXbool isItemCurrent(const FXTreeItem* item) const;
  FXbool isItemVisible(const FXTreeItem* item) const;
  FXbool isItemOpen(const FXTreeItem* item) const;
  FXbool isItemExpanded(const FXTreeItem* item) const;
  FXbool isItemLeaf(const FXTreeItem* item) const;
  FXbool hitItem(const FXTreeItem* item,FXint x,FXint y) const;
  FXbool hitItemBox(const FXTreeItem* item,FXint x,FXint y) const;
  void markItems();
  FXbool selectItemRange(FXTreeItem* beg,FXTreeItem* end);
  FXbool deselectItemRange(FXTreeItem* beg,FXTreeItem* end);
  FXbool toggleItemRange(FXTreeItem* beg,FXTreeItem* end);
  FXbool restoreItemRange(FXTreeItem* beg,FXTreeItem* end);
  FXbool selectItem(FXTreeItem* item);
  FXbool toggleItem(FXTreeItem* item);
  FXbool deselectItem(FXTreeItem* item);
  FXbool restoreItem(FXTreeItem* item);
  void sortChildItems(FXTreeItem* item);
  void sortItems();
  void setCurrentItem(FXTreeItem* item);
  FXTreeItem* getCurrentItem() const { return currentitem; }
  void setAnchorItem(FXTreeItem* item);
  FXTreeItem* getAnchorItem() const { return anchoritem; }
  FXbool extendSelection(FXTreeItem* item);
  void setItemText(FXTreeItem* item,const char* text);
  const char* getItemText(const FXTreeItem* item) const;
  void setItemOpenIcon(FXTreeItem* item,FXIcon* icon);
  FXIcon* getItemOpenIcon(const FXTreeItem* item) const;
  void setItemClosedIcon(FXTreeItem* item,FXIcon* icon);
  FXIcon* getItemClosedIcon(const FXTreeItem* item) const;
  void setItemData(FXTreeItem* item,void* ptr) const;
  void* getItemData(const FXTreeItem* item) const;
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  FXPixel getSelBackColor() const { return selbackColor; }
  void setSelBackColor(FXPixel clr);
  FXPixel getSelTextColor() const { return seltextColor; }
  void setSelTextColor(FXPixel clr);
  void setIconWidth(FXint w);
  FXint getIconWidth() const { return iconWidth; }
  void setIconHeight(FXint h);
  FXint getIconHeight() const { return iconHeight; }
  void setIndent(FXint in);
  FXint getIndent() const { return indent; }
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  FXItemSortFunc getSortFunc() const { return sortfunc; }
  void setSortFunc(FXItemSortFunc func){ sortfunc=func; }
  FXuint getListStyle() const;
  void setListStyle(FXuint style);
  virtual ~FXTreeList();
  };


#endif
