/********************************************************************************
*                                                                               *
*                     A r r o w   B u t t o n    O b j e c t                    *
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
* $Id: FXArrowButton.h,v 1.4 1998/10/30 04:49:04 jeroen Exp $                   *
********************************************************************************/
#ifndef FXARROWBUTTON_H
#define FXARROWBUTTON_H



// Arrow options
enum FXArrowButtonStyle {
  ARROW_UP     = 0x04000000,
  ARROW_DOWN   = 0x08000000,
  ARROW_LEFT   = 0x10000000,
  ARROW_RIGHT  = 0x20000000,
  ARROW_REPEAT = 0x40000000,
  ARROW_MASK   = ARROW_UP|ARROW_DOWN|ARROW_LEFT|ARROW_RIGHT|ARROW_REPEAT
  };


// Button with an arrow
class FXArrowButton : public FXButton {
  FXDECLARE(FXArrowButton)
protected:
  FXTimer *repeater;
protected:
  FXArrowButton();
  FXArrowButton(const FXArrowButton&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onRepeat(FXObject*,FXSelector,void*);
public:
  enum {
    ID_REPEAT=FXButton::ID_LAST,
    ID_LAST
    };
public:
  FXArrowButton(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_RAISED|FRAME_THICK|ARROW_UP,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual ~FXArrowButton();
  };

  
#endif
