/********************************************************************************
*                                                                               *
*                          B M P   I n p u t / O u t p u t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: fxbmpio.cpp,v 1.20.4.3 2002/11/20 07:45:52 fox Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"



/*
  To do:
  - Writer should use fxezquantize() and if the number of colors is less than
    256, use 8bpp RLE compressed output; if less that 4, use 4bpp RLE compressed
    output, else if less than 2, use monochrome.
    Writer should do this only when no loss of fidelity occurs.
*/

#define BIH_RGB      0
#define BIH_RLE8     1
#define BIH_RLE4     2

#define WIN_OS2_OLD 12
#define WIN_NEW     40
#define OS2_NEW     64


// MONO returns total intensity of r,g,b triple (i = .33R + .5G + .17B)
#define MONO(r,g,b) (((FXuint)(r)*11+(FXuint)(g)*16+(FXuint)(b)*5)>>5)

/*******************************************************************************/


/// Load a bmp file from a stream
extern FXAPI FXbool fxloadBMP(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a bmp file to a stream
extern FXAPI FXbool fxsaveBMP(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


/*******************************************************************************/

static FXuint read32(FXStream& store){
  FXuchar c1,c2,c3,c4;
  store >> c1 >> c2 >> c3 >> c4;
  return ((FXuint)c1) | (((FXuint)c2)<<8) | (((FXuint)c3)<<16) | (((FXuint)c4)<<24);
  }


static FXuint read16(FXStream& store){
  FXuchar c1,c2;
  store >> c1 >> c2;
  return ((FXuint)c1) | (((FXuint)c2)<<8);
  }


static FXbool loadBMP1(FXStream& store,FXuchar* pic8,FXint w,FXint h){
  FXint   i,j,bitnum,padw;
  FXuchar *pp,c;
  c=0;
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


static FXbool loadBMP4(FXStream& store,FXuchar* pic8,FXint w,FXint h,FXint comp){
  FXint    i,j,x,y,nybnum,padw;
  FXuchar *pp,c,c1;
  c=c1=0;

  // Read uncompressed data
  if(comp==BIH_RGB){
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
    }

  // Read RLE4 compressed data
  else if(comp==BIH_RLE4){
    x=y=0;
    pp=pic8+x+(h-y-1)*w;
    while(y<h){
      store>>c;

      // Encoded mode c!=0
      if(c){
        store>>c1;
        for(i=0; i<c; i++,x++,pp++){
          *pp=(i&1)?(c1&0x0f):((c1>>4)&0x0f);
          }
        }

      // Escape codes: c==0
      else{
        store>>c;

        // End of line
        if(c==0){
          x=0;
          y++;
          pp=pic8+x+(h-y-1)*w;
          }

              // End of pic8
        else if(c==0x01){
          break;
          }

        // Delta
        else if(c==0x02){
          store>>c; x+=c;
          store>>c; y+=c;
          pp=pic8+x+(h-y-1)*w;
          }

        // Absolute mode
        else{
          for(i=0; i<c; i++,x++,pp++){
            if((i&1)==0) store>>c1;
            *pp=(i&1)?(c1&0x0f):((c1>>4)&0x0f);
                  }

          // Read pad byte
          if(((c&3)==1)||((c&3)==2)) store>>c1;
          }
        }
      }
    }

  // Unknown compression type
  else{
    return FALSE;
    }
  return TRUE;
  }



static FXbool loadBMP8(FXStream& store,FXuchar* pic8,FXint w,FXint h,FXint comp){
  FXint    i,j,padw,x,y;
  FXuchar *pp,c,c1;

  // Read uncompressed data
  if(comp==BIH_RGB){
    padw=((w+3)/4)*4;
    for(i=h-1; i>=0; i--){
      pp=pic8+(i*w);
      for(j=0; j<padw; j++){
        store>>c;
        if(j<w) *pp++=c;
        }
      }
    }

  // Read RLE8 compressed data
  else if(comp==BIH_RLE8){
    x=y=0;
    pp=pic8+x+(h-y-1)*w;
    while(y<h){
      store>>c;

      // Encoded mode
      if(c){
        store>>c1;
        for(i=0; i<c; i++,x++,pp++) *pp=c1;
        }

      // Escape codes: c==0
      else{
        store>>c;

        // End of line
        if(c==0x00){
          x=0;
          y++;
          pp=pic8+x+(h-y-1)*w;
          }

        // End of pic8 */
        else if(c==0x01){
          break;
          }

        // delta
        else if(c==0x02){
          store>>c; x+=c;
          store>>c; y+=c;
          pp=pic8+x+(h-y-1)*w;
          }

        // Absolute mode
        else{
          for(i=0; i<c; i++,x++,pp++){
            store>>c1;
            *pp=c1;
            }

          // Odd length run: read an extra pad byte
          if(c&1) store>>c1;
          }
        }
      }
    }

  // Unknown compression type
  else{
    return FALSE;
    }
  return TRUE;
  }


// Contributed by Janusz Ganczarski <janusz.ganczarski@wp.pl>
static FXbool loadBMP16(FXStream& store,FXuchar* pic16,FXint w,FXint h){
  register int i,j,padb;
  FXuchar *pp,c;
  FXuint rgb16;
  padb=(4-((w*2)%4))&0x03;
  for(i=h-1; i>=0; i--){
    pp=pic16+(i*w*3);
    for(j=0; j<w; j++){
      rgb16=read16(store);
      *pp++=((rgb16 >> 10) & 0x1F) << 3; // R
      *pp++=((rgb16 >> 5) & 0x1F) << 3;  // G
      *pp++=(rgb16 & 0x1F) << 3;         // B
      }
    for(j=0; j<padb; j++) store >> c;
    }
  return TRUE;
  }


static FXbool loadBMP24(FXStream& store,FXuchar* pic24,FXint w,FXint h){
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


static FXbool loadBMP32(FXStream& store,FXuchar* pic32,FXint w,FXint h){
  register int i,j;
  FXuchar *pp,c;
  for(i=h-1; i>=0; i--){
    pp=pic32+(i*w*3);
    for(j=0; j<w; j++){
      store >> pp[2];       // Blue
      store >> pp[1];       // Green
      store >> pp[0];       // Red
      store >> c;
      pp += 3;
      }
    }
  return TRUE;
  }

/*******************************************************************************/


// Load image from stream
FXbool fxloadBMP(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  FXuchar c1,c2;
  FXint bfSize, bfOffBits, biSize, biWidth, biHeight, biPlanes;
  FXint biBitCount, biCompression, biSizeImage, biXPelsPerMeter;
  FXint biYPelsPerMeter, biClrUsed, biClrImportant,bPad,maxpixels;
  FXuchar colormap[256*3];
  FXint i,j,ix,ok,cmaplen=0;

  // Check signature
  store >> c1 >> c2;
  if(!(c1=='B' && c2=='M')) return FALSE;

  // Get size and offset
  bfSize=read32(store);
  read16(store);
  read16(store);
  bfOffBits=read32(store);

  biSize=read32(store);

  // New bitmap format
  if(biSize == WIN_NEW || biSize == OS2_NEW){
    biWidth         = read32(store);
    biHeight        = read32(store);
    biPlanes        = read16(store);
    biBitCount      = read16(store);
    biCompression   = read32(store);
    biSizeImage     = read32(store);
    biXPelsPerMeter = read32(store);
    biYPelsPerMeter = read32(store);
    biClrUsed       = read32(store);
    biClrImportant  = read32(store);
    }

  // Old format
  else{
    biWidth         = read16(store);
    biHeight        = read16(store);
    biPlanes        = read16(store);
    biBitCount      = read16(store);

    // Not in old versions so have to compute them
    biSizeImage = (((biPlanes*biBitCount*biWidth)+31)/32)*4*biHeight;

    biCompression   = BIH_RGB;
    biXPelsPerMeter = biYPelsPerMeter = 0;
    biClrUsed       = biClrImportant  = 0;
    }

  FXTRACE((150,"fxloadBMP: width=%d height=%d nbits=%d compression=%d\n",biWidth,biHeight,biBitCount,biCompression));

  // Ought to be 1
  if(biPlanes!=1) return FALSE;

  // Check for supported depths
  if(biBitCount!=1 && biBitCount!=4 && biBitCount!=8 && biBitCount!=16 && biBitCount!=24 && biBitCount!=32) return FALSE;

  // Check for supported compressions
  if(biCompression!=BIH_RGB && biCompression!=BIH_RLE4 && biCompression!=BIH_RLE8) return FALSE;

  // Skip ahead to colormap
  bPad=0;
  if(biSize!=WIN_OS2_OLD){

    // 40 bytes read from biSize to biClrImportant
    j=biSize-40;
    for(i=0; i<j; i++) store >> c1;
    bPad=bfOffBits-(biSize+14);
    }

  // load up colormap, if any
  if(biBitCount!=24 && biBitCount!=16 && biBitCount!=32){
    cmaplen = biClrUsed ? biClrUsed : 1 << biBitCount;
    for(i=0; i<cmaplen; i++){
      store >> colormap[3*i+2];
      store >> colormap[3*i+1];
      store >> colormap[3*i+0];
      if(biSize!=WIN_OS2_OLD){
        store >> c1;
        bPad -= 4;
        }
      }
    }

  // Waste any unused bytes between the colour map (if present)
  // and the start of the actual bitmap data.
  if(biSize!=WIN_OS2_OLD){
    while(bPad>0){
      store >> c1;
      bPad--;
      }
    }

  // Allocate memory
  maxpixels=biWidth*biHeight;
  FXMALLOC(&data,FXuchar,maxpixels*3);
  if(!data) return FALSE;

  // load up the image
  if(biBitCount==1){
    ok=loadBMP1(store,&data[2*maxpixels],biWidth,biHeight);
    }
  else if(biBitCount==4){
    ok=loadBMP4(store,&data[2*maxpixels],biWidth,biHeight,biCompression);
    }
  else if(biBitCount==8){
    ok=loadBMP8(store,&data[2*maxpixels],biWidth,biHeight,biCompression);
    }
  else if(biBitCount==16){
    ok=loadBMP16(store,data,biWidth,biHeight);
    }
  else if(biBitCount==24){
    ok=loadBMP24(store,data,biWidth,biHeight);
    }
  else{
    ok=loadBMP32(store,data,biWidth,biHeight);
    }

  if(!ok) return FALSE;

  width=biWidth;
  height=biHeight;

  // Apply colormap
  if(biBitCount!=24 && biBitCount!=16 && biBitCount!=32){
    for(i=0; i<maxpixels; i++){
      ix=data[2*maxpixels+i];
      data[3*i+0]=colormap[3*ix+0];
      data[3*i+1]=colormap[3*ix+1];
      data[3*i+2]=colormap[3*ix+2];
      }
    }

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


static void write32(FXStream& store,FXuint i){
  FXuchar c1,c2,c3,c4;
  c1=i&0xff;
  c2=(i>>8)&0xff;
  c3=(i>>16)&0xff;
  c4=(i>>24)&0xff;
  store << c1 << c2 << c3 << c4;
  }


static FXbool writeBMP1(FXStream& store,const FXuchar* pic8,FXint w,FXint h){
  FXint    i,j,bitnum,padw;
  const FXuchar *pp;
  FXuchar c=0;
  padw=((w+31)/32)*32;
  for(i=h-1; i>=0; i--){
    pp=pic8+(i*w);
    for(j=bitnum=c=0; j<=padw; j++,bitnum++){
      if(bitnum==8){
        store << c;
        bitnum=c=0;
        }
      c<<=1;
      if(j<w){
        c|=(0x01 & *pp++);
        }
      }
    }
  return TRUE;
  }


static FXbool writeBMP4(FXStream& store,const FXuchar* pic8,FXint w,FXint h){
  FXint    i,j,nybnum,padw;
  const FXuchar *pp;
  FXuchar c=0;
  padw=((w+7)/8)*8;
  for(i=h-1; i>=0; i--){
    pp=pic8+i*w;
    for(j=nybnum=c=0; j<=padw; j++,nybnum++){
      if(nybnum==2){
        store << c;
        nybnum=c=0;
        }
      c<<=4;
      if(j<w){
        c|=(0x0f & *pp++);
        }
      }
    }
  return TRUE;
  }


static FXbool writeBMP8(FXStream& store,const FXuchar* pic8,FXint w,FXint h){
  FXint    i,j,padw;
  const FXuchar *pp;
  FXuchar c=0;
  padw=((w+3)/4)*4;
  for(i=h-1; i>=0; i--){
    pp=pic8+i*w;
    for(j=0; j<w; j++)  store << *pp++;
    for( ; j<padw; j++) store << c;
    }
  return TRUE;
  }


static FXbool writeBMP24(FXStream& store,const FXuchar* pic24,FXint w,FXint h){
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


// Save a bmp file to a stream
FXbool fxsaveBMP(FXStream& store,const FXuchar *data,FXColor,FXint width,FXint height){
  FXuchar rmap[256],gmap[256],bmap[256],c1;
  FXint bperlin,biBitCount,biClrUsed,i,ok;

  biBitCount=24;
  biClrUsed=0;

  bperlin=((width*biBitCount+31)/32)*4;     // # bytes written per line

  // Compute file size
  i = 14 +                        // size of bitmap file header
      40 +                        // size of bitmap info header
      biClrUsed*4 +               // size of colormap
      bperlin*height;             // size of image data

  // BitmapFileHeader
  store << 'B';                   // Magic number
  store << 'M';
  write32(store,i);                       // Size of file
  write16(store,0);                       // reserved1
  write16(store,0);                       // reserved2
  write32(store,14+40+(biClrUsed*4));     // offset from BOfile to BObitmap

  // BitmapInfoHeader
  write32(store,40);              // biSize: size of bitmap info header
  write32(store,width);           // biWidth
  write32(store,height);          // biHeight
  write16(store,1);               // biPlanes:  must be '1'
  write16(store,biBitCount);      // 1,4,8, or 24
  write32(store,BIH_RGB);         // biCompression:  BIH_RGB, BIH_RLE8 or BIH_RLE4
  write32(store,bperlin*height);  // biSizeImage:  size of raw image data
  write32(store,75*39);           // biXPelsPerMeter: (75dpi * 39" per meter)
  write32(store,75*39);           // biYPelsPerMeter: (75dpi * 39" per meter)
  write32(store,biClrUsed);       // Number of colors used in cmap
  write32(store,biClrUsed);       // Important colors

  // Win3ColorTable
  if(biBitCount!=24){
    c1=0;
    for(i=0; i<biClrUsed; i++){
      store << bmap[i];
      store << gmap[i];
      store << rmap[i];
      store << c1;
      }
    }

  // Image
  if(biBitCount==1){
    ok=writeBMP1(store,data,width,height);
    }
  else if(biBitCount==4){
    ok=writeBMP4(store,data,width,height);
    }
  else if(biBitCount==8){
    ok=writeBMP8(store,data,width,height);
    }
  else{
    ok=writeBMP24(store,data,width,height);
    }
  return ok;
  }
