/********************************************************************************
*                                                                               *
*                         T r e e   L i s t   W i d g e t                       *
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
* $Id: FXTreeList.h,v 1.50 2002/01/18 22:42:55 jeroen Exp $                     *
********************************************************************************/
#ifndef FXTREELIST_H
#define FXTREELIST_H

#ifndef FXSCROLLAREA_H
#include "FXScrollArea.h"
#endif



struct FXTimer;
class FXIcon;
class FXFont;
class FXTreeList;
class FXDirList;


/// Tree list styles
enum {
  TREELIST_EXTENDEDSELECT = 0,            /// Extended selection mode allows for drag-selection of ranges of items
  TREELIST_SINGLESELECT   = 0x00100000,   /// Single selection mode allows up to one item to be selected
  TREELIST_BROWSESELECT   = 0x00200000,   /// Browse selection mode enforces one single item to be selected at all times
  TREELIST_MULTIPLESELECT = 0x00300000,   /// Multiple selection mode is used for selection of individual items
  TREELIST_AUTOSELECT     = 0x00400000,   /// Automatically select under cursor
  TREELIST_SHOWS_LINES    = 0x00800000,   /// Lines shown
  TREELIST_SHOWS_BOXES    = 0x01000000,   /// Boxes to expand shown
  TREELIST_ROOT_BOXES     = 0x02000000,   /// Display root boxes also
  TREELIST_NORMAL         = TREELIST_EXTENDEDSELECT
  };


/// Tree list Item
class FXAPI FXTreeItem : public FXObject {
  FXDECLARE(FXTreeItem)
  friend class FXTreeList;
  friend class FXDirList;
protected:
  FXTreeItem *prev;
  FXTreeItem *next;
  FXTreeItem *parent;
  FXTreeItem *first;
  FXTreeItem *last;
  FXString    label;
  FXIcon     *openIcon;
  FXIcon     *closedIcon;
  void       *data;
  FXuint      state;
  FXint       x,y;
protected:
  FXTreeItem():prev(NULL),next(NULL),parent(NULL),first(NULL),last(NULL),openIcon(NULL),closedIcon(NULL),data(NULL),state(0),x(0),y(0){}
  virtual void draw(const FXTreeList* list,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
  virtual FXint hitItem(const FXTreeList* list,FXint x,FXint y) const;
protected:
  enum{
    SELECTED        = 1,
    FOCUS           = 2,
    DISABLED        = 4,
    OPENED          = 8,
    EXPANDED        = 16,
    HASITEMS        = 32,
    DRAGGABLE       = 64,
    OPENICONOWNED   = 128,
    CLOSEDICONOWNED = 256
    };
public:
  FXTreeItem(const FXString& text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL):prev(NULL),next(NULL),parent(NULL),first(NULL),last(NULL),label(text),openIcon(oi),closedIcon(ci),data(ptr),state(0),x(0),y(0){}
  FXTreeItem* getParent() const { return parent; }
  FXTreeItem* getNext() const { return next; }
  FXTreeItem* getPrev() const { return prev; }
  FXTreeItem* getFirst() const { return first; }
  FXTreeItem* getLast() const { return last; }
  FXTreeItem* getBelow() const;
  FXTreeItem* getAbove() const;
  FXint getNumChildren() const;
  virtual void setText(const FXString& txt){ label=txt; }
  FXString getText() const { return label; }
  virtual void setOpenIcon(FXIcon* icn){ openIcon=icn; }
  FXIcon* getOpenIcon() const { return openIcon; }
  virtual void setClosedIcon(FXIcon* icn){ closedIcon=icn; }
  FXIcon* getClosedIcon() const { return closedIcon; }
  void setData(void* ptr){ data=ptr; }
  void* getData() const { return data; }
  virtual void setFocus(FXbool focus);
  FXbool hasFocus() const { return (state&FOCUS)!=0; }
  virtual void setSelected(FXbool selected);
  FXbool isSelected() const { return (state&SELECTED)!=0; }
  virtual void setOpened(FXbool opened);
  FXbool isOpened() const { return (state&OPENED)!=0; }
  virtual void setExpanded(FXbool expanded);
  FXbool isExpanded() const { return (state&EXPANDED)!=0; }
  virtual void setEnabled(FXbool enabled);
  FXbool isEnabled() const { return (state&DISABLED)==0; }
  virtual void setDraggable(FXbool draggable);
  FXbool isDraggable() const { return (state&DRAGGABLE)!=0; }
  virtual void setIconOwned(FXuint owned=(OPENICONOWNED|CLOSEDICONOWNED));
  FXuint isIconOwned() const { return (state&(OPENICONOWNED|CLOSEDICONOWNED)); }
  virtual FXint getWidth(const FXTreeList* list) const;
  virtual FXint getHeight(const FXTreeList* list) const;
  virtual void create();
  virtual void detach();
  virtual void destroy();
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXTreeItem();
  };



/// Tree item collate function
typedef FXint (*FXTreeListSortFunc)(const FXTreeItem*,const FXTreeItem*);



/// Tree list Widget
class FXAPI FXTreeList : public FXScrollArea {
  FXDECLARE(FXTreeList)
protected:
  FXTreeItem        *firstitem;         // First root item
  FXTreeItem        *lastitem;          // Last root item
  FXTreeItem        *anchoritem;        // Selection anchor item
  FXTreeItem        *currentitem;       // Current item
  FXTreeItem        *extentitem;        // Selection extent
  FXTreeItem        *cursoritem;        // Item under cursor
  FXFont            *font;              // Font
  FXTreeListSortFunc sortfunc;          // Item sort function
  FXColor            textColor;         // Text color
  FXColor            selbackColor;      // Selected background color
  FXColor            seltextColor;      // Selected text color
  FXColor            lineColor;         // Line color
  FXint              treeWidth;         // Tree width
  FXint              treeHeight;        // Tree height
  FXint              visible;           // Number of visible items
  FXint              indent;            // Parent to child indentation
  FXint              grabx;             // Grab point x
  FXint              graby;             // Grab point y
  FXString           lookup;            // Lookup string
  FXTimer           *timer;             // Tip timer
  FXTimer           *lookuptimer;       // Lookup timer
  FXString           help;              // Help string
  FXbool             state;             // State of item
protected:
  FXTreeList();
  virtual void layout();
  virtual FXTreeItem* createItem(const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr);
  void sort(FXTreeItem*& f1,FXTreeItem*& t1,FXTreeItem*& f2,FXTreeItem*& t2,int n);
  void recompute();
private:
  FXTreeList(const FXTreeList&);
  FXTreeList& operator=(const FXTreeList&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onUngrabbed(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onTipTimer(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);
  long onCommand(FXObject*,FXSelector,void*);
  long onSelected(FXObject*,FXSelector,void*);
  long onDeselected(FXObject*,FXSelector,void*);
  long onOpened(FXObject*,FXSelector,void*);
  long onClosed(FXObject*,FXSelector,void*);
  long onExpanded(FXObject*,FXSelector,void*);
  long onCollapsed(FXObject*,FXSelector,void*);
  long onLookupTimer(FXObject*,FXSelector,void*);
public:
  static FXint ascending(const FXTreeItem* a,const FXTreeItem* b);
  static FXint descending(const FXTreeItem* a,const FXTreeItem* b);
public:
  enum {
    ID_TIPTIMER=FXScrollArea::ID_LAST,
    ID_LOOKUPTIMER,
    ID_LAST
    };
public:

  /// Construct a tree list with nvis visible items; the tree list is initially empty
  FXTreeList(FXComposite *p,FXint nvis,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=TREELIST_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

  /// Create server-side resources
  virtual void create();

  /// Detach server-side resources
  virtual void detach();

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();

  /// Compute and return content width
  virtual FXint getContentWidth();

  /// Return content height
  virtual FXint getContentHeight();

  /// Recalculate layout
  virtual void recalc();

  /// Tree list can receive focus
  virtual FXbool canFocus() const;

  /// Move the focus to this window
  virtual void setFocus();

  /// Remove the focus from this window
  virtual void killFocus();

  /// Return number of items
  FXint getNumItems() const;

  /// Return number of visible items
  FXint getNumVisible() const { return visible; }

  /// Change number of visible items
  void setNumVisible(FXint nvis);

  /// REturn first root item
  FXTreeItem* getFirstItem() const { return firstitem; }

  /// Return last root item
  FXTreeItem* getLastItem() const { return lastitem; }

  /// Prepend new [possibly subclassed] item as first child of p
  FXTreeItem* addItemFirst(FXTreeItem* p,FXTreeItem* item,FXbool notify=FALSE);

  /// Prepend new item with given text and optional icon, and user-data pointer as first child of p
  FXTreeItem* addItemFirst(FXTreeItem* p,const FXString& text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL,FXbool notify=FALSE);

  /// Append new [possibly subclassed] item as last child of p
  FXTreeItem* addItemLast(FXTreeItem* p,FXTreeItem* item,FXbool notify=FALSE);

  /// Append new item with given text and optional icon, and user-data pointer as last child of p
  FXTreeItem* addItemLast(FXTreeItem* p,const FXString& text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL,FXbool notify=FALSE);

  /// Append new [possibly subclassed] item after to other item
  FXTreeItem* addItemAfter(FXTreeItem* other,FXTreeItem* item,FXbool notify=FALSE);

  /// Append new item with given text and optional icon, and user-data pointer after to other item
  FXTreeItem* addItemAfter(FXTreeItem* other,const FXString& text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL,FXbool notify=FALSE);

  /// Prepend new [possibly subclassed] item prior to other item
  FXTreeItem* addItemBefore(FXTreeItem* other,FXTreeItem* item,FXbool notify=FALSE);

  /// Prepend new item with given text and optional icon, and user-data pointer prior to other item
  FXTreeItem* addItemBefore(FXTreeItem* other,const FXString& text,FXIcon* oi=NULL,FXIcon* ci=NULL,void* ptr=NULL,FXbool notify=FALSE);

  /// Remove item
  void removeItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Remove items in range [fm, to] inclusively
  void removeItems(FXTreeItem* fm,FXTreeItem* to,FXbool notify=FALSE);

  /// Remove all items from list
  void clearItems(FXbool notify=FALSE);

  /// Return item width
  FXint getItemWidth(const FXTreeItem* item) const;

  /// Return item height
  FXint getItemHeight(const FXTreeItem* item) const;

  /// Get item at x,y, if any
  virtual FXTreeItem* getItemAt(FXint x,FXint y) const;

  /**
  * Search items for item by name, starting from start item; the
  * flags argument controls the search direction, and case sensitivity.
  */
  FXTreeItem* findItem(const FXString& text,FXTreeItem* start=NULL,FXuint flags=SEARCH_FORWARD|SEARCH_WRAP) const;

  /// Scroll to make item visible
  void makeItemVisible(FXTreeItem* item);

  /// Change item's text
  void setItemText(FXTreeItem* item,const FXString& text);

  /// Return item's text
  FXString getItemText(const FXTreeItem* item) const;

  /// Change item's open icon
  void setItemOpenIcon(FXTreeItem* item,FXIcon* icon);

  /// Return item's open icon
  FXIcon* getItemOpenIcon(const FXTreeItem* item) const;

  /// Chance item's closed icon
  void setItemClosedIcon(FXTreeItem* item,FXIcon* icon);

  /// Return item's closed icon
  FXIcon* getItemClosedIcon(const FXTreeItem* item) const;

  /// Change item user-data pointer
  void setItemData(FXTreeItem* item,void* ptr) const;

  /// Return item user-data pointer
  void* getItemData(const FXTreeItem* item) const;

  /// Return TRUE if item is selected
  FXbool isItemSelected(const FXTreeItem* item) const;

  /// Return TRUE if item is current
  FXbool isItemCurrent(const FXTreeItem* item) const;

  /// Return TRUE if item is visible
  FXbool isItemVisible(const FXTreeItem* item) const;

  /// Return TRUE if item opened
  FXbool isItemOpened(const FXTreeItem* item) const;

  /// Return TRUE if item expanded
  FXbool isItemExpanded(const FXTreeItem* item) const;

  /// Return TRUE if item is a leaf-item, i.e. has no children
  FXbool isItemLeaf(const FXTreeItem* item) const;

  /// Return TRUE if item is enabled
  FXbool isItemEnabled(const FXTreeItem* item) const;

  /// Return item hit code: 0 outside, 1 icon, 2 text, 3 box
  FXint hitItem(const FXTreeItem* item,FXint x,FXint y) const;

  /// Repaint item
  void updateItem(FXTreeItem* item);

  /// Enable item
  FXbool enableItem(FXTreeItem* item);

  /// Disable item
  FXbool disableItem(FXTreeItem* item);

  /// Select item
  FXbool selectItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Deselect item
  FXbool deselectItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Toggle item selection
  FXbool toggleItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Open item
  FXbool openItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Close item
  FXbool closeItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Collapse tree
  FXbool collapseTree(FXTreeItem* tree,FXbool notify=FALSE);

  /// Expand tree
  FXbool expandTree(FXTreeItem* tree,FXbool notify=FALSE);

  /// Reparent item under parent p
  void reparentItem(FXTreeItem* item,FXTreeItem* p);

  /// Change current item
  void setCurrentItem(FXTreeItem* item,FXbool notify=FALSE);

  /// Return current item, if any
  FXTreeItem* getCurrentItem() const { return currentitem; }

  /// Change anchor item
  void setAnchorItem(FXTreeItem* item);

  /// Return anchor item, if any
  FXTreeItem* getAnchorItem() const { return anchoritem; }

  /// Return item under cursor, if any
  FXTreeItem* getCursorItem() const { return cursoritem; }

  /// Extend selection from anchor item to item
  FXbool extendSelection(FXTreeItem* item,FXbool notify=FALSE);

  /// Deselect all items
  FXbool killSelection(FXbool notify=FALSE);

  /// Sort root items
  void sortItems();

  /// Sort children of item
  void sortChildItems(FXTreeItem* item);

  /// Change text font
  void setFont(FXFont* fnt);

  /// Return text font
  FXFont* getFont() const { return font; }

  /// Change parent-child indent amount
  void setIndent(FXint in);

  /// Return parent-child indent amount
  FXint getIndent() const { return indent; }

  /// Return normal text color
  FXColor getTextColor() const { return textColor; }

  /// Change normal text color
  void setTextColor(FXColor clr);

  /// Return selected text background
  FXColor getSelBackColor() const { return selbackColor; }

  /// Change selected text background
  void setSelBackColor(FXColor clr);

  /// Return selected text color
  FXColor getSelTextColor() const { return seltextColor; }

  /// Change selected text color
  void setSelTextColor(FXColor clr);

  /// Return line color
  FXColor getLineColor() const { return lineColor; }

  /// Change line color
  void setLineColor(FXColor clr);

  /// Return sort function
  FXTreeListSortFunc getSortFunc() const { return sortfunc; }

  /// Change sort function
  void setSortFunc(FXTreeListSortFunc func){ sortfunc=func; }

  /// Return list style
  FXuint getListStyle() const;

  /// Change list style
  void setListStyle(FXuint style);

  /// Set the status line help text for this list
  void setHelpText(const FXString& text);

  /// Get the status line help text for this list
  FXString getHelpText() const { return help; }

  /// Save object to a stream
  virtual void save(FXStream& store) const;

  /// Load object from a stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXTreeList();
  };


#endif
