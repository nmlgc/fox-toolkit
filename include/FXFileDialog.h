/********************************************************************************
*                                                                               *
*                    F i l e   S e l e c t i o n   D i a l o g                  *
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
* $Id: FXFileDialog.h,v 1.8 1998/04/22 00:06:02 jeroen Exp $                    *
********************************************************************************/
#ifndef FXFILEDIALOG_H
#define FXFILEDIALOG_H


// File selection dialog
class FXFileDialog : public FXDialogBox {
  FXDECLARE(FXFileDialog)
protected:
  FXFileSelector *filebox;
protected:
  FXFileDialog(){}
  FXFileDialog(const FXFileDialog&){}
public:
  FXFileDialog(FXApp* a,const char* name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=500,FXint h=300);
  void setFilename(const FXchar* path);
  const FXchar* getFilename() const;
  void setPattern(const FXchar* ptrn);
  const FXchar* getPattern() const;
  virtual ~FXFileDialog();
  };


#endif
