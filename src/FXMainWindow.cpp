/********************************************************************************
*                                                                               *
*                     M a i n   W i n d o w   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXMainWindow.cpp,v 1.13 2002/01/18 22:43:01 jeroen Exp $                 *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXCursor.h"
#include "FXRootWindow.h"
#include "FXMainWindow.h"

/*
  Notes:
  - allow resize option..
  - Iconified/normal.
  - Want unlimited number of main windows.
*/

/*******************************************************************************/


// Map
FXDEFMAP(FXMainWindow) FXMainWindowMap[]={
  FXMAPFUNC(SEL_CLOSE,0,FXMainWindow::onClose),
  };


// Object implementation
FXIMPLEMENT(FXMainWindow,FXTopWindow,FXMainWindowMap,ARRAYNUMBER(FXMainWindowMap))


// Make main window
FXMainWindow::FXMainWindow(FXApp* a,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXTopWindow(a,name,ic,mi,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  if(getApp()->mainWindow){ fxwarning("Warning: creating multiple main windows\n"); }
  getApp()->mainWindow=this;
  }


// Unless target catches it, close down the app
long FXMainWindow::onClose(FXObject*,FXSelector,void*){

  // If handled, we're not closing the window after all
  if(target && target->handle(this,MKUINT(message,SEL_CLOSE),NULL)) return 1;

  // Otherwise, we will quit the application
  getApp()->handle(this,MKUINT(FXApp::ID_QUIT,SEL_COMMAND),NULL);

  return 1;
  }


// Destroy
FXMainWindow::~FXMainWindow(){
  getApp()->mainWindow=NULL;
  }
