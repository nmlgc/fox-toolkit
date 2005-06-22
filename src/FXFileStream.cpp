/********************************************************************************
*                                                                               *
*                       F i l e   S t r e a m   C l a s s                       *
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
* $Id: FXFileStream.cpp,v 1.13.2.1 2005/03/18 05:37:07 fox Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXStream.h"
#include "FXFileStream.h"


/*
  Notes:
  - Future verions will use native system calls under WIN32.
*/

using namespace FX;


/*******************************************************************************/

namespace FX {


// Initialize file stream
FXFileStream::FXFileStream(const FXObject* cont):FXStream(cont){
  file=-1;
  }


// Write at least count bytes from the buffer
unsigned long FXFileStream::writeBuffer(unsigned long){
  register long m,n;
  if(dir!=FXStreamSave){fxerror("FXFileStream::writeBuffer: wrong stream direction.\n");}
  FXASSERT(begptr<=rdptr);
  FXASSERT(rdptr<=wrptr);
  FXASSERT(wrptr<=endptr);
  if(code==FXStreamOK){
    m=wrptr-rdptr;
    n=::write(file,rdptr,m);
    if(0<n){
      m-=n;
      if(m){memmove(begptr,rdptr+n,m);}
      rdptr=begptr;
      wrptr=begptr+m;
      }
    return endptr-wrptr;
    }
  return 0;
  }


// Read at least count bytes into the buffer
unsigned long FXFileStream::readBuffer(unsigned long){
  register long m,n;
  if(dir!=FXStreamLoad){fxerror("FXFileStream::readBuffer: wrong stream direction.\n");}
  FXASSERT(begptr<=rdptr);
  FXASSERT(rdptr<=wrptr);
  FXASSERT(wrptr<=endptr);
  if(code==FXStreamOK){
    m=wrptr-rdptr;
    if(m){memmove(begptr,rdptr,m);}
    rdptr=begptr;
    wrptr=begptr+m;
    n=::read(file,wrptr,endptr-wrptr);
    if(0<n){
      wrptr+=n;
      }
    return wrptr-rdptr;
    }
  return 0;
  }


// Open file stream
FXbool FXFileStream::open(const FXString& filename,FXStreamDirection save_or_load,unsigned long size){
  if(save_or_load!=FXStreamSave && save_or_load!=FXStreamLoad){fxerror("FXFileStream::open: illegal stream direction.\n");}
  if(!dir){
    if(save_or_load==FXStreamLoad){
#ifndef WIN32
      file=::open(filename.text(),O_RDONLY);
#else
      file=::open(filename.text(),O_RDONLY|O_BINARY);
#endif
      if(file<0){ code=FXStreamNoRead; return FALSE; }
      }
    else if(save_or_load==FXStreamSave){
#ifndef WIN32
      file=::open(filename.text(),O_RDWR|O_CREAT|O_TRUNC,0666);
#else
      file=::open(filename.text(),O_RDWR|O_CREAT|O_TRUNC|O_BINARY,0666);
#endif
      if(file<0){ code=FXStreamNoWrite; return FALSE; }
      }
    return FXStream::open(save_or_load,size);
    }
  return FALSE;
  }


// Close file stream
FXbool FXFileStream::close(){
  if(dir){
    if(dir==FXStreamSave) flush();
    ::close(file);
    return FXStream::close();
    }
  return FALSE;
  }


// Move to position
FXbool FXFileStream::position(long offset,FXWhence whence){
  register long p;
  if(dir==FXStreamDead){ fxerror("FXMemoryStream::position: stream is not open.\n"); }
  if(code==FXStreamOK){
    FXASSERT(FXFromStart==SEEK_SET);
    FXASSERT(FXFromCurrent==SEEK_CUR);
    FXASSERT(FXFromEnd==SEEK_END);
    if(dir==FXStreamSave){

      // Flush unwritten data
      writeBuffer(0);

      // System's view of file pointer lags behind ours
      if(whence==FXFromCurrent) offset+=wrptr-rdptr;
      if((p=lseek(file,offset,whence))<0){ code=FXStreamFull; return FALSE; }
      wrptr=begptr;
      rdptr=begptr;
      }
    else{

      // System's view of file pointer ahead of ours
      if(whence==FXFromCurrent) offset-=wrptr-rdptr;
      if((p=lseek(file,offset,whence))<0){ code=FXStreamEnd; return FALSE; }
      wrptr=begptr;
      rdptr=begptr;
      }
    pos=p;
    return TRUE;
    }
  return FALSE;
  }


// Close file stream
FXFileStream::~FXFileStream(){
  close();
  }


}
