/********************************************************************************
*                                                                               *
*                         Persistent Storage Stream                             *
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
* $Id: FXStream.h,v 1.12 2002/01/18 22:42:55 jeroen Exp $                       *
********************************************************************************/
#ifndef FXSTREAM_H
#define FXSTREAM_H



/// Stream data flow direction
enum FXStreamDirection {
  FXStreamDead=0,               // Unopened stream
  FXStreamSave=1,               // Saving stuff to stream
  FXStreamLoad=2                // Loading stuff from stream
  };


/// Stream status codes
enum FXStreamStatus {
  FXStreamOK=0,                 // OK
  FXStreamEnd=1,                // Try read past end of stream
  FXStreamFull=2,               // Filled up stream buffer or disk full
  FXStreamNoWrite=3,            // Unable to open for write
  FXStreamNoRead=4,             // Unable to open for read
  FXStreamFormat=5,             // Stream format error
  FXStreamUnknown=6,            // Trying to read unknown class
  FXStreamAlloc=7,              // Alloc failed
  FXStreamFailure=8             // General failure
  };


// Hash table entry
struct FXStreamHashEntry {
  FXuint    ref;                // Object reference number
  FXObject* obj;                // Pointer to object
  };


class FXString;


/************************  Persistent Store Definition  *************************/

/// Persistent store definition
class FXAPI FXStream {

  FXStreamHashEntry* table;     // Hash table
  FXuint             ntable;    // Amount of table that is filled
  FXuint             ninit;     // Table size
  FXuint             no;        // Count objects
  FXbool             swap;      // Swap bytes on readin
  const FXObject*    parent;    // Parent object
  void               grow();    // Enlarge the table

protected:
  FXStreamDirection  dir;       // Direction of current transfer
  FXStreamStatus     code;      // Status code
  unsigned long      pos;       // Position

protected:

  /// Save bunch of items
  virtual void saveItems(const void *buf,FXuint n);

  /// Load bunch of items
  virtual void loadItems(void *buf,FXuint n);

public:

  /// Constructor
  FXStream(const FXObject* cont=NULL);

  /// Open archive return TRUE if OK
  FXbool open(FXStreamDirection save_or_load);

  /// Close; return TRUE if OK
  FXbool close();

  /// Get status code
  FXStreamStatus status() const { return code; }

  /// Set status code
  void setError(FXStreamStatus err);

  /// Obtain direction
  FXStreamDirection direction() const { return dir; }

  /// Get parent object
  const FXObject* container() const { return parent; }

  /// Get position
  unsigned long position() const { return pos; }

  /// Move to position
  virtual FXbool position(unsigned long p);

  /// Change swap bytes flag
  void swapBytes(FXbool s){ swap=s; }

  /// Get swap bytes flag
  FXbool swapBytes() const { return swap; }

  /// Return implementation's endianness
  static FXbool isLittleEndian(){ return !FOX_BIGENDIAN; }

  /// Save to stream
  virtual FXStream& operator<<(const FXuchar& v);
  virtual FXStream& operator<<(const FXchar& v);
  FXStream& operator<<(const FXushort& v);
  FXStream& operator<<(const FXshort& v);
  FXStream& operator<<(const FXuint& v);
  FXStream& operator<<(const FXint& v);
  FXStream& operator<<(const FXfloat& v);
  FXStream& operator<<(const FXdouble& v);

#ifdef FX_LONG
  FXStream& operator<<(const FXlong& v);
  FXStream& operator<<(const FXulong& v);
#endif

  FXStream& save(const FXuchar* p,FXuint n);
  FXStream& save(const FXchar* p,FXuint n);
  FXStream& save(const FXushort* p,FXuint n);
  FXStream& save(const FXshort* p,FXuint n);
  FXStream& save(const FXuint* p,FXuint n);
  FXStream& save(const FXint* p,FXuint n);
  FXStream& save(const FXfloat* p,FXuint n);
  FXStream& save(const FXdouble* p,FXuint n);

#ifdef FX_LONG
  FXStream& save(const FXlong* p,FXuint n);
  FXStream& save(const FXulong* p,FXuint n);
#endif

  /// Save object
  FXStream& saveObject(const FXObject* v);

  /// Load from stream
  virtual FXStream& operator>>(FXuchar& v);
  virtual FXStream& operator>>(FXchar& v);
  FXStream& operator>>(FXushort& v);
  FXStream& operator>>(FXshort& v);
  FXStream& operator>>(FXuint& v);
  FXStream& operator>>(FXint& v);
  FXStream& operator>>(FXfloat& v);
  FXStream& operator>>(FXdouble& v);

#ifdef FX_LONG
  FXStream& operator>>(FXlong& v);
  FXStream& operator>>(FXulong& v);
#endif

  FXStream& load(FXuchar* p,FXuint n);
  FXStream& load(FXchar* p,FXuint n);
  FXStream& load(FXushort* p,FXuint n);
  FXStream& load(FXshort* p,FXuint n);
  FXStream& load(FXuint* p,FXuint n);
  FXStream& load(FXint* p,FXuint n);
  FXStream& load(FXfloat* p,FXuint n);
  FXStream& load(FXdouble* p,FXuint n);

#ifdef FX_LONG
  FXStream& load(FXlong* p,FXuint n);
  FXStream& load(FXulong* p,FXuint n);
#endif

  /// Load object
  FXStream& loadObject(FXObject*& v);

  /// Destructor
  virtual ~FXStream();
  };



/***************************  File Store Definition  ***************************/

/// File Store Definition
class FXAPI FXFileStream : public FXStream {
  void* file;               // File being dealt with
protected:
  virtual void saveItems(const void *buf,FXuint n);
  virtual void loadItems(void *buf,FXuint n);

public:

  /// Create file store
  FXFileStream(const FXObject* cont=NULL);

  /// Open file store
  FXbool open(const FXString& filename,FXStreamDirection save_or_load);

  /// Close file store
  FXbool close();

  /// Move to position
  virtual FXbool position(unsigned long p);

  /// Save to stream
  virtual FXStream& operator<<(const FXuchar& v);
  virtual FXStream& operator<<(const FXchar& v);

  /// Load from stream
  virtual FXStream& operator>>(FXuchar& v);
  virtual FXStream& operator>>(FXchar& v);

  /// Destructor
  virtual ~FXFileStream();
  };


/**************************  Memory Store Definition  **************************/

/// Memory Store Definition
class FXAPI FXMemoryStream : public FXStream {
  FXuchar *ptr;             // Memory pointer
  FXuint   space;           // Space in buffer
  FXbool   owns;            // Owns the data array
protected:
  virtual void saveItems(const void *buf,FXuint n);
  virtual void loadItems(void *buf,FXuint n);

public:

  /// Create memory store
  FXMemoryStream(const FXObject* cont=NULL);

  /// Open file store
  FXbool open(FXuchar* data,FXStreamDirection save_or_load);

  /// Open memory store
  FXbool open(FXuchar* data,FXuint sp,FXStreamDirection save_or_load);

  /// Get available space
  FXuint getSpace() const { return space; }

  /// Set available space
  void setSpace(FXuint sp);

  /// Take buffer away from stream
  void takeBuffer(FXuchar*& buffer,FXuint& sp);

  /// Give buffer to stream
  void giveBuffer(FXuchar *buffer,FXuint sp);

  /// Close memory store
  FXbool close();

  /// Move to position
  virtual FXbool position(unsigned long p);

  /// Save to stream
  virtual FXStream& operator<<(const FXuchar& v);
  virtual FXStream& operator<<(const FXchar& v);

  /// Load from stream
  virtual FXStream& operator>>(FXuchar& v);
  virtual FXStream& operator>>(FXchar& v);

  /// Destructor
  virtual ~FXMemoryStream();
  };



#endif
