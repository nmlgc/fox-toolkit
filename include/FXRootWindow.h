/********************************************************************************
*                                                                               *
*                     R o o t   W i n d o w   O b j e c t                       *
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
* $Id: FXRootWindow.h,v 1.3 1998/08/26 07:41:12 jeroen Exp $                    *
********************************************************************************/
#ifndef FXROOTWINDOW_H
#define FXROOTWINDOW_H



// Root window
class FXRootWindow : public FXComposite {
  FXDECLARE(FXRootWindow)
protected:
  FXRootWindow(){}
  FXRootWindow(const FXRootWindow&){}
  virtual void layout();
public:
  FXRootWindow(FXApp* a);
  virtual void create();
  virtual void destroy();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void recalc();
  virtual ~FXRootWindow();
  };

  
#endif
