/********************************************************************************
*                                                                               *
*                         J P E G   I c o n   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by David Tyree.   All Rights Reserved.                *
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
* $Id: FXJPGIcon.cpp,v 1.4 2002/01/18 22:55:04 jeroen Exp $                     *
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
#include "FXJPGIcon.h"


/*
  Notes:
  - Requires JPEG library.
*/


/*******************************************************************************/

FXIMPLEMENT(FXJPGIcon,FXIcon,NULL,0)


// Initialize
FXJPGIcon::FXJPGIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXIcon(a,NULL,clr,opts&~IMAGE_ALPHA,w,h){
  quality=75;
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar *)pix,FXStreamLoad);
    loadPixels(ms);
    ms.close();
    }
  }


// Save pixels only
void FXJPGIcon::savePixels(FXStream& store) const {
  FXASSERT(!(options&IMAGE_ALPHA));
  fxsaveJPG(store,data,transp,width,height,quality);
  }


// Load pixels only
void FXJPGIcon::loadPixels(FXStream& store){
  FXColor clearcolor=0;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadJPG(store,data,clearcolor,width,height,quality);
  if(!(options&IMAGE_ALPHACOLOR)) transp=clearcolor;
  if(options&IMAGE_ALPHAGUESS) transp=guesstransp();
  if(transp==0) options|=IMAGE_OPAQUE;
  options&=~IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXJPGIcon::~FXJPGIcon(){
  }
