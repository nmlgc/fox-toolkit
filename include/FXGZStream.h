/********************************************************************************
*                                                                               *
*                        G Z S t r e a m  C l a s s e s                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002,2005 by Sander Jansen.   All Rights Reserved.              *
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
* $Id: FXGZStream.h,v 1.9 2005/01/16 16:06:06 fox Exp $                         *
********************************************************************************/
#ifdef HAVE_ZLIB_H
#ifndef FXGZSTREAM_H
#define FXGZSTREAM_H

#ifndef FXSTREAM_H
#include "FXStream.h"
#endif


namespace FX {


/// GZIP compressed stream
class FXAPI FXGZFileStream : public FXStream {
private:
  void *file;
protected:
  virtual unsigned long writeBuffer(unsigned long count);
  virtual unsigned long readBuffer(unsigned long count);
public:

  /// Create GZIP compressed file stream
  FXGZFileStream(const FXObject* cont=NULL);

  /// Open file stream
  FXbool open(const FXString& filename,FXStreamDirection save_or_load,unsigned long size=8192);

  /// Close file stream
  virtual FXbool close();

  /// Get position
  FXlong position() const { return FXStream::position(); }

  /// Move to position
  virtual FXbool position(FXlong,FXWhence){ return FALSE; }

  /// Save single items to stream
  FXGZFileStream& operator<<(const FXuchar& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXchar& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXushort& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXshort& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXuint& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXint& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXfloat& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXdouble& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXlong& v){ FXStream::operator<<(v); return *this; }
  FXGZFileStream& operator<<(const FXulong& v){ FXStream::operator<<(v); return *this; }

  /// Save arrays of items to stream
  FXGZFileStream& save(const FXuchar* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXchar* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXushort* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXshort* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXuint* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXint* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXfloat* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXdouble* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXlong* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXGZFileStream& save(const FXulong* p,unsigned long n){ FXStream::save(p,n); return *this; }

  /// Load single items from stream
  FXGZFileStream& operator>>(FXuchar& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXchar& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXushort& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXshort& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXuint& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXint& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXfloat& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXdouble& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXlong& v){ FXStream::operator>>(v); return *this; }
  FXGZFileStream& operator>>(FXulong& v){ FXStream::operator>>(v); return *this; }

  /// Load arrays of items from stream
  FXGZFileStream& load(FXuchar* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXchar* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXushort* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXshort* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXuint* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXint* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXfloat* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXdouble* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXlong* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXGZFileStream& load(FXulong* p,unsigned long n){ FXStream::load(p,n); return *this; }

  /// Save object
  FXGZFileStream& saveObject(const FXObject* v){ FXStream::saveObject(v); return *this; }

  /// Load object
  FXGZFileStream& loadObject(FXObject*& v){ FXStream::loadObject(v); return *this; }

  /// Clean up
  virtual ~FXGZFileStream();
  };

}


#endif
#endif
