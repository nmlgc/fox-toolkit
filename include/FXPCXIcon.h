/********************************************************************************
*                                                                               *
*                        P C X   I c o n   O b j e c t                          *
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
* $Id: FXPCXIcon.h,v 1.4 2002/01/18 22:55:03 jeroen Exp $                       *
********************************************************************************/
#ifndef FXPCXICON_H
#define FXPCXICON_H

#ifndef FXICON_H
#include "FXIcon.h"
#endif


/// PCX icon
class FXAPI FXPCXIcon : public FXIcon {
  FXDECLARE(FXPCXIcon)
protected:
  FXPCXIcon(){}
private:
  FXPCXIcon(const FXPCXIcon&);
  FXPCXIcon &operator=(const FXPCXIcon&);
public:

  /// Construct icon from memory stream formatted in PCX file format
  FXPCXIcon(FXApp* a,const void *pix=NULL,FXColor clr=FXRGB(192,192,192),FXuint opts=0,FXint w=1,FXint h=1);

  /// Save pixels into stream in PCX file format
  virtual void savePixels(FXStream& store) const;

  /// Load pixels from stream in PCX file format
  virtual void loadPixels(FXStream& store);

  /// Destroy icon
  virtual ~FXPCXIcon();
  };


/// Load a PCX file from a stream
extern FXAPI FXbool fxloadPCX(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a PCX file to a stream
extern FXAPI FXbool fxsavePCX(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


#endif
