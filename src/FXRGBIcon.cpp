/********************************************************************************
*                                                                               *
*                      I R I S   R G B   I c o n   O b j e c t                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXRGBIcon.cpp,v 1.2 2002/01/22 18:36:14 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXObject.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXRGBIcon.h"


/*
  Notes:
*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXRGBIcon,FXIcon,NULL,0)


// Initialize nicely
FXRGBIcon::FXRGBIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXIcon(a,NULL,clr,opts&~IMAGE_ALPHA,w,h){
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    loadPixels(ms);
    ms.close();
    }
  }


// Save object to stream
void FXRGBIcon::savePixels(FXStream& store) const {
  FXASSERT(!(options&IMAGE_ALPHA));
  fxsaveRGB(store,data,transp,width,height);
  }


// Load object from stream
void FXRGBIcon::loadPixels(FXStream& store){
  FXColor clearcolor=0;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadRGB(store,data,clearcolor,width,height);
  if(!(options&IMAGE_ALPHACOLOR)) transp=clearcolor;
  if(options&IMAGE_ALPHAGUESS) transp=guesstransp();
  if(transp==0) options|=IMAGE_OPAQUE;
  options&=~IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXRGBIcon::~FXRGBIcon(){
  }


