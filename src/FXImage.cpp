/********************************************************************************
*                                                                               *
*                             I m a g e    O b j e c t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: FXImage.cpp,v 1.32 1998/10/28 15:09:53 jvz Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"



/*
  To do:
  - Need a class for 1-bit images (bitmaps)
  - Need API's to set/clear clipping bitmaps ``masks''
  - Need API's for tiling and stippling
  
  - FXImage::create() renders rgb[a] data into X image
  - FXIcon::create() ditto, and also into X shape bitmap
  
  - Subclasses read in from particular format; like
     
      FXGIFImage(FXApp*,const FXuchar pixels,...)
      
    So you can:
    
      new FXLabel(dialog,NULL,new FXGIFImage(app,doc_icon),LAYOUT_FILL_X,...)
      
    Which should be much easier to use.
    
  - Use XSHM just added

*/

/*******************************************************************************/

/*
// Bit-reversal
static const FXuchar reverse_byte[256]={
  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
  };
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXImage,FXDrawable,NULL,0)


// For deserialization
FXImage::FXImage(){
  data=NULL;
  options=0;
  }


// Initialize
FXImage::FXImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):FXDrawable(a,w,h,0){
  if(pix) opts&=~IMAGE_OWNED;
  //opts|=(IMAGE_SHMI|IMAGE_SHMP);
  data=(FXuchar*)pix;
  options=opts;
  }


// Create image
void FXImage::create(){
  if(!xid){
    
    // App should exist
    if(!getApp()->display){ fxerror("%s::create: trying to create image before opening display.\n",getClassName()); }
  
    // Get depth
    depth=DefaultDepth(getApp()->display,DefaultScreen(getApp()->display));
    
    // Make pixmap
    xid=XCreatePixmap(getApp()->display,XDefaultRootWindow(getApp()->display),width,height,depth);
    if(!xid){ fxerror("%s::create: unable to create image.\n",getClassName()); }
  
    // Render pixels
    render();
    
    // Zap data 
    if(!(options&IMAGE_KEEP) && (options&IMAGE_OWNED)){
      options&=~IMAGE_OWNED;
      FXFREE(&data);
      }
    }
  }


// Destroy image
void FXImage::destroy(){
  if(xid){
    XFreePixmap(getApp()->display,xid);
    xid=0;
    }
  }


// // Restore image from drawable
// int FXImage::restore(){
//   if(xid){
//     if(image) XDestroyImage(image); 
//     image=XGetImage(getDisplay(),xid,0,0,width,height,AllPlanes,ZPixmap);
//     }
//   return image!=0;
//   }


// Get shift
FXuint getshift(FXuint mask){
  register FXuint sh=0;
  while(!(mask&0x80000000) && mask){ mask<<=1; sh++; }
  return sh;
  }

/*

       >  R  G  B
       R  x  4  2
       G     x  1
       B        x

*/


enum {
  RGB = 7,   // RGB 111
  BGR = 0,   // BGR 000
  RBG = 6,   // RBG 110
  GBR = 1,   // GBR 001
  BRG = 4,   // BRG 100
  GRB = 3    // GRB 011
  };
 

// True color
void FXImage::render_true(XImage *xim,FXuchar *img,FXuint step){
  register FXuint rmsk,gmsk,bmsk,rsh,gsh,bsh,val,r,g,b,jmp,rgborder;
  register FXuchar *pix;
  register FXint w,h;
  Visual *visual;
  FXuint bits;
  visual=DefaultVisual(getApp()->display,DefaultScreen(getApp()->display));
  rmsk=visual->red_mask;
  gmsk=visual->green_mask;
  bmsk=visual->blue_mask;
  rgborder=0;
  if(rmsk>gmsk) rgborder|=4;
  if(rmsk>bmsk) rgborder|=2;
  if(gmsk>bmsk) rgborder|=1;
  pix=(FXuchar*)xim->data;
  bits=xim->bits_per_pixel;
  if(xim->byte_order==MSBFirst) bits|=0x80; 
  switch(bits){
    
    case 0x08:
    case 0x88:                                        // MSB/LSB 8-bit true color (rare)
      jmp=xim->bytes_per_line-width;
      rsh=getshift(rmsk)-24;
      gsh=getshift(gmsk)-24;
      bsh=getshift(bmsk)-24;
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=((r>>rsh)&rmsk)|((g>>gsh)&gmsk)|((b>>bsh)&bmsk);
          *pix++=(FXuchar)val;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;

    case 0x8f:                                        // MSB 15-bit true color (5,5,5)
      jmp=xim->bytes_per_line-(width<<1);
      switch(rgborder){
        case RGB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<7) | ((g&0xf8)<<2) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<7) | ((g&0xf8)<<2) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<7) | ((b&0xf8)<<2) | ((g&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xf8)<<7) | ((b&0xf8)<<2) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<7) | ((r&0xf8)<<2) | ((g&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xf8)<<7) | ((r&0xf8)<<2) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;

    case 0x90:                                        // MSB 16-bit true color (5,6,5)
      jmp=xim->bytes_per_line-(width<<1);
      switch(rgborder){
        case RGB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<8) | ((g&0xfc)<<3) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<8) | ((g&0xfc)<<3) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<8) | ((b&0xf8)<<3) | ((g&0xfc)>>2);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xfc)<<8) | ((b&0xf8)<<2) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<8) | ((r&0xf8)<<3) | ((g&0xfc)>>2);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xfc)<<8) | ((r&0xf8)<<2) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;
/*
    case 0x8f:                                        // MSB 15-bit true color
    case 0x90:                                        // MSB 16-bit true color
      jmp=xim->bytes_per_line-(width<<1);
      rsh=getshift(rmsk)-16;
      gsh=getshift(gmsk)-16;
      bsh=getshift(bmsk)-16;
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(((r<<8)>>rsh)&rmsk)|(((g<<8)>>gsh)&gmsk)|(((b<<8)>>bsh)&bmsk);
          pix[0]=(FXuchar)(val>>8); pix[1]=(FXuchar)val; pix+=2;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
*/
      
    case 0x0f:                                        // LSB 15-bit true color (5,5,5)
      jmp=xim->bytes_per_line-(width<<1);
      switch(rgborder){
        case RGB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<7) | ((g&0xf8)<<2) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<7) | ((g&0xf8)<<2) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<7) | ((b&0xf8)<<2) | ((g&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xf8)<<7) | ((b&0xf8)<<2) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<7) | ((r&0xf8)<<2) | ((g&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xf8)<<7) | ((r&0xf8)<<2) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;

    case 0x10:                                        // LSB 16-bit true color (5,6,5) (very common!)
      jmp=xim->bytes_per_line-(width<<1);
      switch(rgborder){
        case RGB:                             // This one is two ops less; do this everywhere!
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=(FXuchar)(((g&0xfc)<<3)|(b>>3)); pix[1]=(FXuchar)((r&0xf8)|(g>>5)); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<8) | ((g&0xfc)<<3) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((r&0xf8)<<8) | ((b&0xf8)<<3) | ((g&0xfc)>>2);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xfc)<<8) | ((b&0xf8)<<2) | ((r&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((b&0xf8)<<8) | ((r&0xf8)<<3) | ((g&0xfc)>>2);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              val = ((g&0xfc)<<8) | ((r&0xf8)<<2) | ((b&0xf8)>>3);
              pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;
/*
    case 0x0f:                                        // LSB 15-bit true color
    case 0x10:                                        // LSB 16-bit true color
      jmp=xim->bytes_per_line-(width<<1);
      rsh=getshift(rmsk)-16;
      gsh=getshift(gmsk)-16;
      bsh=getshift(bmsk)-16;
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(((r<<8)>>rsh)&rmsk)|(((g<<8)>>gsh)&gmsk)|(((b<<8)>>bsh)&bmsk);
          pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix+=2;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
*/

    case 0x18:                                        // LSB 24-bit true color
      rgborder^=7;
    case 0x98:                                        // MSB 24-bit true color
//fprintf(stderr,"new 24bpp render\n");
      jmp=xim->bytes_per_line-(width*3);
      switch(rgborder){
        case RGB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=r; pix[1]=g; pix[2]=b; pix+=3;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=b; pix[1]=g; pix[2]=r; pix+=3;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=r; pix[1]=b; pix[2]=g; pix+=3;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=g; pix[1]=b; pix[2]=r; pix+=3;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=b; pix[1]=r; pix[2]=g; pix+=3;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=g; pix[1]=r; pix[2]=b; pix+=3;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;
/*
    case 0x98:                                        // MSB 24-bit true color
      jmp=xim->bytes_per_line-(width*3);
      rsh=getshift(rmsk)-8;
      gsh=getshift(gmsk)-8;
      bsh=getshift(bmsk)-8;
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(((r<<16)>>rsh)&rmsk)|(((g<<16)>>gsh)&gmsk)|(((b<<16)>>bsh)&bmsk);
          pix[0]=(FXuchar)(val>>16); pix[1]=(FXuchar)(val>>8); pix[2]=(FXuchar)val; pix+=3;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;

    case 0x18:                                        // LSB 24-bit true color
      jmp=xim->bytes_per_line-(width*3);
      rsh=getshift(rmsk)-8;
      gsh=getshift(gmsk)-8;
      bsh=getshift(bmsk)-8;
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(((r<<16)>>rsh)&rmsk)|(((g<<16)>>gsh)&gmsk)|(((b<<16)>>bsh)&bmsk);
          pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix[2]=(FXuchar)(val>>16); pix+=3;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
*/
    case 0x20:                                        // LSB 32-bit true color
//fprintf(stderr,"new LSB 32bpp render\n");
      jmp=xim->bytes_per_line-(width<<2);
      switch(rgborder){
        case RGB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=b; pix[1]=g; pix[2]=r; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=r; pix[1]=g; pix[2]=b; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=g; pix[1]=b; pix[2]=r; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=r; pix[1]=b; pix[2]=g; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=g; pix[1]=r; pix[2]=b; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[0]=b; pix[1]=r; pix[2]=g; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;
      
    case 0xa0:                                        // MSB 32-bit true color
//fprintf(stderr,"new MSB 32bpp render\n");
      jmp=xim->bytes_per_line-(width<<2);
      switch(rgborder){
        case RGB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[1]=r; pix[2]=g; pix[3]=b; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BGR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[1]=b; pix[2]=g; pix[3]=r; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case RBG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[1]=r; pix[2]=b; pix[3]=g; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GBR:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[1]=g; pix[2]=b; pix[3]=r; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case BRG:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[1]=b; pix[2]=r; pix[3]=g; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        case GRB:
          h=height-1;
          do{
            w=width-1;
            do{
              r=img[0]; g=img[1]; b=img[2]; img+=step;
              pix[1]=g; pix[2]=r; pix[3]=b; pix+=4;
              }
            while(--w>=0);
            pix+=jmp;
            }
          while(--h>=0);
          break;
        }
      break;
/*
    case 0xa0:                                        // MSB 32-bit true color
      jmp=xim->bytes_per_line-(width<<2);
      rsh=getshift(rmsk);
      gsh=getshift(gmsk);
      bsh=getshift(bmsk);
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(((r<<24)>>rsh)&rmsk)|(((g<<24)>>gsh)&gmsk)|(((b<<24)>>bsh)&bmsk);
          pix[0]=(FXuchar)(val>>24); pix[1]=(FXuchar)(val>>16); pix[2]=(FXuchar)(val>>8); pix[3]=(FXuchar)val; pix+=4;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
      
    case 0x20:                                        // LSB 32-bit true color
      jmp=xim->bytes_per_line-(width<<2);
      rsh=getshift(rmsk);
      gsh=getshift(gmsk);
      bsh=getshift(bmsk);
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(((r<<24)>>rsh)&rmsk)|(((g<<24)>>gsh)&gmsk)|(((b<<24)>>bsh)&bmsk);
          pix[0]=(FXuchar)val; pix[1]=(FXuchar)(val>>8); pix[2]=(FXuchar)(val>>16); pix[3]=(FXuchar)(val>>24); pix+=4;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
*/

    default:                                          // Unsupported mode
      fxerror("%s::render: unimplemented true-color mode: %dbpp.\n",xim->bits_per_pixel,getClassName());
      break;
    }
  }


// Render index-color mode
void FXImage::render_index(XImage *xim,FXuchar *img,FXuint step){
  register FXuint val,jmp,half;
  register FXuchar *pix;
  register FXint w,h;
  Visual *visual;
  FXuint bits;
  visual=DefaultVisual(getApp()->display,DefaultScreen(getApp()->display));
  pix=(FXuchar*)xim->data;
  bits=xim->bits_per_pixel;
  if(xim->byte_order==MSBFirst) bits|=0x80;
  switch(bits){

    case 0x08:                                        // MSB/LSB 8-bit pseudocolor
    case 0x88:
      jmp=xim->bytes_per_line-width;
      h=height-1;
      do{
        w=width-1;
        do{
          val=getApp()->matchColor(img[0],img[1],img[2]); img+=step;
          *pix++=(FXuchar)val;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;

    case 0x84:                                        // MSB 4-bit pseudocolor 
      jmp=xim->bytes_per_line-width;
      half=0;
      h=height-1;
      do{
        w=width-1;
        half=0;
        do{
          val=0x0f&getApp()->matchColor(img[0],img[1],img[2]); img+=step;
          if(half){ *pix++|=val; } else { *pix=val<<4; }
          half^=1;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;

    case 0x04:                                        // LSB 4-bit pseudocolor 
      jmp=xim->bytes_per_line-width;
      half=0;
      h=height-1;
      do{
        w=width-1;
        half=0;
        do{
          val=0x0f&getApp()->matchColor(img[0],img[1],img[2]); img+=step;
          if(half){ *pix++|=val<<4; } else { *pix=val; }
          half^=1;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;

    case 0x82:                                        // MSB 2-bit pseudocolor
      jmp=xim->bytes_per_line-width;
      h=height-1;
      do{
        w=width-1;
        half=0;
        do{
          val=0x03&getApp()->matchColor(img[0],img[1],img[2]); img+=step;
          if(half==0){ *pix=val<<6; } else if(half==1){ *pix|=val<<4; } else if(half==2){ *pix|=val<<2; } else { *pix++|=val; }
          half=(half+1)&3;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
      
    case 0x02:                                        // LSB 2-bit pseudocolor
      jmp=xim->bytes_per_line-width;
      h=height-1;
      do{
        w=width-1;
        half=0;
        do{
          val=0x03&getApp()->matchColor(img[0],img[1],img[2]); img+=step;
          if(half==0){ *pix=val; } else if(half==1){ *pix|=val<<2; } else if(half==2){ *pix|=val<<4; } else { *pix++|=val<<6; }
          half=(half+1)&3;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;

    default:
      fxerror("%s::render: unimplemented pseudo-color depth: %dbpp.\n",xim->bits_per_pixel,getClassName());
      break;
    } 
  }


// Render gray mode
void FXImage::render_gray(XImage *xim,FXuchar *img,FXuint step){
  register FXuint val,r,g,b,jmp;
  register FXuchar *pix;
  register FXint w,h;
  Visual *visual;
  FXuint bits;
  visual=DefaultVisual(getApp()->display,DefaultScreen(getApp()->display));
  pix=(FXuchar*)xim->data;
  bits=xim->bits_per_pixel;
  if(xim->byte_order==MSBFirst) bits|=0x80; 
  switch(bits){
    
    case 0x08:
    case 0x88:                                        // MSB/LSB 8-bit gray color (works!)
      jmp=xim->bytes_per_line-width;
      h=height-1;
      do{
        w=width-1;
        do{
          r=img[0]; g=img[1]; b=img[2]; img+=step;
          val=(255*(77*r+152*g+28*b))/65535;
          *pix++=(FXuchar)val;
          }
        while(--w>=0);
        pix+=jmp;
        }
      while(--h>=0);
      break;
      
    default:
      fxerror("%s::render: unimplemented gray-color depth: %dbpp.\n",xim->bits_per_pixel,getClassName());
      break;
    }
  }


// Render into pixmap
void FXImage::render(){
#ifdef HAVE_XSHM
  XShmSegmentInfo shminfo;
#endif
  register FXuint step=3;
  register FXbool shmi=FALSE;
  register XImage *xim=NULL;
  register Visual *visual;
  XGCValues values;
  GC gc;

  // Can not render before creation
  if(!xid){ fxerror("%s::render: trying to render image before it has been created.\n",getClassName()); }

  // Check for legal size
  if(width<2 || height<2){ fxerror("%s::render: illegal image size.\n",getClassName()); }

  // Just leave if black if no data
  if(data){
    
    // Have alpha?
    if(options&IMAGE_ALPHA) step=4;
    
    // Get Visual
    visual=DefaultVisual(getApp()->display,DefaultScreen(getApp()->display));
  
    // Make GC
    values.foreground=1;
    values.background=0;
    gc=XCreateGC(getDisplay(),xid,GCForeground|GCBackground,&values);
    
    // Turn it on iff both supported and desired
#ifdef HAVE_XSHM
    if(options&IMAGE_SHMI) shmi=getApp()->shmi;
#endif
    
    // First try XShm
#ifdef HAVE_XSHM
    if(shmi){
      xim=XShmCreateImage(getDisplay(),visual,depth,(depth==1)?XYPixmap:ZPixmap,NULL,&shminfo,width,height);
      if(!xim){ shmi=0; }
      if(shmi){
        shminfo.shmid=shmget(IPC_PRIVATE,xim->bytes_per_line*xim->height,IPC_CREAT|0777);
        if(shminfo.shmid==-1){ XDestroyImage(xim); xim=NULL; shmi=0; }
        if(shmi){
          shminfo.shmaddr=xim->data=(char*)shmat(shminfo.shmid,0,0);
          shminfo.readOnly=FALSE;
          XShmAttach(getDisplay(),&shminfo);
//fprintf(stderr,"RGBPixmap XSHM attached at memory=0x%08x (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height);
          }
        }
      }
#endif
    
    // Try the old fashioned way
    if(!shmi){
      xim=XCreateImage(getDisplay(),visual,depth,(depth==1)?XYPixmap:ZPixmap,0,NULL,width,height,32,0);
      if(!xim){ fxerror("%s::render: unable to render image.\n",getClassName()); }

      // Try create temp pixel store
      xim->data=(char*)malloc(xim->bytes_per_line*height);

      // Failed completely
      if(!xim->data){ fxerror("%s::render: unable to allocate memory.\n",getClassName()); }
      }
    
    // Should have succeeded
    FXASSERT(xim);
    
// fprintf(stderr,"im format = %d\n",xim->format);
// fprintf(stderr,"im byte_order = %d\n",xim->byte_order);
// fprintf(stderr,"im bitmap_unit = %d\n",xim->bitmap_unit);
// fprintf(stderr,"im bitmap_bit_order = %d\n",xim->bitmap_bit_order);
// fprintf(stderr,"im bitmap_pad = %d\n",xim->bitmap_pad);
// fprintf(stderr,"im bitmap_unit = %d\n",xim->bitmap_unit);
// fprintf(stderr,"im depth = %d\n",xim->depth);
// fprintf(stderr,"im bytes_per_line = %d\n",xim->bytes_per_line);
// fprintf(stderr,"im bits_per_pixel = %d\n",xim->bits_per_pixel);
// fprintf(stderr,"vi rmask = %08x\n",visual->red_mask);
// fprintf(stderr,"vi gmask = %08x\n",visual->green_mask);
// fprintf(stderr,"vi bmask = %08x\n",visual->blue_mask);
    
    // Determine what to do
    switch(visual->c_class){

      // True color
      case DirectColor:
      case TrueColor:
        render_true(xim,data,step);
        break;

      // Gray ramp
      case GrayScale:
      case StaticGray:
        render_gray(xim,data,step);
        break;
        
      // Pseudo color or gray ramp
      case StaticColor:
      case PseudoColor:
        render_index(xim,data,step);
        break;
      }

    // Transfer image with shared memory
#ifdef HAVE_XSHM
    if(shmi){
      XShmPutImage(getDisplay(),xid,gc,xim,0,0,0,0,width,height,False);
      XSync(getDisplay(),False);
//fprintf(stderr,"RGBPixmap XSHM detached at memory=0x%08x (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height);
      XShmDetach(getDisplay(),&shminfo);
      XDestroyImage(xim);
      shmdt(shminfo.shmaddr);
      shmctl(shminfo.shmid,IPC_RMID,0);
      }
#endif
   
    // Transfer the image old way
    if(!shmi){
      XPutImage(getDisplay(),xid,gc,xim,0,0,0,0,width,height);
#ifndef WIN32
      //// Need to use something other than malloc for WIN32....
      XDestroyImage(xim);
#endif
      }
    
    // We're done
    XFreeGC(getDisplay(),gc);
    }
  }


// Save pixel data only
void FXImage::savePixels(FXStream& store) const {
  FXuint size=width*height;
  if(options&IMAGE_ALPHA) size*=4; else size*=3;
  store.save(data,size);
  }


// Load pixel data only
void FXImage::loadPixels(FXStream& store){
  FXuint size=width*height;
  if(options&IMAGE_ALPHA) size*=4; else size*=3;
  if(options&IMAGE_OWNED) FXFREE(&data);
  FXMALLOC(&data,FXuchar,size);
  store.load(data,size);
  options|=IMAGE_OWNED;
  }


// Save data
void FXImage::save(FXStream& store) const {
  FXuchar haspixels=(data!=NULL);
  FXDrawable::save(store);
  store << options;
  store << haspixels;
  if(haspixels) savePixels(store);
  }


// Load data
void FXImage::load(FXStream& store){
  FXuchar haspixels;
  FXDrawable::load(store);
  store >> options;
  store >> haspixels;
  if(haspixels) loadPixels(store);
  }


// Clean up
FXImage::~FXImage(){
  if(xid){XFreePixmap(getApp()->display,xid);}
  if(options&IMAGE_OWNED){FXFREE(&data);}
  data=(FXuchar*)-1;
  }
  
