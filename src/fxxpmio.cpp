/********************************************************************************
*                                                                               *
*                          X P M   I n p u t / O u t p u t                      *
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
* $Id: fxxpmio.cpp,v 1.17 2002/01/18 22:43:08 jeroen Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "fxpriv.h"



/*
  Notes:
  - The transparent color hopefully does not occur in the image.
  - If the image is rendered opaque, the transparent is close to white.
  - References:
      http://www-sop.inria.fr/koala/lehors/xpm.html
*/

#define MAXPRINTABLE  92
#define MAXVALUE      96
#define ALPHA_COLOR   FXRGB(252,253,254)
//#define ALPHA_COLOR   FXRGB(178,192,220)


#define HASH1(x,n) (((unsigned int)(x)*13)%(n))           // Number [0..n-1]
#define HASH2(x,n) (1|(((unsigned int)(x)*17)%((n)-1)))   // Number [1..n-1]


/*******************************************************************************/

/// Load an X Pixmap from array of strings
extern FXAPI FXbool fxloadXPM(const FXchar **pix,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Load an X Pixmap file from a stream
extern FXAPI FXbool fxloadXPM(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save an X Pixmap file to a stream
extern FXAPI FXbool fxsaveXPM(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


/*******************************************************************************/


// Load image from array of strings
FXbool fxloadXPM(const FXchar **pixels,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  FXuint    ncolortable,index,ncolors,cpp,c;
  FXColor  *colortable=NULL;
  FXint     ww,hh,i,j;
  FXchar    name[100];
  FXchar    type[10];
  const FXchar *ptr;
  FXuchar  *pix;
  FXColor   color;

  data=NULL;
  ptr=*pixels++;

  // Parse values
  sscanf(ptr,"%d %d %u %u",&ww,&hh,&ncolors,&cpp);

  //FXTRACE((150,"fxloadXPM: width=%d height=%d ncolors=%d cpp=%d\n",ww,hh,ncolors,cpp));

  // Check size
  if(ww<1 || hh<1 || ww>16384 || hh>16384) return FALSE;

  // Check number of colors, and number of characters/color
  if(cpp<1 || cpp>2 || ncolors<1 || ncolors>9216) return FALSE;

  // Color lookup table
  ncolortable = (cpp==1) ? MAXVALUE : MAXVALUE*MAXVALUE;

  // Check if characters/color is consistent with number of colors
  if(ncolortable<ncolors) return FALSE;

  // Make color table
  FXMALLOC(&colortable,FXColor,ncolortable);
  if(!colortable){return FALSE;}

  // Read the color table
  for(c=0; c<ncolors; c++){
    ptr=*pixels++;
    if(!ptr){FXFREE(&colortable);return FALSE;}
    index=*ptr++ - ' ';
    if(cpp==2) index=index+MAXVALUE*(*ptr++ - ' ');
    if(index>ncolortable){FXFREE(&colortable);return FALSE;}
    sscanf(ptr,"%s %s",type,name);
    if(type[0]!='c') sscanf(ptr,"%*s %*s %s %s",type,name);
    color=fxcolorfromname(name);
    if(color==0) color=transp=ALPHA_COLOR;
    colortable[index]=color;
    //FXTRACE((150,"fxloadXPM: color %d: index=%d type=%s name=%s color=#%08x\n",c,index,type,name,color));
    }

  // Allocate pixels
  FXMALLOC(&data,FXuchar,3*ww*hh);
  if(!data){FXFREE(&colortable);return FALSE;}

  // Read the pixels
  for(i=0,pix=data; i<hh; i++){
    ptr=*pixels++;
    if(!ptr){FXFREE(&colortable);FXFREE(&data);return FALSE;}
    for(j=0; j<ww; j++){
      index=*ptr++ - ' ';
      if(cpp==2) index=index+MAXVALUE*(*ptr++ - ' ');
      if(index>ncolortable){FXFREE(&colortable);FXFREE(&data);return FALSE;}
      color=colortable[index];
      *pix++=FXREDVAL(color);
      *pix++=FXGREENVAL(color);
      *pix++=FXBLUEVAL(color);
      }
    }
  FXFREE(&colortable);
  width=ww;
  height=hh;
  //FXTRACE((150,"fxloadXPM: done\n"));
  return TRUE;
  }


/*******************************************************************************/


static void readbuffer(FXStream& store,FXchar* buffer,FXuint size){
  FXchar ch;
  while((store.status()!=FXStreamEnd) && (store>>ch,ch!='"'));
  while((store.status()!=FXStreamEnd) && (store>>ch,ch!='"') && size--) *buffer++=ch;
  while((store.status()!=FXStreamEnd) && (store>>ch,ch!='\n'));
  *buffer=0;
  }


// Load image from stream
FXbool fxloadXPM(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  FXuint    ncolortable,index,ncolors,cpp,c;
  FXColor  *colortable=NULL;
  FXint     ww,hh,i,j;
  FXchar    header[256];
  FXchar    name[100];
  FXchar    type[10];
  FXchar    ch;
  FXchar   *ptr;
  FXuchar  *pix;
  FXColor   color;

  data=NULL;

  readbuffer(store,header,sizeof(header));
  if(store.status()!=FXStreamOK) return FALSE;

  // Parse values
  sscanf(header,"%d %d %u %u",&ww,&hh,&ncolors,&cpp);

  //FXTRACE((150,"fxloadXPM: width=%d height=%d ncolors=%d cpp=%d\n",ww,hh,ncolors,cpp));

  // Check size
  if(ww<1 || hh<1 || ww>16384 || hh>16384) return FALSE;

  // Check number of colors, and number of characters/color
  if(cpp<1 || cpp>2 || ncolors<1 || ncolors>9216) return FALSE;

  // Color lookup table
  ncolortable = (cpp==1) ? MAXVALUE : MAXVALUE*MAXVALUE;

  // Check if characters/color is consistent with number of colors
  if(ncolortable<ncolors) return FALSE;

  // Make color table
  FXMALLOC(&colortable,FXColor,ncolortable);
  if(!colortable){return FALSE;}

  // Read the color table
  for(c=0; c<ncolors; c++){
    readbuffer(store,header,sizeof(header));
    if(store.status()!=FXStreamOK) return FALSE;
    ptr=header;
    index=*ptr++ - ' ';
    if(cpp==2) index=index+MAXVALUE*(*ptr++ - ' ');
    if(index>ncolortable){FXFREE(&colortable);return FALSE;}
    sscanf(ptr,"%s %s",type,name);
    if(type[0]!='c') sscanf(ptr,"%*s %*s %s %s",type,name);
    color=fxcolorfromname(name);
    if(color==0) color=transp=ALPHA_COLOR;
    colortable[index]=color;
    //FXTRACE((150,"fxloadXPM: color %d: index=%d type=%s name=%s color=#%08x\n",c,index,type,name,color));
    }

  // Allocate pixels
  FXMALLOC(&data,FXuchar,3*ww*hh);
  if(!data){FXFREE(&colortable);return FALSE;}

  // Read the pixels
  for(i=0,pix=data; i<hh; i++){
    while((store.status()!=FXStreamEnd) && (store>>ch,ch!='"'));
    for(j=0; j<ww; j++){
      store >> ch;
      index=ch-' ';
      if(cpp==2){ store >> ch; index=index+MAXVALUE*(ch-' '); }
      if(index>ncolortable){FXFREE(&colortable);FXFREE(&data);return FALSE;}
      color=colortable[index];
      *pix++=FXREDVAL(color);
      *pix++=FXGREENVAL(color);
      *pix++=FXBLUEVAL(color);
      }
    while((store.status()!=FXStreamEnd) && (store>>ch,ch!='\n'));
    if(store.status()!=FXStreamOK) return FALSE;
    }
  FXFREE(&colortable);
  width=ww;
  height=hh;
  //FXTRACE((150,"fxloadXPM: done\n"));
  return TRUE;
  }


/*******************************************************************************/


// Save image to a stream
FXbool fxsaveXPM(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height){
  const FXchar printable[]=" .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";
  const FXchar quote='"';
  const FXchar comma=',';
  const FXchar newline='\n';
  FXuchar   rmap[256];      // Red colormap
  FXuchar   gmap[256];      // Green colormap
  FXuchar   bmap[256];      // Blue colormap
  FXint     numpixels=width*height;
  FXint     ncolors,cpp,len,i,j,c1,c2,ok;
  FXchar    header[200];
  FXColor   color;
  FXuchar  *pixels,*ptr,pix;

  // Allocate temp buffer for pixels
  if(!FXMALLOC(&pixels,FXuchar,numpixels)) return FALSE;

  // First, try EZ quantization, because it is exact; a previously
  // loaded XPM will be re-saved with exactly the same colors.
  ok=fxezquantize(pixels,data,rmap,gmap,bmap,ncolors,width,height,256);

  if(!ok){

    // Floyd-Steinberg quantize full 24 bpp to 256 colors, organized as 3:3:2
    fxfsquantize(pixels,data,rmap,gmap,bmap,ncolors,width,height,256);
    }

  FXASSERT(ncolors<=256);

  // How many characters needed to represent one pixel, characters per line
  cpp=(ncolors>MAXPRINTABLE)?2:1;

  //FXTRACE((150,"fxsaveXPM: width=%d height=%d ncolors=%d cpp=%d\n",width,height,ncolors,cpp));

  // Save header
  store.save("/* XPM */\nstatic char * image[] = {\n",36);

  // Save values
  len=sprintf(header,"\"%d %d %d %d\",\n",width,height,ncolors,cpp);
  store.save(header,len);

  // Save the colors
  for(i=0; i<ncolors; i++){
    color=FXRGB(rmap[i],gmap[i],bmap[i]);
    c1=printable[i%MAXPRINTABLE];
    c2=printable[i/MAXPRINTABLE];
    if(color!=transp){
      len=sprintf(header,"\"%c%c c #%02x%02x%02x\",\n",c1,c2,rmap[i],gmap[i],bmap[i]);
      store.save(header,len);
      }
    else{
      len=sprintf(header,"\"%c%c c None\",\n",c1,c2);
      store.save(header,len);
      }
    }

  // Save the image
  ptr=pixels;
  for(i=0; i<height; i++){
    store << quote;
    for(j=0; j<width; j++){
      pix=*ptr++;
      if(cpp==1){
        store << printable[pix];
        }
      else{
        store << printable[pix%MAXPRINTABLE];
        store << printable[pix/MAXPRINTABLE];
        }
      }
    store << quote;
    if(i<height-1) store << comma;
    store << newline;
    }
  store.save("};\n",3);
  FXFREE(&pixels);
  //FXTRACE((150,"fxsaveXPM: done\n"));
  return TRUE;
  }



