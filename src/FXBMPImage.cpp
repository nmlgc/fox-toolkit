/********************************************************************************
*                                                                               *
*                            B M P   I m a g e   O b j e c t                    *
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
* $Id: FXBMPImage.cpp,v 1.8 1998/09/23 20:04:05 jvz Exp $                     *
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
#include "FXBMPImage.h"



/*
  To do:
  - Add bmp writer
  - more error checking
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXBMPImage,FXImage,NULL,0)


// Initialize
FXBMPImage::FXBMPImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):
  FXImage(a,NULL,opts,w,h){
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    loadPixels(ms);
    ms.close();
    }
  }


// Save pixel data only
void FXBMPImage::savePixels(FXStream& store) const {
  FXColor transp=FXRGB(192,192,192);
  fxsaveBMP(store,data,transp,width,height);
  }


// Load pixel data only
void FXBMPImage::loadPixels(FXStream& store){
  FXColor transp;
  fxloadBMP(store,data,transp,width,height);
  }


// Clean up
FXBMPImage::~FXBMPImage(){
  }



