/********************************************************************************
*                                                                               *
*                            O b j e c t   L i s t                              *
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
* $Id: FXObjectList.h,v 1.6 1998/10/28 21:16:31 jvz Exp $                    *
********************************************************************************/
#ifndef FXOBJECTLIST_H
#define FXOBJECTLIST_H


class FXObjectList : public FXObject {
  FXDECLARE(FXObjectList)
protected:
  FXObject  **data;		// List of items
  FXuint      num;		// Used slots
  FXuint      max;		// Total slots
public:

  // Constructor
  FXObjectList();

  // Return number of elements
  FXuint no() const { return num; }

  // Set number of elements
  void no(FXuint n);

  // Return size of list
  FXuint size() const { return max; }

  // Set max number of elements
  void size(FXuint m);
    
  // Access to constant list
  FXObject* const& operator[](FXuint i) const { FXASSERT(i<num); return data[i]; }
  FXObject* const& list(FXuint i) const { FXASSERT(i<num); return data[i]; }
  
  // Access to list
  FXObject*& operator[](FXuint i){ FXASSERT(i<num); return data[i]; }
  FXObject*& list(FXuint i){ FXASSERT(i<num); return data[i]; }
  
  // Access to content array
  FXObject** list() const { return data; }

  // Insert element at certain position
  void insert(FXuint pos,FXObject* p);

  // Append element
  void append(FXObject* p);

  // Remove element at pos
  void remove(FXuint pos);

  // Remove element p
  void remove(const FXObject* p);

  // Remove all elements
  void clear();

  // Check if element in list, return position
  int find(FXuint& pos,const FXObject *p);

  // Implements saving/loading
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);

  // Destructor
  virtual ~FXObjectList();
  };


// Specialize list to pointers to TYPE
template<class TYPE>
class FXObjectListOf : public FXObjectList {
public:
  FXObjectListOf(){}
  
  // Access to constant list
  TYPE *const& operator[](const FXuint i) const { FXASSERT(i<num); return(TYPE *const&)data[i]; }
  TYPE *const& list(const FXuint i) const { FXASSERT(i<num); return(TYPE *const&)data[i]; }
  
  // Access to list
  TYPE*& operator[](const FXuint i){ FXASSERT(i<num); return(TYPE*&)data[i]; }
  TYPE*& list(const FXuint i){ FXASSERT(i<num); return (TYPE*&)data[i]; }
  
  // Access to content array
  TYPE** list() const { return (TYPE**)data; }
  };

#endif
