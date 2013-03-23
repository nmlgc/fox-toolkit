/********************************************************************************
*                                                                               *
*                         Q u e u e   O f   P o i n t e r s                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 2006,2013 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#ifndef FXPTRQUEUE_H
#define FXPTRQUEUE_H


namespace FX {


/// Queue of void pointers
class FXAPI FXPtrQueue {
private:
  FXPtrList       list;         // List of pointers
  volatile FXuint head;         // Write side
  volatile FXuint tail;         // Read side
private:
  FXPtrQueue(const FXPtrQueue&);
  FXPtrQueue &operator=(const FXPtrQueue&);
public:

  /// Create initially empty queue
  FXPtrQueue();

  /// Create queue with initial size
  FXPtrQueue(FXuint sz);

  /// Change size of queue; return true if success
  FXbool setSize(FXuint sz);

  /// Return size
  FXuint getSize() const { return list.no(); }

  /// Return number of used slots
  FXuint getUsed() const;

  /// Return number of free slots
  FXuint getFree() const;

  /// Check if queue is full
  FXbool isFull() const;

  /// Check if queue is empty
  FXbool isEmpty() const;

  /// Peek for item
  FXbool peek(FXptr& ptr);

  /// Add item to queue, return true if success
  FXbool push(FXptr ptr);

  /// Remove item from queue, return true if success
  FXbool pop(FXptr& ptr);

  /// Drop item from queue, return true if success
  FXbool pop();

  /// Destroy queue
 ~FXPtrQueue();
  };


/// Queue of pointers to TYPE
template <class TYPE>
class FXPtrQueueOf : public FXPtrQueue {
public:
  FXPtrQueueOf(){}
  FXPtrQueueOf(FXuint sz):FXPtrQueue(sz){}
  FXbool peek(TYPE*& ptr){ return FXPtrQueue::peek((FXptr&)ptr); }
  FXbool push(TYPE* ptr){ return FXPtrQueue::push((FXptr)ptr); }
  FXbool pop(TYPE*& ptr){ return FXPtrQueue::pop((FXptr&)ptr); }
  };

}

#endif