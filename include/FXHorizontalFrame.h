/********************************************************************************
*                                                                               *
*              H o r i z o n t a l   C o n t a i n e r   O b j e c t            *
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
* $Id: FXHorizontalFrame.h,v 1.4 1998/08/26 07:41:11 jeroen Exp $               *
********************************************************************************/
#ifndef FXHORIZONTALFRAME_H
#define FXHORIZONTALFRAME_H

  
// Horizontal layout convenience class
class FXHorizontalFrame : public FXPacker {
  FXDECLARE(FXHorizontalFrame)
protected:
  FXHorizontalFrame(){}
  FXHorizontalFrame(const FXHorizontalFrame&){}
  virtual void layout();
public:
  long onFocusLeft(FXObject*,FXSelector,void*);
  long onFocusRight(FXObject*,FXSelector,void*);
public:
  FXHorizontalFrame(FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };



#endif
