/********************************************************************************
*                                                                               *
*                        G Z S t r e a m   C l a s s e s                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002,2004 by Sander Jansen.   All Rights Reserved.              *
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
* $Id: FXGZStream.cpp,v 1.5 2004/02/08 17:29:06 fox Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXGZStream.h"

#ifdef HAVE_ZLIB_H
#include "zlib.h"

/*
  Notes:
  - Very basic compressed file I/O only.
  - Updated for new stream classes 2003/07/08.
*/

/*******************************************************************************/

namespace FX {


// Initialize file stream
FXGZFileStream::FXGZFileStream(const FXObject* cont):FXStream(cont){
  file=NULL;
  }


// Save to a file
unsigned long FXGZFileStream::writeBuffer(unsigned long){
  register long m,n;
  if(dir!=FXStreamSave){fxerror("FXGZFileStream::writeBuffer: wrong stream direction.\n");}
  FXASSERT(begptr<=rdptr);
  FXASSERT(rdptr<=wrptr);
  FXASSERT(wrptr<=endptr);
  if(code==FXStreamOK){
    m=wrptr-rdptr;
    n=gzwrite((gzFile*)file,rdptr,m);
    if(n<=0){
      code=FXStreamFull;
      return endptr-wrptr;
      }
    m-=n;
    if(m){memmove(begptr,rdptr+n,m);}
    rdptr=begptr;
    wrptr=begptr+m;
    return endptr-wrptr;
    }
  return 0;
  }


// Load from file
unsigned long FXGZFileStream::readBuffer(unsigned long){
  register long m,n;
  if(dir!=FXStreamLoad){fxerror("FXGZFileStream::readBuffer: wrong stream direction.\n");}
  FXASSERT(begptr<=rdptr);
  FXASSERT(rdptr<=wrptr);
  FXASSERT(wrptr<=endptr);
  if(code==FXStreamOK){
    m=wrptr-rdptr;
    if(m){memmove(begptr,rdptr,m);}
    rdptr=begptr;
    wrptr=begptr+m;
    n=gzread((gzFile*)file,wrptr,endptr-wrptr);
    if(n<=0){
      code=FXStreamEnd;
      return wrptr-rdptr;
      }
    wrptr+=n;
    return wrptr-rdptr;
    }
  return 0;
  }


// Try open file stream
FXbool FXGZFileStream::open(const FXString& filename,FXStreamDirection save_or_load,unsigned long size){
  if(save_or_load!=FXStreamSave && save_or_load!=FXStreamLoad){fxerror("FXGZFileStream::open: illegal stream direction.\n");}
  if(!dir){
    if(save_or_load==FXStreamLoad){
      file=gzopen(filename.text(),"rb");
      if(file==NULL){ code=FXStreamNoRead; return FALSE; }
      }
    else if(save_or_load==FXStreamSave){
      file=gzopen(filename.text(),"wb");
      if(file==NULL){ code=FXStreamNoWrite; return FALSE; }
      }
    return FXStream::open(save_or_load,size);
    }
  return FALSE;
  }


// Close file stream
FXbool FXGZFileStream::close(){
  if(dir){
    if(dir==FXStreamSave) flush();
    gzclose((gzFile*)file);
    return FXStream::close();
    }
  return FALSE;
  }


// Destructor
FXGZFileStream::~FXGZFileStream(){
  close();
  }

}

#endif
