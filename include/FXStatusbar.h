/********************************************************************************
*                                                                               *
*                         S t a t u s b a r   O b j e c t s                     *
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
* $Id: FXStatusbar.h,v 1.7 1998/10/19 21:10:45 jvz Exp $                     *
********************************************************************************/
#ifndef FXSTATUSBAR_H
#define FXSTATUSBAR_H

enum FXStatusbarStyle {
  STATUSBAR_WITH_DRAGCORNER = 0x00008000
  };


// Drag corner
class FXDragCorner : public FXFrame {
  FXDECLARE(FXDragCorner)
protected:
  FXint   oldw;
  FXint   oldh;
  FXint   xoff;
  FXint   yoff;
protected:
  FXDragCorner(){}
  FXDragCorner(const FXDragCorner&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
public:
  FXDragCorner(FXComposite* p);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };

  
// Status line
class FXStatusline : public FXFrame {
  FXDECLARE(FXStatusline)
protected:
  FXString  status;             // Status text
  FXFont   *font;               // Font
  FXPixel   textColor;          // Status text color
protected:
  FXStatusline(){}
  FXStatusline(const FXStatusline&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
  long onCmdGetStringValue(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
public:
  FXStatusline(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setText(const FXchar *text);
  const FXchar* getText() const { return status.text(); }
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  virtual ~FXStatusline();
  };

  
// Scroll bar
class FXStatusbar : public FXComposite {
  FXDECLARE(FXStatusbar)
protected:
  FXDragCorner *corner;
  FXStatusline *status;
protected:
  FXStatusbar(){}
  FXStatusbar(const FXStatusbar&){}
  virtual void layout();
public:
  FXStatusbar(FXComposite* p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  FXStatusline *getStatusline() const { return status; }
  FXDragCorner *getDragCorner() const { return corner; }
  virtual ~FXStatusbar();
  };


#endif
