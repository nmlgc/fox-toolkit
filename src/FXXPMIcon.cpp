/********************************************************************************
*                                                                               *
*                        X P M   I c o n   O b j e c t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXXPMIcon.cpp,v 1.13 2002/01/18 22:43:07 jeroen Exp $                    *
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
#include "FXXPMIcon.h"


/*
  Notes:
*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXXPMIcon,FXIcon,NULL,0)


// Initialize nicely
FXXPMIcon::FXXPMIcon(FXApp* a,const FXchar **pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXIcon(a,NULL,clr,opts&~IMAGE_ALPHA,w,h){
  if(pix){
    FXColor clearcolor=0;
    fxloadXPM(pix,data,clearcolor,width,height);
    if(!(options&IMAGE_ALPHACOLOR)) transp=clearcolor;
    if(options&IMAGE_ALPHAGUESS) transp=guesstransp();
    if(transp==0) options|=IMAGE_OPAQUE;
    options|=IMAGE_OWNED;
    }
  }


// Save object to stream
void FXXPMIcon::savePixels(FXStream& store) const {
  FXASSERT(!(options&IMAGE_ALPHA));
  fxsaveXPM(store,data,transp,width,height);
  }


// Load object from stream
void FXXPMIcon::loadPixels(FXStream& store){
  FXColor clearcolor=0;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadXPM(store,data,clearcolor,width,height);
  if(!(options&IMAGE_ALPHACOLOR)) transp=clearcolor;
  if(options&IMAGE_ALPHAGUESS) transp=guesstransp();
  if(transp==0) options|=IMAGE_OPAQUE;
  options&=~IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXXPMIcon::~FXXPMIcon(){
  }


