/********************************************************************************
*                                                                               *
*                            B M P   I m a g e   O b j e c t                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXBMPImage.h,v 1.5 1998/09/23 20:04:02 jvz Exp $                      *
********************************************************************************/
#ifndef FXBMPIMAGE_H
#define FXBMPIMAGE_H



// Image class
class FXBMPImage : public FXImage {
  FXDECLARE(FXBMPImage)
protected:
  FXBMPImage(){}
  FXBMPImage(const FXBMPImage&){}
public:
  FXBMPImage(FXApp* a,const void *pix=NULL,FXuint opts=0,FXint w=1,FXint h=1);
  virtual void savePixels(FXStream& store) const;
  virtual void loadPixels(FXStream& store);
  virtual ~FXBMPImage();
  };
  
    
#endif
