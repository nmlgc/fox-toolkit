/********************************************************************************
*                                                                               *
*                        G I F   I n p u t / O u t p u t                        *
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
* $Id: fxgifio.cpp,v 1.31.4.3 2003/05/12 11:04:35 fox Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "fxpriv.h"



/*
  Notes:

    "The Graphics Interchange Format(c) is the Copyright property of
    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    CompuServe Incorporated."
*/

#define ASCPECTEXT        'R'
#define COMMENTEXT        0xFE
#define PLAINTEXTEXT      0x01
#define APPLICATIONEXT    0xFF
#define GRAPHICCONTROLEXT 0xF9
#ifdef EXTENSION
#undef EXTENSION /* Borland C++ 5.5 had this defined for something else */
#endif
#define EXTENSION         0x21
#define IMAGESEP          0x2c
#define TRAILER           0x3b
#define INTERLACE         0x40
#define COLORMAP          0x80


/*******************************************************************************/

/// Load a gif file from a stream
extern FXAPI FXbool fxloadGIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a gif file to a stream
extern FXAPI FXbool fxsaveGIF(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);




/*******************************************************************************/


// Load image from stream
FXbool fxloadGIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  const   FXint Yinit[4]={0,4,2,1};
  const   FXint Yinc[4]={8,8,4,2};
  FXint   imwidth,imheight,interlace,ncolors,resolution,npixels,maxpixels;
  FXuchar c1,c2,c3,sbsize,flags,*ptr;
  FXuchar colormap[256*3];
  FXbool  version_89a,havealpha;
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

  // Check signature
  store >> c1 >> c2 >> c3;
  if(!(c1=='G' && c2=='I' && c3=='F')) return FALSE;

  // Check version
  store >> c1 >> c2 >> c3;
  if(c1=='8' && c2=='7' && c3=='a') version_89a=FALSE;
  else if(c1=='8' && c2=='9' && c3=='a') version_89a=TRUE;
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

  // Preset first in case index refers outside map
  memset(colormap,0,256*3);

  // If no colormap, spec says first 2 colors are black and white
  colormap[0]=colormap[1]=colormap[2]=0;
  colormap[3]=colormap[4]=colormap[5]=255;

  // Get colormap
  if(flags&COLORMAP){
    store.load(colormap,3*ncolors);
    }

  // Assume no alpha
  transp=0;
  havealpha=0;

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
        store >> flags;         // Flags
        store >> c3 >> c3;      // Delay time
        store >> alpha;         // Alpha color index
        store >> c3;
        havealpha=(flags&1);
        // Make unique transparency color; patch
        // from Daniel Gehriger <bulk@linkcad.com>
        if(havealpha){
          for(i=ncolors-1; i>=0; --i){
            if(colormap[3*i+0]==colormap[3*alpha+0] && colormap[3*i+1]==colormap[3*alpha+1] && colormap[3*i+2]==colormap[3*alpha+2] && i!=alpha){
              if(++colormap[3*alpha+0]==0){
                if(++colormap[3*alpha+1]==0){
                  ++colormap[3*alpha+2];
                  }
                }
              i=ncolors;        // Try again with new value
              }
            }
          }
        }

      // Other extension
      else{
        do{
          store >> sbsize;
          for(i=0; i<sbsize; i++) store >> c3;
          }
        while(sbsize>0 && store.status()==FXStreamOK);
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
      FXCALLOC(&data,FXuchar,3*maxpixels);
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

//       FXTRACE((200,"CodeSize = %d\n",CodeSize));
//       FXTRACE((200,"ClearCode = %d\n",ClearCode));
//       FXTRACE((200,"EOFCode = %d\n",EOFCode));
//       FXTRACE((200,"FreeCode = %d\n",FreeCode));
//       FXTRACE((200,"InitCodeSize = %d\n",InitCodeSize));
//       FXTRACE((200,"MaxCode = %d\n",MaxCode));
//       FXTRACE((200,"ReadMask = %d\n",ReadMask));


      // Read all blocks into memory, reusing pixel storage.
      // We assume that since it's compressed, it should take less room!
      ptr=data;
      do{
        store >> sbsize;
        store.load(ptr,sbsize);
        ptr+=sbsize;
        }
      while(sbsize>0 && store.status()==FXStreamOK);

//       FXTRACE((200,"maxpixels=%d bytesread=%d\n",maxpixels,ptr-data));

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
                Pass++;
                YC=Yinit[Pass&3];
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
                  Pass++;
                  YC=Yinit[Pass&3];
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

//       // If we had a transparent color, use it
//       if(havealpha){
//         transp=FXRGB(colormap[3*alpha],colormap[3*alpha+1],colormap[3*alpha+2]);
//         }

      // Remark:- the above code is correct, but all our icons don't have
      // an alpha (they're GIF87a's) and so we need to fix those first...
      transp=FXRGB(colormap[3*alpha],colormap[3*alpha+1],colormap[3*alpha+2]);

      // We're done!
      store >> c1;
      
      return TRUE;
      }

    // Non of the above, we fail!
    return FALSE;
    }

  // Shouldn't get here, but to satisfy compiler
  return FALSE;
  }



/*******************************************************************************/

/* Largest value that will fit in N bits */
#define MAXCODE(n_bits)        ((1 << (n_bits)) - 1)


// Private version of data destination object
struct GIFOUTPUT {
  FXStream *stream;           // Stream to save to

  // State for packing variable-width codes into a bitstream
  FXint     n_bits;           // current number of bits/code
  FXint     maxcode;          // maximum code, given n_bits
  FXint     cur_accum;        // holds bits not yet output
  FXint     cur_bits;         // # of bits in cur_accum

  // State for GIF code assignment/
  FXint     ClearCode;        // clear code (doesn't change)
  FXint     EOFCode;          // EOF code (ditto)
  FXint     code_counter;     // counts output symbols

  // GIF data packet construction buffer
  FXint     bytesinpkt;       // # of bytes in current packet
  FXuchar   packetbuf[256];   // workspace for accumulating packet
  };



// Routines to package finished data bytes into GIF data blocks.
// A data block consists of a count byte (1..255) and that many data bytes.
static void flush_packet(GIFOUTPUT& go){
  if(go.bytesinpkt>0){                              // never write zero-length packet
    go.packetbuf[0] = (char) go.bytesinpkt++;
    go.stream->save(go.packetbuf,go.bytesinpkt);
    go.bytesinpkt = 0;
    }
  }


// Add a character to current packet; flush to disk if necessary
static void char_out(GIFOUTPUT& go,FXuchar c){
  go.packetbuf[++go.bytesinpkt]=c;
  if(go.bytesinpkt>=255) flush_packet(go);
  }


// Routine to convert variable-width codes into a byte stream
// Emit a code of n_bits bits.  Uses cur_accum and cur_bits to
// reblock into 8-bit bytes
static void output(GIFOUTPUT& go,FXint code){
  go.cur_accum|=(code<<go.cur_bits);
  go.cur_bits+=go.n_bits;
  while(go.cur_bits>=8){
    char_out(go,(go.cur_accum&0xFF));
    go.cur_accum>>=8;
    go.cur_bits-=8;
    }
  }


// The pseudo-compression algorithm.
//
// In this module we simply output each pixel value as a separate symbol;
// thus, no compression occurs.  In fact, there is expansion of one bit per
// pixel, because we use a symbol width one bit wider than the pixel width.
//
// GIF ordinarily uses variable-width symbols, and the decoder will expect
// to ratchet up the symbol width after a fixed number of symbols.
// To simplify the logic and keep the expansion penalty down, we emit a
// GIF Clear code to reset the decoder just before the width would ratchet up.
// Thus, all the symbols in the output file will have the same bit width.
// Note that emitting the Clear codes at the right times is a mere matter of
// counting output symbols and is in no way dependent on the LZW patent.
//
// With a small basic pixel width (low color count), Clear codes will be
// needed very frequently, causing the file to expand even more.  So this
// simplistic approach wouldn't work too well on bilevel images, for example.
// But for output of JPEG conversions the pixel width will usually be 8 bits
// (129 to 256 colors), so the overhead added by Clear symbols is only about
// one symbol in every 256.

/* Initialize pseudo-compressor */
static void compress_init(GIFOUTPUT& go,FXint i_bits){
  go.n_bits=i_bits;                                 // init all the state variables
  go.maxcode=MAXCODE(go.n_bits);
  go.ClearCode=(1<<(i_bits-1));
  go.EOFCode=go.ClearCode+1;
  go.code_counter=go.ClearCode+2;
  go.bytesinpkt=0;                                  // init output buffering vars
  go.cur_accum=0;
  go.cur_bits=0;
  output(go,go.ClearCode);                          // GIF specifies an initial Clear code
  }


// Accept and "compress" one pixel value.
// The given value must be less than n_bits wide.
static void compress_pixel(GIFOUTPUT& go,FXint c){

  // Output the given pixel value as a symbol
  output(go,c);

  // Issue Clear codes often enough to keep the reader from ratcheting up
  // its symbol size.
  if(go.code_counter<go.maxcode){
    go.code_counter++;
    }
  else{
    output(go,go.ClearCode);
    go.code_counter=go.ClearCode+2;                 // Reset the counter
    }
  }


// Clean up at end
static void compress_term(GIFOUTPUT& go){
  output(go,go.EOFCode);                            // Send an EOF code
  if(go.cur_bits>0){                                // Flush the bit-packing buffer
    char_out(go,(go.cur_accum&0xFF));
    }
  flush_packet(go);                                 // Flush the packet buffer
  }


static void write16(FXStream& store,FXuint i){
  FXuchar c1,c2;
  c1=(0xff&i);
  c2=(0xff&(i>>8));
  store << c1 << c2;
  }


// Save a gif file to a stream
FXbool fxsaveGIF(FXStream& store,const FXuchar *data,FXColor,FXint width,FXint height){
  FXuchar   rmap[256];      // Red colormap
  FXuchar   gmap[256];      // Green colormap
  FXuchar   bmap[256];      // Blue colormap
  FXint     NumPixels;      // Total number of pixels
  FXint     NumColors;      // Number of colors
  FXint     ColorMapSize;   // Colormap size
  FXint     BitsPerPixel;   // Bits per pixel (Codes uses at least this + 1)
  FXint     InitCodeSize;   // Initial code size
  GIFOUTPUT go;             // Struct passed around
  FXuchar  *pixels,c1,bg;
  FXint     i;
  FXbool    ok;

  // How many pixels
  NumPixels=width*height;

  // Allocate temp buffer for pixels
  if(!FXMALLOC(&pixels,FXuchar,NumPixels)) return FALSE;

  // First, try EZ quantization, because it is exact; a previously
  // loaded GIF will be re-saved with exactly the same colors.
  ok=fxezquantize(pixels,data,rmap,gmap,bmap,NumColors,width,height,256);

  if(!ok){

    // Floyd-Steinberg quantize full 24 bpp to 256 colors, organized as 3:3:2
    fxfsquantize(pixels,data,rmap,gmap,bmap,NumColors,width,height,256);
    }

  FXASSERT(NumColors<=256);

  go.stream=&store;

  // File signature
  store.save("GIF89a",6);

  // Screen header
  write16(store,width);
  write16(store,height);

  // Figure out bits per pixel
  for(BitsPerPixel=1; NumColors>(1<<BitsPerPixel); BitsPerPixel++);

  // Colormap size
  ColorMapSize = 1 << BitsPerPixel;

  // Initial code size
  InitCodeSize=(BitsPerPixel<=1) ? 2 : BitsPerPixel;

  c1=0x80;                    // Yes, there is a color map
  c1|=(BitsPerPixel-1)<<4;    // OR in the color resolution (hardwired 8)
  c1|=(BitsPerPixel-1);       // OR in the # of bits per pixel
  store << c1;                // Flags
  store << bg;                // background color
  c1=0;
  store << c1;                // future expansion byte

  // Output colormap
  for(i=0; i<ColorMapSize; i++){
    store << rmap[i];
    store << gmap[i];
    store << bmap[i];
    }

  // Image descriptor
  c1=',';
  store << c1;                // Image separator
  write16(store,0);           // Image offset X
  write16(store,0);           // Image offset Y
  write16(store,width);       // Image width
  write16(store,height);      // Image height
  c1=0;
  store << c1;                // Flags (no local map)

  // Now for the beef...
  c1=InitCodeSize;
  store << c1;                      // Write the Code size

  // Initialize for "compression" of image data
  compress_init(go,InitCodeSize+1);

  // Output the "compressed" pixels
  for(i=0; i<NumPixels; i++){
    compress_pixel(go,pixels[i]);
    }

  // Flush "compression" mechanism
  compress_term(go);

  c1=0;
  store << c1;                // Zero length data block marks end
  c1=';';
  store << c1;                // File terminator

  // Free storage
  FXFREE(&pixels);

  return TRUE;
  }

