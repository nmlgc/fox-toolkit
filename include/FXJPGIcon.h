/********************************************************************************
*                                                                               *
*                         J P E G   I c o n   O b j e c t                       *
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
* $Id: FXJPGIcon.h,v 1.4 2002/01/18 22:55:03 jeroen Exp $                       *
********************************************************************************/
#ifndef FXJPGICON_H
#define FXJPGICON_H

#ifndef FXICON_H
#include "FXIcon.h"
#endif



/// JPEG Icon class
class FXAPI FXJPGIcon : public FXIcon {
  FXDECLARE(FXJPGIcon)
protected:
  FXint quality;
protected:
  FXJPGIcon(){}
private:
  FXJPGIcon(const FXJPGIcon&);
  FXJPGIcon &operator=(const FXJPGIcon&);
public:

  /// Construct an icon from memory stream formatted in JPEG format
  FXJPGIcon(FXApp *a,const void *pix=NULL,FXColor clr=FXRGB(192,192,192),FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in JPEG format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in JPEG format
  virtual void loadPixels(FXStream& store);

  /// Set image quality to save with
  void setQuality(FXint q){ quality=q; }

  /// Get image quality setting
  FXint getQuality() const { return quality; }

  /// Destroy
  virtual ~FXJPGIcon();
  };


/// Load a jpeg from a stream
extern FXAPI FXbool fxloadJPG(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height,FXint& quality);


/// Save a jpeg to a stream
extern FXAPI FXbool fxsaveJPG(FXStream& store,const FXuchar* data,FXColor transp,FXint width,FXint height,FXint quality);


typedef FXJPGIcon FXJPEGIcon;


#endif
