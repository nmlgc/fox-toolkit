/********************************************************************************
*                                                                               *
*                             I m a g e    O b j e c t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXImage.h,v 1.32 2002/01/18 22:42:53 jeroen Exp $                        *
********************************************************************************/
#ifndef FXIMAGE_H
#define FXIMAGE_H

#ifndef FXDRAWABLE_H
#include "FXDrawable.h"
#endif



/// Image rendering hints
enum {
  IMAGE_KEEP       = 0x00000001,      /// Keep pixel data in client
  IMAGE_OWNED      = 0x00000002,      /// Pixel data is owned by image
  IMAGE_DITHER     = 0,               /// Dither image to look better
  IMAGE_NEAREST    = 0x00000004,      /// Turn off dithering and map to nearest color
  IMAGE_ALPHA      = 0x00000008,      /// Data has alpha channel
  IMAGE_OPAQUE     = 0x00000010,      /// Force opaque background
  IMAGE_ALPHACOLOR = 0x00000020,      /// Override transparancy color
  IMAGE_SHMI       = 0x00000040,      /// Using shared memory image
  IMAGE_SHMP       = 0x00000080,      /// Using shared memory pixmap
  IMAGE_ALPHAGUESS = 0x00000100       /// Guess transparency color from corners
  };


class FXDC;
class FXDCWindow;
class FXDrawable;
class FXTopWindow;


/// Image class
class FXAPI FXImage : public FXDrawable {
  FXDECLARE(FXImage)
  friend class FXDC;
  friend class FXDCWindow;
  friend class FXDrawable;
  friend class FXTopWindow;
private:
#ifdef WIN32
  virtual FXID GetDC() const;
  virtual int ReleaseDC(FXID) const;
#endif
protected:
  FXuchar *data;                // Pixel data
  FXuint   options;             // Options
  FXuint   channels;            // Number of channels 3 or 4
protected:
  FXImage();
#ifndef WIN32
  void render_true_32(void *xim,FXuchar *img);
  void render_true_24(void *xim,FXuchar *img);
  void render_true_16_fast(void *xim,FXuchar *img);
  void render_true_16_dither(void *xim,FXuchar *img);
  void render_true_8_fast(void *xim,FXuchar *img);
  void render_true_8_dither(void *xim,FXuchar *img);
  void render_true_N_fast(void *xim,FXuchar *img);
  void render_true_N_dither(void *xim,FXuchar *img);
  void render_index_4_fast(void *xim,FXuchar *img);
  void render_index_4_dither(void *xim,FXuchar *img);
  void render_index_8_fast(void *xim,FXuchar *img);
  void render_index_8_dither(void *xim,FXuchar *img);
  void render_index_N_fast(void *xim,FXuchar *img);
  void render_index_N_dither(void *xim,FXuchar *img);
  void render_gray_8_fast(void *xim,FXuchar *img);
  void render_gray_8_dither(void *xim,FXuchar *img);
  void render_gray_N_fast(void *xim,FXuchar *img);
  void render_gray_N_dither(void *xim,FXuchar *img);
  void render_mono_1_fast(void *xim,FXuchar *img);
  void render_mono_1_dither(void *xim,FXuchar *img);
#endif
private:
  FXImage(const FXImage&);
  FXImage &operator=(const FXImage&);
public:

  /// Create an image
  FXImage(FXApp* a,const void *pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);

  /// To get to the pixel data
  FXuchar* getData() const { return data; }

  /// To get to the option flags
  FXuint getOptions() const { return options; }

  /// Change options
  void setOptions(FXuint opts);

  /// Return number of channels, RGB or RGBA
  FXint getChannels() const { return channels; }

  /// Get pixel at x,y
  FXColor getPixel(FXint x,FXint y) const;

  /// Change pixel at x,y
  void setPixel(FXint x,FXint y,FXColor color);

  /// Create image resource
  virtual void create();

  /// Detach image resource
  virtual void detach();

  /// Destroy image resource
  virtual void destroy();

  /// Restore client-side pixel buffer from image
  virtual void restore();

  /// Render the image from client-side pixel buffer
  virtual void render();

  /// Resize pixmap to the specified width and height
  virtual void resize(FXint w,FXint h);

  /// Rescale pixels image to the specified width and height
  virtual void scale(FXint w,FXint h);

  /// Mirror image horizontally and/or vertically
  virtual void mirror(FXbool horizontal,FXbool vertical);

  /// Rotate image by degrees ccw
  virtual void rotate(FXint degrees);

  /// Crop image to given rectangle
  virtual void crop(FXint x,FXint y,FXint w,FXint h);

  /// Save object to stream
  virtual void save(FXStream& store) const;

  /// Load object from stream
  virtual void load(FXStream& store);

  /// Save pixel data only
  virtual void savePixels(FXStream& store) const;

  /// Load pixel data only
  virtual void loadPixels(FXStream& store);

  /// Destructor
  virtual ~FXImage();
  };


#endif
