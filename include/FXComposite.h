/********************************************************************************
*                                                                               *
*                C o m p o s i t e   W i n d o w   O b j e c t                  *
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
* $Id: FXComposite.h,v 1.9 1998/09/22 20:33:43 jvz Exp $                     *
********************************************************************************/
#ifndef FXCOMPOSITE_H
#define FXCOMPOSITE_H



// Base composite
class FXComposite : public FXWindow {
  FXDECLARE(FXComposite)
protected:
  FXComposite(){}
  FXComposite(const FXComposite&){}
  FXComposite(FXApp* a);
  FXComposite(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h);
  virtual void layout();
public:
  long onFocusNext(FXObject*,FXSelector,void*);
  long onFocusPrev(FXObject*,FXSelector,void*);
public:
  FXComposite(FXComposite* p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void destroy();
  FXint maxChildWidth() const;
  FXint maxChildHeight() const;
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool isComposite() const;
  virtual ~FXComposite();
  };

  
#endif
