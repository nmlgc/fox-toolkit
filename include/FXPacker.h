/********************************************************************************
*                                                                               *
*                P a c k e r   C o n t a i n e r   O b j e c t                  *
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
* $Id: FXPacker.h,v 1.8 1998/09/18 22:07:12 jvz Exp $                        *
********************************************************************************/
#ifndef FXPACKER_H
#define FXPACKER_H

// Default spacing
#define DEFAULT_SPACING        4

  

// Packs children in cavity
class FXPacker : public FXComposite {
  FXDECLARE(FXPacker)
protected:
  FXPixel baseColor;
  FXPixel hiliteColor;
  FXPixel shadowColor;
  FXPixel borderColor;
  FXint   padtop;
  FXint   padbottom;
  FXint   padleft;
  FXint   padright;
  FXint   hspacing;
  FXint   vspacing;
  FXint   border;
protected:
  FXPacker(){}
  FXPacker(const FXPacker&){}
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXPacker(FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
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
  FXPixel getHiliteColor() const { return hiliteColor; }
  FXPixel getShadowColor() const { return shadowColor; }
  FXPixel getBorderColor() const { return borderColor; }
  FXPixel getBaseColor() const { return baseColor; }
  void setHiliteColor(FXPixel clr);
  void setShadowColor(FXPixel clr);
  void setBorderColor(FXPixel clr);
  void setBaseColor(FXPixel clr);
  FXint getPadTop() const { return padtop; }
  FXint getPadBottom() const { return padbottom; }
  FXint getPadLeft() const { return padleft; }
  FXint getPadRight() const { return padright; }
  void setPadTop(FXint pt);
  void setPadBottom(FXint pb);
  void setPadLeft(FXint pl);
  void setPadRight(FXint pr);
  FXint getHSpacing() const { return hspacing; }
  FXint getVSpacing() const { return vspacing; }
  void setHSpacing(FXint hs);
  void setVSpacing(FXint vs);
  FXint getBorderWidth() const { return border; }
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };

  
#endif
