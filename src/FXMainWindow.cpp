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
* $Id: FXMainWindow.cpp,v 1.5 1998/10/23 22:07:37 jvz Exp $                  *
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
#include "FXMainWindow.h"

/*

  To do:
  - allow resize option..
  - setting icons
  - Iconified/normal
  - FXApp should keep track of toplevel windows, and if last one is closed,
    end the application
*/

/*******************************************************************************/


// Map
FXDEFMAP(FXMainWindow) FXMainWindowMap[]={
  FXMAPFUNC(SEL_COMMAND,FXTopWindow::ID_CLOSE,FXMainWindow::onCmdClose),
  };


// Object implementation
FXIMPLEMENT(FXMainWindow,FXTopWindow,FXMainWindowMap,ARRAYNUMBER(FXMainWindowMap))


// Make main window
FXMainWindow::FXMainWindow(FXApp* a,const char* name,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint hs,FXint vs):
  FXTopWindow(a,name,opts,x,y,w,h,hs,vs){
  }


// Catch client message
long FXMainWindow::onCmdClose(FXObject*,FXSelector,void*){
//delete this;////// TEST
  getApp()->exit(0);
  return 1;
  }

