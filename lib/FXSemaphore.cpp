/********************************************************************************
*                                                                               *
*                          S e m a p h o r e   C l a s s                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 2004,2013 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXSemaphore.h"

/*
  Notes:

  - Semaphore variable.
*/

using namespace FX;


namespace FX {


/*******************************************************************************/

// Initialize semaphore with given count
FXSemaphore::FXSemaphore(FXint count){
#if defined(WIN32)
  data[0]=(FXuval)CreateSemaphore(NULL,count,0x7fffffff,NULL);
#elif defined(__minix)
  //// NOT SUPPORTED ////
#else
  // If this fails on your machine, determine what value
  // of sizeof(sem_t) is supposed to be on your
  // machine and mail it to: jeroen@fox-toolkit.com!!
  //FXTRACE((150,"sizeof(sem_t)=%d\n",sizeof(sem_t)));
  FXASSERT(sizeof(data)>=sizeof(sem_t));
  sem_init((sem_t*)data,0,(unsigned int)count);
#endif
  }


// Decrement semaphore, waiting if count is zero
void FXSemaphore::wait(){
#if defined(WIN32)
  WaitForSingleObject((HANDLE)data[0],INFINITE);
#elif defined(__minix)
  //// NOT SUPPORTED ////
#else
  sem_wait((sem_t*)data);
#endif
  }


// Try decrement semaphore; return false if timed out
FXbool FXSemaphore::wait(FXTime nsec){
#if defined(WIN32)
  if(0<nsec){
    if(nsec<forever){
      DWORD delay=(DWORD)(nsec/1000000);
      return WaitForSingleObject((HANDLE)data[0],delay)==WAIT_OBJECT_0;
      }
    return WaitForSingleObject((HANDLE)data[0],INFINITE)==WAIT_OBJECT_0;
    }
  return WaitForSingleObject((HANDLE)data[0],0)==WAIT_OBJECT_0;
#elif defined(__minix)
  //// NOT SUPPORTED ////
  return false;
#else
  if(0<nsec){
    if(nsec<forever){
#if (_POSIX_C_SOURCE >= 199309L)
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME,&ts);
      ts.tv_sec=ts.tv_sec+(ts.tv_nsec+nsec)/1000000000;
      ts.tv_nsec=(ts.tv_nsec+nsec)%1000000000;
      return sem_timedwait((sem_t*)data,&ts)==0;
#else
      struct timespec ts;
      struct timeval tv;
      gettimeofday(&tv,NULL);
      tv.tv_usec*=1000;
      ts.tv_sec=tv.tv_sec+(tv.tv_usec+nsec)/1000000000;
      ts.tv_nsec=(tv.tv_usec+nsec)%1000000000;
      return sem_timedwait((sem_t*)data,&ts)==0;
#endif
      }
    return sem_wait((sem_t*)data)==0;
    }
  return sem_trywait((sem_t*)data)==0;
#endif
  }


// Try decrement semaphore; and return false if count is zero
FXbool FXSemaphore::trywait(){
#if defined(WIN32)
  return WaitForSingleObject((HANDLE)data[0],0)==WAIT_OBJECT_0;
#elif defined(__minix)
  //// NOT SUPPORTED ////
  return false;
#else
  return sem_trywait((sem_t*)data)==0;
#endif
  }


// Increment semaphore
void FXSemaphore::post(){
#if defined(WIN32)
  ReleaseSemaphore((HANDLE)data[0],1,NULL);
#elif defined(__minix)
  //// NOT SUPPORTED ////
#else
  sem_post((sem_t*)data);
#endif
  }


// Delete semaphore
FXSemaphore::~FXSemaphore(){
#if defined(WIN32)
  CloseHandle((HANDLE)data[0]);
#elif defined(__minix)
  //// NOT SUPPORTED ////
#else
  sem_destroy((sem_t*)data);
#endif
  }

}