/********************************************************************************
*                                                                               *
*                G r o u p  B o x   W i n d o w   O b j e c t                   *
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
* $Id: FXGroupBox.h,v 1.6 1998/08/26 07:41:10 jeroen Exp $                      *
********************************************************************************/
#ifndef FXGROUPBOX_H
#define FXGROUPBOX_H



// Group box options
enum FXGroupboxStyle {
  GROUPBOX_TITLE_NONE   = 0,
  GROUPBOX_TITLE_LEFT   = 0x00020000,
  GROUPBOX_TITLE_CENTER = 0x00040000,
  GROUPBOX_TITLE_RIGHT  = 0x00080000,
  GROUPBOX_TITLE_MASK   = GROUPBOX_TITLE_LEFT|GROUPBOX_TITLE_CENTER|GROUPBOX_TITLE_RIGHT
  };



// Group box 
class FXGroupBox : public FXPacker {
  FXDECLARE(FXGroupBox)
protected:
  FXString  label; 
  FXFont   *font;
  FXPixel   textColor;
protected:
  FXGroupBox();
  FXGroupBox(const FXGroupBox&){}
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onUncheckOther(FXObject*,FXSelector,void*);
public:
  FXGroupBox(FXComposite* p,const char* text,FXuint opts=GROUPBOX_TITLE_LEFT,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setText(const FXchar* text);
  const FXchar* getText() const { return label.text(); }
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  };


#endif
