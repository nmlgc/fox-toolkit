/********************************************************************************
*                                                                               *
*                      T A R G A   I n p u t / O u t p u t                      *
*                                                                               *
*********************************************************************************
* Author: Janusz Ganczarski (POWER)   Email: JanuszG@enter.net.pl               *
*                                     WWW: http://www.januszg.hg.pl             *
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
* $Id: fxtargaio.cpp,v 1.3 2002/01/21 19:26:59 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"


/// Load a TARGA file from a stream
extern FXAPI FXbool fxloadTGA(FXStream& store,FXuchar*& data,FXuint& channels,FXint& width,FXint& height);


/// Save a TARGA file to a stream
extern FXAPI FXbool fxsaveTGA(FXStream& store,const FXuchar *data,FXuint channels,FXint width,FXint height);


/*******************************************************************************/

static FXuint read16(FXStream& store){
  FXuchar c1,c2;
  store >> c1 >> c2;
  return ((FXuint)c1) | (((FXuint)c2)<<8);
  }


/*******************************************************************************/

static FXbool loadTarga32(FXStream& store,FXuchar* data,FXint width,FXint height,FXuchar imgdescriptor,FXuchar ImageType){
  int i,j,rc;
  FXuchar *pp,c;
  FXuchar R,G,B,A;

  // 2 - Uncompressed, RGB images.
  if(ImageType == 2){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        pp=data+(i*width*3);
        for(j=0; j<width; j++){
          store >> pp[2];       // Blue
          store >> pp[1];       // Green
          store >> pp[0];       // Red
          store >> pp[3];       // Alpha
          pp += 4;
          }
        }
      }
    else{
      // Origin in lower left-hand corner
      for(i=height-1; i>=0; i--){
        pp=data+(i*width*4);
        for(j=0; j<width; j++){
          store >> pp[2];       // Blue
          store >> pp[1];       // Green
          store >> pp[0];       // Red
          store >> pp[3];       // Alpha
          pp += 4;
          }
        }
      }
    }

  // 10 - Runlength encoded RGB images.
  else if(ImageType==10){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        j = 0;
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if (c > 127){
            rc = c - 127;
            j += rc;

            // read Pixel Value field - get R, G, B, A values
            store >> B;
            store >> G;
            store >> R;
            store >> A;

            while(rc--){
              *data++=R; // Red
              *data++=G; // Green
              *data++=B; // Blue
              *data++=A; // Alpha
              }
            }

          // Raw Packet
          else{
            rc = c + 1;
            j += rc;
            while(rc--){
              store >> B;
              store >> G;
              store >> R;
              store >> A;
              *data++=R; // Red
              *data++=G; // Green
              *data++=B; // Blue
              *data++=A; // Alpha
              }
            }
          }
        }
      }
    else{
      // Origin in lower left-hand corner
      for(i=height-1; i>=0; i--){
        j=0;
        pp=data+(i*width*4);
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if (c > 127){
            rc = c - 127;
            j += rc;

            // read Pixel Value field - get R,G,B,A values
            store >> B;
            store >> G;
            store >> R;
            store >> A;
            while(rc--){
              *pp++=R; // Red
              *pp++=G; // Green
              *pp++=B; // Blue
              *pp++=A; // Alpha
              }
            }
          // Raw Packet
          else{
            rc=c+1;
            j+=rc;
            while(rc--){
              store >> B;
              store >> G;
              store >> R;
              store >> A;
              *pp++=R; // Red
              *pp++=G; // Green
              *pp++=B; // Blue
              *pp++=A; // Alpha
              }
            }
          }
        }
      }
    }
  return TRUE;
  }

/*******************************************************************************/

static FXbool loadTarga24(FXStream& store,FXuchar* data,FXint width,FXint height,FXuchar imgdescriptor,FXuchar ImageType){
  int i,j,rc;
  FXuchar *pp,c;
  FXuchar R,G,B;

  // 2 - Uncompressed, RGB images.
  if(ImageType == 2){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor & 0x20) == 0x20){
      for(i=0; i<height; i++){
        pp=data+(i*width*3);
        for(j=0; j<width; j++){
          store >> pp[2];       // Blue
          store >> pp[1];       // Green
          store >> pp[0];       // Red
          pp+=3;
          }
        }
      }
    // Origin in lower left-hand corner
    else{
      for(i=height-1; i>=0; i--){
        pp=data+(i*width*3);
        for(j=0; j<width; j++){
          store >> pp[2];       // Blue
          store >> pp[1];       // Green
          store >> pp[0];       // Red
          pp+=3;
          }
        }
      }
    }

  // 10 - Runlength encoded RGB images.
  else if(ImageType==10){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        j=0;
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if (c > 127){
            rc = c - 127;
            j += rc;

            // read Pixel Value field - get R, G, B values
            store >> B;
            store >> G;
            store >> R;

            while(rc--){
              *data++=R; // Red
              *data++=G; // Green
              *data++=B; // Blue
              }
            }
          // Raw Packet
          else{
            rc = c + 1;
            j += rc;
            while(rc--){
              store >> B;
              store >> G;
              store >> R;
              *data++=R; // Red
              *data++=G; // Green
              *data++=B; // Blue
              }
            }
          }
        }
      }
    else{
      // Origin in lower left-hand corner
      for(i=height-1; i>=0; i--){
        j = 0;
        pp=data+(i*width*3);
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if (c > 127){
            rc = c - 127;
            j += rc;

            // read Pixel Value field - get R,G,B values
            store >> B;
            store >> G;
            store >> R;
            while(rc--){
              *pp++=R; // Red
              *pp++=G; // Green
              *pp++=B; // Blue
              }
            }
          // Raw Packet
          else{
            rc = c + 1;
            j += rc;
            while(rc--){
              store >> B;
              store >> G;
              store >> R;
              *pp++=R; // Red
              *pp++=G; // Green
              *pp++=B; // Blue
              }
            }
          }
        }
      }
    }
  return TRUE;
  }

/*******************************************************************************/

static FXbool loadTarga16(FXStream& store,FXuchar* data,FXint width,FXint height,FXuchar imgdescriptor,FXuchar ImageType){
  int i,j,rc;
  FXuchar *pp,c;
  FXuchar R,G,B;
  FXuint bgr16;

  // 2 - Uncompressed, RGB images.
  if(ImageType==2){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        for(j=0; j<width; j++){
          bgr16=read16(store);
          *data++=(bgr16&0x7c00) << 1;  // Red
          *data++=(bgr16&0x03e0) << 6;  // Green
          *data++=(bgr16&0x001f) << 11; // Blue
          }
        }
      }
    else{
      // Origin in lower left-hand corner
      for(i=height-1; i>=0; i--){
        pp=data+(i*width*3);
        for(j=0; j<width; j++){
          bgr16=read16(store);
          *pp++=(bgr16&0x7c00) << 1;  // Red
          *pp++=(bgr16&0x03e0) << 6;  // Green
          *pp++=(bgr16&0x001f) << 11; // Blue
          }
        }
      }
    }

  // 10 - Runlength encoded RGB images.
  else if(ImageType==10){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        j=0;
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if(c>127){
            rc=c-127;
            j+=rc;

            // read Pixel Value field -
            bgr16 = read16 (store);

            // get R, G, B values
            R=(bgr16&0x7c00) << 1;  // Red
            G=(bgr16&0x03e0) << 6;  // Green
            B=(bgr16&0x001f) << 11; // Blue
            while(rc--){
              *data++=R; // Red
              *data++=G; // Green
              *data++=B; // Blue
              }
            }

          // Raw Packet
          else{
            rc=c+1;
            j+=rc;
            while(rc--){
              bgr16=read16(store);
              *data++=(bgr16&0x7c00) << 1;  // Red
              *data++=(bgr16&0x03e0) << 6;  // Green
              *data++=(bgr16&0x001f) << 11; // Blue
              }
            }
          }
        }
      }

    // Origin in lower left-hand corner
    else{
      for(i=height-1; i>=0; i--){
        j=0;
        pp=data+(i*width*3);
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if(c>127){
            rc=c-127;
            j+=rc;

            // read Pixel Value field
            bgr16=read16(store);

            // get R, G, B values
            R=(bgr16&0x7c00) << 1;  // Red
            G=(bgr16&0x03e0) << 6;  // Green
            B=(bgr16&0x001f) << 11; // Blue
            while(rc--){
              *pp++=R; // Red
              *pp++=G; // Green
              *pp++=B; // Blue
              }
            }

          // Raw Packet
          else{
            rc=c+1;
            j+=rc;
            while(rc--){
              bgr16=read16(store);
              *pp++=(bgr16&0x7c00) << 1;  // Red
              *pp++=(bgr16&0x03e0) << 6;  // Green
              *pp++=(bgr16&0x001f) << 11; // Blue
              }
            }
          }
        }
      }
    }
  return TRUE;
  }


/*******************************************************************************/

static FXbool loadTarga8(FXStream& store,FXuchar* data,FXint width,FXint height,FXuchar* colormap,FXuchar imgdescriptor,FXuchar ImageType){
  FXint i,j,rc;
  FXuchar *pp,c;
  FXuchar R,G,B;

  // 1 - Uncompressed, color-mapped images
  if(ImageType==1){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        for(j=0; j<width; j++){
          store>>c;
          *data++=colormap[3*c+2]; // Red
          *data++=colormap[3*c+1]; // Green
          *data++=colormap[3*c];   // Blue
          }
        }
      }
    // Origin in lower left-hand corner
    else{
      for(i=height-1; i>=0; i--){
        pp=data+(i*width*3);
        for(j=0; j<width; j++){
          store>>c;
          *pp++=colormap[3*c+2]; // Red
          *pp++=colormap[3*c+1]; // Green
          *pp++=colormap[3*c];   // Blue
          }
        }
      }
    }

  // 9 - Runlength encoded color-mapped images
  else if(ImageType==9){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        j=0;
        while(j<width){
          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if(c>127){
            rc=c-127;
            j+=rc;

            // read Pixel Value field
            store >> c;

            // get R,G,B values
            R=colormap[3*c+2];
            G=colormap[3*c+1];
            B=colormap[3*c];
            while(rc--){
              *data++=R; // Red
              *data++=G; // Green
              *data++=B; // Blue
              }
            }

          // Raw Packet
          else{
            rc=c+1;
            j+=rc;
            while(rc--){
              store >> c;
              *data++=colormap[3*c+2]; // Red
              *data++=colormap[3*c+1]; // Green
              *data++=colormap[3*c];   // Blue
              }
            }
          }
        }
      }

    // Origin in lower left-hand corner
    else{
      for(i=height-1; i>=0; i--){
        j=0;
        pp=data+(i*width*3);
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if(c>127){
            rc=c-127;
            j+=rc;

            // read Pixel Value field
            store >> c;

            // get R,G,B values
            R=colormap[3*c+2];
            G=colormap[3*c+1];
            B=colormap[3*c];
            while(rc--){
              *pp++=R; // Red
              *pp++=G; // Green
              *pp++=B; // Blue
              }
            }

          // Raw Packet
          else{
            rc = c + 1;
            j += rc;
            while(rc--){
              store >> c;
              *pp++=colormap[3*c+2]; // Red
              *pp++=colormap[3*c+1]; // Green
              *pp++=colormap[3*c];   // Blue
              }
            }
          }
        }
      }
    }
  return TRUE;
  }

/*******************************************************************************/

static FXbool loadTargaGray(FXStream& store,FXuchar* data,FXint width,FXint height,FXuchar imgdescriptor,FXuchar ImageType){
  FXint i,j,rc;
  FXuchar *pp,c;

  // 3 - Uncompressed, black and white images.
  if(ImageType==3){
    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        for(j=0; j<width; j++){
          store>>c;
          *data++=c;
          *data++=c;
          *data++=c;
          }
        }
      }

    // Origin in lower left-hand corner
    else{
      for(i=height-1; i>=0; i--){
        pp=data+(i*width*3);
        for(j=0; j<width; j++){
          store>>c;
          *pp++=c;
          *pp++=c;
          *pp++=c;
          }
        }
      }
    }

  // 11 - Compressed, black and white images.
  else if(ImageType==11){

    // check Image Descriptor
    // Origin in upper left-hand corner
    if((imgdescriptor&0x20)==0x20){
      for(i=0; i<height; i++){
        j=0;
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if(c>127){
            rc=c-127;
            j+=rc;

            // read Pixel Value field
            store >> c;

            while(rc--){
              *data++=c;
              *data++=c;
              *data++=c;
              }
            }

          // Raw Packet
          else{
            rc=c+1;
            j+=rc;
            while(rc--){
              store >> c;
              *data++=c;
              *data++=c;
              *data++=c;
              }
            }
          }
        }
      }

    // Origin in lower left-hand corner
    else{
      for(i=height-1; i>=0; i--){
        j = 0;
        pp=data+(i*width*3);
        while(j<width){

          // read Repetition Count field
          store >> c;

          // check for Run-length Packet
          if(c>127){
            rc=c-127;
            j+=rc;

            // read Pixel Value field
            store >> c;

            while(rc--){
              *pp++=c;
              *pp++=c;
              *pp++=c;
              }
            }

          // Raw Packet
          else{
            rc = c + 1;
            j += rc;
            while(rc--){
              store >> c;
              *pp++=c;
              *pp++=c;
              *pp++=c;
              }
            }
          }
        }
      }
    }
  return TRUE;
  }


/*******************************************************************************/
// Load Targa image from stream
/*******************************************************************************/

FXbool fxloadTGA(FXStream& store,FXuchar*& data,FXuint& channels,FXint& width,FXint& height){
  FXuchar IDLength,ColorMapType,ImageType,ColorMapEntrySize,PixelDepth,ImageDescriptor;
  FXuchar colormap [256*3],c;
  FXuint rgb16,ColorMapLength;
  FXint i;

  // length of Image ID Field
  store >> IDLength;

  // type of color map (if any) included with the image
  // 0 - indicates that no color-map data is included with this image
  // 1 - indicates that a color-map is included with this image
  store >> ColorMapType;

  // Image Type
  //  0 - No image data included.
  //  1 - Uncompressed, color-mapped images.
  //  2 - Uncompressed, RGB images.
  //  3 - Uncompressed, black and white images.
  //  9 - Runlength encoded color-mapped images.
  // 10 - Runlength encoded RGB images.
  // 11 - Compressed, black and white images.
  // 32 - Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
  // 33 - Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
  //      4-pass quadtree-type process.
  store >> ImageType;

  // check for supported image type
  if(ImageType!=1 && ImageType!=2 && ImageType!=3 && ImageType!=9 && ImageType!=10 && ImageType!=11) return FALSE;

  // Color Map Specification

  // FirstEntryIndex - index of the first color map entry
  read16(store);

  // Color map Length
  ColorMapLength=read16(store);

  // Color map Entry Size
  // Establishes the number of bits per entry.
  // Typically 15, 16, 24 or 32-bit values are used.
  store >> ColorMapEntrySize;

  // Image Specification Field

  // X-origin of Image and Y-origin of Image
  read16(store);
  read16(store);

  // This field specifies the width of the image in pixels
  width=read16(store);

  // This field specifies the height of the image in pixels
  height=read16(store);

  // This field indicates the number of bits per pixel. This number includes
  // the Attribute or Alpha channel bits. Common values are 8, 16, 24 and 32
  // but other pixel depths could be used.
  store >> PixelDepth;
  if(PixelDepth!=1 && PixelDepth!=8 && PixelDepth!=16 && PixelDepth!=24 && PixelDepth!=32) return FALSE;

  // Bits 3-0 - number of attribute bits associated with each pixel
  // Bit 4    - reserved.  Must be set to 0
  // Bit 5    - screen origin bit:
  //            0 = Origin in lower left-hand corner
  //            1 = Origin in upper left-hand corner
  //            Must be 0 for Truevision images
  // Bits 7-6 - Data storage interleaving flag:
  //            00 = non-interleaved
  //            01 = two-way (even/odd) interleaving
  //            10 = four way interleaving
  //            11 = reserved
  store >> ImageDescriptor;

  // skip Image ID Field (18 - standard header length)
  store.position (18 + IDLength);

  // color map
  if(ColorMapLength>0){
    switch (ColorMapEntrySize){
      case 15:
      case 16:
        for(i=0; i<ColorMapLength; i++){
          rgb16 = read16 (store);
          colormap[i*3] = (rgb16 & 0x7c00) << 1;
          colormap[i*3+1] = (rgb16 & 0x03e0) << 6;
          colormap[i*3+2] = (rgb16 & 0x001f) << 11;
          }
        break;

      // R,G,B
      case 24:
        for(i=0; i<ColorMapLength*3; i++){
          store >> colormap[i];
          }
        break;

      // R,G,B,A
      case 32:
        for(i=0; i<ColorMapLength*4; i++){
          store >> colormap[i];
          store >> colormap[i+1];
          store >> colormap[i+2];
          store >> c;
          }
        break;

      // ?
      default:
        return FALSE;
      }
    }

  FXTRACE((150,"fxloadTARGA: width=%d height=%d nbits=%d\n",width,height,PixelDepth));

  // Allocate memory
  if(PixelDepth==32)
    FXMALLOC(&data,FXuchar,width*height*4);
  else
    FXMALLOC(&data,FXuchar,width*height*3);

  if(!data) return FALSE;


  // load up the image
  if(PixelDepth==32 && (ImageType==2 || ImageType==10)){
    channels=4;
    return loadTarga32(store,data,width,height,ImageDescriptor,ImageType);
    }

  if(PixelDepth==24 && (ImageType==2 || ImageType==10)){
    channels=3;
    return loadTarga24(store,data,width,height,ImageDescriptor,ImageType);
    }

  if(PixelDepth==16 && (ImageType==2 || ImageType==10)){
    channels=3;
    return loadTarga16(store,data,width,height,ImageDescriptor,ImageType);
    }

  if(PixelDepth==8 && (ImageType==1 || ImageType==9)){
    channels=3;
    return loadTarga8(store,data,width,height,colormap,ImageDescriptor,ImageType);
    }

  if(ImageType==3 || ImageType==11){
    channels=3;
    return loadTargaGray(store,data,width,height,ImageDescriptor,ImageType);
    }

  return FALSE;
  }

/*******************************************************************************/

static void write16(FXStream& store,FXuint i){
  FXuchar c1,c2;
  c1=i&0xff;
  c2=(i>>8)&0xff;
  store << c1 << c2;
  }

/*******************************************************************************/

static FXbool writeTarga32(FXStream& store,const FXuchar* pic32,FXint w,FXint h){
  FXint i,j;
  const FXuchar *pp;
  for(i=h-1; i>=0; i--){
    pp=pic32+(i*w*4);
    for(j=0; j<w; j++){
      store << pp[2];     // blue
      store << pp[1];     // green
      store << pp[0];     // red
      store << pp[3];     // alpha
      pp+=4;
      }
    }
  return TRUE;
  }

/*******************************************************************************/

static FXbool writeTarga24(FXStream& store,const FXuchar* pic24,FXint w,FXint h){
  FXint i,j;
  const FXuchar *pp;
  for(i=h-1; i>=0; i--){
    pp=pic24+(i*w*3);
    for(j=0; j<w; j++){
      store << pp[2];    // blue
      store << pp[1];    // green
      store << pp[0];    // red
      pp+=3;
      }
    }
  return TRUE;
  }

/*******************************************************************************/
// Save a Targa file to a stream
/*******************************************************************************/

FXbool fxsaveTGA(FXStream& store,const FXuchar *data,FXuint channels,FXint width,FXint height){
  FXuchar IDLength,ColorMapType,ImageType,ColorMapEntrySize,PixelDepth,ImageDescriptor;

  IDLength=0;
  ColorMapType=0;
  ImageType=2;
  if(channels==3)
    PixelDepth=24;
  else
    PixelDepth=32;

  ImageDescriptor=0;
  ColorMapEntrySize=0;

  // length of Image ID Field
  store << IDLength;

  // type of color map (if any) included with the image
  // 0 - indicates that no color-map data is included with this image
  // 1 - indicates that a color-map is included with this image
  store << ColorMapType;

  // Image Type
  //  0 - No image data included.
  //  1 - Uncompressed, color-mapped images.
  //  2 - Uncompressed, RGB images.
  //  3 - Uncompressed, black and white images.
  //  9 - Runlength encoded color-mapped images.
  // 10 - Runlength encoded RGB images.
  // 11 - Compressed, black and white images.
  // 32 - Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
  // 33 - Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
  //      4-pass quadtree-type process.
  store << ImageType;

  // Color Map Specification

  // Index of the first color map entry
  write16 (store,0);

  // Color map Length
  write16 (store,0);

  // Color map Entry Size
  // Establishes the number of bits per entry.
  // Typically 15, 16, 24 or 32-bit values are used.
  store << ColorMapEntrySize;

  // Image Specification Field

  // X-origin of Image and Y-origin of Image
  write16 (store,0);
  write16 (store,0);

  // This field specifies the width of the image in pixels
  write16 (store,width);

  // This field specifies the height of the image in pixels
  write16 (store,height);

  // This field indicates the number of bits per pixel. This number includes
  // the Attribute or Alpha channel bits. Common values are 8, 16, 24 and 32
  // but other pixel depths could be used.
  store << PixelDepth;

  // Bits 3-0 - number of attribute bits associated with each pixel
  // Bit 4    - reserved.  Must be set to 0
  // Bit 5    - screen origin bit:
  //            0 = Origin in lower left-hand corner
  //            1 = Origin in upper left-hand corner
  //            Must be 0 for Truevision images
  // Bits 7-6 - Data storage interleaving flag:
  //            00 = non-interleaved
  //            01 = two-way (even/odd) interleaving
  //            10 = four way interleaving
  //            11 = reserved
  store << ImageDescriptor;

  if(channels==3){
    return writeTarga24(store,data,width,height);
    }

  if(channels==4){
    return writeTarga32(store,data,width,height);
    }

  return FALSE;
  }

