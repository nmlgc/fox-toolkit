/********************************************************************************
*                                                                               *
*                     D i r e c t o r y   L i s t   O b j e c t                 *
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
* $Id: FXDirList.h,v 1.13 1998/09/16 05:38:21 jeroen Exp $                      *
********************************************************************************/
#ifndef FXDIRLIST_H
#define FXDIRLIST_H


// // Directory List options
// enum FXDirListStyle {
//   DIRLIST_MASK          = 0x02000000
//   };


struct FXDirItem;


// Generic tab item
class FXDirList : public FXTreeList {
  FXDECLARE(FXDirList)
protected:
  FXTimer    *refresh;                // Refresh timer
  FXGIFIcon  *closed_folder;
  FXGIFIcon  *open_folder;
  FXGIFIcon  *mini_doc;
protected:
  FXDirList(){}
  FXDirList(const FXTreeList&){}
  virtual FXTreeItem* createItem();
  FXchar *getpath(FXTreeItem* item,FXchar* pathname);
  FXbool listDirectory(FXDirItem *par);
  FXbool sortDirectory(FXDirItem *par);
  FXbool scanDirectory(FXDirItem *par);
  FXbool listSubDirectories(FXDirItem *par);
public:
  long onRefresh(FXObject*,FXSelector,void*);
  long onSelectionRequest(FXObject*,FXSelector,void* ptr);
  long onItemExpanded(FXObject*,FXSelector,void*);
  long onItemCollapsed(FXObject*,FXSelector,void*);
public:
  static FXbool cmpName(const FXItem* a,const FXItem* b);
public:
  FXDirList(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  // Will change
  FXchar* itempath(FXDirItem* item,FXchar* pathname);
  virtual void destroy();
  virtual ~FXDirList();
  };


#endif
