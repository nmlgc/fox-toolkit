/********************************************************************************
*                                                                               *
*                     M a i n   W i n d o w   O b j e c t                       *
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
* $Id: FXMainWindow.h,v 1.3 1998/03/24 03:04:03 jeroen Exp $                    *
********************************************************************************/
#ifndef FXMAINWINDOW_H
#define FXMAINWINDOW_H



// Main application window 
class FXMainWindow : public FXTopWindow {
  FXDECLARE(FXMainWindow)
public:
  long onCmdClose(FXObject*,FXSelector,void*);
protected:
  FXMainWindow(){}
  FXMainWindow(const FXMainWindow&){}
public:
  FXMainWindow(FXApp* a,const char* name,FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint hs=4,FXint vs=4);
  };




#endif
