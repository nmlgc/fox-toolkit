/********************************************************************************
*                                                                               *
*                           Generic Element Handling                            *
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
* $Id: FXElement.h,v 1.3 1998/02/27 04:30:36 jeroen Exp $                       *
********************************************************************************/
#ifndef FXELEMENT_H
#define FXELEMENT_H


/****************************  D e f i n i t i o n  ****************************/

// Generic implementations for generic objects


// Construct some elements at a location
template<class TYPE>
inline void constructElms(TYPE* ptr,unsigned int n){
  while(n--){ ::new ((void*)ptr) TYPE; ptr++; }
  }


// Destruct some elements at a location
template<class TYPE>
inline void destructElms(TYPE* ptr,unsigned int n){
  while(n--){ ptr->~TYPE(); ptr++; }
  }


// Copy some elements from one place to another
template<class TYPE>
inline void copyElms(TYPE* dst,const TYPE* src,unsigned int n){
  while(n--){ *dst++ = *src++; }
  }


// Move some elements from overlapping place to another
template<class TYPE>
inline void moveElms(TYPE* dst,const TYPE* src,unsigned int n){
  if(src>dst){ 
    while(n--){ *dst++ = *src++; } 
    }
  else if(dst>src){  
    dst+=n; 
    src+=n; 
    while(n--){ *--dst = *--src; } 
    }
  }


// Save some elements to persistent store
template<class TYPE>
inline void saveElms(FXStream& store,TYPE* ptr,unsigned int n){
  while(n--){ store << *ptr; ptr++; }
  }


// Load some elements from persistent store
template<class TYPE>
inline void loadElms(FXStream& store,TYPE* ptr,unsigned int n){
  while(n--){ store >> *ptr; ptr++; }
  }


// Allocate array of elements, uninitialized
template<class TYPE>
inline void allocElms(TYPE*& ptr,unsigned int n){
  fxmalloc((void**)&ptr,sizeof(TYPE)*n);
  }


// Allocate array of elements, initialized with zero
template<class TYPE>
inline void callocElms(TYPE*& ptr,unsigned int n){
  fxcalloc((void**)&ptr,sizeof(TYPE)*n);
  }


// Resize array of elements, without constructor or destructor
template<class TYPE>
inline void resizeElms(TYPE*& ptr,unsigned int n){
  fxresize((void**)&ptr,sizeof(TYPE)*n);
  }


// Free array of elements, without destruction
template<class TYPE>
inline void freeElms(TYPE*& ptr){ 
  fxfree((void**)&ptr);
  }


/**********************  I m p l e m e n t a t i o n  ************************/

// Specific implementations for built-in types


// No-op constructors for array of basic types
inline void constructElms(FXuchar*,unsigned int){ }
inline void constructElms(FXchar*,unsigned int){ }
inline void constructElms(FXushort*,unsigned int){ }
inline void constructElms(FXshort*,unsigned int){ }
inline void constructElms(FXuint*,unsigned int){ }
inline void constructElms(FXint*,unsigned int){ }
inline void constructElms(FXfloat*,unsigned int){ }
inline void constructElms(FXdouble*,unsigned int){ }
inline void constructElms(void**,unsigned int){ }

// No-op destructors for array of basic types
inline void destructElms(FXuchar*,unsigned int){ }
inline void destructElms(FXchar*,unsigned int){ }
inline void destructElms(FXushort*,unsigned int){ }
inline void destructElms(FXshort*,unsigned int){ }
inline void destructElms(FXuint*,unsigned int){ }
inline void destructElms(FXint*,unsigned int){ }
inline void destructElms(FXfloat*,unsigned int){ }
inline void destructElms(FXdouble*,unsigned int){ }
inline void destructElms(void**,unsigned int){ }

// Simple bit-wise copy for basic types
inline void copyElms(FXuchar* dst,const FXuchar* src,unsigned int n){ memcpy(dst,src,n); }
inline void copyElms(FXchar* dst,const FXchar* src,unsigned int n){ memcpy(dst,src,n); }
inline void copyElms(FXushort* dst,const FXushort* src,unsigned int n){ memcpy(dst,src,n<<1); }
inline void copyElms(FXshort* dst,const FXshort* src,unsigned int n){ memcpy(dst,src,n<<1); }
inline void copyElms(FXuint* dst,const FXuint* src,unsigned int n){ memcpy(dst,src,n<<2); }
inline void copyElms(FXint* dst,const FXint* src,unsigned int n){ memcpy(dst,src,n<<2); }
inline void copyElms(FXfloat* dst,const FXfloat* src,unsigned int n){ memcpy(dst,src,n<<2); }
inline void copyElms(FXdouble* dst,const FXdouble* src,unsigned int n){ memcpy(dst,src,n<<3); }
inline void copyElms(void** dst,const void** src,unsigned int n){ memcpy(dst,src,n<<2); }

// Simple bit-wise move for basic types
inline void moveElms(FXuchar* dst,const FXuchar* src,unsigned int n){ memmove(dst,src,n); }
inline void moveElms(FXchar* dst,const FXchar* src,unsigned int n){ memmove(dst,src,n); }
inline void moveElms(FXushort* dst,const FXushort* src,unsigned int n){ memmove(dst,src,n<<1); }
inline void moveElms(FXshort* dst,const FXshort* src,unsigned int n){ memmove(dst,src,n<<1); }
inline void moveElms(FXuint* dst,const FXuint* src,unsigned int n){ memmove(dst,src,n<<2); }
inline void moveElms(FXint* dst,const FXint* src,unsigned int n){ memmove(dst,src,n<<2); }
inline void moveElms(FXfloat* dst,const FXfloat* src,unsigned int n){ memmove(dst,src,n<<2); }
inline void moveElms(FXdouble* dst,const FXdouble* src,unsigned int n){ memmove(dst,src,n<<3); }
inline void moveElms(void** dst,const void** src,unsigned int n){ memmove(dst,src,n<<2); }

// Type-safe save for basic types
inline void saveElms(FXStream& store,FXuchar* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXchar* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXushort* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXshort* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXuint* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXint* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXfloat* ptr,unsigned int n){ store.save(ptr,n); }
inline void saveElms(FXStream& store,FXdouble* ptr,unsigned int n){ store.save(ptr,n); }

// Type-safe load for basic types
inline void loadElms(FXStream& store,FXuchar* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXchar* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXushort* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXshort* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXuint* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXint* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXfloat* ptr,unsigned int n){ store.load(ptr,n); }
inline void loadElms(FXStream& store,FXdouble* ptr,unsigned int n){ store.load(ptr,n); }


#endif
