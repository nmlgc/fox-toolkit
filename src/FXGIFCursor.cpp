/********************************************************************************
*                                                                               *
*                        G I F   C u r s o r   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Daniel Gehriger.   All Rights Reserved.            *
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
* $Id: FXGIFCursor.cpp,v 1.10 2002/01/18 22:43:00 jeroen Exp $                   *
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
#include "FXGIFCursor.h"


/*
  Notes:
  Jeroen's notes for FXGIFIcon also apply to FXGIFCursor:

  - Best is to use the actual alpha color from the GIF file.
  - Next, one can try the background color from the GIF file.
  - You can also let the system guess a transparancy color based on the corners.
  - If that doesn't work, you can force a specific transparency color.
  - Want to add some similar options like in FXIcon w.r.t. when pixel buffer
    gets thrown away, and to force certain alpha colors or opaqueness etc.
  - We could then extend the collection to include XPM (particularly
    interesting as that would allow making cursors with nother other than vi).
  - Maybe want to have some API to turn FXBitmap into FXCursor.
*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXGIFCursor,FXCursor,NULL,0)


/*-----------------------------------------------------------------*\
 *
 * Constructor
 *
\*-----------------------------------------------------------------*/
FXGIFCursor::FXGIFCursor(FXApp* a,const void *pix,FXint hx,FXint hy):FXCursor(a,NULL,NULL,0,0,hx,hy){
  if(pix){
    FXuchar* pbData;
    FXColor  clrPixel,clrTransp;
    FXint    h,w,nWidth,nHeight,nWidthBytes,nOffset;
    FXMemoryStream ms;
    ms.open((FXuchar*)pix,FXStreamLoad);
    if(fxloadGIF(ms,pbData,clrTransp,nWidth,nHeight)){
      if(nWidth>32 || nHeight>32){ fxerror("%s::create: cursor exceeds maximum size of 32x32 pixels\n",getClassName()); }
      width=nWidth;
      height=nHeight;
      nWidthBytes=(nWidth+7)/8;
      FXCALLOC(&source,FXuchar,nWidthBytes*nHeight);
      FXCALLOC(&mask,FXuchar,nWidthBytes*nHeight);
      owned=TRUE;
      for(h=0; h<nHeight; ++h){
        for(w=0; w<nWidth; ++w){
          nOffset=3*(w+nWidth*h);
          clrPixel=FXRGB(pbData[nOffset],pbData[nOffset+1],pbData[nOffset+2]);
          if(clrPixel==FXRGB(0,0,0)){
            source[(w+nWidth*h)/8]|=1<<(w%8);
            }
          if(clrPixel!=clrTransp){
            mask[(w+nWidth*h)/8]|=1<<(w%8);
            }
          }
        }
      FXFREE(&pbData);    // Free original pixels
      }
    ms.close();
    }
  }


