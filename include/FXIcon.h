/********************************************************************************
*                                                                               *
*                               I c o n - O b j e c t                           *
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
* $Id: FXIcon.h,v 1.7 1998/04/30 00:56:56 jeroen Exp $                          *
********************************************************************************/
#ifndef FXICON_H
#define FXICON_H


class FXIcon : public FXImage {
  FXDECLARE(FXIcon)
  friend class FXDrawable;
protected:
  FXID     shape;             // Shape pixmap
  FXColor  transp;            // Transparency color
protected:
  FXIcon(){}
  FXIcon(const FXIcon&){}
  void render_bitmap(XImage *xim,FXuchar *img);
public:
  FXIcon(FXApp* a,const void *pix=NULL,FXColor clr=0,FXuint opts=0,FXint w=1,FXint h=1);
  virtual void create();
  virtual void destroy();
  virtual void render();
  virtual ~FXIcon();
  };
  
    
#endif
