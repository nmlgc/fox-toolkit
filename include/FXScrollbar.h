/********************************************************************************
*                                                                               *
*                       S c r o l l   B a r   W i d g e t                       *
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
* $Id: FXScrollbar.h,v 1.17 2002/01/18 22:42:54 jeroen Exp $                    *
********************************************************************************/
#ifndef FXSCROLLBAR_H
#define FXSCROLLBAR_H

#ifndef FXWINDOW_H
#include "FXWindow.h"
#endif



struct FXTimer;


/// Scrollbar styles
enum {
  SCROLLBAR_HORIZONTAL = 0x00020000,      /// Horizontally oriented
  SCROLLBAR_VERTICAL   = 0                /// Vertically oriented
  };



/**
* The scroll bar is used when a document has a larger content than may be made
* visible.  The range is the total size of the document, the page is the part
* of the document which is visible.  The size of the scrollbar thumb is adjusted
* to give feedback of the relative sizes of each.
* The scroll bar may be manipulated by the left mouse (normal scrolling), right
* mouse (vernier or fine-scrolling), or middle mouse (same as the left mouse only
* the scroll position can hop to the place where the click is made).
* Finally, if the mouse sports a wheel, the scroll bar can be manipulated by means
* of the mouse wheel as well.  Holding down the Control-key during wheel motion
* will cause the scrolling to go faster than normal.
* While moving the scroll bar, a message of type SEL_CHANGED will be sent to the
* target, and the message data will reflect the current position of type FXint.
* At the end of the interaction, the scroll bar will send a message of type
* SEL_COMMAND to notify the target of the final position.
*/
class FXAPI FXScrollbar : public FXWindow {
  FXDECLARE(FXScrollbar)
protected:
  FXint      range;           // Scrollable range
  FXint      page;            // Page size
  FXint      line;            // Line size
  FXint      pos;             // Position
  FXint      thumbsize;       // Thumb size
  FXint      thumbpos;        // Thumb position
  FXColor    hiliteColor;     // Hightlight color
  FXColor    shadowColor;     // Shadow color
  FXColor    borderColor;     // Border color
  FXTimer   *timer;           // Autoscroll timer
  FXint      dragpoint;       // Point where grabbed
  FXint      dragjump;        // Jump this much
  FXuchar    pressed;         // Action being undertaken
protected:
  FXScrollbar();
  void drawButton(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down);
  void drawLeftArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down);
  void drawRightArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down);
  void drawUpArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down);
  void drawDownArrow(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXbool down);
  virtual void layout();
private:
  FXScrollbar(const FXScrollbar&);
  FXScrollbar &operator=(const FXScrollbar&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onMouseWheel(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onUngrabbed(FXObject*,FXSelector,void*);
  long onTimeIncPix(FXObject*,FXSelector,void*);
  long onTimeIncLine(FXObject*,FXSelector,void*);
  long onTimeIncPage(FXObject*,FXSelector,void*);
  long onTimeDecPix(FXObject*,FXSelector,void*);
  long onTimeDecLine(FXObject*,FXSelector,void*);
  long onTimeDecPage(FXObject*,FXSelector,void*);
  long onTimeWheel(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
  long onCmdSetIntRange(FXObject*,FXSelector,void*);
  long onCmdGetIntRange(FXObject*,FXSelector,void*);
public:
  enum{
    ID_TIMEWHEEL=FXWindow::ID_LAST,
    ID_AUTOINC_LINE,
    ID_AUTODEC_LINE,
    ID_AUTOINC_PAGE,
    ID_AUTODEC_PAGE,
    ID_AUTOINC_PIX,
    ID_AUTODEC_PIX,
    ID_LAST
    };
public:

  /// Construct scroll bar
  FXScrollbar(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=SCROLLBAR_VERTICAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();

  /// Set content size range
  void setRange(FXint r);

  /// Return content size range
  FXint getRange() const { return range; }

  /// Set viewport page size
  void setPage(FXint p);

  /// Return page size
  FXint getPage() const { return page; }

  /// Set scoll increment for line
  void setLine(FXint l);

  /// Return line increment
  FXint getLine() const { return line; }

  /// Change current scroll position
  void setPosition(FXint p);

  /// return scroll position
  FXint getPosition() const { return pos; }

  /// Change highlight color
  void setHiliteColor(FXColor clr);

  /// Return highlight color
  FXColor getHiliteColor() const { return hiliteColor; }

  /// Change shadow color
  void setShadowColor(FXColor clr);

  /// Return shadow color
  FXColor getShadowColor() const { return shadowColor; }

  /// Change border color
  FXColor getBorderColor() const { return borderColor; }

  /// Return border color
  void setBorderColor(FXColor clr);

  /// Change the scrollbar style
  FXuint getScrollbarStyle() const;

  /// Get the current scrollbar style
  void setScrollbarStyle(FXuint style);

  /// Save to stream
  virtual void save(FXStream& store) const;

  /// Load from stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXScrollbar();
  };


/// Corner between scroll bars
class FXAPI FXScrollCorner : public FXWindow {
  FXDECLARE(FXScrollCorner)
protected:
  FXScrollCorner();
private:
  FXScrollCorner(const FXScrollCorner&);
  FXScrollCorner &operator=(const FXScrollCorner&);
public:
  long onPaint(FXObject*,FXSelector,void*);
public:

  /// Constructor
  FXScrollCorner(FXComposite* p);

  /// Can not be enabled
  virtual void enable();

  /// Can not be disabled
  virtual void disable();
  };


#endif
