/********************************************************************************
*                                                                               *
*              P e r s i s t e n t   S t o r a g e   S t r e a m                *
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
* $Id: FXStream.cpp,v 1.17 2002/01/18 22:43:04 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"


/*
  Notes:
  - Defer malloc in FXStream::open till object is actually being saved/loaded!
  - Need some sort of seek() or position() API's.
  - Programming errors are punished by calling fxerror(); for example, saving
    into a stream which is set for loading, NULL buffer pointers, and so on.
  - Status codes are set when a correct program encounters errors such as
    writing to full disk, running out of memory, and so on.
  - Single character insert/extract operators are virtual so subclasses can
    overload these specific cases for greater speed.
  - Need to redesign a bit some day for greater i/o speed (buffering).
*/


/*****************************  Definitions  ***********************************/

#define MAXCLASSNAME       256          // Maximum class name length
#define DEF_HASH_SIZE      32           // Initial table size (MUST be power of 2)
#define MAX_LOAD           80           // Maximum hash table load factor (%)
#define FUDGE              5            // Fudge for hash table size
#define UNUSEDSLOT         0xffffffff   // Unsused slot marker
#define CLASSIDFLAG        0x80000000   // Marks it as a class ID

#define HASH1(x,n) (((unsigned int)(long)(x)*13)%(n))           // Number [0..n-1]
#define HASH2(x,n) (1|(((unsigned int)(long)(x)*17)%((n)-1)))   // Number [1..n-1]



/***************************  Stream Implementation  ****************************/


// Create PersistentStore object
FXStream::FXStream(const FXObject *cont){
  table=NULL;
  ntable=0;
  ninit=0;
  no=0;
  swap=FALSE;
  parent=cont;
  dir=FXStreamDead;
  code=FXStreamOK;
  pos=0;
  }


// Destroy PersistentStore object
FXStream::~FXStream(){
  FXFREE(&table);
  table=(FXStreamHashEntry*)-1;
  }


// Default implementation is a null-stream
void FXStream::saveItems(const void *,FXuint){ }


// Default implementation is a null-stream
void FXStream::loadItems(void*,FXuint){ }


// Set status code, unless there was a previous code already
void FXStream::setError(FXStreamStatus err){
  if(code==FXStreamOK) code=err;
  }


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
  pos+=(unsigned long)(n<<1);
  return *this;
  }

FXStream& FXStream::save(const FXshort* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<1);
  pos+=(unsigned long)(n<<1);
  return *this;
  }

FXStream& FXStream::save(const FXuint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<2);
  pos+=(unsigned long)(n<<2);
  return *this;
  }

FXStream& FXStream::save(const FXint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<2);
  pos+=(unsigned long)(n<<2);
  return *this;
  }

FXStream& FXStream::save(const FXfloat* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<2);
  pos+=(unsigned long)(n<<2);
  return *this;
  }

FXStream& FXStream::save(const FXdouble* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<3);
  pos+=(unsigned long)(n<<3);
  return *this;
  }

#ifdef FX_LONG
FXStream& FXStream::save(const FXulong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<3);
  pos+=(unsigned long)(n<<3);
  return *this;
  }

FXStream& FXStream::save(const FXlong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  saveItems(p,n<<3);
  pos+=(unsigned long)(n<<3);
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
  pos+=(unsigned long)(n<<1);
  if(swap&&n){do{swap2(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXuint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<2);
  pos+=(unsigned long)(n<<2);
  if(swap&&n){do{swap4(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXint* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<2);
  pos+=(unsigned long)(n<<2);
  if(swap&&n){do{swap4(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXfloat* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<2);
  pos+=(unsigned long)(n<<2);
  if(swap&&n){do{swap4(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXdouble* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<3);
  pos+=(unsigned long)(n<<3);
  if(swap&&n){do{swap8(p++);}while(--n);}
  return *this;
  }

#ifdef FX_LONG
FXStream& FXStream::load(FXulong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<3);
  pos+=(unsigned long)(n<<3);
  if(swap&&n){do{swap8(p++);}while(--n);}
  return *this;
  }

FXStream& FXStream::load(FXlong* p,FXuint n){
  FXASSERT(n==0 || (n>0 && p!=NULL));
  loadItems(p,n<<3);
  pos+=(unsigned long)(n<<3);
  if(swap&&n){do{swap8(p++);}while(--n);}
  return *this;
  }
#endif


/*******************************************************************************/


// Open for save or load
FXbool FXStream::open(FXStreamDirection save_or_load){
  unsigned int i,p;
  if(dir!=FXStreamDead){ fxerror("FXStream::open: stream is already open.\n"); }
  if(ninit<DEF_HASH_SIZE) ninit=DEF_HASH_SIZE;
  FXMALLOC(&table,FXStreamHashEntry,ninit);
  if(!table){code=FXStreamAlloc;return FALSE;}
  ntable=ninit;
  for(i=0; i<ntable; i++){table[i].ref=UNUSEDSLOT;}
  no=0;
  dir=save_or_load;
  pos=0;
  if(parent){
    if(dir==FXStreamSave){
      p=HASH1(parent,ntable);
      FXASSERT(p<ntable);
      table[p].obj=(FXObject*)parent;
      table[p].ref=no;
      }
    else{
      table[no].obj=(FXObject*)parent;
      table[no].ref=no;
      }
    no++;
    }
  code=FXStreamOK;
  return TRUE;
  }


// Close store; return TRUE if no errors have been encountered
FXbool FXStream::close(){
  if(dir==FXStreamDead){ fxerror("FXStream::close: stream is not open.\n"); }
  ninit=ntable;
  while((ninit>>1)>no) ninit>>=1;
  dir=FXStreamDead;
  FXFREE(&table);
  ntable=0;
  no=0;
  return code==FXStreamOK;
  }



// Grow hash table
void FXStream::grow(){
  register FXuint i,n,p,x;
  FXStreamHashEntry *h;
  n=ntable<<1;
  FXMALLOC(&h,FXStreamHashEntry,n);
  if(!h){code=FXStreamAlloc;return;}
  switch(dir){

    // Rehash table when FXStreamSave
    case FXStreamSave:
      for(i=0; i<n; i++) h[i].ref=UNUSEDSLOT;
      for(i=0; i<ntable; i++){
        if(table[i].ref==UNUSEDSLOT) continue;
        p = HASH1(table[i].obj,n);
        FXASSERT(p<n);
        x = HASH2(table[i].obj,n);
        FXASSERT(1<=x && x<n);
        while(h[p].ref!=UNUSEDSLOT) p=(p+x)%n;
        h[p].ref=table[i].ref;
        h[p].obj=table[i].obj;
        }
      break;

    // Simply copy over when FXStreamLoad
    case FXStreamLoad:
      for(i=0; i<ntable; i++){
        h[i].ref=table[i].ref;
        h[i].obj=table[i].obj;
        }
      break;

    // Nothing for FXStreamDead
    case FXStreamDead:
      break;
      }

  // Ditch old table
  FXFREE(&table);
  table=h;
  ntable=n;
  }


// Save object
FXStream& FXStream::saveObject(const FXObject* v){
  const FXMetaClass *cls;
  register FXuint p,x;
  FXuint tag,esc=0;
  if(dir!=FXStreamSave){ fxerror("FXStream::saveObject: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    if(v==NULL){
      tag=0;
      *this << tag;
      return *this;
      }
    p=HASH1(v,ntable);
    FXASSERT(p<ntable);
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
    if(tag>MAXCLASSNAME){
      code=FXStreamFormat;                    // Class name too long
      return *this;
      }
    *this << tag;
    *this << esc;                             // Escape code for future expension; must be 0 for now
    save(cls->className,cls->namelen);
    FXTRACE((100,"saveObject(%s)\n",v->getClassName()));
    v->save(*this);
    }
  return *this;
  }


// Load object
FXStream& FXStream::loadObject(FXObject*& v){
  const FXMetaClass *cls;
  FXchar obnam[MAXCLASSNAME];
  FXuint tag,esc;
  if(dir!=FXStreamLoad){ fxerror("FXStream::loadObject: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    *this >> tag;
    if(tag==0){
      v=NULL;
      return *this;
      }
    if(tag&0x80000000){
      tag&=0x7fffffff;
      if(tag>=no){                            // Out-of-range reference number
        code=FXStreamFormat;                  // Bad format in stream
        return *this;
        }
      if(table[tag].ref!=tag){                // We should have constructed the object already!
        code=FXStreamFormat;                  // Bad format in stream
        return *this;
        }
      FXASSERT(tag<ntable);
      v=table[tag].obj;
      FXASSERT(v);
      return *this;
      }
    if(tag>MAXCLASSNAME){                     // Out-of-range class name string
      code=FXStreamFormat;                    // Bad format in stream
      return *this;
      }
    *this >> esc;                             // Read but ignore escape code
    load(obnam,tag);
    cls=FXMetaClass::getMetaClassFromName(obnam);
    if(cls==NULL){                            // No FXMetaClass with this class name
      code=FXStreamUnknown;                   // Unknown class
      return *this;
      }
    v=cls->makeInstance();                    // Build some object!!
    if(v==NULL){
      code=FXStreamAlloc;                     // Unable to construct object
      return *this;
      }
    FXASSERT(no<ntable);
    table[no].obj=v;                          // Save pointer in table
    table[no].ref=no;                         // Save reference number also!
    no++;
    if(no>=ntable) grow();
    FXTRACE((100,"loadObject(%s)\n",v->getClassName()));
    v->load(*this);
    }
  return *this;
  }


// Move to position
FXbool FXStream::position(unsigned long p){
  if(dir==FXStreamDead){fxerror("FXStream::position: stream is not open.\n");}
  if(code==FXStreamOK){
    pos=p;
    return TRUE;
    }
  return FALSE;
  }


/*************************  File Store Implementation  *************************/


// Initialize file stream
FXFileStream::FXFileStream(const FXObject* cont):FXStream(cont){
  file=NULL;
  }


// Save to a file
void FXFileStream::saveItems(const void *buf,FXuint n){
  if(dir!=FXStreamSave){ fxerror("FXFileStream::saveItems: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    if(fwrite(buf,1,n,(FILE*)file)!=n){code=FXStreamFull;}
    }
  }


// Load from file
void FXFileStream::loadItems(void *buf,FXuint n){
  if(dir!=FXStreamLoad){ fxerror("FXFileStream::loadItems: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    if(fread(buf,1,n,(FILE*)file)!=n){code=FXStreamEnd;}
    }
  }


// Try open file stream
FXbool FXFileStream::open(const FXString& filename,FXStreamDirection save_or_load){

  // Stream should not yet be open
  if(dir!=FXStreamDead){ fxerror("FXFileStream::open: stream is already open.\n"); }

  // Open for read
  if(save_or_load==FXStreamLoad){
    file=fopen(filename.text(),"rb");
    if(file==NULL){code=FXStreamNoRead;return FALSE;}
    }

  // Open for write
  else{
    file=fopen(filename.text(),"wb");
    if(file==NULL){code=FXStreamNoWrite;return FALSE;}
    }

  // Do the generic book-keeping
  return FXStream::open(save_or_load);
  }


// Close file stream
FXbool FXFileStream::close(){
  if(file){fclose((FILE*)file);file=NULL;}
  return FXStream::close();
  }


// Some systems don't have it
#ifndef SEEK_SET
#define SEEK_SET 0
#endif


// Move to position
FXbool FXFileStream::position(unsigned long p){
  if(dir==FXStreamDead){ fxerror("FXFileStream::position: stream is not open.\n"); }
  if(code==FXStreamOK){

    // Seek file from beginning
    if(dir==FXStreamSave){
      if(fseek((FILE*)file,p,SEEK_SET)!=0){ code=FXStreamFull; return FALSE; }
      }
    else{
      if(fseek((FILE*)file,p,SEEK_SET)!=0){ code=FXStreamEnd; return FALSE; }
      }

    // Read back to make sure we're in sync
    pos=ftell((FILE*)file);
    return TRUE;
    }
  return FALSE;
  }


FXStream& FXFileStream::operator<<(const FXuchar& v){
  if(code==FXStreamOK){
    register FXint c=putc(v,(FILE*)file);
    if(c==EOF){code=FXStreamFull;}
    }
  pos+=1;
  return *this;
  }


FXStream& FXFileStream::operator<<(const FXchar& v){
  if(code==FXStreamOK){
    register FXint c=putc(v,(FILE*)file);
    if(c==EOF){code=FXStreamFull;}
    }
  pos+=1;
  return *this;
  }


FXStream& FXFileStream::operator>>(FXuchar& v){
  if(code==FXStreamOK){
    register FXint c=getc((FILE*)file);
    if(c==EOF){code=FXStreamEnd;}
    v=c;
    }
  pos+=1;
  return *this;
  }


FXStream& FXFileStream::operator>>(FXchar& v){
  if(code==FXStreamOK){
    register FXint c=getc((FILE*)file);
    if(c==EOF){code=FXStreamEnd;}
    v=c;
    }
  pos+=1;
  return *this;
  }


// Close file stream
FXFileStream::~FXFileStream(){
  if(file){fclose((FILE*)file);}
  file=(void*)-1;
  }


/***********************  Memory Store Implementation  *************************/


// Initialize memory stream
FXMemoryStream::FXMemoryStream(const FXObject* cont):FXStream(cont){
  ptr=NULL;
  space=0;
  owns=FALSE;
  }


// Set available space
void FXMemoryStream::setSpace(FXuint sp){
  if(!owns){ fxerror("FXMemoryStream::setSpace: cannot resize non-owned data buffer.\n"); }
  if(sp!=space){
    if(!FXRESIZE(&ptr,FXuchar,sp)){code=FXStreamAlloc;return;}
    space=sp;
    }
  }


// Save stuff into the data buffer
void FXMemoryStream::saveItems(const void *buf,FXuint n){
  if(dir!=FXStreamSave){fxerror("FXMemoryStream::saveItems: wrong stream direction.\n");}
  if(code==FXStreamOK){
    if(n){
      if(pos+n>space){
        if(!owns){code=FXStreamFull;return;}
        setSpace(pos+n);
        if(pos+n>space) return;
        }
      FXASSERT(pos+n<=space);
      memcpy(&ptr[pos],buf,n);
      }
    }
  }


// Load stuff from data buffer
void FXMemoryStream::loadItems(void *buf,FXuint n){
  if(dir!=FXStreamLoad){ fxerror("FXMemoryStream::loadItems: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    if(n){
      if(pos+n>space){code=FXStreamEnd;return;}
      FXASSERT(pos+n<=space);
      memcpy(buf,&ptr[pos],n);
      }
    }
  }

FXStream& FXMemoryStream::operator<<(const FXuchar& v){
  if(code==FXStreamOK){
    if(pos>=space){
      if(!owns){code=FXStreamFull;return *this;}
      setSpace(pos+1);
      if(pos>=space) return *this;
      }
    ptr[pos]=v;
    }
  pos+=1;
  return *this;
  }


FXStream& FXMemoryStream::operator<<(const FXchar& v){
  if(code==FXStreamOK){
    if(pos>=space){
      if(!owns){code=FXStreamFull;return *this;}
      setSpace(pos+1);
      if(pos>=space) return *this;
      }
    ptr[pos]=(FXuchar)v;
    }
  pos+=1;
  return *this;
  }


FXStream& FXMemoryStream::operator>>(FXuchar& v){
  if(code==FXStreamOK){
    if(pos>=space){code=FXStreamEnd;return *this;}
    v=ptr[pos];
    }
  pos+=1;
  return *this;
  }


FXStream& FXMemoryStream::operator>>(FXchar& v){
  if(code==FXStreamOK){
    if(pos>=space){code=FXStreamEnd;return *this;}
    v=(FXchar)ptr[pos];
    }
  pos+=1;
  return *this;
  }


// Open a stream, possibly with an initial data array
FXbool FXMemoryStream::open(FXuchar* data,FXStreamDirection save_or_load){
  if(data){
    ptr=data;
    space=4294967295U;
    owns=FALSE;
    }
  else{
    FXCALLOC(&ptr,FXuchar,1);
    if(!ptr){code=FXStreamAlloc;return FALSE;}
    space=1;
    owns=TRUE;
    }
  return FXStream::open(save_or_load);
  }


// Open a stream, possibly with initial data array of certain size
FXbool FXMemoryStream::open(FXuchar* data,FXuint sp,FXStreamDirection save_or_load){
  if(data){
    ptr=data;
    space=sp;
    owns=FALSE;
    }
  else{
    if(sp==0) sp=1;
    FXCALLOC(&ptr,FXuchar,sp);
    if(!ptr){code=FXStreamAlloc;return FALSE;}
    space=sp;
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
  if(owns){FXFREE(&ptr);}
  ptr=buffer;
  space=sp;
  owns=TRUE;
  }


// Close the stream
FXbool FXMemoryStream::close(){
  if(owns){FXFREE(&ptr);}
  ptr=NULL;
  space=0;
  owns=FALSE;
  return FXStream::close();
  }


// Move to position; if saving and we own the buffer, try to resize
// and 0-fill the space; if loading and not out of range, move the pointer;
// otherwise, return error code.
FXbool FXMemoryStream::position(unsigned long p){
  if(dir==FXStreamDead){ fxerror("FXMemoryStream::position: stream is not open.\n"); }
  if(code==FXStreamOK){
    if(dir==FXStreamSave){
      if(p>space){
        if(!owns){ code=FXStreamFull; return FALSE; }
        setSpace(p);
        if(p>space) return FALSE;
        }
      }
    else{
      if(p>space){ code=FXStreamEnd; return FALSE; }
      }
    pos=p;
    return TRUE;
    }
  return FALSE;
  }


// Cleanup of memory stream
FXMemoryStream::~FXMemoryStream(){
  if(owns){FXFREE(&ptr);}
  ptr=(FXuchar*)-1;
  }


