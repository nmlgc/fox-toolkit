/********************************************************************************
*                                                                               *
*                        D D S   I c o n   O b j e c t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 2008 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
*********************************************************************************
* $Id: FXDDSImage.cpp,v 1.5 2008/06/30 15:50:44 fox Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXThread.h"
#include "FXStream.h"
#include "FXMemoryStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXDDSImage.h"



/*
  Notes:
  - Support for displaying texture images; note, FOX decompresses these back to RGB;
    for direct use, the non-decompressed image data should be handed directly to
    OpenGL.
  - FIXME saving to this format is not yet supported.
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Suggested file extension
const FXchar FXDDSImage::fileExt[]="dds";


// Suggested mime type
const FXchar FXDDSImage::mimeType[]="image/x-dds";


// Object implementation
FXIMPLEMENT(FXDDSImage,FXImage,NULL,0)


// Initialize
FXDDSImage::FXDDSImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):FXImage(a,NULL,opts,w,h){
  if(pix){
    FXMemoryStream ms;
    ms.open(FXStreamLoad,(FXuchar*)pix);
    loadPixels(ms);
    ms.close();
    }
  }


// Save pixel data only
FXbool FXDDSImage::savePixels(FXStream&) const {
  // FIXME saving not yet supported FIXME //
  return false;
  }


// Load pixel data only
FXbool FXDDSImage::loadPixels(FXStream& store){
  FXColor *pixels; FXint w,h,d;
  if(fxloadDDS(store,pixels,w,h,d)){
    setData(pixels,IMAGE_OWNED,w,h);
    return true;
    }
  return false;
  }


// Clean up
FXDDSImage::~FXDDSImage(){
  }

}