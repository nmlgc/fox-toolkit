/********************************************************************************
*                                                                               *
*                             B i t m a p    O b j e c t                        *
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
* $Id: FXBitmap.h,v 1.19 2002/01/18 22:42:51 jeroen Exp $                       *
********************************************************************************/
#ifndef FXBITMAP_H
#define FXBITMAP_H

#ifndef FXDRAWABLE_H
#include "FXDrawable.h"
#endif


// Image rendering hints
enum {
  BITMAP_KEEP       = 0x00000001,       // Keep pixel data in client
  BITMAP_OWNED      = 0x00000002,       // Pixel data is owned by image
  BITMAP_SHMI       = 0x00000020,       // Using shared memory image
  BITMAP_SHMP       = 0x00000040        // Using shared memory pixmap
  };


// Forward declarations
class FXDC;
class FXDCWindow;
class FXDrawable;
class FXTopWindow;


/**
* Bitmap is a one bit/pixel image used for patterning and
* stippling operations.
*/
class FXAPI FXBitmap : public FXDrawable {
  FXDECLARE(FXBitmap)
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
protected:
  FXBitmap();
private:
  FXBitmap(const FXBitmap&);
  FXBitmap &operator=(const FXBitmap&);
public:

  /// Create an image
  FXBitmap(FXApp* a,const void *pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);

  /// Create bitmap
  virtual void create();

  /// Detach from bitmap
  virtual void detach();

  /// Destroy bitmap
  virtual void destroy();

  /// Render pixels
  virtual void render();

  /// Get pixel at x,y
  FXbool getPixel(FXint x,FXint y) const;

  /// Change pixel at x,y
  void setPixel(FXint x,FXint y,FXbool color);

  /// Resize bitmap to the specified width and height; the contents become undefined
  virtual void resize(FXint w,FXint h);

  /// Save object to stream
  virtual void save(FXStream& store) const;

  /// Load object from stream
  virtual void load(FXStream& store);

  /// Save pixel data only
  virtual void savePixels(FXStream& store) const;

  /// Load pixel data only
  virtual void loadPixels(FXStream& store);

  /// Cleanup
  virtual ~FXBitmap();
  };


#endif
