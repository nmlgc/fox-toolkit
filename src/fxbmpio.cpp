/********************************************************************************
*                                                                               *
*                          B M P   I n p u t / O u t p u t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: fxbmpio.cpp,v 1.3 1998/09/10 15:15:52 jvz Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"



/*
  To do:
  - Add bmp writer
  - more error checking
*/

#define BI_RGB      0
#define BI_RLE8     1
#define BI_RLE4     2

#define WIN_OS2_OLD 12
#define WIN_NEW     40
#define OS2_NEW     64



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
  int   i,j,x,y,nybnum,padw;
  FXuchar *pp,c,c1;
  
  c=c1=0;
  
  // Read uncompressed data
  if(comp==BI_RGB){           
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
  else if(comp==BI_RLE4){  
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
  int   i,j,padw,x,y;
  FXuchar *pp,c,c1;
  
  // Read uncompressed data
  if(comp==BI_RGB){   
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
  else if(comp==BI_RLE8){  
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



/*******************************************************************************/

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


/*******************************************************************************/


// Load image from stream
FXbool fxloadBMP(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  FXuchar c1,c2;
  FXint bfSize, bfOffBits, biSize, biWidth, biHeight, biPlanes;
  FXint biBitCount, biCompression, biSizeImage, biXPelsPerMeter;
  FXint biYPelsPerMeter, biClrUsed, biClrImportant,bPad,maxpixels;
  FXuchar colormap[256*3];
  FXint i,j,ix,ok,cmaplen=0;
    
  // Free old data, if any
  FXFREE(&data);
  
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
    
    biCompression   = BI_RGB; 
    biXPelsPerMeter = biYPelsPerMeter = 0;
    biClrUsed       = biClrImportant  = 0;
    }


  // Error checking
  if((biBitCount!=1 && biBitCount!=4 && biBitCount!=8 && biBitCount!=24) || biPlanes!=1 || biCompression>BI_RLE4){
    return FALSE;
    }

  // More checking
  if(((biBitCount==1 || biBitCount==24) && biCompression!=BI_RGB) || (biBitCount==4 && biCompression==BI_RLE8) || (biBitCount==8 && biCompression==BI_RLE4)) {
    return FALSE;
    }

  // Skip ahead to colormap
  bPad = 0;
  if(biSize!=WIN_OS2_OLD){
    
    // 40 bytes read from biSize to biClrImportant
    j=biSize-40;    
    for(i=0; i<j; i++) store >> c1;
    bPad=bfOffBits-(biSize+14);
    }

  // load up colormap, if any
  if(biBitCount!=24){
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
  if(biSize!=WIN_OS2_OLD) {
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
  else{
    ok=loadBMP24(store,data,biWidth,biHeight);
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
  
  // Transparency is standard GUI background color
  transp=FXRGB(192,192,192);
  
  return TRUE;
  }  


/*******************************************************************************/

// Save a bmp file to a stream
FXbool fxsaveBMP(FXStream& store,const FXuchar *const& data,const FXColor& transp,const FXint& width,const FXint& height){
  fxerror("fxsaveBMP: unimplemented.\n");
  return FALSE;
  }  


