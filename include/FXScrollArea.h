/********************************************************************************
*                                                                               *
*                       S c r o l l A r e a   W i d g e t                       *
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
* $Id: FXScrollArea.h,v 1.15 2002/01/18 22:42:54 jeroen Exp $                   *
********************************************************************************/
#ifndef FXSCROLLAREA_H
#define FXSCROLLAREA_H

#ifndef FXCOMPOSITE_H
#include "FXComposite.h"
#endif


/// Scrollbar options
enum {
  SCROLLERS_NORMAL     = 0,                                 /// Show the scrollbars when needed
  HSCROLLER_ALWAYS     = 0x00008000,                        /// Always show horizontal scrollers
  HSCROLLER_NEVER      = 0x00010000,                        /// Never show horizontal scrollers
  VSCROLLER_ALWAYS     = 0x00020000,                        /// Always show vertical scrollers
  VSCROLLER_NEVER      = 0x00040000,                        /// Never show vertical scrollers
  HSCROLLING_ON        = 0,                                 /// Horizontal scrolling turned on (default)
  HSCROLLING_OFF       = HSCROLLER_NEVER|HSCROLLER_ALWAYS,  /// Horizontal scrolling turned off
  VSCROLLING_ON        = 0,                                 /// Vertical scrolling turned on (default)
  VSCROLLING_OFF       = VSCROLLER_NEVER|VSCROLLER_ALWAYS,  /// Vertical scrolling turned off
  SCROLLERS_TRACK      = 0,                                 /// Scrollers track continuously for smooth scrolling
  SCROLLERS_DONT_TRACK = 0x00080000                         /// Scrollers don't track continuously
  };


class FXScrollbar;
class FXScrollCorner;
struct FXTimer;


/**
* The scroll area widget manages a content area and a viewport
* area through which the content is viewed.  When the content area
* becomes larger than the viewport area, scrollbars are placed to
* permit viewing of the entire content by scrolling the content.
* Depending on the mode, scrollbars may be displayed on an as-needed
* basis, always, or never.
* Normally, the scroll area's size and the content's size are independent;
* however, it is possible to disable scrolling in the horizontal
* (vertical) direction.  In this case, the content width (height)
* will influence the width (height) of the scroll area widget.
* For content which is time-consuming to repaint, continuous
* scrolling may be turned off.
*/
class FXAPI FXScrollArea : public FXComposite {
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
  FXbool startAutoScroll(FXint x,FXint y,FXbool onlywheninside=FALSE);
  void stopAutoScroll();
  virtual void layout();
  FXScrollArea(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h);
private:
  FXScrollArea(const FXScrollArea&);
  FXScrollArea &operator=(const FXScrollArea&);
public:
  long onHMouseWheel(FXObject*,FXSelector,void*);
  long onVMouseWheel(FXObject*,FXSelector,void*);
  long onHScrollerChanged(FXObject*,FXSelector,void*);
  long onVScrollerChanged(FXObject*,FXSelector,void*);
  long onHScrollerDragged(FXObject*,FXSelector,void*);
  long onVScrollerDragged(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
public:

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();

  /// Return viewport size
  virtual FXint getViewportHeight();
  virtual FXint getViewportWidth();

  /// Return content size
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();

  /// Change scroll style
  void setScrollStyle(FXuint style);

  /// Return scroll style
  FXuint getScrollStyle() const;

  /// Return TRUE if horizontally scrollable
  FXbool isHorizontalScrollable() const;

  /// Return TRUE if vertically scrollable
  FXbool isVerticalScrollable() const;

  /// Return a pointer to the horizontal scrollbar
  FXScrollbar* horizontalScrollbar() const { return horizontal; }

  /// Return a pointer to the vertical scrollbar
  FXScrollbar* verticalScrollbar() const { return vertical; }

  /// Return the current x-position
  FXint getXPosition() const { return pos_x; }

  /// Return the current y-position
  FXint getYPosition() const { return pos_y; }

  /// Set the current position
  void setPosition(FXint x,FXint y);

  /// Get the current position
  void getPosition(FXint& x,FXint& y) const { x=pos_x; y=pos_y; }

  /// Move contents to the specified position
  virtual void moveContents(FXint x,FXint y);

  /// Destructor
  virtual ~FXScrollArea();
  };



#endif
