/********************************************************************************
*                                                                               *
*                             L a b e l   O b j e c t s                         *
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
* $Id: FXLabel.h,v 1.13 1998/09/25 23:23:37 jvz Exp $                        *
********************************************************************************/
#ifndef FXLABEL_H
#define FXLABEL_H

// Default padding
#define DEFAULT_PAD         2


// Separator Options
enum FXSeparatorStyle {
  SEPARATOR_NONE    = 0,
  SEPARATOR_GROOVE  = 0x00008000,
  SEPARATOR_RIDGE   = 0x00010000,
  SEPARATOR_LINE    = 0x00020000
  };


// Justification modes
enum FXJusifyStyle {
  JUSTIFY_NORMAL    = 0,
  JUSTIFY_CENTER_X  = 0,
  JUSTIFY_LEFT      = 0x00008000,
  JUSTIFY_RIGHT     = 0x00010000,
  JUSTIFY_HZ_APART  = (JUSTIFY_LEFT|JUSTIFY_RIGHT),
  JUSTIFY_CENTER_Y  = 0,
  JUSTIFY_TOP       = 0x00020000,
  JUSTIFY_BOTTOM    = 0x00040000,
  JUSTIFY_VT_APART  = (JUSTIFY_TOP|JUSTIFY_BOTTOM)
  };



// Relationship options for icon-labels
enum FXIconPosition {
  ICON_UNDER_TEXT   = 0,
  ICON_AFTER_TEXT   = 0x00080000,
  ICON_BEFORE_TEXT  = 0x00100000,
  ICON_ABOVE_TEXT   = 0x00200000,
  ICON_BELOW_TEXT   = 0x00400000,
  TEXT_OVER_ICON    = ICON_UNDER_TEXT,
  TEXT_AFTER_ICON   = ICON_BEFORE_TEXT,
  TEXT_BEFORE_ICON  = ICON_AFTER_TEXT,
  TEXT_ABOVE_ICON   = ICON_BELOW_TEXT,
  TEXT_BELOW_ICON   = ICON_ABOVE_TEXT
  };



// Horizontal separator
class FXHorizontalSeparator : public FXFrame {
  FXDECLARE(FXHorizontalSeparator)
protected:
  FXHorizontalSeparator(){}
  FXHorizontalSeparator(const FXHorizontalSeparator&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXHorizontalSeparator(FXComposite* p,FXuint opts=SEPARATOR_GROOVE|LAYOUT_FILL_X,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };



// Vertical separator
class FXVerticalSeparator : public FXFrame {
  FXDECLARE(FXVerticalSeparator)
protected:
  FXVerticalSeparator(){}
  FXVerticalSeparator(const FXVerticalSeparator&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXVerticalSeparator(FXComposite* p,FXuint opts=SEPARATOR_GROOVE|LAYOUT_FILL_Y,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };

  
// Cell
class FXCell : public FXFrame {
  FXDECLARE(FXCell)
protected:
  FXint    padtop;
  FXint    padbottom;
  FXint    padleft;
  FXint    padright;
protected:
  FXCell();
  FXCell(const FXCell&){}
  void just_x(FXint& tx,FXint& ix,FXint tw,FXint iw);
  void just_y(FXint& ty,FXint& iy,FXint th,FXint ih);
public:
  FXCell(FXComposite* p,FXuint opts=JUSTIFY_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  FXint getPadTop() const { return padtop; }
  FXint getPadBottom() const { return padbottom; }
  FXint getPadLeft() const { return padleft; }
  FXint getPadRight() const { return padright; }
  void setPadTop(FXint pt);
  void setPadBottom(FXint pb);
  void setPadLeft(FXint pl);
  void setPadRight(FXint pr);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };


// Label
class FXLabel : public FXCell {
  FXDECLARE(FXLabel)
protected:
  FXString label; 
  FXIcon*  icon;
  FXFont*  font;
  FXHotKey hotkey;
  FXint    hotoff;
  FXPixel  textColor;
protected:
  FXLabel();
  FXLabel(const FXLabel&){}
  FXint labelHeight() const;
  FXint labelWidth() const;
  void drawLabel(FXint tx,FXint ty,FXint tw,FXint th);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onHotKeyPress(FXObject*,FXSelector,void*);
  long onHotKeyRelease(FXObject*,FXSelector,void*);
  long onCmdGetStringValue(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
public:
  FXLabel(FXComposite* p,const char* text,FXIcon* ic=0,FXuint opts=(JUSTIFY_NORMAL|ICON_BEFORE_TEXT),FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setText(const FXchar* text);
  const FXchar* getText() const { return label.text(); }
  void setIcon(FXIcon* ic);
  FXIcon* getIcon() const { return icon; }
  void setFont(FXFont *fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXLabel();
  };



#endif
