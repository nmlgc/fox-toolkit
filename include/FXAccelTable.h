/********************************************************************************
*                                                                               *
*                   A c c e l e r a t o r   T a b l e   C l a s s               *
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
* $Id: FXAccelTable.h,v 1.4 1998/10/27 04:57:40 jeroen Exp $                    *
********************************************************************************/
#ifndef FXACCELTABLE_H
#define FXACCELTABLE_H


struct FXAccelKey;


// Matrix packer
class FXAccelTable : public FXObject {
  FXDECLARE(FXAccelTable)
private:
  FXAccelKey *key;          // Accelerator table
  FXuint      nkey;         // Accelerator table size
  FXuint      num;          // Number of entries
private:
  void grow();
protected:
  FXAccelTable(const FXAccelTable&){}
public:
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
public:
  FXAccelTable();
  void addAccel(FXHotKey hotkey,FXObject* target=NULL,FXSelector seldn=0,FXSelector selup=0);
  FXbool hasAccel(FXHotKey hotkey) const;
  void removeAccel(FXHotKey hotkey);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXAccelTable();
  };


#endif
