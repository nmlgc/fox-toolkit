/********************************************************************************
*                                                                               *
*                        C U R   C u r s o r    O b j e c t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 by Sander Jansen.   All Rights Reserved.              *
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
* $Id: FXCURCursor.cpp,v 1.6 2002/01/18 22:42:58 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXCURCursor.h"

/*
 Notes:

 - The inversion of the bits and mask should happen in the fxloadCUR function.
*/


#define DWORD_ALIGN_BITS(b)   (((b) / 32) + (((b) % 32 > 0) ? 1 : 0))



/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXCURCursor,FXCursor,NULL,0)


/*-----------------------------------------------------------------*\
 *
 * Constructor
 *
\*-----------------------------------------------------------------*/
FXCURCursor::FXCURCursor(FXApp* a,const void *pix):FXCursor(a,NULL,NULL,0,0,0,0){
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    if(fxloadCUR(ms,source,mask,width,height,hotx,hoty)) owned=TRUE;
    ms.close();
    }
  }


void FXCURCursor::loadPixels(FXStream & store){
  if(fxloadCUR(store,source,mask,width,height,hotx,hoty)) owned=TRUE;
  }


FXCURCursor::~FXCURCursor(){
  }


