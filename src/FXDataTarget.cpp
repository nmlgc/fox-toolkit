/********************************************************************************
*                                                                               *
*                              D a t a   T a r g e t                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXDataTarget.cpp,v 1.20 2002/01/18 22:42:59 jeroen Exp $                 *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXWindow.h"
#include "FXDataTarget.h"


/*
  Notes:
  - DataTarget connects GUI to basic values such as flags (FXbool), integral
    or real numbers, and strings (FXString).
  - Values in the application program may get updated from the GUI, and
    vice-versa GUI gets updated when the program has changed a value as well.
  - Would be nice to set value from message ID also...
  - When the sender of onCmdValue does not understand the ID_GETXXXXVALUE message,
    the data target keeps the same value as before.
  - Catch SEL_CHANGED when we have expunged this from FXTextField.
*/


/*******************************************************************************/


// Map
FXDEFMAP(FXDataTarget) FXDataTargetMap[]={
  FXMAPFUNC(SEL_COMMAND,FXDataTarget::ID_VALUE,FXDataTarget::onCmdValue),
  FXMAPFUNC(SEL_CHANGED,FXDataTarget::ID_VALUE,FXDataTarget::onCmdValue),
  FXMAPFUNC(SEL_UPDATE,FXDataTarget::ID_VALUE,FXDataTarget::onUpdValue),
  FXMAPFUNCS(SEL_COMMAND,FXDataTarget::ID_OPTION-10001,FXDataTarget::ID_OPTION+10000,FXDataTarget::onCmdOption),
  FXMAPFUNCS(SEL_UPDATE,FXDataTarget::ID_OPTION-10001,FXDataTarget::ID_OPTION+10000,FXDataTarget::onUpdOption),
  };


// Object implementation
FXIMPLEMENT(FXDataTarget,FXObject,FXDataTargetMap,ARRAYNUMBER(FXDataTargetMap))


// Value changed from widget
long FXDataTarget::onCmdValue(FXObject* sender,FXSelector sel,void*){
  FXdouble d;
  FXint    i;
  switch(type){
    case DT_CHAR:
      i=*((FXchar*)data);
      sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i);
      *((FXchar*)data)=i;
      break;
    case DT_UCHAR:
      i=*((FXuchar*)data);
      sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i);
      *((FXuchar*)data)=i;
      break;
    case DT_SHORT:
      i=*((FXshort*)data);
      sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i);
      *((FXshort*)data)=i;
      break;
    case DT_USHORT:
      i=*((FXushort*)data);
      sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&i);
      *((FXushort*)data)=i;
      break;
    case DT_INT:
      sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),data);
      break;
    case DT_UINT:
      sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),data);
      break;
    case DT_FLOAT:
      d=*((FXfloat*)data);
      sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),(void*)&d);
      *((FXfloat*)data)=(FXfloat)d;
      break;
    case DT_DOUBLE:
      sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),data);
      break;
    case DT_STRING:
      sender->handle(this,MKUINT(FXWindow::ID_GETSTRINGVALUE,SEL_COMMAND),data);
      break;
    }
  if(target){
    target->handle(this,MKUINT(message,SELTYPE(sel)),data);
    }
  return 1;
  }


// Widget changed from value
long FXDataTarget::onUpdValue(FXObject* sender,FXSelector,void*){
  FXdouble d;
  FXint    i;
  switch(type){
    case DT_CHAR:
      i=*((FXchar*)data);
      sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i);
      break;
    case DT_UCHAR:
      i=*((FXuchar*)data);
      sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i);
      break;
    case DT_SHORT:
      i=*((FXshort*)data);
      sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i);
      break;
    case DT_USHORT:
      i=*((FXushort*)data);
      sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&i);
      break;
    case DT_INT:
      sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),data);
      break;
    case DT_UINT:
      sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),data);
      break;
    case DT_FLOAT:
      d=*((FXfloat*)data);
      sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&d);
      break;
    case DT_DOUBLE:
      sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),data);
      break;
    case DT_STRING:
      sender->handle(this,MKUINT(FXWindow::ID_SETSTRINGVALUE,SEL_COMMAND),data);
      break;
    }
  return 1;
  }


// Value set from message id
long FXDataTarget::onCmdOption(FXObject*,FXSelector sel,void*){
  FXint num=((FXint)SELID(sel))-ID_OPTION;
  switch(type){
    case DT_CHAR:
      *((FXchar*)data)=num;
      break;
    case DT_UCHAR:
      *((FXuchar*)data)=num;
      break;
    case DT_SHORT:
      *((FXshort*)data)=num;
      break;
    case DT_USHORT:
      *((FXushort*)data)=num;
      break;
    case DT_INT:
      *((FXint*)data)=num;
      break;
    case DT_UINT:
      *((FXuint*)data)=num;
      break;
    case DT_FLOAT:
      *((FXfloat*)data)=(FXfloat)num;
      break;
    case DT_DOUBLE:
      *((FXdouble*)data)=num;
      break;
    }
  if(target){
    target->handle(this,MKUINT(message,SELTYPE(sel)),data);
    }
  return 1;
  }


// Check widget whose message id matches
long FXDataTarget::onUpdOption(FXObject* sender,FXSelector sel,void*){
  FXint num=((FXint)SELID(sel))-ID_OPTION;
  FXint i=0;
  switch(type){
    case DT_CHAR:
      i=*((FXchar*)data);
      break;
    case DT_UCHAR:
      i=*((FXuchar*)data);
      break;
    case DT_SHORT:
      i=*((FXshort*)data);
      break;
    case DT_USHORT:
      i=*((FXushort*)data);
      break;
    case DT_INT:
      i=*((FXint*)data);
      break;
    case DT_UINT:
      i=*((FXuint*)data);
      break;
    case DT_FLOAT:
      i=(FXint) *((FXfloat*)data);
      break;
    case DT_DOUBLE:
      i=(FXint) *((FXdouble*)data);
      break;
    }
  if(i==num){
    sender->handle(this,MKUINT(FXWindow::ID_CHECK,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(FXWindow::ID_UNCHECK,SEL_COMMAND),NULL);
    }
  return 1;
  }

