/********************************************************************************
*                                                                               *
*       D i r e c t o r y   I t e m   P r i v a t e   H e a d e r   F i l e     *
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
* $Id: FXDirItem.h,v 1.9 1998/10/02 14:40:51 jvz Exp $                       *
********************************************************************************/
#ifndef FXFILEITEM_H
#define FXFILEITEM_H


// Icon item states
enum FXFileItemState {
  DIRITEM_CURRENT    = TREEITEM_CURRENT,
  DIRITEM_SELECTED   = TREEITEM_SELECTED,
  DIRITEM_HISTORY    = TREEITEM_HISTORY,
  DIRITEM_MARK       = TREEITEM_MARK,
  DIRITEM_OPEN       = TREEITEM_OPEN,
  DIRITEM_EXPANDED   = TREEITEM_EXPANDED
  };
     

struct FXDirItem : public FXTreeItem {
  FXDirItem *iprev;
  FXDirItem *inext;
  FXDirItem *list;
  FXint      timestamp;
  };

#endif
