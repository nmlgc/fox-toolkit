/********************************************************************************
*                                                                               *
*                       F r a m e   W i n d o w   O b j e c t                   *
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
* $Id: FXFrame.h,v 1.5 1998/08/26 07:41:10 jeroen Exp $                         *
********************************************************************************/
#ifndef FXFRAME_H
#define FXFRAME_H



// Base Frame
class FXFrame : public FXWindow {
  FXDECLARE(FXFrame)
protected:
  FXPixel baseColor;
  FXPixel hiliteColor;
  FXPixel shadowColor;
  FXPixel borderColor;
  FXint   border;
protected:
  FXFrame();
  FXFrame(const FXFrame&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXFrame(FXComposite* p,FXuint opts=FRAME_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void drawBorderRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawSunkenRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRidgeRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawGrooveRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawFocusRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawFrame(FXint x,FXint y,FXint w,FXint h);
  void setFrameStyle(FXuint style);
  FXuint getFrameStyle() const;
  FXint getBorderWidth() const { return border; }
  FXPixel getHiliteColor() const { return hiliteColor; }
  FXPixel getShadowColor() const { return shadowColor; }
  FXPixel getBorderColor() const { return borderColor; }
  FXPixel getBaseColor() const { return baseColor; }
  void setHiliteColor(FXPixel clr);
  void setShadowColor(FXPixel clr);
  void setBorderColor(FXPixel clr);
  void setBaseColor(FXPixel clr);
  };



#endif
