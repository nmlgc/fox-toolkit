/********************************************************************************
*                                                                               *
*                         C o l o r W e l l   C l a s s                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXColorWell.h,v 1.14 1998/08/26 07:41:10 jeroen Exp $                    *
********************************************************************************/
#ifndef FXCOLORWELL_H
#define FXCOLORWELL_H



// Cell
class FXColorWell : public FXCell {
  FXDECLARE(FXColorWell)
protected:
  FXPixel    wellColor[2];            // Pixel value of RGBA over black and white
  FXColor    rgba;                    // Color with RGB and Alpha
protected:
  static FXDragType colorDragType;    // Drag type
protected:
  FXColorWell();
  FXColorWell(const FXColorWell&){}
  FXPixel rgbaoverblack(FXColor clr);
  FXPixel rgbaoverwhite(FXColor clr);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onDNDEnter(FXObject*,FXSelector,void*);
  long onDNDLeave(FXObject*,FXSelector,void*);
  long onDNDMotion(FXObject*,FXSelector,void*);
  long onDNDDrop(FXObject*,FXSelector,void*);
  long onSelectionRequest(FXObject*,FXSelector,void*);
public:
  static const FXchar colorDragTypeName[];
public:
  FXColorWell(FXComposite* p,FXColor clr=0,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=JUSTIFY_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool canFocus() const;
  void setRGBA(FXColor clr);
  FXColor getRGBA() const;
  };


#endif
