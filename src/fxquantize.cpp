/********************************************************************************
*                                                                               *
*                      C o l o r   Q u a n t i z a t i o n                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: fxquantize.cpp,v 1.8 2002/01/18 22:43:08 jeroen Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"




// Up to 256 colors: 3 bits R, 3 bits G, 2 bits B  (RRRGGGBB)
#define REDMASK          0xe0
#define REDSHIFT         0
#define GREENMASK        0xe0
#define GREENSHIFT       3
#define BLUEMASK         0xc0
#define BLUESHIFT        6


// Floyd-Steinberg quantization full 24 bpp to less than or equal to maxcolors
FXbool fxfsquantize(FXuchar* p8,const FXuchar* p24,FXuchar* rmap,FXuchar* gmap,FXuchar* bmap,FXint& actualcolors,FXint w,FXint h,FXint){
  int *thisline,*nextline,*thisptr,*nextptr,*tmpptr,*begptr;
  int i,j,val,pwide3,imax,jmax,r1,g1,b1;
  FXuchar *pp;


  FXTRACE((100,"fxfsquantize\n"));

  pp=p8;
  pwide3=w*3;
  imax=h-1;
  jmax=w-1;

  // Load up colormap.
  // Note that 0 and 255 of each color are always in the map;
  // intermediate values are evenly spaced.
  for(i=0; i<256; i++){
    rmap[i]=(((i<<REDSHIFT)&REDMASK)*255+REDMASK/2)/REDMASK;
    gmap[i]=(((i<<GREENSHIFT)&GREENMASK)*255+GREENMASK/2)/GREENMASK;
    bmap[i]=(((i<<BLUESHIFT)&BLUEMASK)*255+BLUEMASK/2)/BLUEMASK;
    }

  // Temporary storage
  if(!FXMALLOC(&begptr,int,pwide3*2)) return FALSE;
  thisline=begptr;
  nextline=&begptr[pwide3];

  // get first line of picture
  for(j=pwide3,tmpptr=nextline; j; j--) *tmpptr++ = (int) *p24++;

  // Dither loop
  for(i=0; i<h; i++){
    tmpptr=thisline;
    thisline=nextline;
    nextline=tmpptr;

    // get next line
    if(i!=imax){
      for(j=pwide3,tmpptr=nextline; j; j--) *tmpptr++ = (int) *p24++;
      }

    // Dither
    for(j=0,thisptr=thisline,nextptr=nextline; j<w; j++,pp++){
      r1 = *thisptr++;
      g1 = *thisptr++;
      b1 = *thisptr++;
      r1=FXCLAMP(0,r1,255);
      g1=FXCLAMP(0,g1,255);
      b1=FXCLAMP(0,b1,255);

      // choose actual pixel value
      val=(((r1&REDMASK)>>REDSHIFT)|((g1&GREENMASK)>>GREENSHIFT)|((b1&BLUEMASK)>>BLUESHIFT));
      *pp=val;

      // compute color errors
      r1-=rmap[val];
      g1-=gmap[val];
      b1-=bmap[val];

      // Add fractions of errors to adjacent pixels
      if(j!=jmax){                        // Adjust RIGHT pixel
        thisptr[0]+=(r1*7)/16;
        thisptr[1]+=(g1*7)/16;
        thisptr[2]+=(b1*7)/16;
        }
      if(i!=imax){                        // do BOTTOM pixel
        nextptr[0]+=(r1*5)/16;
        nextptr[1]+=(g1*5)/16;
        nextptr[2]+=(b1*5)/16;
        if(j>0){                          // do BOTTOM LEFT pixel
          nextptr[-3]+=(r1*3)/16;
          nextptr[-2]+=(g1*3)/16;
          nextptr[-1]+=(b1*3)/16;
          }
        if(j!=jmax){                      // do BOTTOM RIGHT pixel
          nextptr[3]+=(r1)/16;
          nextptr[4]+=(g1)/16;
          nextptr[5]+=(b1)/16;
          }
        nextptr += 3;
        }
      }
    }
  FXFREE(&begptr);
  actualcolors=256;
  return TRUE;
  }


/*******************************************************************************/


#define HASH1(x,n) (((unsigned int)(x)*13)%(n))           // Number [0..n-1]
#define HASH2(x,n) (1|(((unsigned int)(x)*17)%((n)-1)))   // Number [1..n-1]



// EZ quantization may be used if w*h<=maxcolors, or if the actual colors
// used is less than maxcolors; using fxezquantize assures that no
// loss of data occurs repeatedly loading and saving the same file!
FXbool fxezquantize(FXuchar* p8,const FXuchar* p24,FXuchar* rmap,FXuchar* gmap,FXuchar* bmap,FXint& actualcolors,FXint w,FXint h,FXint maxcolors){
  FXColor colortable[512];                      // Colors encountered in image
  FXuchar mapindex[512];                        // Map index assigned to color
  register FXint ncolors,npixels,i,p,x;
  register const FXuchar *src;
  register FXuchar *dst;
  register FXColor color;

  FXTRACE((100,"fxezquantize (w=%d h=%d)\n",w,h));
  FXASSERT(maxcolors<=256);

  // The number of pixels
  npixels=w*h;

  // Clear hash table and mapindex
  memset(colortable,0,sizeof(FXColor)*512);
  memset(mapindex,0,sizeof(FXuchar)*512);

  // Hash all colors from image
  for(i=0,src=p24,ncolors=0; i<npixels; i++){
    color=FXRGB(src[0],src[1],src[2]);
    p=HASH1(color,512);
    x=HASH2(color,512);
    while(colortable[p]!=0){
      if(colortable[p]==color) goto nxt;
      p=(p+x)%512;
      }

    // If no more room in colormap, we failed
    if(ncolors>=maxcolors) return FALSE;

    // Add new color
    colortable[p]=color;              // Add color not seen before
    mapindex[p]=ncolors;              // Assign new map index for this color
    rmap[ncolors]=src[0];             // Add this color into map
    gmap[ncolors]=src[1];
    bmap[ncolors]=src[2];
    ncolors++;

    // Next pixel
nxt:src+=3;
    }

  // We're still here; we should have fewer than 256
  FXASSERT(ncolors<=maxcolors);

  // Now loop through image, assigning map indices
  // We know all colors to be in the map at this point,
  // so expect to each lookup to be successful.
  for(i=0,src=p24,dst=p8; i<npixels; i++){
    color=FXRGB(src[0],src[1],src[2]);
    p=HASH1(color,512);
    x=HASH2(color,512);
    while(colortable[p]!=color){
      p=(p+x)%512;
      }
    *dst++=mapindex[p];
    src+=3;
    }

  // Actual number of colors used
  actualcolors=ncolors;

  FXTRACE((100,"fxezquantize found only %d colors\n",ncolors));

  return TRUE;
  }
