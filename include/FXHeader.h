/********************************************************************************
*                                                                               *
*                          H e a d e r   O b j e c t                            *
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
* $Id: FXHeader.h,v 1.7 1998/09/15 19:41:05 jvz Exp $                        *
********************************************************************************/
#ifndef FXHEADER_H
#define FXHEADER_H


// Opaque header item
class FXHeaderItem;


// Splitter window 
class FXHeader : public FXFrame {
  FXDECLARE(FXHeader)
private:
  FXHeaderItem *firstitem;
  FXHeaderItem *lastitem;
  FXHeaderItem *before;
  FXFont*       font;
  FXPixel       textColor;
  FXint         split;
  FXint         off;
protected:
  FXHeader();
  FXHeader(const FXHeader&){}
  void moveSplit(FXint amount);
  void adjustLayout();
  void drawSplit(FXint pos);
  virtual FXHeaderItem* createItem();
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
public:
  FXHeader(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  FXint getNumItems() const;
  FXHeaderItem* getFirstItem() const { return firstitem; }
  FXHeaderItem* getLastItem() const { return lastitem; }
  FXHeaderItem* getNextItem(const FXHeaderItem* item) const;
  FXHeaderItem* getPrevItem(const FXHeaderItem* item) const;
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  FXHeaderItem* addItemFirst(const char* text,FXIcon* icon=NULL,FXint size=20,FXObject* tgt=NULL,FXSelector sel=0);
  FXHeaderItem* addItemLast(const char* text,FXIcon* icon=NULL,FXint size=20,FXObject* tgt=NULL,FXSelector sel=0);
  void removeItem(FXHeaderItem* item);
  void removeItems(FXHeaderItem* fm,FXHeaderItem* to);
  void removeAllItems();
  FXint getItemX(const FXHeaderItem* item) const;
  FXint getItemY(const FXHeaderItem* item) const;
  FXint getItemWidth(const FXHeaderItem* item) const;
  FXint getItemHeight(const FXHeaderItem* item) const;
  void setItemText(FXHeaderItem* item,const char* text);
  const char* getItemText(const FXHeaderItem* item) const;
  void setItemTarget(FXHeaderItem* item,FXObject* tgt);
  FXObject* getItemTarget(const FXHeaderItem* item) const;
  void setItemSelector(FXHeaderItem* item,FXSelector sel);
  FXSelector getItemSelector(const FXHeaderItem* item) const;
  void setItemIcon(FXHeaderItem* item,FXIcon* icon);
  FXIcon* getItemIcon(const FXHeaderItem* item) const;
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  virtual ~FXHeader();
  };


#endif
