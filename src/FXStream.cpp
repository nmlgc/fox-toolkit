/********************************************************************************
*                                                                               *
*              P e r s i s t e n t   S t o r a g e   S t r e a m                *
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
* $Id: FXStream.cpp,v 1.24 1998/10/30 02:20:08 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"


/*
  To do:
  - Make GUID's for classes?
  - Build tools for making static hash table GUID->MetaClass mappings
  - defer malloc in FXStream::open till object is actually being saved/loaded!
  - Need byte swap option?
  - Need EOF status flag (and other errors!)
  - Need some sort of seek() or position() API's

    For symmetry you might add these definitions to FXObject.h:

      FXStream& operator<<(FXStream& s, const FXObject& o) {
        o.save(s); return s;
      }

      FXStream& operator>>(FXStream& s, FXObject& o) {
        o.load(s); return s;
      }

    just in case the root object is not a pointer but a regular stack instance.

    Lyle
*/


/*****************************  Definitions  **********************************/

#define DEF_HASH_SIZE      32           // Initial table size (MUST be power of 2)
#define MAX_LOAD           80           // Maximum hash table load factor (%)
#define FUDGE              5            // Fudge for hash table size
#define UNUSEDSLOT         0xffffffff   // Unsused slot marker
#define CLASSIDFLAG        0x80000000   // Marks it as a class ID

#define HASH1(x,n) (((unsigned int)(x)*13)%(n))           // Number [0..n-1]
#define HASH2(x,n) (1|(((unsigned int)(x)*17)%((n)-1)))   // Number [1..n-1]



/***************************  Stream Implementation  ***************************/


// Create PersistentStore object
FXStream::FXStream(const FXObject *cont){
  table = NULL;
  ntable = 0;
  ninit = 0;
  no = 0;
  swap = FALSE;
  dir = FXStreamDead;
  pos = 0;
  parent = cont;
  }


// Destroy PersistentStore object
FXStream::~FXStream(){
  FXFREE(&table);
  }


// Empty default implementations
void FXStream::saveItems(const void *,FXuint){FXASSERT(dir==FXStreamSave);}
void FXStream::loadItems(void *,FXuint){FXASSERT(dir==FXStreamLoad);}


// Swap duplets
static inline void swap2(void *p){
  register FXuchar t;
  t=((FXuchar*)p)[0]; ((FXuchar*)p)[0]=((FXuchar*)p)[1]; ((FXuchar*)p)[1]=t;
  }


// Swap quadruplets
static inline void swap4(void *p){
  register FXuchar t;
  t=((FXuchar*)p)[0]; ((FXuchar*)p)[0]=((FXuchar*)p)[3]; ((FXuchar*)p)[3]=t;
  t=((FXuchar*)p)[1]; ((FXuchar*)p)[1]=((FXuchar*)p)[2]; ((FXuchar*)p)[2]=t;
  }
  

// Swap octuplets
static inline void swap8(void *p){
  register FXuchar t;
  t=((FXuchar*)p)[0]; ((FXuchar*)p)[0]=((FXuchar*)p)[7]; ((FXuchar*)p)[7]=t;
  t=((FXuchar*)p)[1]; ((FXuchar*)p)[1]=((FXuchar*)p)[6]; ((FXuchar*)p)[6]=t;
  t=((FXuchar*)p)[2]; ((FXuchar*)p)[2]=((FXuchar*)p)[5]; ((FXuchar*)p)[5]=t;
  t=((FXuchar*)p)[3]; ((FXuchar*)p)[3]=((FXuchar*)p)[4]; ((FXuchar*)p)[4]=t;
  }


/******************************  Save Basic Types  *****************************/

FXStream& FXStream::operator<<(const FXuchar& v){
  saveItems(&v,1);
  pos+=1;
  return *this;
  }

FXStream& FXStream::operator<<(const FXchar& v){
  saveItems(&v,1);
  pos+=1;
  return *this;
  }

FXStream& FXStream::operator<<(const FXushort& v){
  saveItems(&v,2);
  pos+=2;
  return *this;
  }

FXStream& FXStream::operator<<(const FXshort& v){
  saveItems(&v,2);
  pos+=2;
  return *this;
  }

FXStream& FXStream::operator<<(const FXuint& v){
  saveItems(&v,4);
  pos+=4;
  return *this;
  }

FXStream& FXStream::operator<<(const FXint& v){
  saveItems(&v,4);
  pos+=4;
  return *this;
  }

FXStream& FXStream::operator<<(const FXfloat& v){
  saveItems(&v,4);
  pos+=4;
  return *this;
  }

FXStream& FXStream::operator<<(const FXdouble& v){
  saveItems(&v,8);
  pos+=8;
  return *this;
  }

#ifdef FX_LONG
FXStream& FXStream::operator<<(const FXulong& v){
  saveItems(&v,8);
  pos+=8;
  return *this;
  }

FXStream& FXStream::operator<<(const FXlong& v){
  saveItems(&v,8);
  pos+=8;
  return *this;
  }
#endif

/************************  Save Blocks of Basic Types  *************************/

FXStream& FXStream::save(const FXuchar* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n);
  pos+=n;
  return *this;
  }

FXStream& FXStream::save(const FXchar* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n);
  pos+=n;
  return *this;
  }

FXStream& FXStream::save(const FXushort* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<1);
  pos+=(n<<1);
  return *this;
  }

FXStream& FXStream::save(const FXshort* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<1);
  pos+=(n<<1);
  return *this;
  }

FXStream& FXStream::save(const FXuint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<2);
  pos+=(n<<2);
  return *this;
  }

FXStream& FXStream::save(const FXint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<2);
  pos+=(n<<2);
  return *this;
  }

FXStream& FXStream::save(const FXfloat* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<2);
  pos+=(n<<2);
  return *this;
  }

FXStream& FXStream::save(const FXdouble* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<3);
  pos+=(n<<3);
  return *this;
  }

#ifdef FX_LONG
FXStream& FXStream::save(const FXulong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<3);
  pos+=(n<<3);
  return *this;
  }

FXStream& FXStream::save(const FXlong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<3);
  pos+=(n<<3);
  return *this;
  }
#endif


/*****************************  Load Basic Types  ******************************/

FXStream& FXStream::operator>>(FXuchar& v){
  loadItems(&v,1);
  pos+=1;
  return *this;
  }

FXStream& FXStream::operator>>(FXchar& v){
  loadItems(&v,1);
  pos+=1;
  return *this;
  }

FXStream& FXStream::operator>>(FXushort& v){
  loadItems(&v,2);
  pos+=2;
  if(swap){swap2(&v);}
  return *this;
  }

FXStream& FXStream::operator>>(FXshort& v){
  loadItems(&v,2);
  pos+=2;
  if(swap){swap2(&v);}
  return *this;
  }

FXStream& FXStream::operator>>(FXuint& v){
  loadItems(&v,4);
  pos+=4;
  if(swap){swap4(&v);}
  return *this;
  }

FXStream& FXStream::operator>>(FXint& v){
  loadItems(&v,4);
  pos+=4;
  if(swap){swap4(&v);}
  return *this;
  }

FXStream& FXStream::operator>>(FXfloat& v){
  loadItems(&v,4);
  pos+=4;
  if(swap){swap4(&v);}
  return *this;
  }

FXStream& FXStream::operator>>(FXdouble& v){
  loadItems(&v,8);
  pos+=8;
  if(swap){swap8(&v);}
  return *this;
  }

#ifdef FX_LONG
FXStream& FXStream::operator>>(FXulong& v){
  loadItems(&v,8);
  pos+=8;
  if(swap){swap8(&v);}
  return *this;
  }

FXStream& FXStream::operator>>(FXlong& v){
  loadItems(&v,8);
  pos+=8;
  if(swap){swap8(&v);}
  return *this;
  }
#endif


/************************  Load Blocks of Basic Types  *************************/

FXStream& FXStream::load(FXuchar* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n);
  pos+=n;
  return *this;
  }


FXStream& FXStream::load(FXchar* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n);
  pos+=n;
  return *this;
  }

FXStream& FXStream::load(FXushort* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<1);
  pos+=(n<<1);
  if(swap&&n){do{swap2(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXshort* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<1);
  pos+=(n<<1);
  if(swap&&n){do{swap2(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXuint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<2);
  pos+=(n<<2);
  if(swap&&n){do{swap4(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<2);
  pos+=(n<<2);
  if(swap&&n){do{swap4(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXfloat* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<2);
  pos+=(n<<2);
  if(swap&&n){do{swap4(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXdouble* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<3);
  pos+=(n<<3);
  if(swap&&n){do{swap8(p++);}while(--n);}
  return *this;
  }

#ifdef FX_LONG
FXStream& FXStream::load(FXulong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<3);
  pos+=(n<<3);
  if(swap&&n){do{swap8(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXlong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<3);
  pos+=(n<<3);
  if(swap&&n){do{swap8(p++);}while(--n);}
  return *this;
  }
#endif


/*******************************************************************************/


// Open for save or load
int FXStream::open(FXStreamDirection save_or_load){
  unsigned int i,p;
  if(dir!=FXStreamDead){ fxerror("FXStream::open: stream is already open.\n"); }
  FXASSERT(save_or_load==FXStreamSave || save_or_load==FXStreamLoad);
  if(ninit<DEF_HASH_SIZE) ninit=DEF_HASH_SIZE;
  FXMALLOC(&table,FXStreamHashEntry,ninit);
  ntable = ninit;
  for(i=0; i<ntable; i++){table[i].ref=UNUSEDSLOT;}
  no = 0;
  dir = save_or_load;
  pos = 0;
  if(parent){
    if(dir==FXStreamSave){
      p = HASH1(parent,ntable);
      FXASSERT(p<ntable);
      table[p].obj = (FXObject*)parent;
      table[p].ref = no++;
      }
    else{
      table[no].obj = (FXObject*)parent;
      table[no].ref = no++;
      }
    }
  return 1;
  }


// Close store
int FXStream::close(){
  if(dir==FXStreamDead){ fxerror("FXStream::close: stream is not open.\n"); }
  ninit = ntable;
  while((ninit>>1)>no) ninit>>=1;
  dir = FXStreamDead;
  FXFREE(&table);
  ntable = 0;
  no = 0;
  return 1;
  }
  
  

// Grow hash table
void FXStream::grow(){
  unsigned int i,n,p,x;
  FXStreamHashEntry *h;
  n = ntable<<1;
  FXMALLOC(&h,FXStreamHashEntry,n);
  switch(dir){
  
    // Rehash table when FXStreamSave
    case FXStreamSave:	
      for(i=0; i<n; i++) h[i].ref=UNUSEDSLOT;
      for(i=0; i<ntable; i++){
        if(table[i].ref == UNUSEDSLOT) continue;
        p = HASH1(table[i].obj,n);
        FXASSERT(0<=p && p<n);
        x = HASH2(table[i].obj,n);
        FXASSERT(1<=x && x<n);
        while(h[p].ref!=UNUSEDSLOT) p=(p+x)%n;
        h[p].ref = table[i].ref;
        h[p].obj = table[i].obj;
        }
      break;
      
    // Simply copy over when FXStreamLoad
    case FXStreamLoad:	
      for(i=0; i<ntable; i++){
        h[i].ref = table[i].ref;
        h[i].obj = table[i].obj;
        }
      break;
    case FXStreamDead:
      break;
      }
      
  FXFREE(&table);
  table = h;
  ntable = n;
  }
        

// Save object 
FXStream& FXStream::saveObject(const FXObject *const& v){
  const FXMetaClass *cls;
  register FXuint p,x;
  FXuint tag,esc=0;
  if(dir!=FXStreamSave){ fxerror("FXStream::saveObject: wrong stream mode\n"); }
  if(v==NULL){
    tag=0;
    *this << tag;
    return *this;
    }
  p=HASH1(v,ntable);
  FXASSERT(0<=p && p<ntable);
  x=HASH2(v,ntable);
  FXASSERT(1<=x && x<ntable);
  while(table[p].ref!=UNUSEDSLOT){
    if(table[p].obj==v){
      FXASSERT(table[p].ref<=no);
      tag=table[p].ref|0x80000000;
      *this << tag;
      return *this;
      }
    p = (p+x)%ntable;
    }
  table[p].obj=(FXObject*)v;
  table[p].ref=no++;
  FXASSERT(no<ntable);
  if((100*no)>=(MAX_LOAD*ntable)) grow();
  cls=v->getMetaClass();
  tag=cls->namelen;
  *this << tag;
  *this << esc;//Escape code for future expension; must be 0 for now
  save(cls->className,cls->namelen);
  v->save(*this);
  return *this;
  }


// Load object
FXStream& FXStream::loadObject(FXObject*& v){
  const FXMetaClass *cls;
  FXchar obnam[254];
  FXuint tag,esc;
  if(dir!=FXStreamLoad){ fxerror("FXStream::loadObject: wrong stream mode\n"); }
  *this >> tag;
  if(tag==0){
    v=NULL;
    return *this;
    }
  if(tag&0x80000000){
    v=table[tag&0x7fffffff].obj;
    return *this;
    }
  FXASSERT(tag<sizeof(obnam));
  *this >> esc;//Read but ignore escape code
  load(obnam,tag);
  cls=FXMetaClass::getMetaClassFromName(obnam);
  if(!cls){ fxerror("FXStream::loadObject: unknown class: %s\n",obnam); }
  v=cls->makeInstance();
  FXASSERT(no<ntable);
  table[no].obj=v;                          // Save pointer in table
  table[no].ref=no++;
  if((100*no)>=(MAX_LOAD*ntable)) grow();
  v->load(*this);
  return *this;
  }


/*************************  File Store Implementation  *************************/

FXFileStream::FXFileStream(const FXObject* cont):FXStream(cont){
  file=NULL;
  }


void FXFileStream::saveItems(const void *buf,FXuint n){
  FXASSERT(buf);
  FXASSERT(dir==FXStreamSave);
  if(n) fwrite(buf,1,n,(FILE*)file);
  //if(fwrite(buf,1,n,(FILE*)file)!=n){ throw "Unable to write to file"; } 
  }


void FXFileStream::loadItems(void *buf,FXuint n){
  FXASSERT(buf);
  FXASSERT(dir==FXStreamLoad);
  if(n) fread(buf,1,n,(FILE*)file);
  //if(fread(buf,1,n,(FILE*)file)!=n){ throw "Unable to read from file"; } 
  }


int FXFileStream::open(const char* filename,FXStreamDirection save_or_load){
  if(!filename){ fxerror("FXFileStream::open: NULL filename argument.\n"); }

  // Open for read
  if(save_or_load==FXStreamLoad){
    file=fopen(filename,"rb");
    if(file==NULL) return 0;
    }
 
  // Open for write
  else{
    file=fopen(filename,"wb");
    if(file==NULL) return 0;
    }

  // Do the generic book-keeping
  return FXStream::open(save_or_load);
  }
  

int FXFileStream::close(){
  if(file){ fclose((FILE*)file); file=NULL; }
  return FXStream::close();
  }


// Close file stream 
FXFileStream::~FXFileStream(){
  if(file) fclose((FILE*)file);
  if(dir!=FXStreamDead) close();
  }


/***********************  Memory Store Implementation  *************************/

FXMemoryStream::FXMemoryStream(const FXObject* cont):FXStream(cont){
  ptr=NULL;
  space=0;
  owns=FALSE;
  }


// Set available space
void FXMemoryStream::setSpace(FXuint sp){
  if(!owns){ fxerror("FXMemoryStream::setSpace: cannot resize non-owned data buffer.\n"); }
  if(sp!=space){
    if(!FXRESIZE(&ptr,FXuchar,sp)){ fxerror("FXMemoryStream:saveItems: unable to resize buffer.\n"); }
    space=sp;
    }
  }


// Save stuff into the data buffer
void FXMemoryStream::saveItems(const void *buf,FXuint n){
  FXASSERT(buf);
  FXASSERT(dir==FXStreamSave);
  if((pos+n)>=space){
    if(!owns){ fxerror("FXMemoryStream::saveItems: data buffer is full.\n"); }
    setSpace(pos+n);
    }
  if(n) memcpy(&ptr[pos],buf,n);
  }


// Load stuff from data buffer
void FXMemoryStream::loadItems(void *buf,FXuint n){
  FXASSERT(buf);
  FXASSERT(dir==FXStreamLoad);
  if((pos+n)>=space){
    fxerror("FXMemoryStream::loadItems: at end of data buffer.\n"); 
    }
  if(n) memcpy(buf,&ptr[pos],n);
  }


// Open a stream, possibly with an initial data array
int FXMemoryStream::open(FXuchar* data,FXStreamDirection save_or_load){
  space=4294967295U;
  if(data){
    ptr=data;
    owns=FALSE;
    }
  else{
    space=1;
    if(!FXCALLOC(&ptr,FXuchar,space)) return FALSE;
    owns=TRUE;
    }
  return FXStream::open(save_or_load);
  }


// Open a stream, possibly with initial data array of certain size
int FXMemoryStream::open(FXuchar* data,FXuint sp,FXStreamDirection save_or_load){
  space=sp;
  if(data){
    ptr=data;
    owns=FALSE;
    }
  else{
    if(!FXCALLOC(&ptr,FXuchar,space)) return FALSE;
    owns=TRUE;
    }
  return FXStream::open(save_or_load);
  }


// Take buffer away from stream
void FXMemoryStream::takeBuffer(FXuchar*& buffer,FXuint& sp){
  buffer=ptr;
  sp=space;
  ptr=NULL;
  space=0;
  owns=FALSE;
  }


// Give buffer to stream
void FXMemoryStream::giveBuffer(FXuchar *buffer,FXuint sp){
  if(buffer==NULL){ fxerror("FXMemoryStream::giveBuffer: NULL buffer argument.\n"); }
  if(owns) FXFREE(&ptr);
  ptr=buffer;
  space=sp;
  owns=TRUE;
  }
  

// Close the stream
int FXMemoryStream::close(){
  if(owns) FXFREE(&ptr);
  ptr=NULL;
  space=0;
  owns=FALSE;
  return FXStream::close();
  }

