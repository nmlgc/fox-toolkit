/********************************************************************************
*                                                                               *
*                             S p i n   B u t t o n                             *
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
* $Id: FXSpinner.h,v 1.6 1998/09/24 06:40:04 jeroen Exp $                       *
********************************************************************************/
#ifndef FXSPINNER_H
#define FXSPINNER_H



//  Options
enum FXSpinnerOptions {
  SPIN_NORMAL  =  0x00000000,
  SPIN_NUMBERS =  0x00000000,
  SPIN_CYCLIC  =  0x00008000,       // Cyclic spinner
  SPIN_NOTEXT  =  0x00010000,       // No text visible
  SPIN_NOMAX   =  0x00020000,       // Spin all the way up to infinity
  SPIN_NOMIN   =  0x00040000        // Spin all the way down to -infinity
  };

  
//  Spinner control
class FXSpinner : public FXPacker {
  FXDECLARE(FXSpinner)
protected:
  FXTextField   *textField;
  FXArrowButton *upButton;
  FXArrowButton *downButton;
  FXint          minVal;
  FXint          maxVal;
  FXint          incrementVal;
  FXint          currentVal;
protected:
  FXSpinner();
  FXSpinner(const FXSpinner&){}
  virtual void layout();
  void updateText();
public:
  long onIncrement(FXObject*,FXSelector,void*);
  long onDecrement(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
public:
  enum {
    ID_INCREMENT=FXComposite::ID_LAST,
    ID_DECREMENT,
    ID_LAST
    };
public:
  FXSpinner(FXComposite *p,FXint cols,FXObject *tgt=NULL,FXSelector sel=0,FXuint opts=SPIN_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void create();
  virtual void disable();
  virtual void enable();
  void increment();
  void decrement();
  FXbool isCyclic() const;
  void setCyclic(FXbool);
  FXbool isTextVisible() const;
  void setTextVisible(FXbool);
  void setValue(FXint value);
  FXint getValue() const { return currentVal; }
  void setMinMax(FXint minimum,FXint maximum);
  void getMinMax(FXint& minimum,FXint& maximum) const;
  FXint getMinValue() const { return minVal; }
  FXint getMaxValue() const { return maxVal; }
  void setIncrement(FXint increment);
  FXint getIncrement() const { return incrementVal; }
  void setText(const FXchar* text);
  const FXchar* getText() const;
  virtual ~FXSpinner();

  };

#endif
