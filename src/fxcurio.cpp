/********************************************************************************
*                                                                               *
*                         C U R    I n p u t / O u t p u t                      *
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
* $Id: fxcurio.cpp,v 1.5 2002/01/18 22:43:07 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"


#define DWORD_ALIGN_BITS(b)   (((b) / 32) + (((b) % 32 > 0) ? 1 : 0))

/*******************************************************************************/

extern FXAPI FXbool fxloadCUR(FXStream& store,FXuchar*& source,FXuchar*& mask,FXint& width,FXint& height,FXint & xspot,FXint & yspot);

/*******************************************************************************/

static FXuint read16(FXStream& store){
  FXuchar c1,c2;
  store >> c1 >> c2;
  return ((FXuint)c1) | (((FXuint)c2)<<8);
  }

/*******************************************************************************/

static FXuint read32(FXStream& store){
  FXuchar c1,c2,c3,c4;
  store >> c1 >> c2 >> c3 >> c4;
  return ((FXuint)c1) | (((FXuint)c2)<<8) | (((FXuint)c3)<<16) | (((FXuint)c4)<<24);
  }

/*******************************************************************************/



FXbool fxloadCUR(FXStream& store,FXuchar*& source,FXuchar*& mask,FXint& width,FXint& height,FXint& xspot,FXint& yspot){
  FXuchar c,bWidth, bHeight, bColorCount, bReserved;
  FXint i,j,cmaplen=0,idReserved, idType, idCount, dwImageOffset,
        MaskLength,BytesPerMaskLine,biSize, biWidth, biHeight,tmpmsk,tmpsrc,
        biPlanes,biBitCount, biCompression,biClrUsed, biClrImportant;

  // idReserved, always set to 0
  idReserved = read16(store);
  if(idReserved!=0) return FALSE;

  // idType, always set to 2
  idType = read16(store);
  if(idType != 2) return FALSE;

  // Number of icon images
  idCount = read16 (store);

  // Width
  store >> bWidth;

  // Height
  store >> bHeight;

  // Number of colors used
  store >> bColorCount;

  // Not used, 0
  store >> bReserved;
  //  if (bReserved != 0) return FALSE;  - not in all ICO file....

  xspot = read16(store);
  yspot = read16(store);

  // dwBytesInRes - total number of bytes in images (including palette data)
  read32(store);

  // Location of image from the beginning of file
  dwImageOffset = read32(store);

  store.position(dwImageOffset);

  // Read header
  biSize          = read32(store);
  biWidth         = read32(store);
  biHeight        = read32(store) / 2;
  biPlanes        = read16(store);
  biBitCount      = read16(store);
  biCompression   = read32(store);

  if(biBitCount!=1) return FALSE;
  // biSizeImage
  read32(store);

  // biXPelsPerMeter
  read32(store);

  // biYPelsPerMeter
  read32(store);
  biClrUsed       = read32(store);
  biClrImportant  = read32(store);

  // 40 bytes read from biSize to biClrImportant
  j=biSize-40;
  for(i=0; i<j; i++) store >> c;

  // Load up colormap, if any
  cmaplen = biClrUsed ? biClrUsed : 1<<biBitCount;
  for(i=0; i<cmaplen; i++){
    store >> c >> c >> c >> c;
    }

  // Calculate size in bytes of image and mask
  BytesPerMaskLine = 4*DWORD_ALIGN_BITS(biHeight);
  MaskLength = BytesPerMaskLine * biHeight;

  FXMALLOC(&source,FXuchar,MaskLength);
  if(!source) return FALSE;

  FXMALLOC(&mask,FXuchar,MaskLength);
  if(!mask){ FXFREE(&source); return FALSE; }

  // Read source
  for(i=MaskLength-1; i>=0; i-=BytesPerMaskLine){
    for(j=BytesPerMaskLine-1; j>=0; j--){
      store >> source[i-j];
      }
    }

  // Read mask
  for(i=MaskLength-1; i>=0; i-=BytesPerMaskLine){
    for(j=BytesPerMaskLine-1; j>=0; j--){
      store >> mask[i-j];
      }
    }

  // Convert to pixels and shape mask
  for(i=0; i<MaskLength; i++){
    tmpmsk = (source[i]) & (~mask[i]);
    tmpsrc = (~source[i]) & (~mask[i]);
    source[i] = FXBITREVERSE(tmpsrc);
    mask[i]   = FXBITREVERSE((tmpmsk | tmpsrc));
    }

  width = biWidth;
  height = biHeight;

  return TRUE;
  }


