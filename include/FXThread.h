/********************************************************************************
*                                                                               *
*                 M u l i t h r e a d i n g   S u p p o r t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 2004 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXThread.h,v 1.16 2004/02/24 22:21:42 fox Exp $                          *
********************************************************************************/
#ifndef FXTHREAD_H
#define FXTHREAD_H

namespace FX {


// Thread ID type
#ifndef WIN32
typedef unsigned long FXThreadID;
#else
typedef void*         FXThreadID;
#endif


/**
* FXMutex provides a mutex which can be used to enforce critical
* sections around updates of data shared by multiple threads.
*/
class FXAPI FXMutex {
private:
  unsigned long data[8];
private:
  FXMutex(const FXMutex&);
  FXMutex &operator=(const FXMutex&);
public:

  /// Initialize the mutex
  FXMutex();

  /// Lock the mutex
  FXbool lock();

  /// Try to lock the mutex; return TRUE locked
  FXbool trylock();

  /// Unlock mutex
  FXbool unlock();

  /// Delete the mutex
  ~FXMutex();
  };


/**
* An easy way to establish a correspondence between a C++ scope
* and a critical section is to simply declare an FXMutexLock
* at the beginning of the scope.
* The mutex will be automatically released when the scope is
* left (either by natural means or by means of an exception.
*/
class FXAPI FXMutexLock {
private:
  FXMutex& mutex;
private:
  FXMutexLock(const FXMutexLock&);
  FXMutexLock& operator=(const FXMutexLock&);
public:
  FXMutexLock(FXMutex& m):mutex(m){ lock(); }
  FXbool lock(){ return mutex.lock(); }
  FXbool unlock(){ return mutex.unlock(); }
  ~FXMutexLock(){ unlock(); }
  };


/**
* FXThread provides system-independent support for threads.
* Subclasses must implement the run() function do implement
* the desired functionality of the thread.
* The storage of the FXThread object is to be managed by the
* calling thread, not by the thread itself.
*/
class FXAPI FXThread {
private:
  FXThreadID tid;
#ifdef WIN32
  FXuint     thd;
#endif
private:
  FXThread(const FXThread&);
  FXThread &operator=(const FXThread&);
#ifdef WIN32
  static unsigned int CALLBACK execute(void*);
#else
  static void* execute(void*);
#endif
protected:

  /// All threads execute by deriving the run method of FXThread
  virtual FXint run() = 0;

public:

  /// Initialize thread object.
  FXThread();

  /**
  * Return handle of this thread object.
  * This handle is valid in the context of the thread which
  * called start().
  */
  FXThreadID id() const;

  /// Return TRUE if this thread is the calling thread.
  FXbool iscurrent() const;

  /// Return TRUE if this thread is running.
  FXbool isrunning() const;

  /**
  * Start thread; the thread is started as attached.
  * The thread is given stacksize for its stack; a value of
  * zero for stacksize will give it the default stack size.
  */
  FXbool start(unsigned long stacksize=0);

  /**
  * Suspend calling thread until thread is done.
  */
  FXbool join();

  /**
  * Suspend calling thread until thread is done, and set code to the
  * return value of run() or the argument passed into exit().
  * If an exception happened in the thread, return -1.
  */
  FXbool join(FXint& code);

  /**
  * Cancel the thread, stopping it immediately, running or not.
  * If the calling thread is this thread, nothing happens.
  * It is probably better to wait until it is finished, in case the
  * thread currently holds mutexes.
  */
  FXbool cancel();

  /**
  * Detach thread, so that a no join() is necessary to harvest the
  * resources of this thread.
  */
  FXbool detach();

  /**
  * Exit the calling thread.
  * No destructors are invoked for objects on thread's stack;
  * to invoke destructors, throw an exception instead.
  */
  static void exit(FXint code=0);

  /**
  * Return thread handle of calling thread.
  * The handle is valid in the context of the current thread.
  */
  static FXThreadID current();

  /**
  * Destroy the thread immediately, running or not.
  * It is probably better to wait until it is finished, in case
  * the thread currently holds mutexes.
  */
  virtual ~FXThread();
  };

}

#endif

