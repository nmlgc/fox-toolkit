/********************************************************************************
*                                                                               *
*                           B u t t o n    O b j e c t s                        *
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
* $Id: FXButton.h,v 1.31 1998/11/02 02:08:27 jeroen Exp $                       *
********************************************************************************/
#ifndef FXBUTTON_H
#define FXBUTTON_H



// Button state bits
enum FXButtonState {
  STATE_UP        = 0,
  STATE_DOWN      = 1,
  STATE_ENGAGED   = 2,
  STATE_UNCHECKED = STATE_UP,
  STATE_CHECKED   = STATE_ENGAGED
  };


// Button flags
enum FXButtonStyle {
  BUTTON_AUTOGRAY = 0x00800000,
  BUTTON_AUTOHIDE = 0x01000000,
  BUTTON_TOOLBAR  = 0x02000000
  };


// Your basic button
class FXButton : public FXLabel {
  FXDECLARE(FXButton)
protected:
  FXuchar  state;
  FXString tip;
  FXString help;
protected:
  FXButton();
  FXButton(const FXButton&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onCheck(FXObject*,FXSelector,void*);
  long onUncheck(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
public:
  FXButton(FXComposite* p,const char* text=NULL,FXIcon* ic=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_RAISED|FRAME_THICK|JUSTIFY_NORMAL|ICON_BEFORE_TEXT,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual void enable();
  virtual void disable();
  virtual FXbool canFocus() const;
  void setState(FXuint s);
  FXuint getState() const { return state; }
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };

  
#endif
