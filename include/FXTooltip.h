/********************************************************************************
*                                                                               *
*                           T o o l t i p   W i d g e t                         *
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
* $Id: FXTooltip.h,v 1.10 1998/10/19 21:10:45 jvz Exp $                       *
********************************************************************************/
#ifndef FXTOOLTIP_H
#define FXTOOLTIP_H


// Tooltip styles
enum FXTooltipStyle {
  TOOLTIP_PERMANENT  = 0x00020000      // Tooltip stays up indefinitely
  };
  

// Hopefully Helpful Hint message
class FXTooltip : public FXShell {
  FXDECLARE(FXTooltip)
protected:
  FXString  label; 
  FXFont   *font;
  FXPixel   textColor;
  FXTimer  *timer;
protected:
  FXTooltip();
  FXTooltip(const FXTooltip&){}
  virtual FXbool doesOverrideRedirect() const;
  virtual FXbool doesSaveUnder() const;
  void place(FXint x,FXint y);
  void autoplace();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
  long onTipShow(FXObject*,FXSelector,void*);
  long onTipHide(FXObject*,FXSelector,void*);
  long onCmdGetStringValue(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
public:
  enum {
    ID_TIP_SHOW=FXShell::ID_LAST,
    ID_TIP_HIDE,
    ID_LAST
    };
public:
  FXTooltip(FXApp* a,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void show();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setText(const FXchar* text);
  const FXchar* getText() const { return label.text(); }
  void setFont(FXFont *fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  virtual ~FXTooltip();
  };


#endif
