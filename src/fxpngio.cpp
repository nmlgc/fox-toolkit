/********************************************************************************
*                                                                               *
*                         P N G    I n p u t / O u t p u t                      *
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
* $Id: fxpngio.cpp,v 1.15 2002/01/18 22:43:08 jeroen Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"


/*
  Notes:
  - References:
    http://www.w3.org/TR/REC-png.html
    http://www.graphicswiz.com/png/
    http://www.inforamp.net/~poynton
    http://www.libpng.org/pub/png/
*/


/*******************************************************************************/

/// Load a png from a stream
extern FXAPI FXbool fxloadPNG(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a png to a stream
extern FXAPI FXbool fxsavePNG(FXStream& store,const FXuchar* data,FXColor transp,FXint width,FXint height);


/*******************************************************************************/

#ifdef HAVE_PNG_H
#include "png.h"


// // This function is called when there is a warning, but the library thinks
// // it can continue anyway.
// static void user_warning_fn(png_structp, png_const_charp message){
//   fxwarning("FXPNG: %s\n",message);
//   }
//
//
// // This is the error handling function.  Note that replacements for
// // this function MUST NOT RETURN, or the program will likely crash.
// static void user_error_fn(png_structp, png_const_charp message){
//   fxerror("FXPNG: %s\n",message);
//   }


// Custom read function, which will read from the stream in our case
static void user_read_fn(png_structp png_ptr, png_bytep buffer, png_size_t size){
  FXStream *store=(FXStream*)png_get_io_ptr(png_ptr);
  store->load((FXchar*)buffer,size);
  }


// Custom write function, which will write to the stream in our case
static void user_write_fn(png_structp png_ptr, png_bytep buffer, png_size_t size){
  FXStream *store=(FXStream*)png_get_io_ptr(png_ptr);
  store->save((FXchar*)buffer,size);
  }


// Custom output flush function, a no-op in our case
static void user_flush_fn(png_structp ){ }


// Load a PNG image
FXbool fxloadPNG(FXStream& store,FXuchar*& data,FXColor&,FXint& width,FXint& height){
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 ww,hh,i;
  int bit_depth,color_type,interlace_type,number_passes;
  png_bytep *row_pointers;

  // Create png_struct
  //png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,user_error_fn,user_warning_fn);
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  if(!png_ptr) return FALSE;

  // Allocate/initialize the memory for image information
  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr){
    png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
    return FALSE;
    }

  // Set error handling
  if(setjmp(png_ptr->jmpbuf)){

    // Free all of the memory associated with the png_ptr and info_ptr
    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    return FALSE;
    }

  // Using replacement read functions
  png_set_read_fn(png_ptr,(void *)&store,user_read_fn);

  // If we have already read some of the signature
  //png_set_sig_bytes(png_ptr, sig_read);

  // Get all of the information from the PNG file before the first IDAT (image data chunk).
  png_read_info(png_ptr,info_ptr);

  // Get the goods
  png_get_IHDR(png_ptr,info_ptr,&ww,&hh,&bit_depth,&color_type,&interlace_type,NULL,NULL);

  FXTRACE((100,"FXPNG: width=%d height=%d bit_depth=%d color_type=%d\n",(int)ww,(int)hh,bit_depth,color_type));

  // tell libpng to strip 16 bit/color files down to 8 bits/color
  png_set_strip_16(png_ptr);

  // Expand paletted colors into true RGB triplets
  if(color_type==PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);

  // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
  if(color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8) png_set_expand(png_ptr);

  // Expand paletted or RGB images with transparency to full alpha channels
  // so the data will be available as RGBA quartets.
  if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS)) png_set_expand(png_ptr);

  // Grey images are upgraded to RGB
  if(color_type==PNG_COLOR_TYPE_GRAY || color_type==PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);

  // If we don't have true alpha, pad with an alpha channel representing opaque
  png_set_filler(png_ptr,0xff,PNG_FILLER_AFTER);

  // Turn on interlace handling
  number_passes = png_set_interlace_handling(png_ptr);

  // Update image info based on transformations
  png_read_update_info(png_ptr,info_ptr);

  // Make room for data
  FXMALLOC(&data,FXuchar,4*hh*ww);
  if(!data){
    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    return FALSE;
    }

  // Row pointers
  FXMALLOC(&row_pointers,png_bytep,hh);
  if(!row_pointers){
    FXFREE(&data);
    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    return FALSE;
    }

  // Set up row pointers
  for(i=0; i<hh; i++){
    row_pointers[i]=(png_bytep)&data[i*4*ww];
    }

  FXTRACE((100,"Reading image...\n"));

  // Finally...
  png_read_image(png_ptr,row_pointers);

  // read rest of file, and get additional chunks in info_ptr
  png_read_end(png_ptr,info_ptr);

  // clean up after the read, and free any memory allocated
  png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);

  // Get rid of it
  FXFREE(&row_pointers);

  width=ww;
  height=hh;

  return TRUE;
  }


/*******************************************************************************/



// Save a PNG image
FXbool fxsavePNG(FXStream& store,const FXuchar* data,FXColor,FXint width,FXint height){
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int i;

  // Create and initialize the png_struct with the desired error handler functions.
  //png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,user_error_fn,user_warning_fn);
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  if(!png_ptr) return FALSE;

  // Allocate/initialize the image information data.
  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr){
    png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
    return FALSE;
    }

  // Set error handling.
  if(setjmp(png_ptr->jmpbuf)){
    png_destroy_write_struct(&png_ptr,&info_ptr);
    return FALSE;
    }

  // Using replacement read functions
  png_set_write_fn(png_ptr,(void *)&store,user_write_fn,user_flush_fn);

  // Set the header
  png_set_IHDR(png_ptr,info_ptr,width,height,8,PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr,info_ptr);

  // Row pointers
  FXMALLOC(&row_pointers,png_bytep,height);
  if(!row_pointers){
    png_destroy_write_struct(&png_ptr,&info_ptr);
    return FALSE;
    }

  // Set up row pointers
  for(i=0; i<height; i++){
    row_pointers[i]=(png_bytep)&data[i*4*width];
    }

  // Save entire image
  png_write_image(png_ptr,row_pointers);

  // Wrap up
  png_write_end(png_ptr,info_ptr);

  // clean up after the write, and free any memory allocated
  png_destroy_write_struct(&png_ptr,&info_ptr);

  // Get rid of it
  FXFREE(&row_pointers);

  return TRUE;
  }


/*******************************************************************************/


#else


// Stub routine
FXbool fxloadPNG(FXStream&,FXuchar*& data,FXColor& transp,FXint& width,FXint& height){
  register FXint x,y,p;
  static const FXuchar png_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x80, 0xfd, 0xff, 0xff, 0xbf,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0xc5, 0x23, 0xc4, 0xa1,
   0x45, 0x24, 0x24, 0xa2, 0x45, 0x64, 0x24, 0xa0, 0x45, 0xa4, 0x24, 0xa0,
   0x45, 0x24, 0x25, 0xa0, 0xc5, 0x23, 0x26, 0xa3, 0x45, 0x20, 0x24, 0xa2,
   0x45, 0x20, 0x24, 0xa2, 0x45, 0x20, 0xc4, 0xa1, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0xfd, 0xff, 0xff, 0xbf,
   0x01, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff};
  FXMALLOC(&data,FXuchar,32*32*4);
  for(y=0; y<32*32*4; y+=32*4){
    for(x=0; x<32*4; x+=4){
      p=(png_bits[(y+x)>>5]&(1<<((x>>2)&7)))?0:255;
      data[y+x+0]=p;
      data[y+x+1]=p;
      data[y+x+2]=p;
      data[y+x+3]=255;
      }
    }
  transp=0;
  width=32;
  height=32;
  return FALSE;
  }


// Stub routine
FXbool fxsavePNG(FXStream&,const FXuchar*,FXColor transp,FXint,FXint){
  return FALSE;
  }


#endif
