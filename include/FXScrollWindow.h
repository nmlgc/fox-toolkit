/********************************************************************************
*                                                                               *
*                         S c r o l l   W i n d o w                             *
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
* $Id: FXScrollWindow.h,v 1.15 1998/10/19 16:23:21 jvz Exp $                 *
********************************************************************************/
#ifndef FXSCROLLWINDOW_H
#define FXSCROLLWINDOW_H


// Scrollbar options
enum FXScrollStyle {
  SCROLLERS_NORMAL     = 0,
  HSCROLLER_ALWAYS     = 0x00008000,                        // Always show horizontal scrollers
  HSCROLLER_NEVER      = 0x00010000,                        // Never show horizontal scrollers
  VSCROLLER_ALWAYS     = 0x00020000,                        // Always show vertical scrollers
  VSCROLLER_NEVER      = 0x00040000,                        // Never show vertical scrollers
  HSCROLLING_ON        = 0,
  HSCROLLING_OFF       = HSCROLLER_NEVER|HSCROLLER_ALWAYS,  // Horizontal scrolling turned off
  VSCROLLING_ON        = 0,
  VSCROLLING_OFF       = VSCROLLER_NEVER|VSCROLLER_ALWAYS,  // Vertical scrolling turned off
  SCROLLERS_TRACK      = 0,
  SCROLLERS_DONT_TRACK = 0x00080000,                        // Scrollers don't track continuously
  SCROLLER_MASK        = HSCROLLER_ALWAYS|HSCROLLER_NEVER|VSCROLLER_ALWAYS|VSCROLLER_NEVER|SCROLLERS_DONT_TRACK
  };



// Base class for scrolled stuff
class FXScrollArea : public FXComposite {
  FXDECLARE(FXScrollArea)
protected:
  FXScrollbar    *horizontal;
  FXScrollbar    *vertical;
  FXScrollCorner *corner;
  FXTimer        *scrolltimer;
  FXint           viewport_w;
  FXint           viewport_h;
  FXint           content_w;
  FXint           content_h;
  FXint           pos_x;
  FXint           pos_y;
protected:
  FXScrollArea();
  FXScrollArea(const FXScrollArea&){}
  virtual void layout();
  FXbool startAutoScroll(FXint x,FXint y,FXbool onlywheninside=FALSE);
  void stopAutoScroll();
public:
  long onHScrollerChanged(FXObject*,FXSelector,void*);
  long onVScrollerChanged(FXObject*,FXSelector,void*);
  long onHScrollerDragged(FXObject*,FXSelector,void*);
  long onVScrollerDragged(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
public:
  FXScrollArea(FXComposite* p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void recalc();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXint getViewportHeight();
  virtual FXint getViewportWidth();
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  void setScrollStyle(FXuint style);
  FXuint getScrollStyle() const;
  FXbool isHorizontalScrollable() const;
  FXbool isVerticalScrollable() const;
  FXScrollbar* horizontalScrollbar() const { return horizontal; }
  FXScrollbar* verticalScrollbar() const { return vertical; }
  FXint getXPosition() const { return pos_x; }
  FXint getYPosition() const { return pos_y; }
  void setPosition(FXint x,FXint y);
  void getPosition(FXint& x,FXint& y) const { x=pos_x; y=pos_y; }
  virtual void moveContents(FXint x,FXint y);
  virtual ~FXScrollArea();
  };


// Automatic scroll area
class FXScrollWindow : public FXScrollArea {
  FXDECLARE(FXScrollWindow)
protected:
  FXScrollWindow(){}
  FXScrollWindow(const FXScrollWindow&){}
  virtual void layout();
public:
  FXScrollWindow(FXComposite* p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  FXWindow* contentWindow() const;
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  virtual void moveContents(FXint x,FXint y);
  };


#endif
