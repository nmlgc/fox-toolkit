/********************************************************************************
*                                                                               *
*                        B M P   I c o n   O b j e c t                          *
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
* $Id: FXBMPIcon.cpp,v 1.6 1998/09/23 20:04:05 jvz Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXBMPIcon.h"


/*
  To do:
  - Use corner color as transparency color, unless override.
*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXBMPIcon,FXIcon,NULL,0)


// Initialize nicely
FXBMPIcon::FXBMPIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXIcon(a,NULL,clr,opts,w,h){
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    loadPixels(ms);
    ms.close();
    }
  }
  
// Save object to stream
void FXBMPIcon::savePixels(FXStream& store) const {
  fxsaveBMP(store,data,transp,width,height);
  }


// Load object from stream
void FXBMPIcon::loadPixels(FXStream& store){
  FXColor clearcolor=0;
  fxloadBMP(store,data,clearcolor,width,height);
  if(!(options&IMAGE_ALPHACOLOR)) transp=clearcolor;
  if(transp==0) options|=IMAGE_OPAQUE;
  }


// Clean up
FXBMPIcon::~FXBMPIcon(){
  }
  

