/********************************************************************************
*                                                                               *
*                          I C O   I n p u t / O u t p u t                      *
*                                                                               *
*********************************************************************************
* Author: Janusz Ganczarski (POWER)   Email: JanuszG@enter.net.pl               *
* Based on fxbmpio.cpp (FOX) and ico2xpm.c (Copyright (C) 1998 Philippe Martin) *
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
* $Id: fxicoio.cpp,v 1.4 2001/12/19 18:30:01 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"


/* from fxxpmio.cpp */
#define ALPHA_COLOR   FXRGB(252,253,254)

/* from: ico2xpm.c -- Convert icons to pixmaps
 * Copyright (C) 1998 Philippe Martin
 * Modified by Brion Vibber */

#define DWORD_ALIGN_BITS(b)   (((b) / 32) + (((b) % 32 > 0) ? 1 : 0))
#define MASK_BYTE(x,y)        (BytesPerMaskLine * (height - 1 - (y)) + (x) / 8)
#define MASK_BIT(x,y)         (7 - (x) % 8)
#define IS_TRANSPARENT(x,y)   (mask[MASK_BYTE((x),(y))] & (1 << MASK_BIT((x),(y))))
#define SET_TRANSPARENT(x,y)  (mask[MASK_BYTE((x),(y))] |= (1 << MASK_BIT((x),(y))))

/*
  To do:
*/


/// Load a ICO file from a stream
extern FXAPI FXbool fxloadICO(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a ICO file to a stream
extern FXAPI FXbool fxsaveICO(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);



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

static FXbool loadDIB1(FXStream& store,FXuchar* pic8,FXint w,FXint h){
  FXint   i,j,bitnum,padw;
  FXuchar *pp,c=0;
  padw=((w+31)/32)*32;

  // Read data
  for(i=h-1; i>=0; i--){
    pp=pic8+(i*w);
    for(j=bitnum=0; j<padw; j++,bitnum++){
      if((bitnum&7)==0){
        store>>c;
        bitnum=0;
        }
      if(j<w){
        *pp++=(c&0x80)?1:0;
        c<<=1;
        }
      }
    }
  return TRUE;
  }


static FXbool loadDIB4(FXStream& store,FXuchar* pic8,FXint w,FXint h){
  FXint    i,j,nybnum,padw;
  FXuchar *pp,c=0;

  // Read data
  padw=((w+7)/8)*8;
  for(i=h-1; i>=0; i--){
    pp=pic8+(i*w);
    for(j=nybnum=0; j<padw; j++,nybnum++){
      if((nybnum&1)==0){
        store>>c;
        nybnum=0;
        }
      if(j<w){
        *pp++=(c&0xf0)>>4;
        c<<=4;
        }
      }
    }
  return TRUE;
  }



static FXbool loadDIB8(FXStream& store,FXuchar* pic8,FXint w,FXint h){
  FXint    i,j,padw;
  FXuchar *pp,c;

  // Read data
  padw=((w+3)/4)*4;
  for(i=h-1; i>=0; i--){
    pp=pic8+(i*w);
    for(j=0; j<padw; j++){
      store>>c;
      if(j<w) *pp++=c;
      }
    }
  return TRUE;
  }



static FXbool loadDIB24(FXStream& store,FXuchar* pic24,FXint w,FXint h){
  int   i,j,padb;
  FXuchar *pp,c;
  padb=(4-((w*3)%4))&0x03;
  for(i=h-1; i>=0; i--){
    pp=pic24+(i*w*3);
    for(j=0; j<w; j++){
      store >> pp[2];       // Blue
      store >> pp[1];       // Green
      store >> pp[0];       // Red
      pp += 3;
      }
    for(j=0; j<padb; j++) store>>c;
    }
  return TRUE;
  }


FXbool fxloadDIB(FXStream& store,FXuchar*& data,FXint& width,FXint& height){
  FXuchar c;
  FXint biSize, biWidth, biHeight, biPlanes;
  FXint biBitCount, biCompression;
  FXint biClrUsed, biClrImportant,maxpixels;
  FXuchar colormap[256*3];
  FXint i,j,ix,ok,cmaplen=0;

  // read header
  biSize=read32(store);
  biWidth         = read32(store);
  biHeight        = read32(store) / 2;
  biPlanes        = read16(store);
  biBitCount      = read16(store);
  biCompression   = read32(store);

  // biSizeImage
  read32(store);

  // biXPelsPerMeter
  read32(store);

  // biYPelsPerMeter
  read32(store);
  biClrUsed       = read32(store);
  biClrImportant  = read32(store);

  // Error checking
  if((biBitCount!=1 && biBitCount!=4 && biBitCount!=8 && biBitCount!=24) || biPlanes!=1 || biCompression != 0){
    return FALSE;
    }

  // More checking
  if((biBitCount==1 || biBitCount==24) && biCompression!=0){
    return FALSE;
    }

  // 40 bytes read from biSize to biClrImportant
  j=biSize-40;
  for(i=0; i<j; i++) store >> c;

  // load up colormap, if any
  if(biBitCount!=24){
    cmaplen = biClrUsed ? biClrUsed : 1 << biBitCount;
    for(i=0; i<cmaplen; i++){
      store >> colormap[3*i+2];
      store >> colormap[3*i+1];
      store >> colormap[3*i+0];
      store >> c;
      }
    }

  //FXTRACE((150,"fxloadICO: width=%d height=%d nbits=%d\n",biWidth,biHeight,biBitCount));

  // Allocate memory
  maxpixels=biWidth*biHeight;
  FXMALLOC(&data,FXuchar,maxpixels*3);
  if(!data) return FALSE;

  // load up the image
  if(biBitCount==1){
    ok=loadDIB1(store,&data[2*maxpixels],biWidth,biHeight);
    }
  else if(biBitCount==4){
    ok=loadDIB4(store,&data[2*maxpixels],biWidth,biHeight);
    }
  else if(biBitCount==8){
    ok=loadDIB8(store,&data[2*maxpixels],biWidth,biHeight);
    }
  else{
    ok=loadDIB24(store,data,biWidth,biHeight);
    }

  if(!ok) return FALSE;

  width=biWidth;
  height=biHeight;

  // Apply colormap
  if(biBitCount!=24){
    for(i=0; i<maxpixels; i++){
      ix=data[2*maxpixels+i];
      data[3*i+0]=colormap[3*ix+0];
      data[3*i+1]=colormap[3*ix+1];
      data[3*i+2]=colormap[3*ix+2];
      }
    }

  return TRUE;
  }



// Load ICO image from stream
FXbool fxloadICO(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  FXint idReserved, idType, idCount, dwImageOffset;
  FXuchar bWidth, bHeight, bColorCount, bReserved;
  FXuchar *mask;
  FXint MaskLength,BytesPerMaskLine;
  FXbool HaveTransparentPixels;
  FXint i,x,y;

  // idReserved, always set to 0
  idReserved = read16(store);
  if(idReserved!=0) return FALSE;

  // idType, always set to 1
  idType = read16(store);
  if(idType != 1) return FALSE;

  // number of icon images
  idCount = read16(store);

  // width
  store >> bWidth;

  // height
  store >> bHeight;

  // number of colors used
  store >> bColorCount;

  // not used, 0
  store >> bReserved;
//  if (bReserved != 0) return FALSE;  - not in all ICO file....

  // wPlanes, not used, 0
  read16(store);

  // wBitCount, not used, 0
  read16(store);

  // dwBytesInRes - total number of bytes in images (including palette data)
  read32(store);

  // location of image from the beginning of file
  dwImageOffset = read32(store);

  // read first icon only!
  //if(idCount>1) store.position(dwImageOffset);
  store.position(dwImageOffset);

  // load DIB
  if(fxloadDIB(store,data,width,height)!=TRUE) return FALSE;

  // calculate size of Mask
  BytesPerMaskLine = 4 * DWORD_ALIGN_BITS (width);
  MaskLength = BytesPerMaskLine * height;
  FXMALLOC(&mask,FXuchar,MaskLength);
  if(!mask) return FALSE;

  // read Mask
  for(i=0; i<MaskLength; i++){
    store >> mask[i];
    }

  // check for at least one transparent pixel
  HaveTransparentPixels = FALSE;
  for(y=0; y<height && HaveTransparentPixels==FALSE; y++){
    for(x=0; x<width; x++){
      if (IS_TRANSPARENT(x,y)){
        HaveTransparentPixels = TRUE;
        break;
        }
      }
    }

  // transparent color = FXRGB(252,253,254)
  if(HaveTransparentPixels==TRUE){
    transp=ALPHA_COLOR;
    for(y=0; y<height ; y++){
      for(x=0; x<width; x++){
        if(IS_TRANSPARENT(x,y)){
          data[(y*width+x)*3] = 252;      // R
          data[(y*width+x)*3+1] = 253;  // G
          data[(y*width+x)*3+2] = 254;  // B
          }
        }
      }
    }
  else{
    transp=0;
    }

  // delete mask
  FXFREE(&mask);

  return TRUE;
  }


/*******************************************************************************/


static void write16(FXStream& store,FXuint i){
  FXuchar c1,c2;
  c1=i&0xff;
  c2=(i>>8)&0xff;
  store << c1 << c2;
  }


static void write32(FXStream& store,FXuint i){
  FXuchar c1,c2,c3,c4;
  c1=i&0xff;
  c2=(i>>8)&0xff;
  c3=(i>>16)&0xff;
  c4=(i>>24)&0xff;
  store << c1 << c2 << c3 << c4;
  }

static FXbool writeDIB24(FXStream& store,const FXuchar* pic24,FXint w,FXint h){
  FXint   i,j,padb;
  const FXuchar *pp;
  FXuchar c=0;
  padb=(4-((w*3)%4))&0x03;
  for(i=h-1; i>=0; i--){
    pp=pic24+(i*w*3);
    for(j=0; j<w; j++){
      store << pp[2];
      store << pp[1];
      store << pp[0];
      pp+=3;
      }
    for(j=0; j<padb; j++) store << c;
    }
  return TRUE;
  }


// Save a ICO file to a stream
FXbool fxsaveICO(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height){
  FXuchar bColorCount, bReserved;
  FXint dwBytesInRes, dwImageOffset;
  FXint biSize, biSizeImage;
  FXuchar *mask,c;
  FXint MaskLength,BytesPerMaskLine;
  FXuint i,x,y;

  // calculate size of Mask
  BytesPerMaskLine = 4 * DWORD_ALIGN_BITS (width);
  MaskLength = BytesPerMaskLine * height;

  bColorCount = 0;
  bReserved = 0;
  dwBytesInRes = 40 + width * height * 3 + MaskLength;
  dwImageOffset = 22;
  biSize = 40;
  biSizeImage = width * height * 3;

  // idReserved, always set to 0
  write16 (store,0);

  // idType, always set to 1
  write16 (store,1);

  // Number of icon images
  write16 (store,1);

  // Width
  store << (FXuchar)width;

  // Height
  store << (FXuchar)height;

  // Number of colors used
  store << bColorCount;

  // not used, 0
  store << bReserved;

  // wPlanes, not used, 0
  write16 (store,0);

  // wBitCount, not used, 0
  write16 (store,0);

  // total number of bytes in images (including palette data)
  write32 (store,dwBytesInRes);

  // location of image from the beginning of file
  write32 (store,dwImageOffset);

  // biSize = 40
  write32 (store,biSize);

  // biWidth
  write32(store,width);

  // biHeight
  write32(store,height + height);

  // biPlanes
  write16(store,1);

  // biBitCount
  write16(store,24);

  // biCompression
  write32(store,0);

  // biSizeImage
  write32(store,biSizeImage);

  // biXPelsPerMeter
  write32(store,75);

  // biYPelsPerMeter
  write32(store,75);

  // biClrUsed
  write32(store,0);

  // biClrImportant
  write32(store,0);

  //FXTRACE((150,"fxsaveICO: width=%d height=%d nbits=%d\n",width,height+height,biBitCount));

  // write DIB
  if(writeDIB24(store,data,width,height)!=TRUE) return FALSE;

  // no transparent color, write mask
  if(transp==0){
    c=0;
    for(i=0; i<MaskLength; i++) store << c;
    }
  else{
    // Get R, G, B value from transparent color
    FXuchar transpR = FXREDVAL(transp);
    FXuchar transpG = FXGREENVAL(transp);
    FXuchar transpB = FXBLUEVAL(transp);

    // create mask array
    FXMALLOC(&mask,FXuchar,MaskLength);
    if(!mask) return FALSE;

    // clear mask
    for(i=0; i<MaskLength; i++) mask[i]=0;

    // get color and compare it with transparent
    const FXuchar *pdata = data;
    for(y=0; y<height ; y++){
      for(x=0; x<width; x++){
        FXuchar R = *pdata++;
        FXuchar G = *pdata++;
        FXuchar B = *pdata++;

        // set transparent byte to the mask
        if(transpR==R && transpG==G && transpB==B) SET_TRANSPARENT(x,y);
        }
      }

    // write mask
    for(i=0; i<MaskLength; i++){
      store << mask [i];
      }

    // delete mask
    FXFREE(&mask);
    }

  return TRUE;
  }

