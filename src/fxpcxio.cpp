/********************************************************************************
*                                                                               *
*                          P C X   I n p u t / O u t p u t                      *
*                                                                               *
*********************************************************************************
* Author: Janusz Ganczarski (POWER)   Email: JanuszG@enter.net.pl               *
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
* $Id: fxpcxio.cpp,v 1.5 2001/12/29 06:27:21 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"



/*
  To do:
   - Load 32 bpp PCX image
   - Save 1, 4 and 256 bpi PCX image
*/


/*******************************************************************************/

/// Load a PCX file from a stream
extern FXAPI FXbool fxloadPCX(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a PCX file to a stream
extern FXAPI FXbool fxsavePCX(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


/*******************************************************************************/

static FXuint read16(FXStream& store){
  FXuchar c1,c2;
  store >> c1 >> c2;
  return ((FXuint)c1) | (((FXuint)c2)<<8);
  }


static FXbool loadPCX1(FXStream& store,FXuchar* pic,FXuchar* colormap,FXint w,FXint h,FXint bpl){
  FXuchar *pp = pic,c,*cm;
  FXint x,y,i,rc;

  // Decompress image
  for(y=0; y<h; y++){
    x=0;
    while(x<bpl){
      store >> c;
      if((c&0xC0)==0xC0){
        rc=c&0x3F;
        store >> c;
        while(rc--){
          for(i=0; i<8; i++){
            if(x*8+i<w){
              cm=colormap+3*((FXuchar)(c<<i)>>7);
              *pp++=*cm++;
              *pp++=*cm++;
              *pp++=*cm;
              }
            }
          x++;
          }
        }
      else{
        for(i=0; i<8; i++){
          if(x*8+i<w){
            cm = colormap + 3 * ((FXuchar)(c << i) >> 7);
            *pp++ = *cm++;
            *pp++ = *cm++;
            *pp++ = *cm;
            }
          }
        x++;
        }
      }
    }
  return TRUE;
  }


/*******************************************************************************/


static FXbool loadPCX4(FXStream& store,FXuchar* pic,FXuchar* colormap,FXint w,FXint h,FXint bpl){
  FXuchar *pp=pic,c,*cm;
  FXint x,y,rc;

  // Decompress image
  for(y = 0; y < h; y++){
    x = 0;
    while (x < bpl){
      store >> c;
      if ((c & 0xC0) == 0xC0){
        rc = c & 0x3F;
        store >> c;
        while (rc--){
          if (x + x < w){
            cm = colormap + 3 * (c >> 4);
            *pp++ = *cm++;
            *pp++ = *cm++;
            *pp++ = *cm;
            }
          if (x + x + 1 < w){
            cm = colormap + 3 * (c & 0x0F);
            *pp++ = *cm++;
            *pp++ = *cm++;
            *pp++ = *cm;
            }
          x++;
          }
        }
      else{
        if (x + x < w){
          cm = colormap + 3 * (c >> 4);
          *pp++ = *cm++;
          *pp++ = *cm++;
          *pp++ = *cm;
          }
        if (x + x + 1 < w){
          cm = colormap + 3 * (c & 0x0F);
          *pp++ = *cm++;
          *pp++ = *cm++;
          *pp++ = *cm;
          }
        x++;
        }
      }
    }
  return TRUE;
  }

/*******************************************************************************/


static FXbool loadPCX8(FXStream& store,FXuchar* pic,FXint w,FXint h,FXint bpl){
  FXuchar colormap[256*3];
  FXint x,y,i,ix,rc;
  FXuchar *cm,c,*pp;
  pp=&pic[w*h*2];

  // Decompress image
  for(y=0; y<h; y++){
    x=0;
    while(x<bpl){
      store >> c;
      if((c&0xC0)==0xC0){
        rc=c&0x3F;
        store >> c;
        while(rc--){
          if(x++<w) *pp++=c;
          }
        }
      else{
        if(x++<w) *pp++=c;
        }
      }
    }

  // Get VGApaletteID
  store >> c;

  // Check VGApaletteID
  if(c!=0xC) return FALSE;

  // Get Colormap
  cm=colormap;
  for(i=0; i<768; i++) store >> *cm++;

  // Apply colormap
  for(i=0; i<w*h; i++){
    ix=pic[2*w*h+i];
    pic[3*i+0]=colormap[3*ix+0];
    pic[3*i+1]=colormap[3*ix+1];
    pic[3*i+2]=colormap[3*ix+2];
    }

  return TRUE;
  }

/*******************************************************************************/


static FXbool loadPCX24(FXStream& store,FXuchar* pic,FXint w,FXint h,FXint bpl){
  FXint x,y,rc,rgb;
  FXuchar c,*pp;

  // Decompress image
  for (y = 0; y < h; y++){
    for (rgb = 0; rgb < 3; rgb++){
      pp = pic + 3 * y * w + rgb;
      x = 0;
      while(x < bpl){
        store >> c;
        if ((c & 0xC0) == 0xC0){
          rc = c & 0x3F;
          store >> c;
          while (rc--){
            if(x++ < w){
              *pp = c;
              pp += 3;
              }
            }
          }
        else{
          if(x++ < w){
            *pp = c;
            pp += 3;
            }
          }
        }
      }
    }
  return TRUE;
  }


/*******************************************************************************/


// Load PCX image from stream

FXbool fxloadPCX (FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  FXuchar Manufacturer, Version, Encoding, BitsPerPixel, Reserved, NPlanes;
  FXint Xmin, Ymin, Xmax, Ymax, BytesPerLine;
  FXuchar Colormap[16*3];
  FXint i,ok=FALSE;

  // Check Manufacturer
  store >> Manufacturer;
  if (Manufacturer != 0x0A) return FALSE;

  // Get Version
  store >> Version;

  // Get Encoding
  store >> Encoding;

  // Get BitsPerPixel
  store >> BitsPerPixel;

  // Get Xmin, Ymin, Xmax, Ymax
  Xmin = read16 (store);
  Ymin = read16 (store);
  Xmax = read16 (store);
  Ymax = read16 (store);

  // HDpi, VDpi
  read16 (store);
  read16 (store);

  // Get EGA/VGA Colormap
  for (i = 0; i < 48; i++)
    store >> Colormap [i];

  // Check Reserved
  store >> Reserved;
  if (Reserved != 0) return FALSE;

  // Get NPlanes
  store >> NPlanes;

  // Get BytesPerLine
  BytesPerLine = read16 (store);

  // Get 60 bytes
  for (i = 0; i < 30; i++) read16 (store);

  // Calculate Width and Height
  width = Xmax - Xmin + 1;
  height = Ymax - Ymin + 1;

  // Error checking
  if (BitsPerPixel != 1 && BitsPerPixel != 4 && BitsPerPixel != 8) return FALSE;
  if (NPlanes != 1 && NPlanes != 3) return FALSE;
  if (BitsPerPixel != 8 && NPlanes != 1) return FALSE;

  FXTRACE ((150,"fxloadPCX: width=%d height=%d nbits=%d\n",width,height,BitsPerPixel));

  // Allocate memory
  FXMALLOC (&data,FXuchar,width * height * 3);
  if(!data) return FALSE;

  // load up the image
  if(BitsPerPixel==1)
    ok=loadPCX1(store,data,Colormap,width,height,BytesPerLine);
  else if(BitsPerPixel==4)
    ok=loadPCX4(store,data,Colormap,width,height,BytesPerLine);
  else if(BitsPerPixel==8 && NPlanes==1)
    ok=loadPCX8(store,data,width,height,BytesPerLine);
  else if(BitsPerPixel==8 && NPlanes==3)
    ok=loadPCX24(store,data,width,height,BytesPerLine);

  if(!ok) return FALSE;

  // No transparent color:- bitmaps are opaque
  transp=0;

  return TRUE;
  }


/*******************************************************************************/


static void write16(FXStream& store,FXuint i){
  FXuchar c1,c2;
  c1=i&0xff;
  c2=(i>>8)&0xff;
  store << c1 << c2;
  }

/*******************************************************************************/


static FXbool writePCX24(FXStream& store,const FXuchar* pic,FXint w,FXint h){
  FXuint rgb;
  FXint x,y;
  const FXuchar *pp;
  FXuchar Current,Last,RLECount;

  for (y = 0; y < h; y++){
      // RGB planes
      for (rgb = 0; rgb < 3; rgb++){
          pp = pic + 3 * y * w + rgb;
          Last = *pp;
          pp += 3;
          RLECount = 1;

          // coding one line
          for (x = 1; x < w; x++){
              Current = *pp;
              pp += 3;

              // Last == Current
              if (Current == Last)
                {
                  RLECount++;
                  if (RLECount == 63)
                    {
                      store << (FXuchar)(0xC0 | RLECount) << Last;
                      RLECount = 0;
                    }
                }
              // Last != Current
              else
                {
                  if (RLECount)
                    if ((RLECount == 1) && (0xC0 != (0xC0 & Last)))
                      store << Last;
                    else
                      {
                        store << (FXuchar)(0xC0 | RLECount) << Last;
                        RLECount = 1;
                      }
                  Last = Current;
                  RLECount = 1;
                }
            }

          if (RLECount)
            if ((RLECount == 1) && (0xC0 != (0xC0 & Last)))
              store << Last;
            else
              store << (FXuchar)(0xC0 | RLECount) << Last;
        }
    }

  return TRUE;
}

/*******************************************************************************/

// Save a PCX file to a stream

FXbool fxsavePCX(FXStream& store,const FXuchar *data,FXColor,FXint width,FXint height){
  FXint i,ok=FALSE;
  FXuchar c;
  FXuchar Manufacturer = 10,
          Version = 5,
          Encoding = 1,
          BitsPerPixel = 8,
          NPlanes = 3,
          Reserved = 0;

  // Manufacturer, Version, Encoding and BitsPerPixel
  store << Manufacturer
        << Version
        << Encoding
        << BitsPerPixel;

  // Xmin = 0
  write16 (store,0);

  // Ymin = 0
  write16 (store,0);

  // Xmax = width - 1
  write16 (store,width - 1);

  // Ymax = height - 1
  write16 (store,height - 1);

  // HDpi = 75
  write16 (store,75);

  // VDpi = 75
  write16 (store,75);

  // Colormap
  for (i = 16; i < 64; i++) store << c;

  // Reserved
  store << Reserved;

  // NPlanes
  store << NPlanes;

  // BytesPerLine = width
  write16 (store,width);

  // PaletteInfo = 1
  write16 (store,1);

  // Filler
  for (i = 0; i < 58; i++) store << c;

  // Image
  if(BitsPerPixel==1)
    ok=FALSE;
  else if(BitsPerPixel==4)
    ok=FALSE;
  else if(BitsPerPixel==8 && NPlanes==1)
    ok=FALSE;
  else if(BitsPerPixel==8 && NPlanes==3)
    ok=writePCX24(store,data,width,height);

  return ok;
  }
