/********************************************************************************
*                                                                               *
*                      I R I S   R G B   I c o n   O b j e c t                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXRGBIcon.h,v 1.1 2002/01/22 18:35:41 jeroen Exp $                       *
********************************************************************************/
#ifndef FXRGBICON_H
#define FXRGBICON_H

#ifndef FXICON_H
#include "FXIcon.h"
#endif



/// IRIS RGB icon
class FXAPI FXRGBIcon : public FXIcon {
  FXDECLARE(FXRGBIcon)
protected:
  FXRGBIcon(){}
private:
  FXRGBIcon(const FXRGBIcon&);
  FXRGBIcon &operator=(const FXRGBIcon&);
public:

  /// Construct icon from memory stream formatted in IRIS-RGB format
  FXRGBIcon(FXApp* a,const void *pix=NULL,FXColor clr=FXRGB(192,192,192),FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in IRIS-RGB format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in IRIS-RGB format
  virtual void loadPixels(FXStream& store);

  /// Destroy icon
  virtual ~FXRGBIcon();
  };


/// Load a IRIS-RGB file from a stream
extern FXAPI FXbool fxloadRGB(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a IRIS-RGB file to a stream
extern FXAPI FXbool fxsaveRGB(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


#endif
