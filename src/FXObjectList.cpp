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
* $Id: FXObjectList.cpp,v 1.16 2002/01/18 22:43:01 jeroen Exp $                 *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXStream.h"
#include "FXObjectList.h"


#define GROW 32



// Default constructor
FXObjectList::FXObjectList(){
  data=NULL;
  num=0;
  max=0;
  }


// Copy constructor
FXObjectList::FXObjectList(const FXObjectList& orig){
  FXMEMDUP(&data,FXObject*,orig.data,orig.max);
  num=orig.num;
  max=orig.max;
  }


// Assignment operator
FXObjectList& FXObjectList::operator=(const FXObjectList& orig){
  if(this!=&orig){
    FXMEMDUP(&data,FXObject*,orig.data,orig.max);
    num=orig.num;
    max=orig.max;
    }
  return *this;
  }


// Set number of elements
void FXObjectList::no(FXint n){
  if(n>max) size(n);
  if(num<n) memset(&data[num],0,sizeof(FXObject*)*(n-num));
  num=n;
  }


// Set new size
void FXObjectList::size(FXint n){
  FXRESIZE(&data,FXObject*,n);
  if(num>n) num=n;
  max=n;
  }


// Insert an object
void FXObjectList::insert(FXint pos,FXObject* p){
  register FXint i;
  FXASSERT(pos<=num);
  if(num==max) size(num+GROW);
  for(i=num; pos<i; i--) data[i]=data[i-1];
  data[pos]=p;
  num++;
  }


// Prepend an object
void FXObjectList::prepend(FXObject* p){
  register FXint i;
  if(num==max) size(num+GROW);
  for(i=num; 0<i; i--) data[i]=data[i-1];
  data[0]=p;
  num++;
  }


// Append an object
void FXObjectList::append(FXObject* p){
  if(num==max) size(num+GROW);
  data[num]=p;
  num++;
  }


// Remove object at pos
void FXObjectList::remove(FXint pos){
  register FXint i;
  FXASSERT(pos<num);
  for(i=pos+1; i<num; i++) data[i-1]=data[i];
  num--;
  }


// Replace element
void FXObjectList::replace(FXint pos,FXObject* p){
  FXASSERT(pos<num);
  data[pos]=p;
  }


// Remove element p
void FXObjectList::remove(const FXObject* p){
  register FXint i;
  for(i=0; i<num; i++){
    if(data[i]==p){
      num--;
      while(i<num){ data[i]=data[i+1]; i++; }
      return;
      }
    }
  }


// Find object in list, searching forward; return position or -1
FXint FXObjectList::findf(const FXObject* obj,FXint pos) const {
  register const FXObject *o=obj;
  register FXint p=pos;
  if(p<0) return -1;
  while(p<num){ if(data[p]==o){ return p; } p++; }
  return -1;
  }


// Find object in list, searching backward; return position or -1
FXint FXObjectList::findb(const FXObject* obj,FXint pos) const {
  register const FXObject *o=obj;
  register FXint p=pos;
  if(p>=num) p=num-1;
  while(0<=p){ if(data[p]==o){ return p; } p--; }
  return -1;
  }


// Clear the list
void FXObjectList::clear(){
  FXFREE(&data);
  num=0;
  max=0;
  }


// Save to stream; children may be NULL
void FXObjectList::save(FXStream& store) const {
  store << num;
  for(FXint i=0; i<num; i++){
    store << data[i];
    }
  }


// Load from stream; children may be NULL
void FXObjectList::load(FXStream& store){
  store >> num;
  size(num);
  for(FXint i=0; i<num; i++){
    store >> data[i];
    }
  }


// Free up nicely
FXObjectList::~FXObjectList(){
  FXFREE(&data);
  }


