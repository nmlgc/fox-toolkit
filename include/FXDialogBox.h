/********************************************************************************
*                                                                               *
*                     D i a l o g   B o x    O b j e c t                        *
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
* $Id: FXDialogBox.h,v 1.4 1998/04/22 00:06:01 jeroen Exp $                     *
********************************************************************************/
#ifndef FXDIALOGBOX_H
#define FXDIALOGBOX_H



// Modal dialog
enum FXDialogStyle {
  DIALOG_MODELESS = 0,                // Modeless dialog
  DIALOG_MODAL    = 0x01000000        // Modal dialog
  };


// Dialog Box window
class FXDialogBox : public FXTopWindow {
  FXDECLARE(FXDialogBox)
protected:
  FXuint  state;                      // Button clicked to exit the dialog
protected:
  FXDialogBox(){}
  FXDialogBox(const FXDialogBox&){}
public:
  long onCmdAccept(FXObject*,FXSelector,void*);
  long onCmdCancel(FXObject*,FXSelector,void*);
public:
  enum{
    ID_CANCEL=FXTopWindow::ID_LAST,
    ID_ACCEPT,
    ID_LAST
    };
public:
  FXDialogBox(FXApp* a,const char* name,FXuint opts=DECOR_TITLE,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint hs=4,FXint vs=4);
  virtual void create();
  virtual void show();
  void setStatus(FXuint s){ state=s; }
  FXuint getStatus() const { return state; }
  FXuint execute();
  };


#endif
