/********************************************************************************
*                                                                               *
*                               F o n t   O b j e c t                           *
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
* $Id: FXFont.h,v 1.3 1998/09/18 22:07:12 jvz Exp $                          *
********************************************************************************/
#ifndef FXFONT_H
#define FXFONT_H


class FXFont : public FXId {
  FXDECLARE(FXFont)
  friend class FXDrawable; 
protected:
  FXString      name;       // Name of the font
  XFontStruct  *font;       // Font information
protected:
  FXFont();
  FXFont(const FXFont&){}
public:
  FXFont(FXApp* a,const char* nm);
  virtual void create();
  virtual void destroy();
  FXbool isFontMono() const;
  FXint getFontWidth() const;
  FXint getFontHeight() const;
  FXint getFontAscent() const;
  FXint getFontDescent() const;
  FXint getTextWidth(const FXchar *text,FXuint n) const;
  FXint getTextHeight(const FXchar *text,FXuint n) const;
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXFont();
  };
  
    
#endif
