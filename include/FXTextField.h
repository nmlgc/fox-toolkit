/********************************************************************************
*                                                                               *
*                           T e x t F i e l d   O b j e c t                     *
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
* $Id: FXTextField.h,v 1.14 1998/10/05 01:20:02 jeroen Exp $                    *
********************************************************************************/
#ifndef FXTEXTFIELD_H
#define FXTEXTFIELD_H




class FXTextField : public FXCell {
  FXDECLARE(FXTextField)
protected:
  FXString     contents;
  FXFont*      font;
  FXPixel      textColor;
  FXPixel      selbackColor;
  FXPixel      seltextColor;
  FXint        cursor;
  FXint        anchor;
  FXTimer     *blinker;
  FXuint       columns;
  FXint        scroll;
  FXString     help;
  FXString     tip;
protected:
  FXTextField(){}
  FXTextField(const FXTextField&){}
  FXint index(FXint x);
  FXint coord(FXint i);
  void drawCursor(FXuint state);
  void drawTextRange(FXuint fm,FXuint to);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onSelectionRequest(FXObject*,FXSelector,void* ptr);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onBlink(FXObject*,FXSelector,void*);
  long onAutoLeftScroll(FXObject*,FXSelector,void*);
  long onAutoRightScroll(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
long onCmdSetValue(FXObject*,FXSelector,void*);/// old
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdSetRealValue(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetRealValue(FXObject*,FXSelector,void*);
  long onCmdGetStringValue(FXObject*,FXSelector,void*);
public:
  FXTextField(FXComposite* p,FXuint cols,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_SUNKEN|FRAME_THICK,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual void enable();
  virtual void disable();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool canFocus() const;
  void setCursorPos(FXint pos);
  FXint getCursorPos() const { return cursor; }
  void setAnchorPos(FXint pos);
  FXint getAnchorPos() const { return anchor; }
  void setText(const FXchar* text);
  const FXchar* getText() const { return contents.text(); }
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  FXPixel getSelBackColor() const { return selbackColor; }
  void setSelBackColor(FXPixel clr);
  FXPixel getSelTextColor() const { return seltextColor; }
  void setSelTextColor(FXPixel clr);
  void setNumColumns(FXuint cols);
  FXuint getNumColumns() const { return columns; }
  void killSelection();
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  virtual ~FXTextField();
  };


#endif
