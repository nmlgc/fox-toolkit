/********************************************************************************
*                                                                               *
*                              D a t a   T a r g e t                            *
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
* $Id: FXDataTarget.cpp,v 1.8 1998/10/29 07:42:24 jeroen Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXDataTarget.h"


/*
  Notes:
  - DataTarget connects GUI to basic values such as flags (FXbool), integral
    or real numbers, and strings (FXString).
  - Values in the application program may get updated from the GUI, and
    vice-versa GUI gets updated when the program has changed a value as well.
*/


/*******************************************************************************/


// Map
FXDEFMAP(FXDataTarget) FXDataTargetMap[]={
  FXMAPFUNC(SEL_COMMAND,FXDataTarget::ID_VALUE,FXDataTarget::onCmdValue),
  FXMAPFUNC(SEL_UPDATE,FXDataTarget::ID_VALUE,FXDataTarget::onUpdValue),
  };


// Object implementation
FXIMPLEMENT(FXDataTarget,FXObject,FXDataTargetMap,ARRAYNUMBER(FXDataTargetMap))



// Value changed from widget
long FXDataTarget::onCmdValue(FXObject* sender,FXSelector sel,void* ptr){
  FXdouble d;
  FXint    i;
  if(data){
    switch(type){
      case DT_VOID: return 1;
      case DT_CHAR: if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i)){ *((FXchar*)data)=i; return 1; } return 0;
      case DT_UCHAR: if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i)){ *((FXuchar*)data)=i; return 1; } return 0;
      case DT_SHORT: if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i)){ *((FXshort*)data)=i; return 1; } return 0;
      case DT_USHORT: if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i)){ *((FXushort*)data)=i; return 1; } return 0;
      case DT_INT: if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i)){ *((FXint*)data)=i; return 1; } return 0;
      case DT_UINT: if(sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i)){ *((FXuint*)data)=i; return 1; } return 0;
      case DT_FLOAT: if(sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),(void*)&d)){ *((FXfloat*)data)=d; return 1; } return 0;
      case DT_DOUBLE: if(sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),(void*)&d)){ *((FXdouble*)data)=d; return 1; } return 0;
      case DT_STRING: if(sender->handle(this,MKUINT(FXWindow::ID_GETSTRINGVALUE,SEL_COMMAND),(void*)data)){ return 1; } return 0;
      }
    }
  return 0;
  }


// Widget changed from value
long FXDataTarget::onUpdValue(FXObject* sender,FXSelector sel,void* ptr){
  FXdouble d;
  FXint    i;
  if(data){
    switch(type){
      case DT_VOID: break;
      case DT_CHAR: i=*((FXchar*)data); sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i); break;
      case DT_UCHAR: i=*((FXuchar*)data); sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i); break;
      case DT_SHORT: i=*((FXshort*)data); sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i); break;
      case DT_USHORT: i=*((FXushort*)data); sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i); break;
      case DT_INT: i=*((FXint*)data); sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i); break;
      case DT_UINT: i=*((FXuint*)data); sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i); break;
      case DT_FLOAT: d=*((FXfloat*)data); sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&d); break;
      case DT_DOUBLE: d=*((FXdouble*)data); sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&d); break;
      case DT_STRING: sender->handle(this,MKUINT(FXWindow::ID_SETSTRINGVALUE,SEL_COMMAND),(void*)data); break;
      }
    }
  return 1;
  }

