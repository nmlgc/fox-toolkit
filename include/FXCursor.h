/********************************************************************************
*                                                                               *
*                         C u r s o r - O b j e c t                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXCursor.h,v 1.2 1998/09/18 22:07:11 jvz Exp $                    *
********************************************************************************/
#ifndef FXCURSOR_H
#define FXCURSOR_H


class FXCursor : public FXId {
  FXDECLARE(FXCursor)
protected:

  FXint    hotx;          // Hot spot
  FXint    hoty;

protected:

  FXCursor(){}
  FXCursor(const FXCursor&){}

  virtual void create();

public:

  // Make ID in an app
  FXCursor(FXApp* a);
  
  // Get hotspot x
  FXint getHotX() const { return hotx; }
  
  // Get hotspot y
  FXint getHotY() const { return hoty; }
  
  // Create from font
  FXint createFromFont(FXint shape);
  
  // Create from bitmap
  FXint createFromBitmapData(const FXchar* image,const FXchar* mask,FXint width,FXint height,FXint hx=-1,FXint hy=-1);
  
  // Destroy cursor
  void destroy();
  
  // Save cursor
  virtual void save(FXStream& store) const;
  
  // Load cursor
  virtual void load(FXStream& store);
  
  // Cleanup
  virtual ~FXCursor();
  };
  
    
#endif
