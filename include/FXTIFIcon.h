/********************************************************************************
*                                                                               *
*                          T I F F   I c o n   O b j e c t                      *
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
* $Id: FXTIFIcon.h,v 1.7 2002/01/18 22:55:03 jeroen Exp $                       *
********************************************************************************/
#ifndef FXTIFICON_H
#define FXTIFICON_H

#ifndef FXICON_H
#include "FXIcon.h"
#endif


/// TIFF Icon class
class FXAPI FXTIFIcon : public FXIcon {
  FXDECLARE(FXTIFIcon)
protected:
  FXushort codec;
protected:
  FXTIFIcon(){}
private:
  FXTIFIcon(const FXTIFIcon&);
  FXTIFIcon &operator=(const FXTIFIcon&);
public:

  /// Construct an icon from memory stream formatted in TIFF format
  FXTIFIcon(FXApp *a,const void *pix=NULL,FXColor clr=FXRGB(192,192,192),FXuint opts=0,FXint w=1,FXint h=1);

  /// Set codec to save with
  void setCodec(FXuint c){ codec=c; }

  /// Get codec setting
  FXuint getCodec() const { return codec; }

  /// Save pixels into stream in TIFF format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in TIFF format
  virtual void loadPixels(FXStream& store);

  /// Destroy
  virtual ~FXTIFIcon();
  };


/// Load a tiff from a stream
extern FXAPI FXbool fxloadTIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height,FXushort& codec);


/// Save a tiff to a stream
extern FXAPI FXbool fxsaveTIF(FXStream& store,const FXuchar* data,FXColor transp,FXint width,FXint height,FXushort codec);


#endif
