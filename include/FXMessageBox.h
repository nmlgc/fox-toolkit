/********************************************************************************
*                                                                               *
*                         M e s s a g e   B o x e s                             *
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
* $Id: FXMessageBox.h,v 1.7 1998/05/15 06:04:31 jeroen Exp $                    *
********************************************************************************/
#ifndef FXMESSAGEBOX_H
#define FXMESSAGEBOX_H



// Message box buttons
enum FXMessageboxStyle {
  MBOX_OK               = 0x10000000,
  MBOX_OK_CANCEL        = 0x20000000,
  MBOX_YES_NO           = 0x30000000,
  MBOX_YES_NO_CANCEL    = 0x40000000,
  MBOX_QUIT_CANCEL      = 0x50000000,
  MBOX_QUIT_SAVE_CANCEL = 0x60000000,
  MBOX_BUTTON_MASK      = 0x70000000
  };

// Return values
enum FXMessageboxReturn {
  MBOX_CLICKED_YES      = 1,
  MBOX_CLICKED_NO       = 2,
  MBOX_CLICKED_OK       = 3,
  MBOX_CLICKED_CANCEL   = 4,
  MBOX_CLICKED_QUIT     = 5,
  MBOX_CLICKED_SAVE     = 6
  };


// Message box
class FXMessageBox : public FXDialogBox {
  FXDECLARE(FXMessageBox)
protected:
  FXMessageBox(){}
  FXMessageBox(const FXMessageBox&){}
public:
  long onCmdClicked(FXObject*,FXSelector,void*);
public:
  enum{
    ID_CLICKED_YES=FXDialogBox::ID_LAST,
    ID_CLICKED_NO,
    ID_CLICKED_OK,
    ID_CLICKED_CANCEL,
    ID_CLICKED_QUIT,
    ID_CLICKED_SAVE,
    ID_LAST
    };
public:
  FXMessageBox(FXApp* a,const char* name,const char* text=NULL,FXIcon* ic=NULL,FXuint opts=DECOR_TITLE|DECOR_BORDER,FXint x=0,FXint y=0);
  };


#endif
