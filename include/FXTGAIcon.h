/********************************************************************************
*                                                                               *
*                     T A R G A   I c o n   O b j e c t                         *
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
* $Id: FXTGAIcon.h,v 1.5 2002/01/18 22:55:03 jeroen Exp $                       *
********************************************************************************/
#ifndef FXTGAICON_H
#define FXTGAICON_H

#ifndef FXICON_H
#include "FXIcon.h"
#endif


/// TARGA image format icon
class FXAPI FXTGAIcon : public FXIcon {
  FXDECLARE(FXTGAIcon)
protected:
  FXTGAIcon(){}
private:
  FXTGAIcon(const FXTGAIcon&);
  FXTGAIcon &operator=(const FXTGAIcon&);
public:

  /// Construct icon from memory stream formatted in TARGA format
  FXTGAIcon(FXApp* a,const void *pix=NULL,FXColor clr=FXRGB(192,192,192),FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in TARGA format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in TARGA format
  virtual void loadPixels(FXStream& store);

  /// Destroy icon
  virtual ~FXTGAIcon();
  };


/// Load a TARGA file from a stream
extern FXAPI FXbool fxloadTGA(FXStream& store,FXuchar*& data,FXuint& channels,FXint& width,FXint& height);


/// Save a TARGA file to a stream
extern FXAPI FXbool fxsaveTGA(FXStream& store,const FXuchar *data,FXuint channels,FXint width,FXint height);


typedef FXTGAIcon FXTargaIcon;

#endif
