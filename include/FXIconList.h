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
* $Id: FXIconList.h,v 1.39 1998/10/19 16:23:21 jvz Exp $                     *
********************************************************************************/
#ifndef FXICONLIST_H
#define FXICONLIST_H





// Icon List options
enum FXIconListStyle {
  ICONLIST_EXTENDEDSELECT = 0,                // Extended selection mode
  ICONLIST_SINGLESELECT   = 0x00100000,       // At most one selected item
  ICONLIST_BROWSESELECT   = 0x00200000,       // Always exactly one selected item
  ICONLIST_MULTIPLESELECT = ICONLIST_SINGLESELECT|ICONLIST_BROWSESELECT,  // Multiple selection mode
  ICONLIST_AUTOSELECT     = 0x00400000,       // Automatically select under cursor
  ICONLIST_DETAILED       = 0,                // List mode
  ICONLIST_MINI_ICONS     = 0x00800000,       // Mini Icon mode
  ICONLIST_BIG_ICONS      = 0x01000000,       // Big Icon mode
  ICONLIST_ROWS           = 0,                // Row-wise mode
  ICONLIST_COLUMNS        = 0x02000000,       // Column-wise mode
  ICONLIST_RANDOM         = 0x04000000,       // Random arrangement
  ICONLIST_WANTSELECTION  = 0x08000000,       // Claim the selection
  ICONLIST_MASK           = ICONLIST_MINI_ICONS|ICONLIST_BIG_ICONS|ICONLIST_COLUMNS|ICONLIST_RANDOM|ICONLIST_WANTSELECTION
  };

  
// Opaque item type
struct FXIconItem;



// Icon List object
class FXIconList : public FXScrollArea {
  FXDECLARE(FXIconList)
protected:
  FXHeader      *header;
  FXIconItem    *firstitem;
  FXIconItem    *lastitem;
  FXIconItem    *anchoritem;
  FXIconItem    *currentitem;
  FXFont        *font;
  FXItemSortFunc sortfunc;
  FXPixel        textColor;
  FXPixel        selbackColor;
  FXPixel        seltextColor;
  FXint          iconWidth;                 // Size of big icon
  FXint          iconHeight;
  FXint          miniIconWidth;             // Size of mini icon
  FXint          miniIconHeight;
  FXint          itemSpace;                 // Space for item label
  FXint          itemWidth;                 // Item size
  FXint          itemHeight;
  FXint          totalWidth;
  FXint          totalHeight;
  FXint          anchorx;                   // Rectangular selection
  FXint          anchory;
  FXint          currentx;
  FXint          currenty;
protected:
  FXIconList();
  FXIconList(const FXIconList&){}
  void drawFocusRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawBigIcon(FXint x,FXint y,FXIconItem* item);
  void drawMiniIcon(FXint x,FXint y,FXIconItem* item);
  void drawDetails(FXint x,FXint y,FXIconItem* item);
  static FXbool before(FXIconItem* a,FXIconItem* b);
  FXbool performSelectionChange(FXIconItem* item,FXuint code);
  FXbool performSelectionRectangle(FXint lx,FXint ty,FXint rx,FXint by,FXuint in,FXuint out);
  void drawLasso(FXint x0,FXint y0,FXint x1,FXint y1);
  virtual FXIconItem* createItem();
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
  long onCmdArrangeByRows(FXObject*,FXSelector,void*);
  long onUpdArrangeByRows(FXObject*,FXSelector,void*);
  long onCmdArrangeByColumns(FXObject*,FXSelector,void*);
  long onUpdArrangeByColumns(FXObject*,FXSelector,void*);
  long onCmdArrangeAtRandom(FXObject*,FXSelector,void*);
  long onUpdArrangeAtRandom(FXObject*,FXSelector,void*);
  long onCmdShowDetails(FXObject*,FXSelector,void*);
  long onUpdShowDetails(FXObject*,FXSelector,void*);
  long onCmdShowBigIcons(FXObject*,FXSelector,void*);
  long onUpdShowBigIcons(FXObject*,FXSelector,void*);
  long onCmdShowMiniIcons(FXObject*,FXSelector,void*);
  long onUpdShowMiniIcons(FXObject*,FXSelector,void*);
  long onHeaderChanged(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onChanged(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
  long onSelected(FXObject*,FXSelector,void*);
  long onDeselected(FXObject*,FXSelector,void*);
public:
  enum {
    ID_SHOW_DETAILS=FXScrollArea::ID_LAST,
    ID_SHOW_MINI_ICONS,
    ID_SHOW_BIG_ICONS,
    ID_ARRANGE_BY_ROWS,
    ID_ARRANGE_BY_COLUMNS,
    ID_ARRANGE_AT_RANDOM,
    ID_LAST
    };
public:
  FXIconList(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void recalc();
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  virtual FXbool canFocus() const;
  virtual FXint getViewportHeight();
  virtual void moveContents(FXint x,FXint y);
  FXint getNumItems() const;
  FXint getNumSelectedItems() const;
  FXIconItem** getSelectedItems() const;
  FXIconItem* getFirstItem() const { return firstitem; }
  FXIconItem* getLastItem() const { return lastitem; }
  FXIconItem* getNextItem(const FXIconItem* item) const;
  FXIconItem* getPrevItem(const FXIconItem* item) const;
  virtual FXIconItem* addItemFirst(const char* text,FXIcon* ic=NULL,FXIcon* mi=NULL,void* ptr=NULL);
  virtual FXIconItem* addItemLast(const char* text,FXIcon* ic=NULL,FXIcon* mi=NULL,void* ptr=NULL);
  virtual FXIconItem* addItemAfter(FXIconItem* other,const char* text,FXIcon* ic=NULL,FXIcon* mi=NULL,void* ptr=NULL);
  virtual FXIconItem* addItemBefore(FXIconItem* other,const char* text,FXIcon* ic=NULL,FXIcon* mi=NULL,void* ptr=NULL);
  virtual void removeItem(FXIconItem* item);
  virtual void removeItems(FXIconItem* fm,FXIconItem* to);
  virtual void removeAllItems();
  virtual FXIconItem* getItemAt(FXint x,FXint y) const;
  virtual FXint getItemX(const FXIconItem* item) const;
  virtual FXint getItemY(const FXIconItem* item) const;
  virtual FXint getItemWidth(const FXIconItem* item) const;
  virtual FXint getItemHeight(const FXIconItem* item) const;
  void updateItem(FXIconItem* item);
  void makeItemVisible(FXIconItem* item);
  FXbool isItemSelected(const FXIconItem* item) const;
  FXbool isItemCurrent(const FXIconItem* item) const;
  FXbool isItemVisible(const FXIconItem* item) const;
  void markItems();
  FXbool selectInRectangle(FXint x,FXint y,FXint w,FXint h);
  FXbool deselectInRectangle(FXint x,FXint y,FXint w,FXint h);
  FXbool toggleInRectangle(FXint x,FXint y,FXint w,FXint h);
  FXbool restoreInRectangle(FXint x,FXint y,FXint w,FXint h);
  FXbool selectItemRange(FXIconItem* beg,FXIconItem* end);
  FXbool deselectItemRange(FXIconItem* beg,FXIconItem* end);
  FXbool toggleItemRange(FXIconItem* beg,FXIconItem* end);
  FXbool restoreItemRange(FXIconItem* beg,FXIconItem* end);
  FXbool selectItem(FXIconItem* item);
  FXbool deselectItem(FXIconItem* item);
  FXbool toggleItem(FXIconItem* item);
  FXbool restoreItem(FXIconItem* item);
  void sortItems();
  void setCurrentItem(FXIconItem* item);
  FXIconItem* getCurrentItem() const { return currentitem; }
  void setAnchorItem(FXIconItem* item);
  FXIconItem* getAnchorItem() const { return anchoritem; }
  FXbool extendSelection(FXIconItem* item);
  void setItemText(FXIconItem* item,const char* text);
  const char* getItemText(const FXIconItem* item) const;
  void setItemMiniIcon(FXIconItem* item,FXIcon* icon);
  FXIcon* getItemMiniIcon(const FXIconItem* item) const;
  void setItemIcon(FXIconItem* item,FXIcon* icon);
  FXIcon* getItemIcon(const FXIconItem* item) const;
  void setItemData(FXIconItem* item,void* ptr) const;
  void* getItemData(const FXIconItem* item) const;
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
  void setItemSpace(FXint s);
  FXint getItemSpace() const { return itemSpace; }
  void setIconWidth(FXint w);
  FXint getIconWidth() const { return iconWidth; }
  void setIconHeight(FXint h);
  FXint getIconHeight() const { return iconHeight; }
  void setMiniIconWidth(FXint w);
  FXint getMiniIconWidth() const { return miniIconWidth; }
  void setMiniIconHeight(FXint h);
  FXint getMiniIconHeight() const { return miniIconHeight; }
  FXuint getListStyle() const;
  void setListStyle(FXuint style);
  virtual ~FXIconList();
  };



#endif
