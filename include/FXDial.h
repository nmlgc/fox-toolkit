/********************************************************************************
*                                                                               *
*                                D i a l   W i d g e t                          *
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
* $Id: FXDial.h,v 1.13 1998/09/18 22:07:11 jvz Exp $                         *
********************************************************************************/
#ifndef FXDIAL_H
#define FXDIAL_H


enum FXDialStyle {
  DIAL_VERTICAL   = 0,
  DIAL_HORIZONTAL = 0x00008000,                   // Horizontal
  DIAL_CYCLIC     = 0x00010000,                   // Value wraps around
  DIAL_HAS_NOTCH  = 0x00020000,                   // Dial has center notch
  DIAL_MASK       = DIAL_HORIZONTAL|DIAL_CYCLIC|DIAL_HAS_NOTCH
  };


// Dial
class FXDial : public FXCell {
  FXDECLARE( FXDial )
protected:
  FXint     range[2];                             // Reported data range
  FXPixel   notchColor;                           // Main notch color
  FXPixel   sideColor[8];                         // Dial side color
  FXint     notchangle;                           // Angle of main notch
  FXint     notchspacing;                         // Angle between notches
  FXint     notchoffset;                          // Notch offset
  FXint     dragpoint;                            // Place where clicked
  FXint     dragpos;                              // Value where clicked
  FXint     incr;                                 // Rate of change/revolution
  FXint     pos;                                  // Reported data position
protected:
  FXDial(){}
  FXDial(const FXDial&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void* );
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
  long onCmdSetRealValue(FXObject*,FXSelector,void*);
  long onCmdGetRealValue(FXObject*,FXSelector,void*);
public:
  FXDial(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool canFocus() const;
  void setRange(FXint lo,FXint hi);
  void getRange(FXint& lo,FXint& hi) const { lo=range[0]; hi=range[1]; }
  void setPosition(FXint p);
  FXint getPosition() const { return pos; }
  void setRevolutionIncrement(FXint i);
  FXint getRevolutionIncrement() const { return incr; }
  void setNotchSpacing(FXint spacing);
  FXint getNotchSpacing() const { return notchspacing; }
  void setNotchOffset(FXint offset);
  FXint getNotchOffset() const { return notchoffset; }
  FXuint getDialStyle() const;
  void setDialStyle(FXuint opts);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };

#endif
