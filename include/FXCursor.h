/********************************************************************************
*                                                                               *
*                         C u r s o r - O b j e c t                             *
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
* $Id: FXCursor.h,v 1.13 2002/01/18 22:42:51 jeroen Exp $                       *
********************************************************************************/
#ifndef FXCURSOR_H
#define FXCURSOR_H

#ifndef FXID_H
#include "FXId.h"
#endif



// Stock cursors
enum FXStockCursor {
  CURSOR_ARROW=1,                   // Default left pointing arrow
  CURSOR_RARROW,                    // Right arrow
  CURSOR_IBEAM,                     // Text I-Beam
  CURSOR_WATCH,                     // Stopwatch or hourglass
  CURSOR_CROSS,                     // Crosshair
  CURSOR_UPDOWN,                    // Move up, down
  CURSOR_LEFTRIGHT,                 // Move left, right
  CURSOR_MOVE                       // Move up,down,left,right
  };



/// Cursor class
class FXAPI FXCursor : public FXId {
  FXDECLARE(FXCursor)
protected:
  FXuchar *source;            // Source data
  FXuchar *mask;              // Mask data
  FXint    width;             // Width
  FXint    height;            // Height
  FXint    hotx;              // Hot spot x
  FXint    hoty;              // Hot spot y
  FXuchar  glyph;             // Glyph type cursor (stock cursor)
  FXbool   owned;             // Owns data
protected:
  FXCursor();
private:
  FXCursor(const FXCursor&);
  FXCursor &operator=(const FXCursor&);
public:

  /// Make stock cursor
  FXCursor(FXApp* a,FXStockCursor curid=CURSOR_ARROW);

  /// Make cursor from source and mask; cursor size should at most 32x32 for portability!
  FXCursor(FXApp* a,const void* src,const void* msk,FXint w=32,FXint h=32,FXint hx=-1,FXint hy=-1);

  /// Width of cursor
  FXint getWidth() const { return width; }

  /// Height or cursor
  FXint getHeight() const { return height; }

  /// Get hotspot x
  FXint getHotX() const { return hotx; }

  /// Get hotspot y
  FXint getHotY() const { return hoty; }

  /// Create cursor
  virtual void create();

  /// Detach cursor
  virtual void detach();

  /// Destroy cursor
  virtual void destroy();

  /// Save pixel data only
  virtual void savePixels(FXStream& store) const;

  /// Load pixel data only
  virtual void loadPixels(FXStream& store);

  /// Save cursor to a stream
  virtual void save(FXStream& store) const;

  /// Load cursor from a stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXCursor();
  };


#endif
