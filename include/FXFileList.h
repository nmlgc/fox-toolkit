/********************************************************************************
*                                                                               *
*                        F i l e    L i s t   O b j e c t                       *
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
* $Id: FXFileList.h,v 1.24 1998/09/29 20:16:38 jeroen Exp $                     *
********************************************************************************/
#ifndef FXFILELIST_H
#define FXFILELIST_H


struct FXFileItem;


// Icon List object
class FXFileList : public FXIconList {
  FXDECLARE(FXFileList)
protected:
  FXString    directory;              // Current directory
  FXString    pattern;                // Pattern of file names
  FXFileItem *list;                   // Items in insertion order
  FXTimer    *refresh;                // Refresh timer
  FXint       timestamp;              // Time when last refreshed
  FXuint      matchmode;              // File wildcard match mode
  FXGIFIcon  *big_folder;             // Fallback icons
  FXGIFIcon  *mini_folder;
  FXGIFIcon  *big_doc;
  FXGIFIcon  *mini_doc;
  FXGIFIcon  *big_app;
  FXGIFIcon  *mini_app;
protected:
  FXFileList(){}
  FXFileList(const FXFileList&){}
  FXbool listDirectory();
  FXbool wipeDirectory();
  FXbool sortDirectory();
  virtual FXIconItem* createItem();
public:
  long onRefresh(FXObject*,FXSelector,void*);
  long onSelectionRequest(FXObject*,FXSelector,void* ptr);
  long onCmdGetStringValue(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
  long onCmdDirectoryUp(FXObject*,FXSelector,void*);
  long onUpdDirectoryUp(FXObject*,FXSelector,void*);
  long onCmdDirectoryNew(FXObject*,FXSelector,void*);
  long onUpdDirectoryNew(FXObject*,FXSelector,void*);
  long onCmdSortByName(FXObject*,FXSelector,void*);
  long onUpdSortByName(FXObject*,FXSelector,void*);
  long onCmdSortByType(FXObject*,FXSelector,void*);
  long onUpdSortByType(FXObject*,FXSelector,void*);
  long onCmdSortBySize(FXObject*,FXSelector,void*);
  long onUpdSortBySize(FXObject*,FXSelector,void*);
  long onCmdSortByTime(FXObject*,FXSelector,void*);
  long onUpdSortByTime(FXObject*,FXSelector,void*);
  long onCmdSortByUser(FXObject*,FXSelector,void*);
  long onUpdSortByUser(FXObject*,FXSelector,void*);
  long onCmdSortByGroup(FXObject*,FXSelector,void*);
  long onUpdSortByGroup(FXObject*,FXSelector,void*);
  long onCmdSortReverse(FXObject*,FXSelector,void*);
  long onUpdSortReverse(FXObject*,FXSelector,void*);
  long onCmdSetPattern(FXObject*,FXSelector,void*);
  long onUpdSetPattern(FXObject*,FXSelector,void*);
  long onCmdSetDirectory(FXObject*,FXSelector,void*);
  long onUpdSetDirectory(FXObject*,FXSelector,void*);
public:
  static FXbool cmpFName(const FXItem* a,const FXItem* b);
  static FXbool cmpFType(const FXItem* a,const FXItem* b);
  static FXbool cmpFSize(const FXItem* a,const FXItem* b);
  static FXbool cmpFTime(const FXItem* a,const FXItem* b);
  static FXbool cmpFUser(const FXItem* a,const FXItem* b);
  static FXbool cmpFGroup(const FXItem* a,const FXItem* b);
  static FXbool cmpRName(const FXItem* a,const FXItem* b);
  static FXbool cmpRType(const FXItem* a,const FXItem* b);
  static FXbool cmpRSize(const FXItem* a,const FXItem* b);
  static FXbool cmpRTime(const FXItem* a,const FXItem* b);
  static FXbool cmpRUser(const FXItem* a,const FXItem* b);
  static FXbool cmpRGroup(const FXItem* a,const FXItem* b);
public:
  enum {
    ID_SORT_BY_NAME=FXIconList::ID_LAST,
    ID_SORT_BY_TYPE,
    ID_SORT_BY_SIZE,
    ID_SORT_BY_TIME,
    ID_SORT_BY_USER,
    ID_SORT_BY_GROUP,
    ID_SORT_REVERSE,
    ID_DIRECTORY_UP,
    ID_DIRECTORY_NEW,
    ID_SET_PATTERN,
    ID_SET_DIRECTORY,
    ID_LAST
    };
public:
  FXFileList(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void destroy();
  void setDirectory(const FXchar* path);
  const FXchar* getDirectory() const { return directory.text(); }
  void setPattern(const FXchar* ptrn);
  const FXchar* getPattern() const { return pattern.text(); }
  FXbool isDirectory(const FXIconItem* item) const;
  FXbool isFile(const FXIconItem* item) const;
  FXString getItemFileName(FXIconItem* item) const;
  FXuint getMatchMode() const { return matchmode; }
  void setMatchMode(FXuint mode);
  virtual ~FXFileList();
  };


#endif
