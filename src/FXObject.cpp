/********************************************************************************
*                                                                               *
*                         T o p l e v el   O b j e c t                          *
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
* $Id: FXObject.cpp,v 1.12.4.1 2003/08/14 03:44:46 fox Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"


/*
  Notes:

  - We have to use extern "C" for the metaClass names, as in some compilers,
    e.g. MS VC++, mangled C++ names are no longer legal identifiers in C or C++.
  - Perhaps hash metaclasses into table at run-time, by giving FXMetaClass a constructor...
    [less efficient, but probably also less error-prone...].
  - We need a table of all metaclasses, as we should be able to create any type of
    object during deserialization...
*/

/*******************************************************************************/


// Allocate memory
FXint fxmalloc(void** ptr,unsigned long size){
  *ptr=NULL;
  if(size!=0){
    if((*ptr=malloc(size))==NULL) return FALSE;
    }
  return TRUE;
  }


// Allocate cleaned memory
FXint fxcalloc(void** ptr,unsigned long size){
  *ptr=NULL;
  if(size!=0){
    if((*ptr=calloc(size,1))==NULL) return FALSE;
    }
  return TRUE;
  }


// Resize memory
FXint fxresize(void** ptr,unsigned long size){
  register void *p=NULL;
  if(size!=0){
    if((p=realloc(*ptr,size))==NULL) return FALSE;
    }
  else{
    if(*ptr) free(*ptr);
    }
  *ptr=p;
  return TRUE;
  }


// Allocate and initialize memory
FXint fxmemdup(void** ptr,unsigned long size,const void* src){
  *ptr=NULL;
  if(size!=0 && src!=NULL){
    if((*ptr=malloc(size))==NULL) return FALSE;
    memcpy(*ptr,src,size);
    }
  return TRUE;
  }


// String duplicate
FXchar *fxstrdup(const FXchar* str){
  register FXchar *copy;
  if(str!=NULL && (copy=(FXchar*)malloc(strlen(str)+1))!=NULL){
    strcpy(copy,str);
    return copy;
    }
  return NULL;
  }


// Free memory, resets ptr to NULL afterward
void fxfree(void** ptr){
  if(*ptr){
    free(*ptr);
    *ptr=NULL;
    }
  }


/*************************  FXMetaClass Implementation  ************************/

// Hash table of metaclasses
static const FXMetaClass** metaClassTable=NULL;
static FXuint              nmetaClassTable=0;
static FXuint              nmetaClasses=0;


// Compute hash string
static FXuint hashstring(const FXchar *str){
  register FXuint h=0;
  register FXuint g;
  while(*str) {
    h=(h<<4)+*str++;
    g=h&0xF0000000UL;
    if(g) h^=g>>24;
    h&=~g;
    }
  return h;
  }


#define HASH1(x,n) (((unsigned int)(x)*13)%(n))           // Number [0..n-1]
#define HASH2(x,n) (1|(((unsigned int)(x)*17)%((n)-1)))   // Number [1..n-1]


// Metaclass initializer
__FXMETACLASSINITIALIZER__::__FXMETACLASSINITIALIZER__(const FXMetaClass* meta){
  const FXMetaClass** newtable;
  register FXuint h=hashstring(meta->className);
  register FXuint p,x,i,n;
//fprintf(stderr,"hash for %s = %d\n",meta->className,h);

  // First time?
  if(nmetaClassTable==0){
    nmetaClassTable=8;
    FXCALLOC(&metaClassTable,FXMetaClass*,nmetaClassTable);
    }

  // Find hash slot
  p=HASH1(h,nmetaClassTable);
  x=HASH2(h,nmetaClassTable);
  while(metaClassTable[p]){
    FXASSERT(metaClassTable[p]!=meta);
    p=(p+x)%nmetaClassTable;
    }

  // Place in table
  metaClassTable[p]=meta;
  nmetaClasses++;

  // Grow table if needed
  if((100*nmetaClasses)>=(80*nmetaClassTable)){
    n=nmetaClassTable*2;
//fprintf(stderr,"growing table from %d to %d (%d entries)\n",nmetaClassTable,n,nmetaClasses);
    FXCALLOC(&newtable,FXMetaClass*,n);
    for(i=0; i<nmetaClassTable; i++){
      if(metaClassTable[i]){
        h=hashstring(metaClassTable[i]->className);
        p=HASH1(h,n);
        x=HASH2(h,n);
        while(newtable[p]){
          p=(p+x)%n;
          }
        newtable[p]=metaClassTable[i];
        }
      }
    FXFREE(&metaClassTable);
    metaClassTable=newtable;
    nmetaClassTable=n;
    }
  }


// Find the FXMetaClass belonging to class name
const FXMetaClass* FXMetaClass::getMetaClassFromName(const FXchar* name){
  register FXuint h=hashstring(name);
  register FXuint p,x;
  p=HASH1(h,nmetaClassTable);
  x=HASH2(h,nmetaClassTable);
  while(metaClassTable[p]){
    if(strcmp(metaClassTable[p]->className,name)==0){
      return metaClassTable[p];
      }
    p=(p+x)%nmetaClassTable;
    }
  return NULL;
  }


// Test if subclass
FXbool FXMetaClass::isSubClassOf(const FXMetaClass* metaclass) const {
  register const FXMetaClass* cls;
  for(cls=this; cls; cls=cls->baseClass){
    if(cls==metaclass) return TRUE;
    }
  return FALSE;
  }


// Create an object instance
FXObject* FXMetaClass::makeInstance() const {
  return (*manufacture)();
  }


// Find function
const void* FXMetaClass::search(FXSelector key) const {
  register const FXObject::FXMapEntry* lst=(const FXObject::FXMapEntry*)assoc;
  register FXuint n=nassocs;
  while(n--){
    if(lst->keylo<=key && key<=lst->keyhi) return lst;
    lst=(const FXObject::FXMapEntry*) (((const FXchar*)lst)+assocsz);
    }
  return NULL;
  }



/***************************  FXObject Implementation  *************************/

// Have to do this one `by hand' as it has no base class
const FXMetaClass FXObject::metaClass={"FXObject",FXObject::manufacture,NULL,NULL,0,0,sizeof("FXObject")};


// Manual initializer
__FXMETACLASSINITIALIZER__ FXObjectInitializer(&FXObject::metaClass);


// Build an object
FXObject* FXObject::manufacture(){return new FXObject;}


// Get class name of object
const FXchar* FXObject::getClassName() const { return getMetaClass()->getClassName(); }


// Check if object belongs to a class
FXbool FXObject::isMemberOf(const FXMetaClass* metaclass) const {
  return getMetaClass()->isSubClassOf(metaclass);
  }


// Save to stream
void FXObject::save(FXStream&) const { }


// Load from stream
void FXObject::load(FXStream&){ }


// Unhandled function
long FXObject::onDefault(FXObject*,FXSelector,void*){ return 0; }


// Handle message
long FXObject::handle(FXObject* sender,FXSelector sel,void* ptr){
  return onDefault(sender,sel,ptr);
  }


// This really messes the object up; note that it is intentional,
// as further references to a destructed object should not happen.
FXObject::~FXObject(){*((void**)this)=(void*)-1L;}
