/********************************************************************************
*                                                                               *
*                        G I F   I n p u t / O u t p u t                        *
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
* $Id: fxgifio.cpp,v 1.6 1998/10/09 23:25:01 jvz Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"



/*
  To do:
  - Add gif writer.
  - more error checking
*/


#define ASCPECTEXT        'R'
#define COMMENTEXT        0xFE
#define PLAINTEXTEXT      0x01
#define APPLICATIONEXT    0xFF
#define GRAPHICCONTROLEXT 0xF9
#define EXTENSION         0x21
#define IMAGESEP          0x2c
#define TRAILER           0x3b
#define INTERLACE         0x40
#define COLORMAP          0x80



/*******************************************************************************/

static const FXuchar EGApalette[16][3]={
  {  0,   0,   0},
  {  0,   0, 128},
  {  0, 128,   0},
  {  0, 128, 128},
  {128,   0,   0},
  {128,   0, 128},
  {128, 128,   0},
  {200, 200, 200},
  {100, 100, 100},
  {100, 100, 255},
  {100, 255, 100},
  {100, 255, 255},
  {255, 100, 100},
  {255, 100, 255},
  {255, 255, 100},
  {255, 255, 255}
  };


/*******************************************************************************/


// Load image from stream
FXbool fxloadGIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  const FXint Yinit[4]={0,4,2,1};
  const FXint Yinc[4]={8,8,4,2};
  FXint   imwidth,imheight,interlace,ncolors,resolution,npixels,maxpixels;
  FXuchar c1,c2,c3,sbsize,flags,*ptr;
  FXuchar colormap[256*3];
  FXbool  version_89a;
  FXuchar alpha;                      // Transparent color
  FXint   BitOffset;                  // Bit Offset of next code
  FXint   XC, YC;                     // Output X and Y coords of current pixel
  FXint   Pass;                       // Used by output routine if interlaced pic
  FXint   OutCount;                   // Decompressor output 'stack count'
  FXint   CodeSize;                   // Code size, read from GIF header
  FXint   InitCodeSize;               // Starting code size, used during Clear
  FXint   Code;                       // Value returned by ReadCode
  FXint   MaxCode;                    // limiting value for current code size
  FXint   ClearCode;                  // GIF clear code
  FXint   EOFCode;                    // GIF end-of-information code
  FXint   CurCode, OldCode, InCode;   // Decompressor variables
  FXint   FirstFree;                  // First free code, generated per GIF spec
  FXint   FreeCode;                   // Decompressor,next free slot in hash table
  FXint   FinChar;                    // Decompressor variable
  FXint   BitMask;                    // AND mask for data size
  FXint   ReadMask;                   // Code AND mask for current code size
  FXint   Prefix[4096];               // The hash table used by the decompressor
  FXint   Suffix[4096];               // The hash table used by the decompressor
  FXint   OutCode[4097];              // An output array used by the decompressor
  FXint   ByteOffset;
  register int i,ix;
  
  // Free old data, if any
  FXFREE(&data);
  
  // Check signature
  store >> c1 >> c2 >> c3;
  if(!(c1=='G' && c2=='I' && c3=='F')) return FALSE;
  
  // Check version
  store >> c1 >> c2 >> c3;
  if(c1=='8' && c2=='7' && c3=='a') version_89a=0;
  else if(c1=='8' && c2=='9' && c3=='a') version_89a=1;
  else return FALSE;
  
  // Get screen descriptor
  store >> c1 >> c2;    // Skip screen width
  store >> c1 >> c2;    // Skip screen height
  store >> flags;       // Get flags
  store >> alpha;       // Background
  store >> c2;          // Skip aspect ratio
  
  // Determine number of colors
  ncolors=2<<(flags&7);
  BitMask=ncolors-1;
  resolution=((flags&0x70)>>3)+1;
  
  // Get colormap
  if(flags&COLORMAP){
    store.load(colormap,3*ncolors);
    }
  
  // Fill with simple palette
  else{
    for(i=0; i<256; i++){
      colormap[3*i+0]=EGApalette[i&15][0];
      colormap[3*i+1]=EGApalette[i&15][1];
      colormap[3*i+2]=EGApalette[i&15][2];
      }
    }
  
  // Assume no alpha
  transp=0;
  
  // Process it
  while(1){
    store >> c1;
    if(c1==EXTENSION){

      // Read extension code
      store >> c2;
      
      // Graphic Control Extension
      if(c2==GRAPHICCONTROLEXT){
        store >> sbsize;
        if(sbsize!=4) return FALSE;
        store >> c3;            // Flags
        store >> c3 >> c3;      // Delay time
        store >> alpha;         // Alpha color index
        store >> c3;
        }
      
      // Other extension
      else{ 
        do{
          store>>sbsize;
	  for(i=0; i<sbsize; i++) store>>c3;
	  }
        while(sbsize>0);
        }
      continue;
      }
    
    // Image separator
    if(c1==IMAGESEP){
      store >> c1 >> c2;
      store >> c1 >> c2;
  
      // Get image width
      store >> c1 >> c2;
      imwidth=c2*256+c1;
  
      // Get image height
      store >> c1 >> c2;
      imheight=c2*256+c1;
  
      // Get image flags
      store >> flags;
  
      maxpixels=imwidth*imheight;
      
      // Allocate memory
      FXMALLOC(&data,FXuchar,3*maxpixels);
      if(!data) return FALSE;
  
      // Has a colormap
      if(flags&COLORMAP){ 
        ncolors=2<<(flags&7);
        store.load(colormap,3*ncolors);
        }

      // Interlaced image
      interlace=(flags&INTERLACE);
  
      // Start reading the raster data. First we get the intial code size
      // and compute decompressor constant values, based on this code size.
      store >> c1;
      CodeSize=c1;

      ClearCode=1<<CodeSize;
      EOFCode=ClearCode+1;
      FreeCode=FirstFree=ClearCode+2;
  
      // The GIF spec has it that the code size is the code size used to
      // compute the above values is the code size given in the file, but the
      // code size used in compression/decompression is the code size given in
      // the file plus one.
      CodeSize++;
      InitCodeSize=CodeSize;
      MaxCode=1<<CodeSize;
      ReadMask=MaxCode-1;

// fprintf(stderr,"CodeSize = %d\n",CodeSize);
// fprintf(stderr,"ClearCode = %d\n",ClearCode);
// fprintf(stderr,"EOFCode = %d\n",EOFCode);
// fprintf(stderr,"FreeCode = %d\n",FreeCode);
// fprintf(stderr,"InitCodeSize = %d\n",InitCodeSize);
// fprintf(stderr,"MaxCode = %d\n",MaxCode);
// fprintf(stderr,"ReadMask = %d\n",ReadMask);


      // Read all blocks into memory, reusing pixel storage.
      // We assume that since it's compressed, it should take less room!
      ptr=data;
      do{
        store >> sbsize;
        store.load(ptr,sbsize);
        ptr+=sbsize;
        }
      while(sbsize>0);
      
// fprintf(stderr,"maxpixels=%d bytesread=%d\n",maxpixels,ptr-data);
      
      npixels=0;
      BitOffset = XC = YC = Pass = OutCount = 0;
      
      // Drop data at the end, so we can resuse pixel memory
      ptr=&data[2*maxpixels];
        
      // Decompress the file, continuing until you see the GIF EOF code.
      // One obvious enhancement is to add checking for corrupt files here.
      while(1){
        
        // Fetch the next code from the raster data stream.  The codes can be
        // any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
        // maintain our location in the source array as a BIT Offset.  We compute
        // the byte Offset into the raster array by dividing this by 8, pick up
        // three bytes, compute the bit Offset into our 24-bit chunk, shift to
        // bring the desired code to the bottom, then mask it off and return it.
        ByteOffset=BitOffset>>3;
        Code=(FXuint)data[ByteOffset]+(((FXuint)data[ByteOffset+1])<<8);
        if(CodeSize>=8) Code=Code+(((FXuint)data[ByteOffset+2])<<16);
        Code>>=(BitOffset&7);
        BitOffset+=CodeSize;
        Code&=ReadMask;
        
        // Are we done?
        if(Code==EOFCode || npixels>=maxpixels) break;
        
        // Clear code sets everything back to its initial value, then reads the
        // immediately subsequent code as uncompressed data.
        if(Code==ClearCode){
          CodeSize=InitCodeSize;
          MaxCode=1<<CodeSize;
          ReadMask=MaxCode-1;
          FreeCode=FirstFree;
          
          // Get next code
          ByteOffset=BitOffset>>3;
          Code=(FXuint)data[ByteOffset]+(((FXuint)data[ByteOffset+1])<<8);
          if(CodeSize>=8) Code=Code+(((FXuint)data[ByteOffset+2])<<16);
          Code>>=(BitOffset&7);
          BitOffset+=CodeSize;
          Code&=ReadMask;
          
          CurCode=OldCode=Code;
          FinChar=CurCode&BitMask;
          if(!interlace){
            *ptr++=FinChar;
            }
          else{
            FXASSERT(0<=YC && YC<imheight);
            FXASSERT(0<=XC && XC<imwidth);
            ptr[YC*imwidth+XC]=FinChar;
            XC+=1;
            if(XC>=imwidth){
              XC=0;
              YC+=Yinc[Pass];
              if(YC>=imheight){
                YC=Yinit[++Pass];
                }
              }
            }
          npixels++;
          }
        
        // If not a clear code, must be data: save same as CurCode and InCode
        else{

          // If we're at maxcode and didn't get a clear, stop loading
          if(FreeCode>=4096){ 
            fxwarning("fxloadGIF: problem!\n");
            FXFREE(&data);
            return FALSE;
            }

          CurCode=InCode=Code;

          // If greater or equal to FreeCode, not in the hash table yet;
          // repeat the last character decoded
          if(CurCode>=FreeCode){
            CurCode=OldCode;
            if(OutCount>4096){
              fxwarning("fxloadGIF: problem!\n");
              FXFREE(&data);
              return FALSE;
              }
            OutCode[OutCount++]=FinChar;
            }

          // Unless this code is raw data, pursue the chain pointed to by CurCode
          // through the hash table to its end; each code in the chain puts its
          // associated output code on the output queue.
          while(CurCode>BitMask){
            if(OutCount>4096) break;   /* corrupt file */
            OutCode[OutCount++]=Suffix[CurCode];
            CurCode=Prefix[CurCode];
            }

          if(OutCount>4096){
            fxwarning("fxloadGIF: problem!\n");
            FXFREE(&data);
            return FALSE;
            }
            
          // The last code in the chain is treated as raw data
          FinChar=CurCode&BitMask;
          OutCode[OutCount++]=FinChar;

          // Now we put the data out to the Output routine.
          // It's been stacked LIFO, so deal with it that way...

          // safety thing: prevent exceeding range 
          if(npixels+OutCount>maxpixels) OutCount=maxpixels-npixels;

          npixels+=OutCount;
          if(!interlace){
            for(i=OutCount-1; i>=0; i--) *ptr++=OutCode[i];
            }
          else{
            for(i=OutCount-1; i>=0; i--){
              FXASSERT(0<=YC && YC<imheight);
              FXASSERT(0<=XC && XC<imwidth);
              ptr[YC*imwidth+XC]=OutCode[i];
              XC+=1;
              if(XC>=imwidth){
                XC=0;
                YC+=Yinc[Pass];
                if(YC>=imheight){
                  YC=Yinit[++Pass];
                  }
                }
              }
            }
          OutCount=0;

          // Build the hash table on-the-fly. No table is stored in the file
          Prefix[FreeCode]=OldCode;
          Suffix[FreeCode]=FinChar;
          OldCode=InCode;

          // Point to the next slot in the table.  If we exceed the current
          // MaxCode value, increment the code size unless it's already 12.  If it
          // is, do nothing: the next code decompressed better be CLEAR
          FreeCode++;
          if(FreeCode>=MaxCode){
            if(CodeSize<12){
              CodeSize++;
              MaxCode*=2;
              ReadMask=(1<<CodeSize)-1;
              }
            }
          }
        }

      // Did the stream stop prematurely?
      if(npixels!=maxpixels){
        fxwarning("fxloadGIF: image truncated\n");
        }

      width=imwidth;
      height=imheight;
            
      // Apply colormap
      for(i=0; i<maxpixels; i++){
        ix=data[2*maxpixels+i];
        data[3*i+0]=colormap[3*ix+0];
        data[3*i+1]=colormap[3*ix+1];
        data[3*i+2]=colormap[3*ix+2];
        }

      // Calculate alpha color
      transp=FXRGB(colormap[3*alpha],colormap[3*alpha+1],colormap[3*alpha+2]);

      // We're done!
      return TRUE;
      }
    
    // Non of the above, we fail!
    return FALSE;
    }
  
  // Shouldn't get here, but to satisfy compiler
  return FALSE;
  }


/*******************************************************************************/

// Save a gif file to a stream
FXbool fxsaveGIF(FXStream& store,const FXuchar *const& data,const FXColor& transp,const FXint& width,const FXint& height){
  fxerror("fxsaveGIF: unimplemented.\n");
  return FALSE;
  }  

