/********************************************************************************
*                                                                               *
*       T r e e L i s t   I t e m   P r i v a t e   H e a d e r   F i l e       *
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
* $Id: FXTreeItem.h,v 1.18 1998/10/02 14:40:51 jvz Exp $                     *
********************************************************************************/
#ifndef FXTREEITEM_H
#define FXTREEITEM_H


enum FXTreeItemState {
  TREEITEM_CURRENT    = 1, 
  TREEITEM_SELECTED   = 2, 
  TREEITEM_HISTORY    = 4, 
  TREEITEM_MARK       = 8,
  TREEITEM_OPEN       = 16, 
  TREEITEM_EXPANDED   = 32, 
  };


struct FXTreeItem {
  FXTreeItem *prev;
  FXTreeItem *next;
  FXTreeItem *parent;
  FXTreeItem *first;
  FXTreeItem *last;
  FXString    label;
  FXIcon     *openIcon;
  FXIcon     *closedIcon;
  FXuint      state;
  void       *data;
  FXint       x,y;
  };


#endif
