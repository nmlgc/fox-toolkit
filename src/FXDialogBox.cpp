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
* $Id: FXDialogBox.cpp,v 1.6 1998/08/25 18:57:12 jvz Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"

/*

  To do:
  - allow resize option..
  - setting icons
  - Iconified/normal
  - Unmap when main window unmapped
  - Transient For stuff
*/

/*******************************************************************************/


// Map
FXDEFMAP(FXDialogBox) FXDialogBoxMap[]={
  FXMAPFUNC(SEL_COMMAND,FXDialogBox::ID_CANCEL,FXDialogBox::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND,FXDialogBox::ID_ACCEPT,FXDialogBox::onCmdAccept),
  };


// Object implementation
FXIMPLEMENT(FXDialogBox,FXTopWindow,FXDialogBoxMap,ARRAYNUMBER(FXDialogBoxMap))


// Contruct
FXDialogBox::FXDialogBox(FXApp* a,const char* name,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint hs,FXint vs):
  FXTopWindow(a,name,opts,x,y,w,h,hs,vs){
  state=0;
  }


// Close dialog with an accept
long FXDialogBox::onCmdAccept(FXObject*,FXSelector,void*){
  hide();
  state=TRUE;
  return 1;
  }


// Close dialog with a cancel
long FXDialogBox::onCmdCancel(FXObject*,FXSelector,void*){
  hide();
  state=FALSE;
  return 1;
  }


// Create window
void FXDialogBox::create(){
  FXTopWindow::create();
  setTransientFor(NULL);
  }


// On show, we reset the state just in case
void FXDialogBox::show(){
  FXTopWindow::show();
  state=0;
  }



// Execute dialog box modally
FXuint FXDialogBox::execute(){
  FXint rx,ry; FXuint state;
  create();
  getRoot()->getCursorPosition(rx,ry,state);
  if(width<10 || height<10) 
    position(rx,ry,getDefaultWidth(),getDefaultHeight());
  else
    position(rx,ry,width,height);
  show();
  getApp()->runModalFor(this);
  return getStatus();
  }
