/********************************************************************************
*                                                                               *
*                               I c o n - O b j e c t                           *
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
* $Id: FXIcon.h,v 1.17 2002/02/20 07:02:12 fox Exp $                         *
********************************************************************************/
#ifndef FXICON_H
#define FXICON_H

#ifndef FXIMAGE_H
#include "FXImage.h"
#endif



class FXDC;
class FXDCWindow;
class FXDrawable;
class FXTopWindow;


/// Icon class
class FXAPI FXIcon : public FXImage {
  FXDECLARE(FXIcon)
  friend class FXDC;
  friend class FXDCWindow;
  friend class FXDrawable;
  friend class FXTopWindow;
protected:
  FXID     shape;             // Shape pixmap
  FXID     etch;              // Etch pixmap
  FXColor  transp;            // Transparency color
protected:
  FXIcon(){}
  FXColor guesstransp();
private:
  FXIcon(const FXIcon&);
  FXIcon &operator=(const FXIcon&);
public:

  /**
  * Create an icon with an initial pixel buffer pix, a transparent color clr,
  * and options as in FXImage.
  */
  FXIcon(FXApp* a,const void *pix=NULL,FXColor clr=0,FXuint opts=0,FXint w=1,FXint h=1);

  /// Create the icon resource
  virtual void create();

  /// Detach the icon resource
  virtual void detach();

  /// Destroy the icon resource
  virtual void destroy();

  /// Render the image from client-side pixel buffer
  virtual void render();

  /// Resize pixmap to the specified width and height
  virtual void resize(FXint w,FXint h);

  /// Obtain transparency color
  FXColor getTransparentColor() const { return transp; }
  
  /// Change transparency color
  void setTransparentColor(FXColor color){ transp=color; }
  
  /// Destructor
  virtual ~FXIcon();
  };


#endif
