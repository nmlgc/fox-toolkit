/********************************************************************************
*                                                                               *
*       P e r s i s t e n t   S t o r a g e   S t r e a m   C l a s s e s       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2004 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXStream.cpp,v 1.47 2004/04/05 14:49:33 fox Exp $                        *
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
  - Programming errors are punished by calling fxerror(); for example, saving
    into a stream which is set for loading, NULL buffer pointers, and so on.
  - Status codes are set when a correct program encounters errors such as
    writing to full disk, running out of memory, and so on.
  - Single character insert/extract operators are virtual so subclasses can
    overload these specific cases for greater speed.
  - Copy byte at a time because don't know about alignment of stream buffer;
    unaligned accesses are disallowed on some RISC cpus.
  - Need to be able to append instead of just overwrite existing file streams.
  - Buffer can not be written till after first call to writeBuffer().
  - Need to haul some memory stream stuff up (buffer mgmt).
  - Need to have load() and save() API's return number of elements ACTUALLY
    transferred (not bytes but whole numbers of elements!).
*/


#define MAXCLASSNAME       256          // Maximum class name length
#define DEF_HASH_SIZE      32           // Initial table size (MUST be power of 2)
#define MAX_LOAD           80           // Maximum hash table load factor (%)
#define FUDGE              5            // Fudge for hash table size
#define UNUSEDSLOT         0xffffffff   // Unsused slot marker
#define CLASSIDFLAG        0x80000000   // Marks it as a class ID

#define HASH1(x,n) (((FXuint)(FXuval)(x)*13)%(n))         // Number [0..n-1]
#define HASH2(x,n) (1|(((FXuint)(FXuval)(x)*17)%((n)-1))) // Number [1..n-2]


using namespace FX;


/*******************************************************************************/

namespace FX {


// Create PersistentStore object
FXStream::FXStream(const FXObject *cont){
  parent=cont;
  table=NULL;
  begptr=NULL;
  endptr=NULL;
  wrptr=NULL;
  rdptr=NULL;
  ntable=0;
  no=0;
  pos=0;
  dir=FXStreamDead;
  code=FXStreamOK;
  swap=FALSE;
  owns=FALSE;
  }


// Destroy PersistentStore object
FXStream::~FXStream(){
  if(owns){FXFREE(&begptr);}
  FXFREE(&table);
  parent=(const FXObject*)-1L;
  table=(FXStreamHashEntry*)-1L;
  begptr=(FXuchar*)-1L;
  endptr=(FXuchar*)-1L;
  wrptr=(FXuchar*)-1L;
  rdptr=(FXuchar*)-1L;
  }


// Grow hash table
void FXStream::grow(){
  register FXuint i,n,p,x;
  FXStreamHashEntry *h;

  // Double table size
  FXASSERT(ntable);
  n=ntable<<1;

  // Allocate new table
  if(!FXMALLOC(&h,FXStreamHashEntry,n)){ code=FXStreamAlloc; return; }

  // Rehash table when FXStreamSave
  if(dir==FXStreamSave){
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
    }

  // Simply copy over when FXStreamLoad
  else if(dir==FXStreamLoad){
    for(i=0; i<ntable; i++){
      h[i].ref=table[i].ref;
      h[i].obj=table[i].obj;
      }
    }

  // Ditch old table
  FXFREE(&table);

  // Point to new table
  table=h;
  ntable=n;
  }


// Write at least count bytes from the buffer; the default
// implementation simply discards all data in the buffer.
// It returns the amount of room available to be written.
unsigned long FXStream::writeBuffer(unsigned long){
  rdptr=begptr;
  wrptr=begptr;
  return endptr-wrptr;
  }


// Read at least count bytes into the buffer; the default
// implementation reads an endless stream of zero's.
// It returns the amount of data available to be read.
unsigned long FXStream::readBuffer(unsigned long){
  rdptr=begptr;
  wrptr=endptr;
  return wrptr-rdptr;
  }


// Set status code
void FXStream::setError(FXStreamStatus err){
  code=err;
  }


/// Get available space
unsigned long FXStream::getSpace() const {
  return endptr-begptr;
  }


// Set available space
void FXStream::setSpace(unsigned long size){
  if(code==FXStreamOK){

    // Changed size?
    if(begptr+size!=endptr){

      // Old buffer location
      register FXuchar *oldbegptr=begptr;

      // Only resize if owned
      if(!owns){ fxerror("FXStream::setSpace: cannot resize external data buffer.\n"); }

      // Resize the buffer
      if(!FXRESIZE(&begptr,FXuchar,size)){ code=FXStreamAlloc; return; }

      // Adjust pointers, buffer may have moved
      endptr=begptr+size;
      wrptr=begptr+(wrptr-oldbegptr);
      rdptr=begptr+(rdptr-oldbegptr);
      if(wrptr>endptr) wrptr=endptr;
      if(rdptr>endptr) rdptr=endptr;
      }
    }
  }


// Open for save or load
FXbool FXStream::open(FXStreamDirection save_or_load,unsigned long size,FXuchar* data){
  register unsigned int i,p;
  if(save_or_load!=FXStreamSave && save_or_load!=FXStreamLoad){fxerror("FXStream::open: illegal stream direction.\n");}
  if(!dir){

    // Allocate hash table
    if(!FXMALLOC(&table,FXStreamHashEntry,DEF_HASH_SIZE)){ code=FXStreamAlloc; return FALSE; }

    // Initialize table to empty
    for(i=0; i<DEF_HASH_SIZE; i++){ table[i].ref=UNUSEDSLOT; }

    // Use external buffer space
    if(data){
      begptr=data;
      if(size==ULONG_MAX)
        endptr=((FXuchar*)NULL)-1;
      else
        endptr=begptr+size;
      wrptr=begptr;
      rdptr=begptr;
      owns=FALSE;
      }

    // Use internal buffer space
    else{
      if(!FXCALLOC(&begptr,FXuchar,size)){ FXFREE(&table); code=FXStreamAlloc; return FALSE; }
      endptr=begptr+size;
      wrptr=begptr;
      rdptr=begptr;
      owns=TRUE;
      }

    // Set variables
    ntable=DEF_HASH_SIZE;
    dir=save_or_load;
    pos=0;
    no=0;

    // Append container object to hash table
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

    // So far, so good
    code=FXStreamOK;

    return TRUE;
    }
  return FALSE;
  }


// Close store; return TRUE if no errors have been encountered
FXbool FXStream::close(){
  if(dir){
    dir=FXStreamDead;
    if(owns){FXFREE(&begptr);}
    FXFREE(&table);
    begptr=NULL;
    wrptr=NULL;
    rdptr=NULL;
    endptr=NULL;
    ntable=0;
    no=0;
    owns=FALSE;
    return code==FXStreamOK;
    }
  return FALSE;
  }


// Flush buffer
FXbool FXStream::flush(){
  writeBuffer(0);
  return code==FXStreamOK;
  }


// Move to position
FXbool FXStream::position(long offset,FXWhence whence){
  if(dir==FXStreamDead){fxerror("FXStream::position: stream is not open.\n");}
  if(code==FXStreamOK){
    if(whence==FXFromCurrent) offset=offset+pos;
    else if(whence==FXFromEnd) offset=offset+endptr-begptr;
    pos=offset;
    return TRUE;
    }
  return FALSE;
  }


/******************************  Save Basic Types  *****************************/

// Write one byte
FXStream& FXStream::operator<<(const FXuchar& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(wrptr+1>endptr && writeBuffer(1)<1){ code=FXStreamFull; return *this; }
    FXASSERT(wrptr+1<=endptr);
    *wrptr++ = v;
    pos++;
    }
  return *this;
  }


// Write one short
FXStream& FXStream::operator<<(const FXushort& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(wrptr+2>endptr && writeBuffer((wrptr-endptr)+2)<2){ code=FXStreamFull; return *this; }
    FXASSERT(wrptr+2<=endptr);
    if(swap){
      wrptr[0]=((const FXuchar*)&v)[1];
      wrptr[1]=((const FXuchar*)&v)[0];
      }
    else{
      wrptr[0]=((const FXuchar*)&v)[0];
      wrptr[1]=((const FXuchar*)&v)[1];
      }
    wrptr+=2;
    pos+=2;
    }
  return *this;
  }


// Write one int
FXStream& FXStream::operator<<(const FXuint& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(wrptr+4>endptr && writeBuffer((wrptr-endptr)+4)<4){ code=FXStreamFull; return *this; }
    FXASSERT(wrptr+4<=endptr);
    if(swap){
      wrptr[0]=((const FXuchar*)&v)[3];
      wrptr[1]=((const FXuchar*)&v)[2];
      wrptr[2]=((const FXuchar*)&v)[1];
      wrptr[3]=((const FXuchar*)&v)[0];
      }
    else{
      wrptr[0]=((const FXuchar*)&v)[0];
      wrptr[1]=((const FXuchar*)&v)[1];
      wrptr[2]=((const FXuchar*)&v)[2];
      wrptr[3]=((const FXuchar*)&v)[3];
      }
    wrptr+=4;
    pos+=4;
    }
  return *this;
  }


// Write one double
FXStream& FXStream::operator<<(const FXdouble& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(wrptr+8>endptr && writeBuffer((wrptr-endptr)+8)<8){ code=FXStreamFull; return *this; }
    FXASSERT(wrptr+8<=endptr);
    if(swap){
      wrptr[0]=((const FXuchar*)&v)[7];
      wrptr[1]=((const FXuchar*)&v)[6];
      wrptr[2]=((const FXuchar*)&v)[5];
      wrptr[3]=((const FXuchar*)&v)[4];
      wrptr[4]=((const FXuchar*)&v)[3];
      wrptr[5]=((const FXuchar*)&v)[2];
      wrptr[6]=((const FXuchar*)&v)[1];
      wrptr[7]=((const FXuchar*)&v)[0];
      }
    else{
      wrptr[0]=((const FXuchar*)&v)[0];
      wrptr[1]=((const FXuchar*)&v)[1];
      wrptr[2]=((const FXuchar*)&v)[2];
      wrptr[3]=((const FXuchar*)&v)[3];
      wrptr[4]=((const FXuchar*)&v)[4];
      wrptr[5]=((const FXuchar*)&v)[5];
      wrptr[6]=((const FXuchar*)&v)[6];
      wrptr[7]=((const FXuchar*)&v)[7];
      }
    wrptr+=8;
    pos+=8;
    }
  return *this;
  }


/************************  Save Blocks of Basic Types  *************************/

// Write array of bytes
FXStream& FXStream::save(const FXuchar* p,unsigned long n){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    while(0<n){
      if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<1){ code=FXStreamFull; return *this; }
      FXASSERT(wrptr<endptr);
      do{
        *wrptr++=*p++;
        pos++;
        n--;
        }
      while(0<n && wrptr<endptr);
      }
    }
  return *this;
  }


// Write array of shorts
FXStream& FXStream::save(const FXushort* p,unsigned long n){
  register const FXuchar *q=(const FXuchar*)p;
  if(code==FXStreamOK){
    n<<=1;
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(swap){
      while(0<n){
        if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<2){ code=FXStreamFull; return *this; }
        FXASSERT(wrptr+2<=endptr);
        do{
          wrptr[0]=q[1];
          wrptr[1]=q[0];
          wrptr+=2;
          pos+=2;
          q+=2;
          n-=2;
          }
        while(0<n && wrptr+2<=endptr);
        }
      }
    else{
      while(0<n){
        if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<2){ code=FXStreamFull; return *this; }
        FXASSERT(wrptr+2<=endptr);
        do{
          wrptr[0]=q[0];
          wrptr[1]=q[1];
          wrptr+=2;
          pos+=2;
          q+=2;
          n-=2;
          }
        while(0<n && wrptr+2<=endptr);
        }
      }
    }
  return *this;
  }


// Write array of ints
FXStream& FXStream::save(const FXuint* p,unsigned long n){
  register const FXuchar *q=(const FXuchar*)p;
  if(code==FXStreamOK){
    n<<=2;
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(swap){
      while(0<n){
        if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<4){ code=FXStreamFull; return *this; }
        FXASSERT(wrptr+4<=endptr);
        do{
          wrptr[0]=q[3];
          wrptr[1]=q[2];
          wrptr[2]=q[1];
          wrptr[3]=q[0];
          wrptr+=4;
          pos+=4;
          q+=4;
          n-=4;
          }
        while(0<n && wrptr+4<=endptr);
        }
      }
    else{
      while(0<n){
        if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<4){ code=FXStreamFull; return *this; }
        FXASSERT(wrptr+4<=endptr);
        do{
          wrptr[0]=q[0];
          wrptr[1]=q[1];
          wrptr[2]=q[2];
          wrptr[3]=q[3];
          wrptr+=4;
          pos+=4;
          q+=4;
          n-=4;
          }
        while(0<n && wrptr+4<=endptr);
        }
      }
    }
  return *this;
  }


// Write array of doubles
FXStream& FXStream::save(const FXdouble* p,unsigned long n){
  register const FXuchar *q=(const FXuchar*)p;
  if(code==FXStreamOK){
    n<<=3;
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(swap){
      while(0<n){
        if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<8){ code=FXStreamFull; return *this; }
        FXASSERT(wrptr+8<=endptr);
        do{
          wrptr[0]=q[7];
          wrptr[1]=q[6];
          wrptr[2]=q[5];
          wrptr[3]=q[4];
          wrptr[4]=q[3];
          wrptr[5]=q[2];
          wrptr[6]=q[1];
          wrptr[7]=q[0];
          wrptr+=8;
          pos+=8;
          q+=8;
          n-=8;
          }
        while(0<n && wrptr+8<=endptr);
        }
      }
    else{
      while(0<n){
        if(wrptr+n>endptr && writeBuffer((wrptr-endptr)+n)<8){ code=FXStreamFull; return *this; }
        FXASSERT(wrptr+8<=endptr);
        do{
          wrptr[0]=q[0];
          wrptr[1]=q[1];
          wrptr[2]=q[2];
          wrptr[3]=q[3];
          wrptr[4]=q[4];
          wrptr[5]=q[5];
          wrptr[6]=q[6];
          wrptr[7]=q[7];
          wrptr+=8;
          pos+=8;
          q+=8;
          n-=8;
          }
        while(0<n && wrptr+8<=endptr);
        }
      }
    }
  return *this;
  }


/*****************************  Load Basic Types  ******************************/

// Read one byte
FXStream& FXStream::operator>>(FXuchar& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(rdptr+1>wrptr && readBuffer(1)<1){ code=FXStreamEnd; return *this; }
    FXASSERT(rdptr+1<=wrptr);
    v=*rdptr++;
    pos++;
    }
  return *this;
  }


// Read one short
FXStream& FXStream::operator>>(FXushort& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(rdptr+2>wrptr && readBuffer((rdptr-wrptr)+2)<2){ code=FXStreamEnd; return *this; }
    FXASSERT(rdptr+2<=wrptr);
    if(swap){
      ((FXuchar*)&v)[1]=rdptr[0];
      ((FXuchar*)&v)[0]=rdptr[1];
      }
    else{
      ((FXuchar*)&v)[0]=rdptr[0];
      ((FXuchar*)&v)[1]=rdptr[1];
      }
    rdptr+=2;
    pos+=2;
    }
  return *this;
  }


// Read one int
FXStream& FXStream::operator>>(FXuint& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(rdptr+4>wrptr && readBuffer((rdptr-wrptr)+4)<4){ code=FXStreamEnd; return *this; }
    FXASSERT(rdptr+4<=wrptr);
    if(swap){
      ((FXuchar*)&v)[3]=rdptr[0];
      ((FXuchar*)&v)[2]=rdptr[1];
      ((FXuchar*)&v)[1]=rdptr[2];
      ((FXuchar*)&v)[0]=rdptr[3];
      }
    else{
      ((FXuchar*)&v)[0]=rdptr[0];
      ((FXuchar*)&v)[1]=rdptr[1];
      ((FXuchar*)&v)[2]=rdptr[2];
      ((FXuchar*)&v)[3]=rdptr[3];
      }
    rdptr+=4;
    pos+=4;
    }
  return *this;
  }


// Read one double
FXStream& FXStream::operator>>(FXdouble& v){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(rdptr+8>wrptr && readBuffer((rdptr-wrptr)+8)<8){ code=FXStreamEnd; return *this; }
    FXASSERT(rdptr+8<=wrptr);
    if(swap){
      ((FXuchar*)&v)[7]=rdptr[0];
      ((FXuchar*)&v)[6]=rdptr[1];
      ((FXuchar*)&v)[5]=rdptr[2];
      ((FXuchar*)&v)[4]=rdptr[3];
      ((FXuchar*)&v)[3]=rdptr[4];
      ((FXuchar*)&v)[2]=rdptr[5];
      ((FXuchar*)&v)[1]=rdptr[6];
      ((FXuchar*)&v)[0]=rdptr[7];
      }
    else{
      ((FXuchar*)&v)[0]=rdptr[0];
      ((FXuchar*)&v)[1]=rdptr[1];
      ((FXuchar*)&v)[2]=rdptr[2];
      ((FXuchar*)&v)[3]=rdptr[3];
      ((FXuchar*)&v)[4]=rdptr[4];
      ((FXuchar*)&v)[5]=rdptr[5];
      ((FXuchar*)&v)[6]=rdptr[6];
      ((FXuchar*)&v)[7]=rdptr[7];
      }
    rdptr+=8;
    pos+=8;
    }
  return *this;
  }



/************************  Load Blocks of Basic Types  *************************/

// Read array of bytes
FXStream& FXStream::load(FXuchar* p,unsigned long n){
  if(code==FXStreamOK){
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    while(0<n){
      if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<1){ code=FXStreamEnd; return *this; }
      FXASSERT(rdptr<wrptr);
      do{
        *p++=*rdptr++;
        pos++;
        n--;
        }
      while(0<n && rdptr<wrptr);
      }
    }
  return *this;
  }


/// Read array of shorts
FXStream& FXStream::load(FXushort* p,unsigned long n){
  register FXuchar *q=(FXuchar*)p;
  if(code==FXStreamOK){
    n<<=1;
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(swap){
      while(0<n){
        if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<2){ code=FXStreamEnd; return *this; }
        FXASSERT(rdptr+2<=wrptr);
        do{
          q[1]=rdptr[0];
          q[0]=rdptr[1];
          rdptr+=2;
          pos+=2;
          q+=2;
          n-=2;
          }
        while(0<n && rdptr+2<=wrptr);
        }
      }
    else{
      while(0<n){
        if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<2){ code=FXStreamEnd; return *this; }
        FXASSERT(rdptr+2<=wrptr);
        do{
          q[0]=rdptr[0];
          q[1]=rdptr[1];
          rdptr+=2;
          pos+=2;
          q+=2;
          n-=2;
          }
        while(0<n && rdptr+2<=wrptr);
        }
      }
    }
  return *this;
  }


// Read array of ints
FXStream& FXStream::load(FXuint* p,unsigned long n){
  register FXuchar *q=(FXuchar*)p;
  if(code==FXStreamOK){
    n<<=2;
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(swap){
      while(0<n){
        if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<4){ code=FXStreamEnd; return *this; }
        FXASSERT(rdptr+4<=wrptr);
        do{
          q[3]=rdptr[0];
          q[2]=rdptr[1];
          q[1]=rdptr[2];
          q[0]=rdptr[3];
          rdptr+=4;
          pos+=4;
          q+=4;
          n-=4;
          }
        while(0<n && rdptr+4<=wrptr);
        }
      }
    else{
      while(0<n){
        if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<4){ code=FXStreamEnd; return *this; }
        FXASSERT(rdptr+4<=wrptr);
        do{
          q[0]=rdptr[0];
          q[1]=rdptr[1];
          q[2]=rdptr[2];
          q[3]=rdptr[3];
          rdptr+=4;
          pos+=4;
          q+=4;
          n-=4;
          }
        while(0<n && rdptr+4<=wrptr);
        }
      }
    }
  return *this;
  }


// Read array of doubles
FXStream& FXStream::load(FXdouble* p,unsigned long n){
  register FXuchar *q=(FXuchar*)p;
  if(code==FXStreamOK){
    n<<=3;
    FXASSERT(begptr<=rdptr);
    FXASSERT(rdptr<=wrptr);
    FXASSERT(wrptr<=endptr);
    if(swap){
      while(0<n){
        if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<8){ code=FXStreamEnd; return *this; }
        FXASSERT(rdptr+4<=wrptr);
        do{
          q[7]=rdptr[0];
          q[6]=rdptr[1];
          q[5]=rdptr[2];
          q[4]=rdptr[3];
          q[3]=rdptr[4];
          q[2]=rdptr[5];
          q[1]=rdptr[6];
          q[0]=rdptr[7];
          rdptr+=8;
          pos+=8;
          q+=8;
          n-=8;
          }
        while(0<n && rdptr+8<=wrptr);
        }
      }
    else{
      while(0<n){
        if(rdptr+n>wrptr && readBuffer((rdptr-wrptr)+n)<8){ code=FXStreamEnd; return *this; }
        FXASSERT(rdptr+4<=wrptr);
        do{
          q[0]=rdptr[0];
          q[1]=rdptr[1];
          q[2]=rdptr[2];
          q[3]=rdptr[3];
          q[4]=rdptr[4];
          q[5]=rdptr[5];
          q[6]=rdptr[6];
          q[7]=rdptr[7];
          rdptr+=8;
          pos+=8;
          q+=8;
          n-=8;
          }
        while(0<n && rdptr+8<=wrptr);
        }
      }
    }
  return *this;
  }


/*******************************  Save Objects  ********************************/

// Save object
FXStream& FXStream::saveObject(const FXObject* v){
  register const FXMetaClass *cls;
  register FXuint p,x;
  FXuint tag,esc=0;
  if(dir!=FXStreamSave){ fxerror("FXStream::saveObject: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    if(v==NULL){                                // Its a NULL
      tag=0;
      *this << tag;
      return *this;
      }
    p=HASH1(v,ntable);
    FXASSERT(p<ntable);
    x=HASH2(v,ntable);
    FXASSERT(1<=x && x<ntable);
    while(table[p].ref!=UNUSEDSLOT){            // Check hash table
      if(table[p].obj==v){
        FXASSERT(table[p].ref<=no);
        tag=table[p].ref|0x80000000;
        *this << tag;                           // Was in table, save reference only
        return *this;
        }
      p=(p+x)%ntable;
      }
    table[p].obj=(FXObject*)v;
    table[p].ref=no++;
    FXASSERT(no<ntable);
    if((100*no)>=(MAX_LOAD*ntable)) grow();
    cls=v->getMetaClass();
    tag=cls->getClassNameLength();
    if(tag>MAXCLASSNAME){                       // Class name too long
      code=FXStreamFormat;
      return *this;
      }
    *this << tag;                               // Save tag
    *this << esc;                               // Escape is zero for now
    save(cls->getClassName(),cls->getClassNameLength());
    FXTRACE((100,"saveObject(%s)\n",v->getClassName()));
    v->save(*this);
    }
  return *this;
  }


/*******************************  Load Objects  ********************************/

// Load object
FXStream& FXStream::loadObject(FXObject*& v){
  register const FXMetaClass *cls;
  FXchar obnam[MAXCLASSNAME+1];
  FXuint tag,esc;
  if(dir!=FXStreamLoad){ fxerror("FXStream::loadObject: wrong stream direction.\n"); }
  if(code==FXStreamOK){
    *this >> tag;
    if(tag==0){                                 // Was a NULL
      v=NULL;
      return *this;
      }
    if(tag&0x80000000){
      tag&=0x7fffffff;
      if(tag>=no){                              // Out-of-range reference number
        code=FXStreamFormat;                    // Bad format in stream
        return *this;
        }
      v=table[tag].obj;
      return *this;
      }
    if(tag>MAXCLASSNAME){                       // Class name too long
      code=FXStreamFormat;                      // Bad format in stream
      return *this;
      }
    *this >> esc;                               // Read escape code
    if(esc!=0){                                 // Escape code is wrong
      code=FXStreamFormat;                      // Bad format in stream
      return *this;
      }
    load(obnam,tag);                            // Load name
    cls=FXMetaClass::getMetaClassFromName(obnam);
    if(cls==NULL){                              // No FXMetaClass with this class name
      code=FXStreamUnknown;                     // Unknown class
      return *this;
      }
    v=cls->makeInstance();                      // Build some object!!
    FXASSERT(no<ntable);
    table[no].obj=v;                            // Save pointer in table
    table[no].ref=no++;                         // Save reference number also!
    if(no>=ntable) grow();
    FXTRACE((100,"loadObject(%s)\n",v->getClassName()));
    v->load(*this);
    }
  return *this;
  }

}
