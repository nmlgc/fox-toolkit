/********************************************************************************
*                                                                               *
*                     T A R G A   I c o n   O b j e c t                         *
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
* $Id: FXTGAIcon.cpp,v 1.3 2002/01/18 22:55:04 jeroen Exp $                     *
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
#include "FXTGAIcon.h"


/*
  Notes:
  - Targa does not support alpha in the file format.
  - You can also let the system guess a transparancy color based on the corners.
  - If that doesn't work, you can force a specific transparency color.
  - This is just an idea at this point:

      // Compute name of image support class
      FXString name="FX"+ext.upper()+"Image";

      // Find the meta class
      const FXMetaClass *meta=FXMetaClass::getMetaClassFromName(name.text());

      // Make instance of this class
      if(meta) img=(FXImage*)meta->makeInstance();

    The above is a simplistic view; we will need to set the image's visual,
    options, and other stuff before this can work.
    Also, when linking statically, we have to convince the linker to include
    the referred image code...

*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXTGAIcon,FXIcon,NULL,0)


// Initialize nicely
FXTGAIcon::FXTGAIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXIcon(a,NULL,clr,opts&~IMAGE_ALPHA,w,h){
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    loadPixels(ms);
    ms.close();
    }
  }

// Save object to stream
void FXTGAIcon::savePixels(FXStream& store) const {
  FXASSERT(!(options&IMAGE_ALPHA));
  fxsaveTGA(store,data,transp,width,height);
  }


// Load object from stream
void FXTGAIcon::loadPixels(FXStream& store){
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadTGA(store,data,channels,width,height);
  if(options&IMAGE_ALPHAGUESS) transp=guesstransp();
  if (channels == 3)
    options&=~IMAGE_ALPHA;
  else
    options|=IMAGE_OPAQUE;
  options|=IMAGE_OWNED;
  }


// Clean up
FXTGAIcon::~FXTGAIcon(){
  }


