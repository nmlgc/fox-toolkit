/********************************************************************************
*                                                                               *
*                          H e a d e r   W i d g e t                            *
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
* $Id: FXHeader.h,v 1.27 2002/01/18 22:42:53 jeroen Exp $                       *
********************************************************************************/
#ifndef FXHEADER_H
#define FXHEADER_H

#ifndef FXFRAME_H
#include "FXFrame.h"
#endif



struct FXTimer;
class FXIcon;
class FXFont;
class FXHeader;


/// Header style options
enum {
  HEADER_BUTTON     = 0x00008000,     /// Button style can be clicked
  HEADER_HORIZONTAL = 0,              /// Horizontal header control (default)
  HEADER_VERTICAL   = 0x00010000,     /// Vertical header control
  HEADER_TRACKING   = 0x00020000,     /// Tracks continuously while moving
  HEADER_NORMAL     = HEADER_HORIZONTAL|FRAME_NORMAL
  };


/// Header item
class FXAPI FXHeaderItem : public FXObject {
  FXDECLARE(FXHeaderItem)
  friend class FXHeader;
protected:
  FXString  label;
  FXIcon   *icon;
  FXint     size;
  void     *data;
  FXbool    arrow;
protected:
  FXHeaderItem(){}
  virtual void draw(const FXHeader* header,FXDC& dc,FXint x,FXint y,FXint w,FXint h);
public:

  /// Construct new item with given text, icon, size, and user-data
  FXHeaderItem(const FXString& text,FXIcon* ic=NULL,FXint s=0,void* ptr=NULL):label(text),icon(ic),size(s),data(ptr),arrow(MAYBE){}

  /// Change item's text label
  virtual void setText(const FXString& txt){ label=txt; }

  /// Return item's text label
  FXString getText() const { return label; }

  /// Change item's icon
  virtual void setIcon(FXIcon* icn){ icon=icn; }

  /// Return item's icon
  FXIcon* getIcon() const { return icon; }

  /// Change item's user data
  void setData(void* ptr){ data=ptr; }

  /// Get item's user data
  void* getData() const { return data; }

  /// Change size
  void setSize(FXint s){ size=s; }

  /// Obtain current size
  FXint getSize() const { return size; }

  /// Change sort direction (FALSE, TRUE, MAYBE)
  void setArrowDir(FXuint dir=MAYBE){ arrow=dir; }

  /// Return sort direction (FALSE, TRUE, MAYBE)
  FXuint getArrowDir() const { return arrow; }

  /// Return the item's width in the header
  virtual FXint getWidth(const FXHeader* header) const;

  /// Return the item's height in the header
  virtual FXint getHeight(const FXHeader* header) const;

  /// Create server-side resources
  virtual void create();

  /// Detach from server-side resources
  virtual void detach();

  /// Destroy server-side resources
  virtual void destroy();

  /// Stream serialization
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXHeaderItem(){}
  };


/**
* Header control may be placed over a table or list to provide a resizable
* captions above a number of columns.
* Each caption comprises a label and an optional icon; in addition, an arrow
* may be shown to indicate whether the items in that column are sorted, and
* if so, whether they are sorted in increasing or decreasing order.
* Each caption can be interactively resized.  During the resizing, if the
* HEADER_TRACKING was specified, the header control sends a SEL_CHANGED message
* to its target, with the message data set to the caption number being resized,
* of the type FXint.
* If the HEADER_TRACKING was not specified the SEL_CHANGED message is sent at
* the end of the resizing operation.
* Clicking on a caption causes a message of type SEL_COMMAND to be sent to the
* target, with the message data set to the caption number being clicked.
*/
class FXAPI FXHeader : public FXFrame {
  FXDECLARE(FXHeader)
protected:
  FXHeaderItem **items;               // Item list
  FXint          nitems;              // Number of items
  FXColor        textColor;           // Text color
  FXFont*        font;                // Text font
  FXTimer       *timer;               // Tip hover timer
  FXString       help;                // Help text
  FXbool         state;               // Button state
  FXint          active;              // Active button
  FXint          activepos;           // Position of active item
  FXint          activesize;          // Size of active item
  FXint          off;
protected:
  FXHeader();
  void drawSplit(FXint pos);
  virtual FXHeaderItem *createItem(const FXString& text,FXIcon* icon,FXint size,void* ptr);
  virtual void layout();
private:
  FXHeader(const FXHeader&);
  FXHeader &operator=(const FXHeader&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onUngrabbed(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onTipTimer(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
public:
  enum {
    ID_TIPTIMER=FXFrame::ID_LAST,
    ID_LAST
    };
public:

  /// Construct new header control
  FXHeader(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=HEADER_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

  /// Create server-side resources
  virtual void create();

  /// Detach server-side resources
  virtual void detach();

  /// Return number of items
  FXint getNumItems() const { return nitems; }

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();

  /// Return item at given index
  FXHeaderItem *retrieveItem(FXint index) const;

  /// Replace the item with a [possibly subclassed] item
  FXint replaceItem(FXint index,FXHeaderItem* item,FXbool notify=FALSE);

  /// Replace items text, icon, and user-data pointer
  FXint replaceItem(FXint index,const FXString& text,FXIcon *icon=NULL,FXint size=0,void* ptr=NULL,FXbool notify=FALSE);

  /// Insert a new [possibly subclassed] item at the give index
  FXint insertItem(FXint index,FXHeaderItem* item,FXbool notify=FALSE);

  /// Insert item at index with given text, icon, and user-data pointer
  FXint insertItem(FXint index,const FXString& text,FXIcon *icon=NULL,FXint size=0,void* ptr=NULL,FXbool notify=FALSE);

  /// Append a [possibly subclassed] item to the list
  FXint appendItem(FXHeaderItem* item,FXbool notify=FALSE);

  /// Append new item with given text and optional icon, and user-data pointer
  FXint appendItem(const FXString& text,FXIcon *icon=NULL,FXint size=0,void* ptr=NULL,FXbool notify=FALSE);

  /// Prepend a [possibly subclassed] item to the list
  FXint prependItem(FXHeaderItem* item,FXbool notify=FALSE);

  /// Prepend new item with given text and optional icon, and user-data pointer
  FXint prependItem(const FXString& text,FXIcon *icon=NULL,FXint size=0,void* ptr=NULL,FXbool notify=FALSE);

  /// Remove item at index
  void removeItem(FXint index,FXbool notify=FALSE);

  /// Remove all items
  void clearItems(FXbool notify=FALSE);

  /// Return item-index given pixel-offset from left
  FXint getItemAt(FXint offset) const;

  /// Change text label for item at index
  void setItemText(FXint index,const FXString& text);

  /// Get text of item at index
  FXString getItemText(FXint index) const;

  /// Change icon of item at index
  void setItemIcon(FXint index,FXIcon* icon);

  /// Return icon of item at index
  FXIcon* getItemIcon(FXint index) const;

  /// Change size of item at index
  void setItemSize(FXint index,FXint size);

  /// Return size of item at index
  FXint getItemSize(FXint index) const;

  /// Compute offset from the left side of item at index
  FXint getItemOffset(FXint index) const;

  /// Change data of item at index
  void setItemData(FXint index,void* ptr);

  /// Return data of item at index
  void* getItemData(FXint index) const;

  /// Change sort direction (FALSE, TRUE, MAYBE)
  void setArrowDir(FXint index,FXbool dir=MAYBE);

  /// Return sort direction (FALSE, TRUE, MAYBE)
  FXbool getArrowDir(FXint index) const;

  /// Change text font
  void setFont(FXFont* fnt);

  /// return text font
  FXFont* getFont() const { return font; }

  /// Return text color
  FXColor getTextColor() const { return textColor; }

  /// Change text color
  void setTextColor(FXColor clr);

  /// Set header style options
  void setHeaderStyle(FXuint style);

  /// Get header style options
  FXuint getHeaderStyle() const;

  /// Set the status line help text for this header
  void setHelpText(const FXString& text);

  /// Get the status line help text for this header
  FXString getHelpText() const { return help; }

  /// Save header to a stream
  virtual void save(FXStream& store) const;

  /// Load header from a stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXHeader();
  };


#endif
