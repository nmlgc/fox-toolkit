/********************************************************************************
*                                                                               *
*                     S h e l l   W i n d o w   O b j e c t                     *
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
* $Id: FXShell.h,v 1.4 1998/04/08 02:59:21 jeroen Exp $                         *
********************************************************************************/
#ifndef FXSHELL_H
#define FXSHELL_H



// A child of the Root window
class FXShell : public FXComposite {
  FXDECLARE(FXShell)
protected:
  FXShell(){}
  FXShell(const FXShell&){}
public:
  long onConfigure(FXObject*,FXSelector,void*);
  long onFocusNext(FXObject*,FXSelector,void*);
  long onFocusPrev(FXObject*,FXSelector,void*);
protected:
  FXShell(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h);
public:
  virtual void create();
  virtual void recalc();
  };


#endif
