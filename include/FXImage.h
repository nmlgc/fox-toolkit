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
* $Id: FXImage.h,v 1.12 1998/10/28 07:58:55 jeroen Exp $                        *
********************************************************************************/
#ifndef FXIMAGE_H
#define FXIMAGE_H


// Image rendering hints
enum FXImageHints {
  IMAGE_KEEP       = 0x00000001,      // Keep pixel data in client
  IMAGE_OWNED      = 0x00000002,      // Pixel data is owned by image
  IMAGE_DITHER     = 0x00000004,      // Dither image to look better
  IMAGE_ALPHA      = 0x00000008,      // Data has alpha channel
  IMAGE_OPAQUE     = 0x00000010,      // Force opaque background
  IMAGE_ALPHACOLOR = 0x00000020,      // Override transparancy color
  IMAGE_SHMI       = 0x00000040,      // Using shared memory image
  IMAGE_SHMP       = 0x00000080       // Using shared memory pixmap
  };
    

// Image class
class FXImage : public FXDrawable {
  FXDECLARE(FXImage)
  friend class FXDrawable;
protected:
  FXuchar *data;                // Pixel data
  FXuint   options;             // Options
protected:
  enum{
    MAXWIDTH=2048,              // Maximum image size
    MAXHEIGHT=2048
    };          
protected:
  FXImage();
  FXImage(const FXImage&){}
  void render_true(XImage *xim,FXuchar *img,FXuint step);
  void render_index(XImage *xim,FXuchar *img,FXuint step);
  void render_gray(XImage *xim,FXuchar *img,FXuint step);
public:

  // Create an image
  FXImage(FXApp* a,const void *pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);
  
  // Create image
  virtual void create();

  // Destroy image
  virtual void destroy();
  
  // Render pixels
  virtual void render();
  
  // Save object to stream
  virtual void save(FXStream& store) const;
  
  // Load object from stream
  virtual void load(FXStream& store);
  
  // Save pixel data only
  virtual void savePixels(FXStream& store) const;
  
  // Load pixel data only
  virtual void loadPixels(FXStream& store);

  // Cleanup
  virtual ~FXImage();
  };
  
    
#endif
