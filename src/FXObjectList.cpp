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
* $Id: FXObjectList.cpp,v 1.8 1998/10/29 05:38:14 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXStream.h"
#include "FXObjectList.h"


#define GROW 32



FXIMPLEMENT(FXObjectList,FXObject,NULL,0)


// Create as empty
FXObjectList::FXObjectList(){
  data=NULL;
  num=0;
  max=0;
  }


// Free up nicely
FXObjectList::~FXObjectList(){
  FXFREE(&data);
  }


// Set number of elements
void FXObjectList::no(FXuint n){
  if(n>max) size(n);
  if(num<n) memset(&data[num],0,sizeof(FXObject*)*(n-num));
  num=n;
  }


// Set new size
void FXObjectList::size(FXuint n){
  FXRESIZE(&data,FXObject*,n);
  if(num>n) num=n;
  max=n;
  }


// Insert an object
void FXObjectList::insert(FXuint pos,FXObject* p){
  register FXuint i;
  FXASSERT(pos<=num);
  if(num==max) size(num+GROW);
  for(i=num; pos<i; i--) data[i]=data[i-1];
  data[pos]=p;
  num++;
  }


// Append an object
void FXObjectList::append(FXObject* p){
  if(num==max) size(num+GROW);
  data[num]=p;
  num++;
  }


// Remove object at pos
void FXObjectList::remove(FXuint pos){
  register FXuint i;
  FXASSERT(pos<num);
  for(i=pos+1; i<num; i++) data[i-1]=data[i];
  num--;
  }


// Remove element p
void FXObjectList::remove(const FXObject* p){
  register FXuint i;
  for(i=0; i<num; i++){ 
    if(data[i]==p){ 
      while(i<num){ data[i]=data[i+1]; i++; }
      num--;
      return;
      }
    }
  }
    

// Clear the list
void FXObjectList::clear(){
  FXFREE(&data);
  num=0;
  max=0;
  }


// Find object in list
int FXObjectList::find(FXuint& pos,const FXObject* p){
  register FXuint i;
  for(i=0; i<num; i++){ if(data[i]==p){ pos=i; return 1; } }
  return 0;
  }


// Save to stream; children may be NULL
void FXObjectList::save(FXStream& store) const {
  store << num;;
  for(FXuint i=0; i<num; i++){ 
    store << data[i]; 
    }
  }


// Load from stream; children may be NULL
void FXObjectList::load(FXStream& store){
  store >> num;
  size(num);
  for(FXuint i=0; i<num; i++){ 
    store >> data[i]; 
    }
  }


