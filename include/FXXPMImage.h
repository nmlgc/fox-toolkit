/********************************************************************************
*                                                                               *
*                            X P M   I m a g e   O b j e c t                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXXPMImage.h,v 1.6 2002/01/18 22:42:55 jeroen Exp $                      *
********************************************************************************/
#ifndef FXXPMIMAGE_H
#define FXXPMIMAGE_H

#ifndef FXIMAGE_H
#include "FXImage.h"
#endif


/// X Pixmap image
class FXAPI FXXPMImage : public FXImage {
  FXDECLARE(FXXPMImage)
protected:
  FXXPMImage(){}
private:
  FXXPMImage(const FXXPMImage&);
  FXXPMImage &operator=(const FXXPMImage&);
public:

  /// Construct image from compiled-in X Pixmap format
  FXXPMImage(FXApp* a,const FXchar **pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in X Pixmap format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in X Pixmap format
  virtual void loadPixels(FXStream& store);

  /// Destroy icon
  virtual ~FXXPMImage();
  };


/// Load an X Pixmap from array of strings
extern FXAPI FXbool fxloadXPM(const FXchar **pix,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Load an X Pixmap file from a stream
extern FXAPI FXbool fxloadXPM(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save an X Pixmap file to a stream
extern FXAPI FXbool fxsaveXPM(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);

#endif
