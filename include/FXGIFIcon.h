/********************************************************************************
*                                                                               *
*                        G I F   I c o n   O b j e c t                          *
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
* $Id: FXGIFIcon.h,v 1.9 2002/01/18 22:42:53 jeroen Exp $                       *
********************************************************************************/
#ifndef FXGIFICON_H
#define FXGIFICON_H

#ifndef FXICON_H
#include "FXIcon.h"
#endif


/// GIF Icon class
class FXAPI FXGIFIcon : public FXIcon {
  FXDECLARE(FXGIFIcon)
protected:
  FXGIFIcon(){}
private:
  FXGIFIcon(const FXGIFIcon&);
  FXGIFIcon &operator=(const FXGIFIcon&);
public:

  /// Construct an icon from memory stream formatted as CompuServe GIF format
  FXGIFIcon(FXApp* a,const void *pix=NULL,FXColor clr=FXRGB(192,192,192),FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in [un]GIF format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in CompuServe GIF format
  virtual void loadPixels(FXStream& store);

  /// Destroy
  virtual ~FXGIFIcon();
  };


/// Load a gif file from a stream
extern FXAPI FXbool fxloadGIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a gif file to a stream
extern FXAPI FXbool fxsaveGIF(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


#endif
