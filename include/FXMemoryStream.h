/********************************************************************************
*                                                                               *
*                   M e m o r y   S t r e a m   C l a s s e s                   *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXMemoryStream.h,v 1.8 2005/01/16 16:06:06 fox Exp $                     *
********************************************************************************/
#ifndef FXMEMORYSTREAM_H
#define FXMEMORYSTREAM_H

#ifndef FXSTREAM_H
#include "FXStream.h"
#endif

namespace FX {


/// Memory Store Definition
class FXAPI FXMemoryStream : public FXStream {
protected:
  virtual unsigned long writeBuffer(unsigned long count);
  virtual unsigned long readBuffer(unsigned long count);
public:

  /// Create memory store
  FXMemoryStream(const FXObject* cont=NULL);

  /// Open file store
  FXbool open(FXStreamDirection save_or_load,FXuchar* data);

  /// Open memory store
  FXbool open(FXStreamDirection save_or_load,unsigned long size,FXuchar* data);

  /// Take buffer away from stream
  void takeBuffer(FXuchar*& data,unsigned long& size);

  /// Give buffer to stream
  void giveBuffer(FXuchar *data,unsigned long size);

  /// Close memory store
  virtual FXbool close();

  /// Get position
  FXlong position() const { return FXStream::position(); }

  /// Move to position
  virtual FXbool position(FXlong offset,FXWhence whence=FXFromStart);

  /// Save single items to stream
  FXMemoryStream& operator<<(const FXuchar& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXchar& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXushort& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXshort& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXuint& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXint& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXfloat& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXdouble& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXlong& v){ FXStream::operator<<(v); return *this; }
  FXMemoryStream& operator<<(const FXulong& v){ FXStream::operator<<(v); return *this; }

  /// Save arrays of items to stream
  FXMemoryStream& save(const FXuchar* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXchar* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXushort* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXshort* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXuint* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXint* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXfloat* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXdouble* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXlong* p,unsigned long n){ FXStream::save(p,n); return *this; }
  FXMemoryStream& save(const FXulong* p,unsigned long n){ FXStream::save(p,n); return *this; }

  /// Load single items from stream
  FXMemoryStream& operator>>(FXuchar& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXchar& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXushort& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXshort& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXuint& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXint& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXfloat& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXdouble& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXlong& v){ FXStream::operator>>(v); return *this; }
  FXMemoryStream& operator>>(FXulong& v){ FXStream::operator>>(v); return *this; }

  /// Load arrays of items from stream
  FXMemoryStream& load(FXuchar* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXchar* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXushort* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXshort* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXuint* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXint* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXfloat* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXdouble* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXlong* p,unsigned long n){ FXStream::load(p,n); return *this; }
  FXMemoryStream& load(FXulong* p,unsigned long n){ FXStream::load(p,n); return *this; }

  /// Save object
  FXMemoryStream& saveObject(const FXObject* v){ FXStream::saveObject(v); return *this; }

  /// Load object
  FXMemoryStream& loadObject(FXObject*& v){ FXStream::loadObject(v); return *this; }
  };

}

#endif
