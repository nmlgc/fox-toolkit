/********************************************************************************
*                                                                               *
*           F i l e   I t e m   P r i v a t e   H e a d e r   F i l e           *
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
* $Id: FXFileItem.h,v 1.10 1998/10/02 14:40:51 jvz Exp $                      *
********************************************************************************/
#ifndef FXFILEITEM_H
#define FXFILEITEM_H


// Icon item states
enum FXFileItemState {
  FILEITEM_CURRENT    = ICONITEM_CURRENT,   // Current item
  FILEITEM_SELECTED   = ICONITEM_SELECTED,  // Has been selected
  FILEITEM_HISTORY    = ICONITEM_HISTORY,   // Was selected
  FILEITEM_MARK       = ICONITEM_MARK,      // Marked selection state
  FILEITEM_DIRECTORY  = 16,                 // Directory item
  FILEITEM_EXECUTABLE = 32,                 // Executable item
  FILEITEM_SYMLINK    = 64,                 // Symbolic linked item
  FILEITEM_CHARDEV    = 128,                // Character special item
  FILEITEM_BLOCKDEV   = 256,                // Block special item
  FILEITEM_FIFO       = 512,                // FIFO item
  FILEITEM_SOCK       = 1024,               // Socket item
  };
     

struct FXFileItem : public FXIconItem {
  FXFileItem *iprev;
  FXFileItem *inext;
  long        size;
  long        date; 
  };

#endif
