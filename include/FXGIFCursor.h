/********************************************************************************
*                                                                               *
*                        G I F   C u r so r   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Daniel Gehriger.   All Rights Reserved.            *
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
* $Id: FXGIFCursor.h,v 1.7 2002/01/18 22:42:53 jeroen Exp $                     *
********************************************************************************/
#ifndef FXGIFCURSOR_H
#define FXGIFCURSOR_H

#ifndef FXCURSOR_H
#include "FXCursor.h"
#endif


//////////// WE PROBABLY WANT SOME OPTIONS HERE SAME AS FXGIFIcon ///////////////

/// GIF Cursor class
class FXAPI FXGIFCursor : public FXCursor {
  FXDECLARE(FXGIFCursor)
protected:
  FXGIFCursor(){}
private:
  FXGIFCursor(const FXGIFCursor&);
  FXGIFCursor &operator=(const FXGIFCursor&);
public:

  /// Construct a cursor from memory stream formatted as CompuServe GIF format
  FXGIFCursor(FXApp* a,const void* pix,FXint hx=-1,FXint hy=-1);

  /// Destroy
  virtual ~FXGIFCursor(){}
  };


/// Load a gif file from a stream
extern FXAPI FXbool fxloadGIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);


/// Save a gif file to a stream
extern FXAPI FXbool fxsaveGIF(FXStream& store,const FXuchar *data,FXColor transp,FXint width,FXint height);


#endif
