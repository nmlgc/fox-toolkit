/********************************************************************************
*                                                                               *
*                                  X - O b j e c t                              *
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
* $Id: FXId.cpp,v 1.9 1998/09/18 05:54:06 jeroen Exp $                           *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"


// Object implementation
FXIMPLEMENT(FXId,FXObject,NULL,0)


// For deserialization
FXId::FXId(){
  app=(FXApp*)-1;
  xid=0;
  }


// Initialize nicely
FXId::FXId(FXApp* a,FXID i){
  if(!a){ fxerror("Application object cannot be NULL\n"); }
  app=a;
  xid=i;
  }


// Save data
void FXId::save(FXStream& store) const {
  FXObject::save(store);
  store << app;
  }


// Load data
void FXId::load(FXStream& store){ 
  FXObject::load(store);
  store >> app;
  }


// Destroy it
FXId::~FXId(){
  app=(FXApp*)-1;
  xid=0;
  }
  

