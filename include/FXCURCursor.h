/********************************************************************************
*                                                                               *
*                        C U R   C u r s o r    O b j e c t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 by Sander Jansen.   All Rights Reserved.              *
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
* $Id: FXCURCursor.h,v 1.5 2002/01/18 22:42:51 jeroen Exp $                     *
********************************************************************************/
#ifndef FXCURCURSOR_H
#define FXCURCURSOR_H

#ifndef FXCURSOR_H
#include "FXCursor.h"
#endif


/// CUR Cursor class
class FXAPI FXCURCursor : public FXCursor {
  FXDECLARE(FXCURCursor)
protected:
  FXCURCursor(){}
private:
  FXCURCursor(const FXCURCursor&);
  FXCURCursor &operator=(const FXCURCursor&);
public:

  /// Construct CUR cursor from memory stream in Microsoft CUR format
  FXCURCursor(FXApp* a,const void* pix);

  /// Load pixel data only
  virtual void loadPixels(FXStream& store);

  /// Destructor
  virtual ~FXCURCursor();
  };


/// Load a cur file from a stream
extern FXAPI FXbool fxloadCUR(FXStream& store,FXuchar*& source,FXuchar*& mask,FXint& width,FXint& height,FXint& xspot,FXint& yspot);


#endif
