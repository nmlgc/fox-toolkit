/********************************************************************************
*                                                                               *
*                      J P E G    I n p u t / O u t p u t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by David Tyree.   All Rights Reserved.                *
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
* $Id: fxjpegio.cpp,v 1.25 2002/01/21 19:28:28 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"


/*
  To Do:
  - Add more options for fast jpeg loading
  - Write a more detailed class that offers more options
  - Add the ability to load jpegs in the background
  - When loading JPEG, we should NOT load an entire buffer but only as much
    as needed; otherwise, we'll be reading data that does not belong to us.
  - We should NOT assume that we can reposition the current stream position;
    for example, with bzip2 or gzip streams this is not possible.
  - References:

      http://www.ijg.org/
      ftp://ftp.uu.net/graphics/jpeg/
      http://the-labs.com
*/

#define JPEG_BUFFER_SIZE 4096

/*******************************************************************************/

/// Load a jpeg from a stream
extern FXAPI FXbool fxloadJPG(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height,FXint& quality);


/// Save a jpeg to a stream
extern FXAPI FXbool fxsaveJPG(FXStream& store,const FXuchar* data,FXColor transp,FXint width,FXint height,FXint quality);


/*******************************************************************************/

#ifdef HAVE_JPEG_H

extern "C" {

/* Theo Veenker <Theo.Veenker@let.uu.nl> says this is needed for CYGWIN */
#if (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER)) && !defined(XMD_H)
#define XMD_H
typedef short INT16;
typedef int INT32;
#include "jpeglib.h"
#undef XMD_H
#else
#include "jpeglib.h"
#endif

}


// Source Manager for libjpeg
struct FOX_jpeg_source_mgr {
  struct jpeg_source_mgr pub;
  JOCTET    buffer[JPEG_BUFFER_SIZE];
  FXStream *stream;
  };


// Destination Manager for libjpeg
struct FOX_jpeg_dest_mgr {
  struct jpeg_destination_mgr pub;
  JOCTET    buffer[JPEG_BUFFER_SIZE];
  FXStream *stream;
  };


// Fatal error use FOX's way of reporing errors
static void fatal_error(j_common_ptr cinfo){
  FXchar message[JMSG_LENGTH_MAX];
  cinfo->err->format_message(cinfo,message);
  fxerror("FXJPEG: %s\n",message);
  }


/*******************************************************************************/


// A no-op in our case
static void init_source(j_decompress_ptr){
  }


// Read JPEG_BUFFER_SIZE bytes into the buffer
// NOTE:- we need to read in one byte at a time, so as to make sure that
// data belonging to the objects following this JPEG remain in the stream!
static boolean fill_input_buffer(j_decompress_ptr cinfo){
  FOX_jpeg_source_mgr *src=(FOX_jpeg_source_mgr*)cinfo->src;
  src->stream->load(src->buffer,1);
  if(src->stream->status()!=FXStreamOK){    // Insert a fake EOI marker
    src->buffer[0]=0xff;
    src->buffer[1]=JPEG_EOI;
    src->pub.next_input_byte=src->buffer;
    src->pub.bytes_in_buffer=2;
    }
  src->pub.next_input_byte=src->buffer;
  src->pub.bytes_in_buffer=1;
  return TRUE;
  }


// Skip ahead some number of bytes
static void skip_input_data(j_decompress_ptr cinfo,long num_bytes){
  FOX_jpeg_source_mgr *src=(FOX_jpeg_source_mgr*)cinfo->src;
  if(num_bytes>0){
    while(num_bytes>(long)src->pub.bytes_in_buffer){
      num_bytes-=(long)src->pub.bytes_in_buffer;
      fill_input_buffer(cinfo);
      }
    src->pub.next_input_byte+=(size_t) num_bytes;
    src->pub.bytes_in_buffer-=(size_t) num_bytes;
    }
  }


// A no-op in our case
static void term_source(j_decompress_ptr){
  }



// Load a JPEG image
FXbool fxloadJPG(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height,FXint&){
  struct jpeg_decompress_struct srcinfo;
  struct jpeg_error_mgr jerr;
  FOX_jpeg_source_mgr src;
  JSAMPARRAY buffer;
  int row_stride;

  // initialize the jpeg data structure;
  jpeg_create_decompress(&srcinfo);

  // setup the error handler
  srcinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit=fatal_error;

  // set our src manager
  srcinfo.src=&src.pub;

  // setup our src manager
  src.pub.init_source=init_source;
  src.pub.fill_input_buffer=fill_input_buffer;
  src.pub.resync_to_restart=jpeg_resync_to_restart;   // Use the default method
  src.pub.skip_input_data=skip_input_data;
  src.pub.term_source=term_source;
  src.pub.bytes_in_buffer=0;
  src.pub.next_input_byte=NULL;
  src.stream=&store;

  // read the header from the jpg;
  jpeg_read_header(&srcinfo,TRUE);

  // make sure the output is RGB
  srcinfo.out_color_space=JCS_RGB;

  jpeg_start_decompress(&srcinfo);

  row_stride=srcinfo.output_width*srcinfo.output_components;
  height=srcinfo.image_height;
  width=srcinfo.image_width;
  transp=0;

  // Buffer to receive
  FXMALLOC(&data,FXuchar,3*height*width);
  if(!data){
    jpeg_destroy_decompress(&srcinfo);
    return FALSE;
    }

  buffer=(*srcinfo.mem->alloc_sarray) ((j_common_ptr)&srcinfo,JPOOL_IMAGE,row_stride,1);
  FXuchar *temp_data=data;
  while(srcinfo.output_scanline<srcinfo.output_height){
    jpeg_read_scanlines(&srcinfo,buffer,1);
    memcpy(temp_data,*buffer,srcinfo.output_width*srcinfo.output_components);
    temp_data=temp_data+(srcinfo.output_width*srcinfo.output_components);
    }

  // wrap up
  jpeg_finish_decompress(&srcinfo);
  jpeg_destroy_decompress(&srcinfo);
  return TRUE;
  }


/*******************************************************************************/


// Initialize the buffer
static void init_destination(j_compress_ptr cinfo){
  FOX_jpeg_dest_mgr *dest=(FOX_jpeg_dest_mgr*)cinfo->dest;
  dest->pub.next_output_byte=dest->buffer;
  dest->pub.free_in_buffer=JPEG_BUFFER_SIZE;
  }


// Write the buffer to the stream
static boolean empty_output_buffer(j_compress_ptr cinfo){
  FOX_jpeg_dest_mgr *dest=(FOX_jpeg_dest_mgr*)cinfo->dest;
  dest->stream->save(dest->buffer,JPEG_BUFFER_SIZE);
  dest->pub.free_in_buffer=JPEG_BUFFER_SIZE;
  dest->pub.next_output_byte=dest->buffer;
  return TRUE;
  }


// Write any remaining data in the buffer to the stream
static void term_destination(j_compress_ptr cinfo){
  FOX_jpeg_dest_mgr *dest=(FOX_jpeg_dest_mgr*)cinfo->dest;
  dest->stream->save(dest->buffer,JPEG_BUFFER_SIZE-dest->pub.free_in_buffer);
  }


// Save a JPEG image
FXbool fxsaveJPG(FXStream& store,const FXuchar* data,FXColor,FXint width,FXint height,FXint quality){
  struct jpeg_compress_struct dstinfo;
  struct jpeg_error_mgr jerr;
  FOX_jpeg_dest_mgr dst;
  JSAMPROW row_pointer[1];
  int row_stride=width*3;

  FXASSERT(data);
  FXASSERT(0<quality && quality<=100);

  // specify the error manager
  dstinfo.err=jpeg_std_error(&jerr);
  jerr.error_exit=fatal_error;

  // initialize the structure
  jpeg_create_compress(&dstinfo);

  // specify the use of our destination manager
  dst.pub.init_destination=init_destination;
  dst.pub.empty_output_buffer=empty_output_buffer;
  dst.pub.term_destination=term_destination;
  dst.pub.free_in_buffer=0;
  dst.pub.next_output_byte=NULL;
  dst.stream=&store;

  // set up the input paramaters for the file
  dstinfo.image_width=width;
  dstinfo.image_height=height;
  dstinfo.input_components=3;
  dstinfo.in_color_space=JCS_RGB;
  dstinfo.dest=&dst.pub;

  jpeg_set_defaults(&dstinfo);
  jpeg_set_quality(&dstinfo,quality,TRUE);
  jpeg_start_compress(&dstinfo,TRUE);

  // write the jpeg data
  while(dstinfo.next_scanline<dstinfo.image_height){
    row_pointer[0]=(JSAMPROW)&data[dstinfo.next_scanline*row_stride];
    jpeg_write_scanlines(&dstinfo,row_pointer,1);
    }

  // wrap up
  jpeg_finish_compress(&dstinfo);
  jpeg_destroy_compress(&dstinfo);
  return TRUE;
  }


/*******************************************************************************/


#else


// Stub routine
FXbool fxloadJPG(FXStream&,FXuchar*& data,FXColor& transp,FXint& width,FXint& height,FXint& quality){
  static const FXuchar jpeg_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x80, 0xfd, 0xff, 0xff, 0xbf,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0xf5, 0x3d, 0x9f, 0xa3,
   0x05, 0x45, 0x41, 0xa4, 0x05, 0x45, 0x41, 0xa0, 0x05, 0x45, 0x47, 0xa0,
   0x05, 0x3d, 0x41, 0xa6, 0x05, 0x05, 0x41, 0xa4, 0x15, 0x05, 0x41, 0xa4,
   0xe5, 0x04, 0x9f, 0xa3, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0xfd, 0xff, 0xff, 0xbf,
   0x01, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff};
  register FXint p,q;
  FXCALLOC(&data,FXuchar,32*32*3);
  for(p=q=0; p<32*32*3; p+=3,q+=1){
    if(!(jpeg_bits[q>>3]&(1<<(q&7)))) data[p+0]=data[p+1]=data[p+2]=255;
    }
  transp=0;
  width=32;
  height=32;
  return FALSE;
  }


// Stub routine
FXbool fxsaveJPG(FXStream&,const FXuchar*,FXColor,FXint,FXint,FXint){
  return FALSE;
  }


#endif
