/********************************************************************************
*                                                                               *
*           I c o n   I t e m   P r i v a t e   H e a d e r   F i l e           *
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
* $Id: FXIconItem.h,v 1.13 1998/10/02 14:40:51 jvz Exp $                     *
********************************************************************************/
#ifndef FXICONITEM_H
#define FXICONITEM_H

// Icon item states
enum FXIconItemState {
  ICONITEM_CURRENT  = 1,
  ICONITEM_SELECTED = 2,
  ICONITEM_HISTORY  = 4,
  ICONITEM_MARK     = 8
  };
     

struct FXIconItem {
  FXIconItem *prev;
  FXIconItem *next;
  FXString    label;
  FXIcon     *icon;
  FXIcon     *miniIcon;
  FXuint      state;
  void       *data;
  FXint       x,y;
  };

#endif
