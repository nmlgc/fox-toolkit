/********************************************************************************
*                                                                               *
*                         S c r o l l b a r   O b j e c t s                     *
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
* $Id: FXScrollbar.h,v 1.5 1998/10/29 05:38:12 jeroen Exp $                     *
********************************************************************************/
#ifndef FXSCROLLBAR_H
#define FXSCROLLBAR_H


// Scrollbar options
enum FXScrollbarOrientation {
  SCROLLBAR_HORIZONTAL = 0x00020000,
  SCROLLBAR_VERTICAL   = 0x00040000
  };

  
// Scrollbar item
class FXScrollbarItem : public FXFrame {
  FXDECLARE(FXScrollbarItem)
protected:
  FXint state;
protected:
  FXScrollbarItem(){}
  FXScrollbarItem(const FXScrollbarItem&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXScrollbarItem(FXComposite* p,FXuint opts=0);
  virtual void create();
  virtual void setState(FXint s);
  };



// Button with an arrow
class FXScrollbarArrow : public FXScrollbarItem {
  FXDECLARE(FXScrollbarArrow)
protected:
  FXScrollbarArrow(){}
  FXScrollbarArrow(const FXScrollbarArrow&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXScrollbarArrow(FXComposite* p,FXuint opts=0);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };



// Scroll bar
class FXScrollbar : public FXComposite {
  FXDECLARE(FXScrollbar)
protected:
  FXScrollbarArrow *increase;
  FXScrollbarArrow *decrease;
  FXScrollbarItem  *slider;
  FXTimer          *timer;
  FXint             thumbsize;
  FXint             thumbpos;
  FXint             dragpoint;
  FXint             range;
  FXint             page;
  FXint             line;
  FXint             pos;
protected:
  FXScrollbar(){}
  FXScrollbar(const FXScrollbar&){}
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onAnyBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onTimeIncLine(FXObject*,FXSelector,void*);
  long onTimeIncPage(FXObject*,FXSelector,void*);
  long onTimeDecLine(FXObject*,FXSelector,void*);
  long onTimeDecPage(FXObject*,FXSelector,void*);
public:
  enum{
    ID_AUTOINC_LINE=FXComposite::ID_LAST,
    ID_AUTODEC_LINE,
    ID_AUTOINC_PAGE,
    ID_AUTODEC_PAGE,
    ID_LAST
    };
public:
  FXScrollbar(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=SCROLLBAR_VERTICAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void create();
  FXScrollbarArrow* increaseButton() const { return increase; }
  FXScrollbarArrow* decreaseButton() const { return decrease; }
  FXScrollbarItem* sliderBar() const { return slider; }
  void setRange(FXint r);
  FXint getRange() const { return range; }
  void setPage(FXint p);
  FXint getPage() const { return page; }
  void setLine(FXint l);
  FXint getLine() const { return line; }
  void setPosition(FXint p);
  FXint getPosition() const { return pos; }
  void autoScrollInc();
  void autoScrollDec();
  void autoScrollPageInc();
  void autoScrollPageDec();
  void stopAutoScroll();
  FXint isAutoScrolling();
  virtual ~FXScrollbar();
  };


// Corner between scroll bars
class FXScrollCorner : public FXFrame {
  FXDECLARE(FXScrollCorner)
protected:
  FXScrollCorner(){}
  FXScrollCorner(const FXScrollCorner&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXScrollCorner(FXComposite* p);
  virtual void create();
  virtual void enable();
  virtual void disable();
  };

  
#endif
