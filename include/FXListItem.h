/********************************************************************************
*                                                                               *
*            L i s t   I t e m   P r i v a t e   H e a d e r   F i l e          *
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
* $Id: FXListItem.h,v 1.12 1998/10/02 06:07:03 jeroen Exp $                     *
********************************************************************************/
#ifndef FXLISTITEM_H
#define FXLISTITEM_H



enum FXListItemState {
  LISTITEM_CURRENT  = 1,
  LISTITEM_SELECTED = 2,
  LISTITEM_HISTORY  = 4,
  LISTITEM_MARK     = 8
  };
  

struct FXListItem {
  FXListItem *prev;
  FXListItem *next;
  FXString    label;
  FXuint      state;
  void       *data;
  FXint       y;
  };


#endif
