/********************************************************************************
*                                                                               *
*                            O b j e c t   L i s t                              *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXObjectList.h,v 1.16 2002/01/18 22:42:54 jeroen Exp $                   *
********************************************************************************/
#ifndef FXOBJECTLIST_H
#define FXOBJECTLIST_H


/// List of pointers to objects
class FXAPI FXObjectList {
protected:
  FXObject  **data;		/// List of items
  FXint       num;		/// Used slots
  FXint       max;		/// Total slots
public:

  /// Default constructor
  FXObjectList();

  /// Copy constructor
  FXObjectList(const FXObjectList& orig);

  /// Assignment operator
  FXObjectList& operator=(const FXObjectList& orig);

  /// Return number of elements
  FXint no() const { return num; }

  /// Set number of elements
  void no(FXint n);

  /// Return size of list
  FXint size() const { return max; }

  /// Set max number of elements
  void size(FXint m);

  /// Indexing operator
  FXObject*& operator[](FXint i){ return data[i]; }
  FXObject* const& operator[](FXint i) const { return data[i]; }

  /// Access to list
  FXObject*& list(FXint i){ return data[i]; }
  FXObject* const& list(FXint i) const { return data[i]; }

  /// Access to content array
  FXObject** list() const { return data; }

  /// Insert element at certain position
  void insert(FXint pos,FXObject* p);

  /// Prepend element
  void prepend(FXObject* p);

  /// Append element
  void append(FXObject* p);

  /// Replace element
  void replace(FXint pos,FXObject* p);

  /// Remove element at pos
  void remove(FXint pos);

  /// Remove element p
  void remove(const FXObject* p);

  /// Find object in list, searching forward; return position or -1
  FXint findf(const FXObject *p,FXint pos=0) const;

  /// Find object in list, searching backward; return position or -1
  FXint findb(const FXObject *p,FXint pos=2147483647) const;

  /// Remove all elements
  void clear();

  /// Save to a stream
  void save(FXStream& store) const;

  /// Load from a stream
  void load(FXStream& store);

  /// Destructor
  virtual ~FXObjectList();
  };


/// Specialize list to pointers to TYPE
template<class TYPE>
class FXObjectListOf : public FXObjectList {
public:
  FXObjectListOf(){}

  /// Indexing operator
  TYPE*& operator[](FXint i){ return (TYPE*&)data[i]; }
  TYPE *const& operator[](FXint i) const { return (TYPE*const&)data[i]; }

  /// Access to list
  TYPE*& list(FXint i){ return (TYPE*&)data[i]; }
  TYPE *const& list(FXint i) const { return (TYPE*const&)data[i]; }

  /// Access to content array
  TYPE** list() const { return (TYPE**)data; }
  };

#endif
