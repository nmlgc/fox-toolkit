/********************************************************************************
*                                                                               *
*                      S e p a r a t o r   W i d g e t s                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXSeparator.h,v 1.9 2002/01/18 22:42:54 jeroen Exp $                     *
********************************************************************************/
#ifndef FXSEPARATOR_H
#define FXSEPARATOR_H

#ifndef FXFRAME_H
#include "FXFrame.h"
#endif



/// Separator Options
enum {
  SEPARATOR_NONE       = 0,               /// Nothing visible
  SEPARATOR_GROOVE     = 0x00008000,      /// Etched-in looking groove
  SEPARATOR_RIDGE      = 0x00010000,      /// Embossed looking ridge
  SEPARATOR_LINE       = 0x00020000       /// Simple line
  };


/// Horizontal separator
class FXAPI FXHorizontalSeparator : public FXFrame {
  FXDECLARE(FXHorizontalSeparator)
protected:
  FXHorizontalSeparator(){}
private:
  FXHorizontalSeparator(const FXHorizontalSeparator&);
  FXHorizontalSeparator &operator=(const FXHorizontalSeparator&);
public:
  long onPaint(FXObject*,FXSelector,void*);
public:

  /// Constructor
  FXHorizontalSeparator(FXComposite* p,FXuint opts=SEPARATOR_GROOVE|LAYOUT_FILL_X,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=1,FXint pr=1,FXint pt=0,FXint pb=0);

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();
  };


/// Vertical separator
class FXAPI FXVerticalSeparator : public FXFrame {
  FXDECLARE(FXVerticalSeparator)
protected:
  FXVerticalSeparator(){}
private:
  FXVerticalSeparator(const FXVerticalSeparator&);
  FXVerticalSeparator &operator=(const FXVerticalSeparator&);
public:
  long onPaint(FXObject*,FXSelector,void*);
public:

  /// Constructor
  FXVerticalSeparator(FXComposite* p,FXuint opts=SEPARATOR_GROOVE|LAYOUT_FILL_Y,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=0,FXint pr=0,FXint pt=1,FXint pb=1);

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();
  };


#endif
