/********************************************************************************
*                                                                               *
*                  R a d i o   B u t t o n    O b j e c t                       *
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
* $Id: FXRadioButton.h,v 1.4 1998/10/30 04:49:05 jeroen Exp $                   *
********************************************************************************/
#ifndef FXRADIOBUTTON_H
#define FXRADIOBUTTON_H



// CheckButton flags
enum FXRadioButtonStyle {
  RADIOBUTTON_AUTOGRAY = 0x00800000,
  RADIOBUTTON_AUTOHIDE = 0x01000000
  };



// Radio button
class FXRadioButton : public FXLabel {
  FXDECLARE(FXRadioButton)
protected:
  FXuchar  check;
  FXuchar  oldcheck;
  FXPixel  radioColor;
  FXString tip;
  FXString help;
protected:
  FXRadioButton();
  FXRadioButton(const FXRadioButton&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onUncheckRadio(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onCheck(FXObject*,FXSelector,void*);
  long onUncheck(FXObject*,FXSelector,void*);
  long onUnknown(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
public:
  FXRadioButton(FXComposite* p,const char* text=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=JUSTIFY_NORMAL|ICON_BEFORE_TEXT,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual void enable();
  virtual void disable();
  virtual FXbool canFocus() const;
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setCheck(FXuint s=TRUE);
  FXuint getCheck() const { return check; }
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  FXPixel getRadioColor() const { return textColor; }
  void setRadioColor(FXPixel clr);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };


#endif
