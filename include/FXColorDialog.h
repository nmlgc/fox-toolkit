/********************************************************************************
*                                                                               *
*                           C o l o r   D i a l o g                             *
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
* $Id: FXColorDialog.h,v 1.3 1998/06/25 03:04:08 jeroen Exp $                   *
********************************************************************************/
#ifndef FXCOLORDIALOG_H
#define FXCOLORDIALOG_H


// File selection dialog
class FXColorDialog : public FXDialogBox {
  FXDECLARE(FXColorDialog)
protected:
  FXColorSelector *colorbox;
protected:
  FXColorDialog(){}
  FXColorDialog(const FXColorDialog&){}
public:
  FXColorDialog(FXApp* a,const char* name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  void setRGBA(FXColor clr);
  FXColor getRGBA() const;
  virtual ~FXColorDialog();
  };


#endif
