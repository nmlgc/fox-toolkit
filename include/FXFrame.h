/********************************************************************************
*                                                                               *
*                       F r a m e   W i n d o w   W i d g e t                   *
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
* $Id: FXFrame.h,v 1.13 2002/01/18 22:42:53 jeroen Exp $                        *
********************************************************************************/
#ifndef FXFRAME_H
#define FXFRAME_H

#ifndef FXWINDOW_H
#include "FXWindow.h"
#endif



// Default padding
#define DEFAULT_PAD   2


/// Base Frame
class FXAPI FXFrame : public FXWindow {
  FXDECLARE(FXFrame)
protected:
  FXColor baseColor;
  FXColor hiliteColor;
  FXColor shadowColor;
  FXColor borderColor;
  FXint   padtop;
  FXint   padbottom;
  FXint   padleft;
  FXint   padright;
  FXint   border;
protected:
  FXFrame();
  void drawBorderRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawRaisedRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawSunkenRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawRidgeRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawGrooveRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawDoubleRaisedRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawDoubleSunkenRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
  void drawFrame(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);
private:
  FXFrame(const FXFrame&);
  FXFrame &operator=(const FXFrame&);
public:
  long onPaint(FXObject*,FXSelector,void*);
public:

  /// Construct frame window
  FXFrame(FXComposite* p,FXuint opts=FRAME_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();

  /// Change frame style
  void setFrameStyle(FXuint style);

  /// Get current frame style
  FXuint getFrameStyle() const;

  /// Get border width
  FXint getBorderWidth() const { return border; }

  /// Change top padding
  void setPadTop(FXint pt);

  /// Get top interior padding
  FXint getPadTop() const { return padtop; }

  /// Change bottom padding
  void setPadBottom(FXint pb);

  /// Get bottom interior padding
  FXint getPadBottom() const { return padbottom; }

  /// Change left padding
  void setPadLeft(FXint pl);

  /// Get left interior padding
  FXint getPadLeft() const { return padleft; }

  /// Change right padding
  void setPadRight(FXint pr);

  /// Get right interior padding
  FXint getPadRight() const { return padright; }

  /// Change highlight color
  void setHiliteColor(FXColor clr);

  /// Get highlight color
  FXColor getHiliteColor() const { return hiliteColor; }

  /// Change shadow color
  void setShadowColor(FXColor clr);

  /// Get shadow color
  FXColor getShadowColor() const { return shadowColor; }

  /// Change border color
  void setBorderColor(FXColor clr);

  /// Get border color
  FXColor getBorderColor() const { return borderColor; }

  /// Change base gui color
  void setBaseColor(FXColor clr);

  /// Get base gui color
  FXColor getBaseColor() const { return baseColor; }

  /// Save to stream
  virtual void save(FXStream& store) const;

  /// Load from stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXFrame();
  };



#endif
