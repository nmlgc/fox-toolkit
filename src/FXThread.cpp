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
* $Id: FXThread.cpp,v 1.15 2004/03/02 00:08:06 fox Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXThread.h"

#ifndef WIN32
#include <pthread.h>
#else
#include <process.h>
#endif


/*
  Notes:
  - We have a amorphous blob of memory reserved for the mutex implementation.
    Since we're trying to avoid having to include platform-specific headers
    in application code, we can't easily know how much to allocate for
    pthread_mutex_t [or CRITICAL_SECTION].

  - We don't want to allocate dynamically because of the performance
    issues, so we just reserve some memory which we will hope to be
    enough.  If it turns out its not enough (on some specific platform),
    then the assert should trigger and we'll just have to change the
    source a bit.

  - If you run into this, try to figure out sizeof(pthread_mutex_t) and
    let me know about it (jeroen@fox-toolkit.org).

  - I do recommend running this in debug mode first time around on a
    new platform.

  - Picked unsigned long so as to ensure alignment issues are taken
    care off.
*/

using namespace FX;


namespace FX {

/*******************************************************************************/

// Unix implementation

#ifndef WIN32


// Initialize mutex
FXMutex::FXMutex(){
  // If this fails on your machine, determine what value
  // of sizeof(pthread_mutex_t) is supposed to be on your
  // machine and mail it to: jeroen@fox-toolkit.org!!
  FXASSERT(sizeof(data)>=sizeof(pthread_mutex_t));
  pthread_mutex_init((pthread_mutex_t*)data,NULL);
  }


// Lock the mutex
FXbool FXMutex::lock(){
  return pthread_mutex_lock((pthread_mutex_t*)data)==0;
  }


// Try lock the mutex
FXbool FXMutex::trylock(){
  return pthread_mutex_trylock((pthread_mutex_t*)data)==0;
  }


// Unlock mutex
FXbool FXMutex::unlock(){
  return pthread_mutex_unlock((pthread_mutex_t*)data)==0;
  }


// Delete mutex
FXMutex::~FXMutex(){
  pthread_mutex_destroy((pthread_mutex_t*)data);
  }


/*******************************************************************************/

// Initialize thread
FXThread::FXThread():tid(0){
  }


// Return thread id of this thread object.
// Purposefully NOT inlined, the tid may be changed by another
// thread and therefore we must force the compiler to fetch
// this value fresh each time it is needed!
FXThreadID FXThread::id() const {
  return tid;
  }


// Return TRUE if this thread is the current one
FXbool FXThread::iscurrent() const {
  return tid==(FXThreadID)pthread_self();
  }


// Return TRUE if this thread is running
FXbool FXThread::isrunning() const {
  return tid!=0;
  }


// Kick off the work; when the thread ends normally,
// the thread id in the FXThread is automatically reset.
// The try catch block ensures that an exception in the
// thread does not bring down the whole program but only
// the thread.
void* FXThread::execute(void* thread){
  register FXint code=-1;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  try{ code=((FXThread*)thread)->run(); } catch(...){ }
  return (void*)(FXival)code;
  }


// Start thread; make sure that stacksize >= PTHREAD_STACK_MIN.
// We can't check for it because not all machines have this the
// PTHREAD_STACK_MIN definition.
FXbool FXThread::start(unsigned long stacksize){
  register FXbool code;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr,PTHREAD_INHERIT_SCHED);
  if(stacksize){ pthread_attr_setstacksize(&attr,stacksize); }
//  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  code=pthread_create((pthread_t*)&tid,&attr,FXThread::execute,(void*)this)==0;
  pthread_attr_destroy(&attr);
  return code;
  }


// Suspend calling thread until thread is done
FXbool FXThread::join(FXint& code){
  void *trc=NULL;
  if(tid && pthread_join((pthread_t)tid,&trc)==0){
    code=(FXint)(FXival)trc;
    tid=0;
    return TRUE;
    }
  return FALSE;
  }


// Suspend calling thread until thread is done
FXbool FXThread::join(){
  if(tid && pthread_join((pthread_t)tid,NULL)==0){
    tid=0;
    return TRUE;
    }
  return FALSE;
  }


// Cancel the thread
FXbool FXThread::cancel(){
  if(tid && pthread_cancel((pthread_t)tid)==0){
    pthread_join((pthread_t)tid,NULL);
    tid=0;
    return TRUE;
    }
  return FALSE;
  }


// Detach thread
FXbool FXThread::detach(){
  return tid && pthread_detach((pthread_t)tid)==0;
  }


// Exit calling thread
void FXThread::exit(FXint code){
  pthread_exit((void*)(FXival)code);
  }


// Return thread id of caller
FXThreadID FXThread::current(){
  return (FXThreadID)pthread_self();
  }


// Destroy; if it was running, stop it
FXThread::~FXThread(){
  if(tid){ pthread_cancel((pthread_t)tid); }
  }


/*******************************************************************************/

// Windows implementation

#else

// Initialize mutex
FXMutex::FXMutex(){
  // If this fails on your machine, determine what value
  // of sizeof(CRITICAL_SECTION) is supposed to be on your
  // machine and mail it to: jeroen@fox-toolkit.org!!
  FXASSERT(sizeof(data)>=sizeof(CRITICAL_SECTION));
  InitializeCriticalSection((CRITICAL_SECTION*)data);
  }


// Lock the mutex
FXbool FXMutex::lock(){
  EnterCriticalSection((CRITICAL_SECTION*)data);
  return TRUE;
  }



// Try lock the mutex
FXbool FXMutex::trylock(){
#if(_WIN32_WINNT >= 0x0400)
  return TryEnterCriticalSection((CRITICAL_SECTION*)data);
#else
  return FALSE;
#endif
  }


// Unlock mutex
FXbool FXMutex::unlock(){
  LeaveCriticalSection((CRITICAL_SECTION*)data);
  return TRUE;
  }


// Delete mutex
FXMutex::~FXMutex(){
  DeleteCriticalSection((CRITICAL_SECTION*)data);
  }


/*******************************************************************************/

// Initialize thread
FXThread::FXThread():tid(0),thd(0){
  }


// Return thread id of this thread object.
// Purposefully NOT inlined, the tid may be changed by another
// thread and therefore we must force the compiler to fetch
// this value fresh each time it is needed!
FXThreadID FXThread::id() const {
  return tid;
  }


// Return TRUE if this thread is the current one
FXbool FXThread::iscurrent() const {
  return thd==GetCurrentThreadId();
  }


// Return TRUE if this thread is running
FXbool FXThread::isrunning() const {
  return tid!=0;
  }


// Kick off the work; when the thread ends normally,
// the thread id in the FXThread is automatically reset.
// The try catch block ensures that an exception in the
// thread does not bring down the whole program but only
// the tread.
unsigned int CALLBACK FXThread::execute(void* thread){
  register FXint code=-1;
  try{ code=((FXThread*)thread)->run(); } catch(...){ }
  return code;
  }


// Start thread
FXbool FXThread::start(unsigned long stacksize){
  tid=(FXThreadID)CreateThread(NULL,stacksize,(LPTHREAD_START_ROUTINE)FXThread::execute,this,0,(DWORD*)&thd);
  return tid!=NULL;
  }


// Suspend calling thread until thread is done
FXbool FXThread::join(FXint& code){
  if(tid && WaitForSingleObject((HANDLE)tid,INFINITE)==WAIT_OBJECT_0){
    GetExitCodeThread((HANDLE)tid,(DWORD*)&code);
    CloseHandle((HANDLE)tid);
    tid=0;
    thd=0;
    return TRUE;
    }
  return FALSE;
  }


// Suspend calling thread until thread is done
FXbool FXThread::join(){
  if(tid && WaitForSingleObject((HANDLE)tid,INFINITE)==WAIT_OBJECT_0){
    CloseHandle((HANDLE)tid);
    tid=0;
    thd=0;
    return TRUE;
    }
  return FALSE;
  }


// Cancel the thread
FXbool FXThread::cancel(){
  if(tid && TerminateThread((HANDLE)tid,0)){
    CloseHandle((HANDLE)tid);
    tid=0;
    thd=0;
    return TRUE;
    }
  return FALSE;
  }


// Detach thread
FXbool FXThread::detach(){
  return tid!=0;
  }


// Exit calling thread
void FXThread::exit(FXint code){
  ExitThread(code);
  }


// Return thread id of caller
FXThreadID FXThread::current(){
  return (FXThreadID)GetCurrentThread();
  }


// Destroy
FXThread::~FXThread(){
  if(tid){ TerminateThread((HANDLE)tid,0); CloseHandle((HANDLE)tid); }
  }


#endif


}
