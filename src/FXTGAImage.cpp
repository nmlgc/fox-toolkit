/********************************************************************************
*                                                                               *
*                       T A R G A  I m a g e   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 by Janusz Ganczarski.   All Rights Reserved.          *
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
* $Id: FXTGAImage.cpp,v 1.4 2002/01/18 22:55:04 jeroen Exp $                    *
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
#include "FXApp.h"
#include "FXTGAImage.h"



/*
  Notes:
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXTGAImage,FXImage,NULL,0)



// Initialize
FXTGAImage::FXTGAImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):
  FXImage(a,NULL,opts&~IMAGE_ALPHA,w,h){
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    fxloadTGA(ms,data,channels,width,height);
    options|=IMAGE_OWNED;
    ms.close();
    }
  }


// Save pixel data only
void FXTGAImage::savePixels(FXStream& store) const {
  FXASSERT(!(options&IMAGE_ALPHA));
  fxsaveTGA(store,data,channels,width,height);
  }


// Load pixel data only
void FXTGAImage::loadPixels(FXStream& store){
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadTGA(store,data,channels,width,height);
  if (channels == 3)
    options&=~IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXTGAImage::~FXTGAImage(){
  }



