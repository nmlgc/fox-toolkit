/********************************************************************************
*                                                                               *
*                      F i l e   S e l e c t i o n   O b j e c t                *
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
* $Id: FXFileSelector.h,v 1.9 1998/09/30 22:03:51 jvz Exp $                  *
********************************************************************************/
#ifndef FXFILESELECTOR_H
#define FXFILESELECTOR_H


// File selection widget
class FXFileSelector : public FXPacker {
  FXDECLARE(FXFileSelector)
protected:
  FXFileList  *filebox;
  FXTextField *filename;
  FXTextField *filefilter;
  FXTextField *directory;
  FXButton    *accept;
  FXButton    *cancel;
  FXGIFIcon   *updiricon;
  FXGIFIcon   *newdiricon;
  FXGIFIcon   *listicon;
  FXGIFIcon   *detailicon;
  FXGIFIcon   *iconsicon;
protected:
  FXFileSelector(){}
  FXFileSelector(const FXFileSelector&){}
public:
  long onCmdName(FXObject*,FXSelector,void*);
  long onCmdFilter(FXObject*,FXSelector,void*);
  long onCmdItemOpened(FXObject*,FXSelector,void*);
  long onCmdItemSelected(FXObject*,FXSelector,void*);
public:
  enum{
    ID_FILEFILTER=FXPacker::ID_LAST,
    ID_FILENAME,
    ID_FILELIST,
    ID_LAST
    };
public:
  FXFileSelector(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  FXButton *acceptButton() const { return accept; }
  FXButton *cancelButton() const { return cancel; }
  void setFilename(const FXchar* path);
  const FXchar* getFilename() const;
  void setPattern(const FXchar* ptrn);
  const FXchar* getPattern() const;
  void setDirectory(const FXchar* path);
  const FXchar* getDirectory() const;
  virtual ~FXFileSelector();
  };


#endif
