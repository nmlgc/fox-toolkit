/********************************************************************************
*                                                                               *
*                          T I F F   I m a g e   O b j e c t                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 Eric Gillet.   All Rights Reserved.                   *
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
* $Id: FXTIFImage.h,v 1.7 2002/01/18 22:55:03 jeroen Exp $                      *
********************************************************************************/
#ifndef FXTIFIMAGE_H
#define FXTIFIMAGE_H

#ifndef FXIMAGE_H
#include "FXImage.h"
#endif


/// TIFF Image class
class FXAPI FXTIFImage : public FXImage {
  FXDECLARE(FXTIFImage)
protected:
  FXushort codec;
protected:
  FXTIFImage(){}
private:
  FXTIFImage(const FXTIFImage&);
  FXTIFImage &operator=(const FXTIFImage&);
public:

  /// Construct an image from memory stream formatted in TIFF format
  FXTIFImage(FXApp *a,const void *pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);

  /// Set codec to save with
  void setCodec(FXuint c){ codec=c; }

  /// Get codec setting
  FXuint getCodec() const { return codec; }

  /// Save pixels into stream in TIFF format
  virtual void loadPixels(FXStream& store);

  /// Load pixels from stream in TIFF format
  virtual void savePixels(FXStream& store) const;

  /// Destroy
  virtual ~FXTIFImage();
  };


/// Load a tiff from a stream
extern FXAPI FXbool fxloadTIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height,FXushort& codec);


/// Save a tiff to a stream
extern FXAPI FXbool fxsaveTIF(FXStream& store,const FXuchar* data,FXColor transp,FXint width,FXint height,FXushort codec);


#endif
