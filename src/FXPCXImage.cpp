/********************************************************************************
*                                                                               *
*                            P C X   I m a g e   O b j e c t                    *
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
* $Id: FXPCXImage.cpp,v 1.3 2002/01/18 22:55:04 jeroen Exp $                    *
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
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXPCXImage.h"



/*
  Notes:
  - Use corner color as transparency color, unless override.
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXPCXImage,FXImage,NULL,0)



// Initialize
FXPCXImage::FXPCXImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):
  FXImage(a,NULL,opts&~IMAGE_ALPHA,w,h){
  if(pix){
    FXMemoryStream ms;
    FXColor clearcolor;
    ms.open((FXuchar*)pix,FXStreamLoad);
    fxloadPCX(ms,data,clearcolor,width,height);
    options|=IMAGE_OWNED;
    ms.close();
    }
  }


// Save pixel data only
void FXPCXImage::savePixels(FXStream& store) const {
  FXColor clearcolor=FXRGB(192,192,192);
  FXASSERT(!(options&IMAGE_ALPHA));
  fxsavePCX(store,data,clearcolor,width,height);
  }


// Load pixel data only
void FXPCXImage::loadPixels(FXStream& store){
  FXColor clearcolor;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadPCX(store,data,clearcolor,width,height);
  options&=~IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXPCXImage::~FXPCXImage(){
  }



