/********************************************************************************
*                                                                               *
*                       T A R G A  I m a g e   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 by Janusz Ganczarski.   All Rights Reserved.          *
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
* $Id: FXTGAImage.h,v 1.4 2002/01/18 22:55:03 jeroen Exp $                      *
********************************************************************************/
#ifndef FXTGAIMAGE_H
#define FXTGAIMAGE_H

#ifndef FXIMAGE_H
#include "FXImage.h"
#endif


/// TARGA graphics file
class FXAPI FXTGAImage : public FXImage {
  FXDECLARE(FXTGAImage)
protected:
  FXTGAImage(){}
private:
  FXTGAImage(const FXTGAImage&);
  FXTGAImage &operator=(const FXTGAImage&);
public:

  /// Construct image from memory stream formatted in TARGA file
  FXTGAImage(FXApp* a,const void *pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in TARGA file
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in TARGA format
  virtual void loadPixels(FXStream& store);

  /// Destroy image
  virtual ~FXTGAImage();
  };


/// Load a TARGA file from a stream
extern FXAPI FXbool fxloadTGA(FXStream& store,FXuchar*& data,FXuint& channels,FXint& width,FXint& height);


/// Save a TARGA file to a stream
extern FXAPI FXbool fxsaveTGA(FXStream& store,const FXuchar *data,FXuint channels,FXint width,FXint height);


typedef FXTGAImage FXTargaImage;

#endif
